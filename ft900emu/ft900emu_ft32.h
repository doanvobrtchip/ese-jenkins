/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_FT32_H
#define FT800EMU_FT32_H
// #include <...>

// System includes
#include "ft900emu_inttypes.h"

// Project includes

#define FT900EMU_FT32_MEMORY_SIZE 0x10000 // 64 KiB
#define FT900EMU_FT32_REGISTER_COUNT 32
#define FT900EMU_FT32_PROGRAM_MEMORY_COUNT 0x10000 // *4=256 KiB

#define FT900EMU_FT32_MAX_IO_CB 16 // Change if necessary

namespace FT900EMU {

class IRQ;

class FT32IO
{
public:
	// io_a is in 32bit indexed address (8bit addr >> 2)
	// io_be is the mask used for accessing sub-bytes FF FF FF FF, 00 00 FF FF etc
	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be);
	virtual void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout);

	// io_a is in 8bit indexed address
	virtual uint8_t ioRd8(uint32_t io_a);
	virtual void ioWr8(uint32_t io_a, uint8_t io_dout);

	// range is in 32bit indexed address
	virtual void ioGetRange(uint32_t &from, uint32_t &to) = 0;

};

/**
 * FT32
 * \brief FT32
 * \date 2014-06-14 16:29GMT
 * \author Jan Boon (Kaetemi)
 */
class FT32
{
public:
	FT32(IRQ *irq);
	~FT32();

	// Run the FT32 processor
	void run();

	// Stop the FT32 processor
	void stop();

	// Soft reset
	void softReset();

	inline void pm(int pma, uint32_t pmd) { m_ProgramMemory[pma] = pmd; }
	void io(FT32IO *io);
	void ioRemove(FT32IO *io);

private:
	uint32_t readMemU32(uint32_t addr);
	void writeMemU32(uint32_t addr, uint32_t value);
	uint16_t readMemU16(uint32_t addr);
	void writeMemU16(uint32_t addr, uint16_t value);
	uint8_t readMemU8(uint32_t addr);
	void writeMemU8(uint32_t addr, uint8_t value);

	void call(uint32_t pma);
	uint32_t exec(uint32_t pma);

	void push(uint32_t v);
	uint32_t pop();

	FT32IO *getIO(uint32_t io_a_32);
	uint32_t ioRd32(uint32_t io_a, uint32_t io_be);
	void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout);
	uint8_t ioRd8(uint32_t io_a);
	void ioWr8(uint32_t io_a, uint8_t io_dout);

private:
	uint8_t m_Memory[FT900EMU_FT32_MEMORY_SIZE];

	// uint8_t m_SafetyPadding[FT900EMU_FT32_MEMORY_SIZE * 8]; // FOR DEBUG

	uint32_t m_Register[FT900EMU_FT32_REGISTER_COUNT];
	uint32_t m_ProgramMemory[FT900EMU_FT32_PROGRAM_MEMORY_COUNT];

	IRQ *m_IRQ;

	int m_IONb;
	uint32_t m_IOFrom[FT900EMU_FT32_MAX_IO_CB];
	uint32_t m_IOTo[FT900EMU_FT32_MAX_IO_CB];
	FT32IO *m_IO[FT900EMU_FT32_MAX_IO_CB];

}; /* class FT32 */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_FT32_H */

/* end of file */
