/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_timer.h"

// System includes
#include <stdio.h>
#include <string.h>
#include <sched.h>

// Library includes
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_timer.h>

// Project includes
#include "ft900emu_intrin.h"
#include "ft900emu_irq.h"

// using namespace ...;

#define FT900_SYSTEM_CLOCK 90000000 // From libft900.h

#define TIMER_CONTROL_0 0x00
#define TIMER_CONTROL_1 0x01
#define TIMER_CONTROL_2 0x02
#define TIMER_CONTROL_3 0x03
#define TIMER_CONTROL_4 0x04
#define TIMER_INT       0x05
#define TIMER_SELECT    0x06
#define TIMER_WDG       0x07
#define TIMER_WRITE_LS  0x08
#define TIMER_WRITE_MS  0x09
#define TIMER_PRESC_LS  0x0A
#define TIMER_PRESC_MS  0x0B
#define TIMER_READ_LS   0x0C
#define TIMER_READ_MS   0x0D

#define TC0_SOFT_RESET      0x01
#define TC0_ACTIVE          0x02
#define TC0_IS_ACTIVE       (m_Register[TIMER_CONTROL_0] & TC0_ACTIVE)

#define TC1_START(ti)      (0x01 << ti)
#define TC1_STOP(ti)       (0x10 << ti)
#define TC1_IS_START(ti)   (m_Register[TIMER_CONTROL_1] & TC1_START(ti))
#define TC1_IS_STOP(ti)    (m_Register[TIMER_CONTROL_1] & TC1_STOP(ti))

#define TC2_PRESCALING(ti)    (0x10 << ti)
#define TC2_IS_PRESCALING(ti) (m_Register[TIMER_CONTROL_2] & TC2_PRESCALING(ti))

#define TC4_CLEAR(ti)      (0x01 << ti)
#define TC4_CLEAR_PRESCALER 0x10

#define TIMER_SELECTED (m_Register[TIMER_SELECT])

#define TC3_ONESHOT(ti)       (0x01 << ti)
#define TC3_COUNTUP(ti)       (0x10 << ti)
#define TC3_IS_ONESHOT(ti)    (m_Register[TIMER_CONTROL_3] & TC3_ONESHOT(ti))
#define TC3_IS_CONTINUOUS(ti) !(TC3_IS_ONESHOT(ti))
#define TC3_IS_COUNTUP(ti)    (m_Register[TIMER_CONTROL_3] & TC3_COUNTUP(ti))
#define TC3_IS_COUNTDOWN(ti)  !(TC3_IS_COUNTUP(ti))

#define TIMER_INT_ENABLED(ti)    (0x02 << (2 * ti))
#define TIMER_INT_IS_ENABLED(ti) (m_Register[TIMER_INT] & TIMER_INT_ENABLED(ti))
#define TIMER_INT_FIRED(ti)      (0x01 << (2 * ti))
#define TIMER_INT_IS_FIRED(ti)   (m_Register[TIMER_INT] & TIMER_INT_FIRED(ti))

#define FT900EMU_TIMER_INTERRUPT 17

