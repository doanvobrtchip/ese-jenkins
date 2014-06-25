/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_chip.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft900emu_intrin.h"
#include "ft900emu_irq.h"
#include "ft900emu_uart.h"
#include "ft900emu_timer.h"

// using namespace ...;

#define FT900EMU_REG_CHIP_ID                 0
#define FT900EMU_REG_CHIP_CONFIGURATION      1
#define FT900EMU_REG_CLOCK_CONFIGURATION     2
#define FT900EMU_REG_MISC_CONFIGURATION      6
#define FT900EMU_REG_PAD_CONFIGURATION_START 7
#define FT900EMU_REG_PAD_48_51               19
#define FT900EMU_REG_PAD_52_55               20
#define FT900EMU_REG_PAD_CONFIGURATION_END   24

#define FT900EMU_CLOCK_UART0_ENA 0x0010
#define FT900EMU_CLOCK_UART1_ENA 0x0008

#define FT900EMU_MISC_PERI_SOFTRESET 0x80000000u
#define FT900EMU_MISC_UART0          0x00380000u
#define FT900EMU_MISC_UART0_INTSEL   0x00080000u
#define FT900EMU_MISC_UART0_FIFOSEL  0x00100000u
#define FT900EMU_MISC_UART0_CLKSEL   0x00200000u

#define FT900EMU_PAD_FUNCTIONALITY(padcfg) ((padcfg & 0xC0u) >> 6)

#define FT900EMU_PAD_F_MASKALL 0xC0C0C0C0u
#define FT900EMU_PAD_F3_ALL    0xC0C0C0C0u

namespace FT900EMU {

Chip::Chip() :
	m_UART0(NULL)
{
	m_IRQ = new IRQ();
	m_FT32 = new FT32(m_IRQ);
	m_FT32->io(m_IRQ);
	m_FT32->io(this);
	m_Timer = new Timer(m_IRQ);
	m_FT32->io(m_Timer);
}

Chip::~Chip()
{
	// TODO: Unregister io
	m_FT32->ioRemove(m_UART0);
	delete m_UART0; m_UART0 = NULL;

	m_FT32->ioRemove(m_Timer);
	delete m_Timer; m_Timer = NULL;
	m_FT32->ioRemove(this);
	m_FT32->ioRemove(m_IRQ);
	delete m_FT32; m_FT32 = NULL;
	delete m_IRQ; m_IRQ = NULL;
}

// io_a is addr / 4 (read per 4 bytes)
uint32_t Chip::ioRd32(uint32_t io_a, uint32_t io_be)
{
	uint idx = io_a - FT900EMU_MEMORY_REGISTER_START;
	printf(F9ED "Chip :: Read register %i (%#x): %#x" F9EE, idx, idx << 2, m_Register[idx]);
	return m_Register[idx] & io_be;
}

void Chip::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	const uint32_t idx = io_a - FT900EMU_MEMORY_REGISTER_START;
	uint32_t v = (io_dout & io_be) | (m_Register[idx] & ~io_be);
	const uint32_t diffmask = v ^ m_Register[idx];
	printf(F9ED "Chip :: Write register %i (%#x): %#x [diffmask = %#x]" F9EE, idx, idx << 2, v, diffmask);

