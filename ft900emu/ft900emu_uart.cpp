/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_uart.h"

// System includes
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#	include <termios.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif

// Project includes
#include "ft900emu_intrin.h"

// using namespace ...;

// Number of sequential reads from LSR register before throttle down
#define FT900EMU_UART_LSR_READ_THROTTLE_LIMIT 8

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
#define R_RHR_THR_DLL  0
#define R_IER_ASR_DLM  1
#define R_ISR_FCR_EFR  2
#define R_LCR_RFL      3
#define R_MCR_XON1_TFL 4
#define R_LSR_XON2_ICR 5
#define R_MSR_XOFF1    6
#define R_SPR_XOFF2    7

#define ACR_950_REGISTERS (icr[ACR] & (1 << 7))
#define ACR_ICR_REGISTERS (icr[ACR] & (1 << 6))
#define LCR_7 (lcr & (1 << 7))
#define EFR_650_ENHANCED_MODE (efr & (1 << 4));

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
	
namespace /* anonymous */ {
	
#ifdef WIN32
	
bool haskey()
{
	return kbhit() != 0;
}

#else

bool haskey()
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);// | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return true;
	}
	return false;
}

#endif

} /* anonymous namespace */

UART::UART(uint32_t id, FT32 *ft32) :
	m_650(false),
	m_Id(0), m_FT32(ft32),
	m_Enabled(false),
	m_ClkSel(false), m_FifoSel(false), m_IntSel(false),
	m_LsrReadThrottleCounter(0)
{
	printf(F9ED "<--#--> UART :: Init" F9EE);
	softReset();
}

void UART::softReset()
{
	memset(icr, 0, sizeof(icr));

	dll_dlm = 0x01;
	mcr = m_ClkSel ? 0 : 1;
	// CPR = 0x20

	// LSR = 0x60 - receiver and transmitter are empty
	// ISR = 0x01 - no interrupt pending
	// ASR = 0x80
	// GDS = 0x01
	// DMS = 0x02

	lcr = 0;
	spr = 0;
	fcr = 0;
	efr = 0;
	m_650 = false;
}

void UART::enablePad(bool enabled)
{
	if (enabled != m_Enabled)
	{
		m_Enabled = enabled;
		// printf("<--#--> UART :: Pad = %s\n", enabled ? "ENABLED" : "DISABLED");
	}
}

void UART::setOptions(bool clkSel, bool fifoSel, bool intSel)
{
	m_ClkSel = clkSel;
	m_FifoSel = fifoSel;
	m_IntSel = intSel;
	// printf("<--#--> UART :: Clk = %s, Fifo = %s, Int = %s\n", clkSel ? "ON" : "OFF", fifoSel ? "ON" : "OFF", intSel ? "ON" : "OFF");
}

