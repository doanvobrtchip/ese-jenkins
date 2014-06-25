/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_CHIP_H
#define FT800EMU_CHIP_H
// #include <...>

// System includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

// Project includes

namespace FT900EMU {

class UART;
class IRQ;

#define FT900EMU_MEMORY_REGISTER_START (0x10000u >> 2)
#define FT900EMU_MEMORY_REGISTER_COUNT 47

class Chip : public FT32IO
{
public:
	Chip();
	~Chip();

	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be);
	virtual void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

	inline FT32 *ft32() { return m_FT32; }
	inline IRQ *irq() { return m_IRQ; }

	inline uint8_t padFunctionality(uint32_t padId);

private:
	void updateUART0Functionality();
	void updateUART0Configuration();
	void padWr(uint32_t pad_id, uint32_t pad_value);

private:
	uint32_t m_Register[FT900EMU_MEMORY_REGISTER_COUNT];

	FT32 *m_FT32;
	IRQ *m_IRQ;

	UART *m_UART0;

}; /* class Chip */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_CHIP_H */

/* end of file */
