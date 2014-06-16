/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_uart.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft900emu_intrin.h"

// using namespace ...;

// r
#define ACR 0x00 // Additional Control Register
#define CPR 0x01 // Clock Prescaler Register
#define TCR 0x02 // Time Clock Register
#define CKS 0x03 // Clock Select Register
#define TTL 0x04 // Transmitter Trigger Level
#define RTL 0x05 // Receiver Trigger Level
#define FCL 0x06 // Flow Control Level (low)
#define FCH 0x07 // Flow Control Level (high)
#define ID1 0x08 // Identification Register 1
#define ID2 0x09 // Identification Register 2
#define ID3 0x0A // Identification Register 3
#define REV 0x0B // Revision Register
#define CSR 0x0C // Channel Software Register
#define NMR 0x0D // Nine-bit mode Register
#define MDM 0x0E // Modem Disable Mask register
#define RFC 0x0F // FCR register read
#define GDS 0x10 // Good Data Status register
#define DMS 0x11 // DMA register
#define CKA 0x13 // Clock Alteration register

// m
#define RHR_THR_DLL 0
#define IER_ASR_DLM 1
#define ISR_FCR_EFR 2
#define LCR_RFL     3
#define MCR_TFL     4
#define LSR_ICR     5
#define MSR         6
#define SPR         7

  //      //                 R    W   650  950  lcr7
  /* 0 */ //union { uint8_t rhr, thr,           dll    ; };
  /* 1 */ //union { uint8_t ier,           asr, dlm    ; };
  /* 2 */ //union { uint8_t isr, fcr, efr              ; };
  /* 3 */ //union { uint8_t lcr,           rfl         ; };
  /* 4 */ //union { uint8_t mcr,           tfl         ; };
  /* 5 */ //union { uint8_t lsr,           icr         ; };
  /* 6 */ //union { uint8_t msr                        ; };
  /* 7 */ //union { uint8_t spr                        ; };

  // CSR = Channel Software Reset

namespace FT900EMU {

UART::UART(uint32_t id, FT32 *ft32) : m_Id(0), m_FT32(ft32),
	m_Enabled(false),
	m_ClkSel(false), m_FifoSel(false), m_IntSel(false)
{
	printf("<--#--> UART :: Init\n");
	ft32->io(this);
	softReset();
}

void UART::softReset()
{
	memset(icr, 0, sizeof(icr));

	// DLL = 0x01
	// MCR = m_ClkSel ? 0 : 1;
	// CPR = 0x20

	// LSR = 0x60 - receiver and transmitter are empty
	// ISR = 0x01 - no interrupt pending
	// ASR = 0x80
	// GDS = 0x01
	// DMS = 0x02
}

void UART::enablePad(bool enabled)
{
	if (enabled != m_Enabled)
	{
		m_Enabled = enabled;
		printf("<--#--> UART :: Pad = %s\n", enabled ? "ENABLED" : "DISABLED");
	}
}

void UART::setOptions(bool clkSel, bool fifoSel, bool intSel)
{
	m_ClkSel = clkSel;
	m_FifoSel = fifoSel;
	m_IntSel = intSel;
	printf("<--#--> UART :: Clk = %s, Fifo = %s, Int = %s\n", clkSel ? "ON" : "OFF", fifoSel ? "ON" : "OFF", intSel ? "ON" : "OFF");
}

// io_a is addr / 4 (read per 4 bytes)
uint32_t UART::ioRd(uint32_t io_a, uint32_t io_be)
{
	uint32_t *mem = reinterpret_cast<uint32_t *>(m);
	uint idx = io_a - (FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id));
	printf("<--#--> UART :: Read mem (%#x): %#x\n", io_a << 2, mem[idx]);
	return mem[idx] & io_be;
}

void UART::ioWr(uint32_t idx, uint8_t d)
{
	printf("<--#--> UART :: Write mem %i = %#x\n", idx, d);

	// WRITE
	m[idx] = d;

	/*switch (idx)
	{
		case LCR_RFL: // 3
		{
			if (d == 0xBF) // LCR
			{
				printf("<--#--> UART :: Wrote 0xBF to LCR\n");
				// FT900EMU_DEBUG_BREAK();
			}
			break;
		}
		case LSR_ICR: // 5
		{
			if (m[LCR] == 0xBF)
			{
				printf("<--#--> UART :: LCR is 0xBF, cannot write\n");
				FT900EMU_DEBUG_BREAK();
			}
			// ICR => Write value to r address at m[SPR]
			printf("<--#--> UART :: Reg write %i <= %#x\n", m[SPR], d);
			r[m[SPR]] = d;
		}
		case SPR: // 7
		{
			if (r[ACR] & 1)
			{
				// Read specified register into ICR
				m[LSR_ICR] = r[d];
			}
			else
			{
				// Enable write to register address at m[SPR]
			}
		}
	}*/
}

void UART::ioWr(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	uint32_t *mem = reinterpret_cast<uint32_t *>(m);
	const uint32_t idx = io_a - (FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id));
	const uint32_t v = (io_dout & io_be) | (mem[idx] & ~io_be);
	const uint32_t diffmask = v ^ mem[idx];
	if (diffmask & 0x000000FFu)
		ioWr((idx << 2), v & 0xFFu);
	if (diffmask & 0x0000FF00u)
		ioWr((idx << 2) + 1, (v >> 8) & 0xFFu);
	if (diffmask & 0x00FF0000u)
		ioWr((idx << 2) + 2, (v >> 16) & 0xFFu);
	if (diffmask & 0xFF000000u)
		ioWr((idx << 2) + 3, (v >> 24) & 0xFFu);
}

void UART::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id);
	to = FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id) + FT900EMU_MEMORY_UART_COUNT;
}

} /* namespace FT900EMU */

/* end of file */