	// PRE-WRITE HANDLING
	switch (idx)
	{
		case FT900EMU_REG_CLOCK_CONFIGURATION:
		{
			printf(F9ED "(!) REG_CLOCK_CONFIGURATION" F9EE);
			if (diffmask & FT900EMU_CLOCK_UART0_ENA)
			{
				printf(F9ED "(!) UART0 = %s" F9EE, v & FT900EMU_CLOCK_UART0_ENA ? "ENABLED" : "DISABLED");
				if (v & FT900EMU_CLOCK_UART0_ENA)
				{
					if (!m_UART0)
					{
						m_UART0 = new UART(0, m_FT32);
						m_FT32->io(m_UART0);
						updateUART0Configuration();
					}
				}
				else
				{
					if (m_UART0)
					{
						m_FT32->ioRemove(m_UART0);
						delete m_UART0; m_UART0 = NULL;
						FT900EMU_DEBUG_BREAK();
					}
				}
				updateUART0Functionality();
			}
			if (diffmask & FT900EMU_CLOCK_UART1_ENA)
			{
				printf(F9ED "(!) UART1 = %s" F9EE, v & FT900EMU_CLOCK_UART1_ENA ? "ENABLED" : "DISABLED");
				FT900EMU_DEBUG_BREAK();
			}
			break;
		}
		case FT900EMU_REG_MISC_CONFIGURATION:
		{
			if (diffmask & FT900EMU_MISC_PERI_SOFTRESET)
			{
				printf(F9ED "(!) SOFTRESET" F9EE);
				v &= ~FT900EMU_MISC_PERI_SOFTRESET;
				// todo: perhaps a beginSoftReset / endSoftReset is needed to guarantee correct dependency ordering
				if (m_UART0) m_UART0->softReset();
				m_Timer->softReset();
				m_FT32->softReset();
				m_IRQ->softReset();
			}
			break;
		}
	}

	// WRITE
	m_Register[idx] = v;

	// POST-WRITE
	switch (idx)
	{
		case FT900EMU_REG_MISC_CONFIGURATION:
		{
			if (diffmask & FT900EMU_MISC_UART0)
			{
				updateUART0Configuration();
			}
			break;
		}
		default:
		{
			if (FT900EMU_REG_PAD_CONFIGURATION_START <= idx && idx < FT900EMU_REG_PAD_CONFIGURATION_END)
			{
				uint32_t padIdxBase = (idx - FT900EMU_REG_PAD_CONFIGURATION_START) << 2;
				if (diffmask & 0x000000FFu)
					padWr(padIdxBase, (v & 0x000000FFu));
				if (diffmask & 0x0000FF00u)
					padWr(padIdxBase + 1, (v & 0x0000FF00u) >> 8);
				if (diffmask & 0x00FF0000u)
					padWr(padIdxBase + 2, (v & 0x00FF0000u) >> 16);
				if (diffmask & 0xFF000000u)
					padWr(padIdxBase + 3, (v & 0xFF000000u) >> 24);
			}
			break;
		}
	}
}

void Chip::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_REGISTER_START;
	to = FT900EMU_MEMORY_REGISTER_START + FT900EMU_MEMORY_REGISTER_COUNT;
}

void Chip::padWr(uint32_t pad_id, uint32_t pad_value) // pad_value is 8 bits
{
	printf(F9ED "[#] PAD %i = %i (%#x)" F9EE, pad_id, pad_value, pad_value);
	if (pad_id >= 48 && pad_id < 56)
		updateUART0Functionality();
}

void Chip::updateUART0Functionality()
{
	if (m_UART0)
	{
		bool enabled = ((m_Register[FT900EMU_REG_PAD_48_51] & FT900EMU_PAD_F_MASKALL) == FT900EMU_PAD_F3_ALL)
			&& ((m_Register[FT900EMU_REG_PAD_52_55] & FT900EMU_PAD_F_MASKALL) == FT900EMU_PAD_F3_ALL);
		m_UART0->enablePad(enabled);
	}
}

void Chip::updateUART0Configuration()
{
	if (m_UART0)
	{
		m_UART0->setOptions(
			m_Register[FT900EMU_REG_MISC_CONFIGURATION] & FT900EMU_MISC_UART0_CLKSEL,
			m_Register[FT900EMU_REG_MISC_CONFIGURATION] & FT900EMU_MISC_UART0_FIFOSEL,
			m_Register[FT900EMU_REG_MISC_CONFIGURATION] & FT900EMU_MISC_UART0_INTSEL);
	}
}

inline uint8_t Chip::padFunctionality(uint32_t padId)
{
	int8_t *padCfg = reinterpret_cast<int8_t *>(&m_Register[FT900EMU_REG_PAD_CONFIGURATION_START]);
	return FT900EMU_PAD_FUNCTIONALITY(padCfg[padId]);
}

} /* namespace FT900EMU */

/* end of file */
