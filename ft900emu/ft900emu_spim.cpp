/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_spim.h"

// System includes
#include <stdio.h>
#include <string.h>
#include <sched.h>

// Library includes
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_timer.h>

// Project includes
#include "ft900emu_intrin.h"
#include "ft900emu_irq.h"
#include "ft900emu_chip.h"

// using namespace ...;

// #define FT900EMU_SPIM_INTERRUPT 8

namespace FT900EMU {

SPIM::SPIM(IRQ *irq, SPISlave **spiSlaves) : m_IRQ(irq), m_SPISlaves(spiSlaves)
{
	printf(F9ED "SPIM Init" F9EE);

	softReset();
}

SPIM::~SPIM()
{

}

void SPIM::softReset()
{

}

uint32_t SPIM::ioRd32(uint32_t io_a, uint32_t io_be)
{
	uint32_t idx = io_a - FT900EMU_MEMORY_SPIM_ADDR;
	printf(F9ED "SPIM RD 32 %i" F9EE, idx);
	return m_Register[idx];
}

void SPIM::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	if (io_be & 0xFF)
	{
		uint32_t idx = io_a - FT900EMU_MEMORY_SPIM_ADDR;
		uint32_t v = (io_dout & io_be) | (m_Register[idx] & ~io_be);
		printf(F9ED "SPIM WR 32 %i <= %#x" F9EE, idx, v);

		// Really just ignore everything that we don't care about...

		m_Register[idx] = v;
	}
	else
	{
		printf(F9EW "Invalid SPIM write" F9EE);
		FT900EMU_DEBUG_BREAK();
	}
}

void SPIM::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_SPIM_ADDR;
	to = FT900EMU_MEMORY_SPIM_ADDR + FT900EMU_MEMORY_SPIM_COUNT;
}

} /* namespace FT900EMU */

/* end of file */
