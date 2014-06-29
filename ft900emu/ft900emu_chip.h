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
class Timer;
class SPIM;

#define FT900EMU_MEMORY_REGISTER_START (0x10000u >> 2)
#define FT900EMU_MEMORY_REGISTER_COUNT 47

#define FT900EMU_SPIM_SS_NB 8

class SPISlave
{
	virtual void cs(bool cs) = 0;
	virtual uint8_t transfer(uint8_t d) = 0;
};

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

	inline void setSPISlave(int ss, SPISlave *spiSlave) { m_SPISlaves[ss] = spiSlave; }

private:
	void updateUART0Functionality();
	void updateUART0Configuration();
	void updateSPIMPads();
	void padWr(uint32_t pad_id, uint32_t pad_value);

private:
	uint32_t m_Register[FT900EMU_MEMORY_REGISTER_COUNT];

	SPISlave *m_SPISlaves[FT900EMU_SPIM_SS_NB];

	FT32 *m_FT32;
	IRQ *m_IRQ;
	Timer *m_Timer;

	UART *m_UART0;
	SPIM *m_SPIM;

}; /* class Chip */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_CHIP_H */

/* end of file */
