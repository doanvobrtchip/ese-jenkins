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
#define FT900EMU_MEMORY_IRQ_BYTES (FT900EMU_MEMORY_IRQ_COUNT * 4)

#define FT900EMU_MEMORY_IRQ_BEGIN32 (FT900EMU_MEMORY_IRQ_START + (0x20u >> 2))
#define FT900EMU_MEMORY_IRQ_END8    (FT900EMU_MEMORY_IRQ_BEGIN32 << 2)
#define FT900EMU_MEMORY_IRQ_START8  (FT900EMU_MEMORY_IRQ_START << 2)

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

	virtual uint8_t ioRd8(uint32_t io_a);
	virtual void ioWr8(uint32_t io_a, uint8_t io_dout);
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
	union
	{
		uint32_t m_Register[FT900EMU_MEMORY_IRQ_COUNT];
		uint32_t m_Register8[FT900EMU_MEMORY_IRQ_BYTES];
	};

	volatile int m_Lock;
	uint32_t m_CurrentDepth;
	uint32_t m_CurrentInt;

	bool m_InterruptCheck;

	std::list<uint32_t> m_Waiting;
/*
	uint32_t m_BuiltinInterrupts; // Never masked, interrupt 0 + builtin stuff (exit etc)

	uint32_t m_Interrupts; // Does not contain interrupt 0
	uint32_t m_InterruptsSorted;
	*/

}; /* class Chip */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_IRQ_H */

/* end of file */
