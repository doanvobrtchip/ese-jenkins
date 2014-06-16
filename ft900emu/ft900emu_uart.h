/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_UART_H
#define FT800EMU_UART_H
// #include <...>

// System includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

// Project includes

namespace FT900EMU {

#define FT900EMU_MEMORY_UART_START (0x10320u >> 2)
#define FT900EMU_MEMORY_UART_COUNT 4
#define FT900EMU_MEMORY_UART_SIZE (FT900EMU_MEMORY_UART_COUNT << 2)

#define FT900EMU_UART_ICR_NB 20

class UART : public FT32IO
{
public:
	UART(uint32_t id, FT32 *ft32);

	void softReset();

	void enablePad(bool enabled);
	void setOptions(bool clkSel, bool fifoSel, bool intSel);

	void ioWr(uint32_t idx, uint8_t d);

	virtual uint32_t ioRd(uint32_t io_a, uint32_t io_be);
	virtual void ioWr(uint32_t io_a, uint32_t io_be, uint32_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

private:
	uint8_t m[FT900EMU_MEMORY_UART_SIZE]; // FIXME
	uint8_t icr[FT900EMU_UART_ICR_NB]; // Indexed Control Registers

	uint32_t m_Id;
	FT32 *m_FT32;
	bool m_Enabled;
	bool m_ClkSel, m_FifoSel, m_IntSel;

}; /* class Chip */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_UART_H */

/* end of file */
