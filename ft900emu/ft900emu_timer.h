/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_TIMER_H
#define FT800EMU_TIMER_H
// #include <...>

// System includes

// Library includes
#include <SDL_atomic.h>

// Project includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

struct SDL_Thread;

namespace FT900EMU {

class IRQ;

#define FT900EMU_MEMORY_TIMER_ADDR  0x10340
#define FT900EMU_MEMORY_TIMER_BYTES 32
#define FT900EMU_MEMORY_TIMER_START (FT900EMU_MEMORY_TIMER_ADDR >> 2)
#define FT900EMU_MEMORY_TIMER_END   ((FT900EMU_MEMORY_TIMER_ADDR + FT900EMU_MEMORY_TIMER_BYTES) >> 2)

#define FT900EMU_TIMER_NB 4

class Timer : public FT32IO
{
public:
	Timer(IRQ *irq);
	~Timer();

	void softReset();

	virtual uint8_t ioRd8(uint32_t io_a);
	virtual void ioWr8(uint32_t io_a, uint8_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

private:
	void startTimer();
	void stopTimer();
	static int timer(void *p);
	void timer();

private:
	uint8_t m_Register[FT900EMU_MEMORY_TIMER_BYTES];
	SDL_SpinLock m_RegIntLock;

	uint8_t m_TimerLS[FT900EMU_TIMER_NB];
	uint8_t m_TimerMS[FT900EMU_TIMER_NB];
	uint32_t m_TimerValue[FT900EMU_TIMER_NB];

	bool m_TimerRunning[FT900EMU_TIMER_NB];
	uint64_t m_TimerCounter[FT900EMU_TIMER_NB];
	SDL_atomic_t m_TimerQueue[FT900EMU_TIMER_NB];

	uint32_t m_Clock; // Ticks per second

	SDL_Thread *m_ThreadRunning;
	uint64_t m_LastTick; // In native system timer ticks

	IRQ *m_IRQ;

}; /* class Timer */

} /* namespace FT900EMU */

#endif /* #ifndef FT800EMU_TIMER_H */

/* end of file */