void UART::ioWr(uint32_t idx, uint8_t d)
{
	// printf("<--#--> UART :: Write mem %i = %#x\n", idx, d);
	m_LsrReadThrottleCounter = 0;
	
	switch (idx)
	{
		case R_RHR_THR_DLL: // 0
		{
			if (LCR_7) // DLL
			{
				// printf("    :: DLL %u\n", (uint32_t)d);
				dll_dlm = (dll_dlm & 0xFF00) | (uint16_t)d;
				// printf("    :: = %u\n", (uint32_t)dll_dlm);
			}
			else // THR
			{
				//printf("    :: THR write not implemented\n");
				//FT900EMU_DEBUG_BREAK();
				/*static int c = 0;
				++c; // %H%\r\n
				if (c == 5)
				FT900EMU_DEBUG_BREAK();*/

				// printf("    :: THR :: (%#x) (%c)\n", (uint32_t)d, d);

				putchar(d);
			}
			break;
		}
		case R_IER_ASR_DLM: // 1
		{
			if (LCR_7) // DLM
			{
				// printf("    :: DLM %u\n", (uint32_t)d);
				dll_dlm = (dll_dlm & 0x00FF) | (uint16_t)d << 8;
				// printf("    :: = %u\n", (uint32_t)dll_dlm);
			}
			else if (ACR_950_REGISTERS) // ASR
			{
				printf(F9EW "    :: ASR write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // IER
			{
				printf(F9EW "    :: IER write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			break;
		}
		case R_ISR_FCR_EFR: // 2
		{
			// printf("R_ISR_FCR_EFR\n");
			if (m_650) // EFR
			{
				// printf("    :: EFR\n");
				if (d & 0x10)
				{
					// printf("    :: Enhanced mode (650) is on!\n");
				}
				if (d & 0xEF)
				{
					// printf("    :: Unknown bits have been set\n");
					FT900EMU_DEBUG_BREAK();
				}
				efr = d;
			}
			else // FCR
			{
				// printf("    :: FCR (TODO)\n");
				// if (d & 0x01) printf("        :: FIFO on (TODO)\n");
				// if (d & 0x02) printf("        :: Flush RHR (TODO)\n");
				// if (d & 0x04) printf("        :: Flush THR (TODO)\n");
				if (d & 0xF8)
				{
					// printf(F9EW "    :: FCR :: Unknown bits have been set %#x" F9EE, d & 0xF8);
					FT900EMU_DEBUG_BREAK();
				}
				fcr = d; // TODO
			}
			// ISR is read
			break;
		}
		case R_LCR_RFL: // 3
		{
			if (ACR_950_REGISTERS) // RFL
			{
				printf(F9EW "    :: RFL write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // LCR
			{
				// printf("    :: LCR\n");
				if (d == 0xBF)
				{
					// printf("    :: Enable 650 register access\n");
					m_650 = true;
					lcr |= 0x80; // Also enable LCR[7] (divider latch registers DLL and DLM)
				}
				else
				{
					// if (m_650) printf("    :: Disable 650 register access\n");
					m_650 = false;
					/*if ((d & 0x03) == 0x03)
					{
						printf("    :: Set to 8 bits\n");
					}*/
					/*if (d & 0x80)
					{
						printf("    :: LCR[7] is on!\n");
					}*/
					if (d & 0x7C)
					{
						printf(F9EW "    :: Unknown LCR bits set" F9EE);
						FT900EMU_DEBUG_BREAK();
					}
					lcr = d;
				}
			}
			break;
		}
		case R_MCR_XON1_TFL: // 4
		{
			if (m_650) // XON1
			{
				printf(F9EW "    :: XON1 write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else if (ACR_950_REGISTERS) // TFL
			{
				printf(F9EW "    :: TFL write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // MCR
			{
				// printf("    :: MCR (TODO)\n");
				/*if (d & 0x02)
				{
					printf("    :: RTS (TODO)\n");
				}*/
				if (d & 0xFD)
				{
					printf(F9EW "    :: MCR :: Unknown bits were set" F9EE);
					FT900EMU_DEBUG_BREAK();
				}
				mcr = d;
			}
			break;
		}
		case R_LSR_XON2_ICR: // 5
		{
			if (m_650) // XON2
			{
				printf(F9EW "    :: XON2 write not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // ICR
			{
				// printf("    :: ICR, write %i\n", d);
				icr[spr] = d;
			}
			// LSR only in read mode
			break;
		}
		case R_SPR_XOFF2: // 7
		{
			if (m_650) // XOFF2
			{
				// printf("    :: XOFF2 write not implemented\n");
				FT900EMU_DEBUG_BREAK();
			}
			else
			{
				// printf("    :: SPR, address ICR %i\n", d);
				spr = d;
			}
			break;
		}
		default:
		{
			printf(F9EW "IO :: Index %i not implemented for write" F9EE, idx);
			FT900EMU_DEBUG_BREAK();
		}
	}
}

uint8_t UART::ioRd(uint32_t idx)
{
	int lsrReadThrottleCounter = m_LsrReadThrottleCounter;
	m_LsrReadThrottleCounter = 0;
	
	// printf("<--#--> UART :: Read mem %i\n", idx);
	switch (idx)
	{
		case R_RHR_THR_DLL: // 0
		{
			if (LCR_7) // DLL
			{
				printf(F9EW "    :: DLL read not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // RHR
			{
				if (!haskey())
				{
					printf(F9EW "    :: RHR has no input" F9EE);
					FT900EMU_DEBUG_BREAK();
				}
				// printf(F9EW "    :: RHR read not implemented" F9EE);
				// FT900EMU_DEBUG_BREAK();
				uint8_t c = getchar();
				// printf("    :: RHR '%c'\n", c);
				return c;
			}
			break;
		}
		case R_ISR_FCR_EFR: // 2
		{
			if (m_650) // EFR
			{
				// printf("    :: EFR\n");
				return efr;
			}
			else // ISR
			{
				printf(F9EW "    :: ISR read not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			// FCR is write
			break;
		}
		case R_LCR_RFL: // 3
		{
			if (ACR_950_REGISTERS) // RFL
			{
				printf(F9EW "    :: RFL read not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // LCR
			{
				if (m_650)
				{
					// The actual LCR value is not overwritten
					// printf("    :: LCR 650\n");
					return 0xBF;
				}
				else
				{
					// printf("    :: LCR %#x\n", lcr);
					return lcr;
				}
			}
			break;
		}
		case R_LSR_XON2_ICR: // 5
		{
			if (m_650) // XON2
			{
				printf(F9EW "    :: XON2 read not implemented" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else if (ACR_950_REGISTERS) // ?? // ICR
			{
				if (ACR_ICR_REGISTERS)
				{
					printf(F9EW "    :: ICR read not implemented" F9EE);
					FT900EMU_DEBUG_BREAK();
				}
				printf(F9EW "    :: ICR read invalid (1)" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else if (ACR_ICR_REGISTERS)
			{
				printf(F9EW "    :: ICR read invalid (2)" F9EE);
				FT900EMU_DEBUG_BREAK();
			}
			else // LSR
			{
				// printf("    :: LSR (Tx available)\n");
				// LSR[6] = 1: Transmitter & THR empty
				// LSR[5] = 1: Trans FIFO empty
				// LSR[0] = 1: Read data available
				m_LsrReadThrottleCounter = lsrReadThrottleCounter + 1;
				// if (m_LsrReadThrottleCounter == FT900EMU_UART_LSR_READ_THROTTLE_LIMIT + 1)
				//	printf(F9ED "    :: LSR throttle on" F9EE);
				if (m_LsrReadThrottleCounter > FT900EMU_UART_LSR_READ_THROTTLE_LIMIT)
				{
					// Sleep the FT32 core when looping LSR read
					// Q: Can we wait for haskey() with timeout of 1ms 
					//    instead?
					// A: No. See notes.
					// NOTE: Must use m_FT32.sleep, so the FT32 can be 
					//       woken up (->wake()) in case of interrupts.
					// NOTE: In case threaded input is implemented (for 
					//       interrupts or such), we can simply wake 
					//       the CPU as soon as new input is available.
					m_FT32->sleep(1);
				}
				bool dr = haskey();
				if (dr)
				{
					m_LsrReadThrottleCounter = 0;
				}
				return dr ? 0x61 : 0x60; // LSR[5] | LSR[6] Transmitter always available, nothing to read
			}
			break;
		}
		default:
		{
			printf(F9EW "    :: Index not implemented" F9EE);
			FT900EMU_DEBUG_BREAK();
		}
	}
	return 0;
}

uint8_t UART::ioRd8(uint32_t io_a)
{
	const uint32_t idx = io_a - ((FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id)) << 2);
	return ioRd(idx);
}

void UART::ioWr8(uint32_t io_a, uint8_t io_dout)
{
	const uint32_t idx = io_a - ((FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id)) << 2);
	ioWr(idx, io_dout);
}

void UART::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id);
	to = FT900EMU_MEMORY_UART_START + (FT900EMU_MEMORY_UART_COUNT * m_Id) + FT900EMU_MEMORY_UART_COUNT;
}

} /* namespace FT900EMU */

/* end of file */
