/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_IRQ_H
#define FT800EMU_IRQ_H
// #include <...>

// System includes
#include <list>

// Project includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

namespace FT900EMU {

#define FT900EMU_MEMORY_IRQ_START (0x100C0u >> 2)
#define FT900EMU_MEMORY_IRQ_COUNT 9

class IRQ : public FT32IO
{
public:
	IRQ();

	void softReset();

	// Called by a module to trigger an interrupt
	void interrupt(uint32_t irq);

	// Called by FT32 to check if there's an interrupt, ~0 if none
	inline uint32_t nextInterrupt()
	{
		// Only do function call if interrupt waiting (direct memread to pointer is fast)
		return (m_InterruptCheck)
			? nextInterruptInternal()
			: ~0;
	}

	// Called by FT32 when interrupt call returns
	void returnInterrupt();

	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be);
	virtual void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

private:
	uint32_t nextInterruptInternal();
	void lock();
	void unlock();

	bool globalInterruptMask();
	bool nestedInterrupt();
	uint32_t nestedDepth();

private:
	uint32_t m_Register[FT900EMU_MEMORY_IRQ_COUNT];

	volatile int m_Lock;
	uint32_t m_CurrentDepth;
	uint32_t m_CurrentInt;

	bool m_InterruptCheck;

	std::list<uint32_t> m_Waiting;

}; /* class Chip */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_IRQ_H */

/* end of file */