namespace FT900EMU {

Timer::Timer(IRQ *irq) : m_IRQ(irq), m_RegIntLock(0), m_ThreadRunning(NULL)
{
	softReset();
	SDL_InitSubSystem(SDL_INIT_TIMER);
}

Timer::~Timer()
{
	if (m_ThreadRunning)
		stopTimer();

	SDL_QuitSubSystem(SDL_INIT_TIMER);
}

void Timer::softReset()
{
	// printf(F9ED "Timer soft reset" F9EE);

	if (m_ThreadRunning)
		stopTimer();

	memset(m_Register, 0, sizeof(m_Register));
	memset(m_TimerLS, 0, sizeof(m_TimerLS));
	memset(m_TimerMS, 0, sizeof(m_TimerMS));
	memset(m_TimerValue, 0, sizeof(m_TimerValue));
	memset(m_TimerRunning, 0, sizeof(m_TimerRunning));
	memset(m_TimerCounter, 0, sizeof(m_TimerCounter));
	memset(m_TimerQueue, 0, sizeof(m_TimerQueue));
	m_Clock = FT900_SYSTEM_CLOCK;
}

uint8_t Timer::ioRd8(uint32_t io_a)
{
	uint32_t idx = io_a - FT900EMU_MEMORY_TIMER_ADDR;
	// printf(F9ED "Timer ioRd8 %#x" F9EE, idx);

	switch (idx)
	{
	case TIMER_WRITE_LS:
	case TIMER_WRITE_MS:
	case TIMER_READ_LS:
	case TIMER_READ_MS:
		printf(F9EW "Timer read %i not implemented" F9EE, idx);
		FT900EMU_DEBUG_BREAK();
		break;
	default:
		break;
	}

	return m_Register[idx];
}

void Timer::ioWr8(uint32_t io_a, uint8_t io_dout)
{
	uint32_t idx = io_a - FT900EMU_MEMORY_TIMER_ADDR;
	// printf(F9ED "Timer ioWr8 %#x, %#x" F9EE, idx, (uint32_t)io_dout);

	// Pre-write hooks
	switch (idx)
	{
	case TIMER_CONTROL_0:
		if (io_dout & 0b11111100)
		{
			printf(F9EW "Unknown bits in TIMER_CONTROL_0" F9EE);
			FT900EMU_DEBUG_BREAK();
		}
		if (io_dout & TC0_SOFT_RESET)
		{
			softReset();
		}
		if (io_dout & TC0_ACTIVE)
		{
			if (!TC0_IS_ACTIVE)
			{
				printf(F9ED "Timer activate" F9EE);
				startTimer();
			}
		}
		else
		{
			if (TC0_IS_ACTIVE)
			{
				printf(F9ED "Timer deactivate" F9EE);
				stopTimer();
			}
		}
		break;
	case TIMER_CONTROL_1:
		// 0b11110000: Stop timers 4, 3, 2, 1
		// 0b00001111: Start timers 4, 3, 2, 1
		for (int ti = 0; ti < FT900EMU_TIMER_NB; ++ti)
			m_TimerRunning[ti] = (io_dout & TC1_START(ti)) ? true
				: ((io_dout & TC1_STOP(ti)) ? false : m_TimerRunning[ti]);
		break;
	case TIMER_CONTROL_2:
		if (io_dout & 0b00001111)
		{
			printf(F9EW "Unknown bits in TIMER_CONTROL_2" F9EE);
			FT900EMU_DEBUG_BREAK();
		}
		// 0b11110000: Enable scaled clock for timers 4, 3, 2, 1
		break;
	case TIMER_CONTROL_3:
		// 0b11110000: Enable countup for timers 4, 3, 2, 1
		// 0b00001111: Enable one shot for timers 4, 3, 2, 1
		break;
	case TIMER_CONTROL_4:
		if (io_dout & 0b11100000)
		{
			printf(F9EW "Unknown bits in TIMER_CONTROL_4" F9EE);
			FT900EMU_DEBUG_BREAK();
		}
		// 0b00001111: Clear timer 4, 3, 2, 1
		for (int ti = 0; ti < FT900EMU_TIMER_NB; ++ti)
			if (io_dout & TC4_CLEAR(ti))
		{
			m_TimerCounter[ti] = 0;
			SDL_AtomicSet(&m_TimerQueue[ti], 0);
		}
		// 0b00010000: Global timer clock prescaler
		if (io_dout & TC4_CLEAR_PRESCALER)
		{
			uint32_t prescaler =
				((uint32_t)m_Register[TIMER_PRESC_LS]
				| ((uint32_t)m_Register[TIMER_PRESC_MS] << 8)) + 1;
			m_Clock = FT900_SYSTEM_CLOCK / prescaler;
			printf(F9ED "Timer prescaler %u (clock: %u)" F9EE, prescaler, m_Clock);
			io_dout &= ~TC4_CLEAR_PRESCALER;
		}
		break;
	case TIMER_INT:
		// 0b10101010: Enable interrupt for timers 4, 3, 2, 1
		// 0b01010101: Timers 4, 3, 2, 1 were fired (read) / were handled (write)
		{
			SDL_AtomicLock(&m_RegIntLock);
			uint32_t intout =
				m_Register[TIMER_INT] & 0b01010101 // Original fired state
				| io_dout & 0b10101010; // New enabled state
			for (int ti = 0; ti < FT900EMU_TIMER_NB; ++ti)
				if (io_dout & TIMER_INT_FIRED(ti))
			{
				// Timer ti was handled, check if more need to be fired
				if (TIMER_INT_IS_ENABLED(ti))
				{
					int queue = SDL_AtomicGet(&m_TimerQueue[ti]);
					if (queue)
					{
						intout &= ~TIMER_INT_FIRED(ti);
						m_IRQ->interrupt(FT900EMU_TIMER_INTERRUPT);
						SDL_AtomicCAS(&m_TimerQueue[ti], queue, queue - 1);
					}
				}
			}
			m_Register[idx] = intout;
			SDL_AtomicUnlock(&m_RegIntLock);
			return; // Already written
		}
	case TIMER_WRITE_LS:
		m_TimerLS[TIMER_SELECTED] = io_dout;
		goto pushValue;
	case TIMER_WRITE_MS:
		m_TimerMS[TIMER_SELECTED] = io_dout;
	pushValue:
		m_TimerValue[TIMER_SELECTED] =
			((uint32_t)m_TimerLS[TIMER_SELECTED]
			| ((uint32_t)m_TimerMS[TIMER_SELECTED] << 8)) + 1;
		printf(F9ED "Timer value %u (ls %#x, ms %#x)" F9EE, m_TimerValue[TIMER_SELECTED], (uint32_t)m_TimerLS[TIMER_SELECTED], (uint32_t)m_TimerMS[TIMER_SELECTED]);
		return; // Don't write this to m_Register
	case TIMER_SELECT:
		// Timer selection
	case TIMER_PRESC_LS:
	case TIMER_PRESC_MS:
		// Write value
		break;
	default:
		printf(F9EW "Timer write %i not handled" F9EE, idx);
		FT900EMU_DEBUG_BREAK();
		break;
	}

	// Write
	m_Register[idx] = io_dout;
}

void Timer::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_TIMER_START;
	to = FT900EMU_MEMORY_TIMER_END;
}

