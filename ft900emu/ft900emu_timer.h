/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_TIMER_H
#define FT800EMU_TIMER_H
// #include <...>

// System includes
#include <list>

// Project includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

namespace FT900EMU {

class IRQ;

#define FT900EMU_MEMORY_TIMER_ADDR  0x10340
#define FT900EMU_MEMORY_TIMER_BYTES 32
#define FT900EMU_MEMORY_TIMER_START (FT900EMU_MEMORY_TIMER_ADDR >> 2)
#define FT900EMU_MEMORY_TIMER_END   ((FT900EMU_MEMORY_TIMER_ADDR + FT900EMU_MEMORY_TIMER_BYTES) >> 2)

class Timer : public FT32IO
{
public:
	Timer(IRQ *irq);

	void softReset();

	virtual uint8_t ioRd8(uint32_t io_a);
	virtual void ioWr8(uint32_t io_a, uint8_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

private:
	// ...

private:
	uint8_t m_Register[FT900EMU_MEMORY_TIMER_BYTES];

	uint32_t m_Clock;

	IRQ *m_IRQ;

}; /* class Timer */

} /* namespace FT900EMU */

#endif /* #ifndef FT800EMU_TIMER_H */

/* end of file */
