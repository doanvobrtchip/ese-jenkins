/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_spim.h"

// System includes
#include <stdio.h>
#include <string.h>

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

#define SPIM_SPCR     0
#define SPIM_SPSR     1
#define SPIM_SPDR     2
#define SPIM_SSCR     3
#define SPIM_SFCR     4
#define SPIM_STFCR    5
#define SPIM_SPDR_BIS 6

namespace FT900EMU {

SPIM::SPIM(IRQ *irq, SPISlave **spiSlaves) : m_IRQ(irq), m_SPISlaves(spiSlaves), m_SS(~0), m_LastAccessed(~0)
{
	printf(F9ED "SPIM Init" F9EE);

	softResetInternal();
}

SPIM::~SPIM()
{

}

void SPIM::softReset()
{
	ioWr32(FT900EMU_MEMORY_SPIM_ADDR + SPIM_SSCR, 0xFF, 0xFF);
	softResetInternal();
}

void SPIM::softResetInternal()
{
	memset(m_Register, 0, sizeof(m_Register));
	m_Register[SPIM_SSCR] = 0xFF;
}

uint32_t SPIM::ioRd32(uint32_t io_a, uint32_t io_be)
{
	uint32_t idx = io_a - FT900EMU_MEMORY_SPIM_ADDR;
	if (idx != SPIM_SSCR && idx != SPIM_SPDR && idx != SPIM_SPSR) printf(F9ED "SPIM RD 32 %i" F9EE, idx);
	uint32_t v = m_Register[idx];
	switch (idx)
	{
		case SPIM_SPDR:
		{
			m_Register[SPIM_SPSR] &= ~0x0C;
			// m_IRQ->interrupt(FT900EMU_BUILTIN_IRQ_REPORT_CUR);
			if (m_LastAccessed == SPIM_SPSR)
				m_Register[SPIM_SPSR] &= ~0xC0;
			break;
		}
	}
	m_LastAccessed = idx;
	return v;
}

void SPIM::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	if (io_be & 0xFF)
	{
		uint32_t idx = io_a - FT900EMU_MEMORY_SPIM_ADDR;
		uint32_t v = (io_dout & io_be) | (m_Register[idx] & ~io_be);
		if (idx != SPIM_SSCR && idx != SPIM_SPDR) printf(F9ED "SPIM WR 32 %i <= %#x" F9EE, idx, v);

		// Really just ignore everything that we don't care about...
		switch (idx)
		{
			case SPIM_SSCR:
			{
				// Disable old
				if (~m_SS && m_SPISlaves[m_SS])
					m_SPISlaves[m_SS]->cs(false);
				m_SS = ~0;
				// Enable new, ignore any double pin selections
				for (int i = 0; i < FT900EMU_SPIM_SS_NB; ++i)
				{
					if (!((v >> i) & 1))
					{
						m_SS = i;
						if (m_SPISlaves[m_SS])
							m_SPISlaves[m_SS]->cs(true);
						break;
					}
				}
				// printf("WRITE SS = %i\n", m_SS);
				break;
			}
			case SPIM_SPDR:
			{
				// Very basic
				if (m_LastAccessed == SPIM_SPSR)
					m_Register[SPIM_SPSR] &= ~0xC0;
				if (~m_SS && m_SPISlaves[m_SS])
				{
					v = m_SPISlaves[m_SS]->transfer(v);
					// printf("transfer, sent %#x, received %#x\n", io_dout, v);
					m_Register[SPIM_SPSR] |= 0x8C;
				}
				break;
			}
		}

		m_LastAccessed = idx;
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