void Timer::startTimer()
{
	SDL_assert(!m_ThreadRunning);
	printf(F9ED "Start timers" F9EE);
	m_ThreadRunning = SDL_CreateThread(timer, "FT900EMU::Timer", this);
}

void Timer::stopTimer()
{
	SDL_assert(m_ThreadRunning);
	printf(F9ED "Stop timers" F9EE);
	// Wait for timer thread to end (for safety)
	SDL_Thread *thread = m_ThreadRunning;
	m_ThreadRunning = NULL; // This stops the thread
	int status;
	SDL_WaitThread(thread, &status);
}

inline void Timer::timer()
{
	// This loop always runs at roughly 1000Hz
	// Higher accuracy cannot be guaranteed by the OS
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
	uint64_t freq = SDL_GetPerformanceFrequency(); // Linux: 1 000 000 000
	m_LastTick = SDL_GetPerformanceCounter();
	SDL_Delay(1);
	while (m_ThreadRunning)
	{
		uint64_t currentTick = SDL_GetPerformanceCounter();
		uint64_t delta = currentTick - m_LastTick;
		for (int ti = 0; ti < FT900EMU_TIMER_NB; ++ti)
			if (m_TimerRunning[ti])
		{
			// Might want to precalculate these
			uint64_t clock = TC2_IS_PRESCALING(ti) ? m_Clock : FT900_SYSTEM_CLOCK;
			uint64_t value = (uint64_t)m_TimerValue[ti] * freq / clock;

			uint64_t counter = m_TimerCounter[ti] + delta;

			// TODO: Backwards timer - what does it do???
			uint64_t queue = counter / value; // Any previous queue is overwritten and lost, this allows ticks to be lost on longer running functions
			counter %= value;

			if (queue)
			{
				// printf("Timer %i tick\n", ti);

				if (TIMER_INT_IS_ENABLED(ti))
				{
					// Call interrupt
					SDL_AtomicLock(&m_RegIntLock);
					m_Register[TIMER_INT] |= TIMER_INT_FIRED(ti);
					SDL_AtomicUnlock(&m_RegIntLock);
					m_IRQ->interrupt(FT900EMU_TIMER_INTERRUPT);
				}

				if (TC3_IS_ONESHOT(ti))
				{
					// Disable after one shot
					m_TimerRunning[ti] = false;
					counter = 0;
					queue = 0;
				}
				else
				{
					--queue;
				}
			}

			m_TimerCounter[ti] = counter;
			SDL_AtomicSet(&m_TimerQueue[ti], queue);
		}
		m_LastTick = currentTick;
		SDL_Delay(1);
	}
}

int Timer::timer(void *p)
{
	static_cast<Timer *>(p)->timer();
	return 0;
}

} /* namespace FT900EMU */

/* end of file */
