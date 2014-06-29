/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT900EMU_SPIM_H
#define FT900EMU_SPIM_H
// #include <...>

// System includes
#include <list>

// Library includes

// Project includes
#include "ft900emu_inttypes.h"
#include "ft900emu_ft32.h"

namespace FT900EMU {

class IRQ;
class SPISlave;

#define FT900EMU_MEMORY_SPIM_ADDR  (0x102A0 >> 2)
#define FT900EMU_MEMORY_SPIM_COUNT 7

#define FT900EMU_TIMER_NB 4

class SPIM : public FT32IO
{
public:
	SPIM(IRQ *irq, SPISlave **spiSlaves);
	~SPIM();

	void softReset();

	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be);
	virtual void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout);
	virtual void ioGetRange(uint32_t &from, uint32_t &to);

private:
	uint32_t m_Register[FT900EMU_MEMORY_SPIM_COUNT];

	IRQ *m_IRQ;
	SPISlave **m_SPISlaves;

}; /* class SPIM */

} /* namespace FT900EMU */

#endif /* #ifndef FT900EMU_SPIM_H */

/* end of file */
