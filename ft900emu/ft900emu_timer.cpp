/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_timer.h"

// System includes
#include <stdio.h>
#include <string.h>
#include <sched.h>

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

#define TC2_PRESCALING(ti)    (0x10 << ti)
#define TC2_IS_PRESCALING(ti) (m_Register[TIMER_CONTROL_2] & TC2_PRESCALING(ti))

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

namespace FT900EMU {

Timer::Timer(IRQ *irq) : m_IRQ(irq)
{
	softReset();
}

void Timer::softReset()
{
	printf(F9ED "Timer soft reset" F9EE);
}

uint8_t Timer::ioRd8(uint32_t io_a)
{
	uint32_t idx = io_a - FT900EMU_MEMORY_TIMER_ADDR;
	printf(F9ED "Timer ioRd8 %#x" F9EE, idx);

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
	printf(F9ED "Timer ioWr8 %#x, %#x" F9EE, idx, (uint32_t)io_dout);

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
				// no-op for now
			}
		}
		else
		{
			if (TC0_IS_ACTIVE)
			{
				printf(F9EW "Timer deactivate, not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
		}
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
		// 0b00001111: Clear timer 4, 3, 2, 1 - TODO - Used to push the configuration?
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
		if (io_dout & 0b01010101)
		{
			printf(F9EW "Unknown bits in TIMER_INT" F9EE);
			FT900EMU_DEBUG_BREAK();
		}
		// 0b10101010: Enable interrupt for timers 4, 3, 2, 1
		break;
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

} /* namespace FT900EMU */

/* end of file */
