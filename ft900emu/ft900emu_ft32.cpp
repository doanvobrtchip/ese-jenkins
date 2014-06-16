/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_ft32.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft900emu_intrin.h"
#include "ft900emu_ft32_def.h"
#include "ft900emu_irq.h"

// using namespace ...;

namespace FT900EMU {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

static FTEMU_FORCE_INLINE uint32_t nunsigned(uint32_t siz, uint32_t bits)
{
	uint32_t mask = (1L << siz) - 1;
	return bits & mask;
}

static FTEMU_FORCE_INLINE int32_t nsigned(uint32_t siz, int32_t bits)
{
	uint32_t shift = (sizeof(int) * 8) - siz;
	return (bits << shift) >> shift;
}

static FTEMU_FORCE_INLINE uint32_t ror(uint32_t n, uint32_t b)
{
	b &= 31;
	return (n >> b) | (n << (32 - b));
}

static FTEMU_FORCE_INLINE uint32_t bins(uint32_t d, uint32_t f, uint32_t len, uint32_t pos)
{
	uint32_t mask = ((1 << len) - 1) << pos;
	return (d & ~mask) | ((f << pos) & mask);
}

static FTEMU_FORCE_INLINE uint32_t flip(uint32_t x, uint32_t b)
{
	if (b & 1)
		x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
	if (b & 2)
		x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
	if (b & 4)
		x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
	if (b & 8)
		x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
	if (b & 16)
		x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
	return x;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

FT32::FT32(IRQ *irq) : m_IRQ(irq),
	m_IONb(0)
{
	io(irq);
	softReset();
}

FT32::~FT32()
{

}

void FT32::run()
{
	m_Running = true;
	call(0);
}

void FT32::stop()
{
	m_Running = false;
}

void FT32::softReset()
{
	memset(m_Memory, 0, FT900EMU_FT32_MEMORY_SIZE);
	m_ProgramMemory[FT900EMU_FT32_PROGRAM_MEMORY_COUNT - 1] = 0x00300023;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void FT32::io(FT32IO *io)
{
	io->ioGetRange(m_IOFrom[m_IONb], m_IOTo[m_IONb]);
	m_IO[m_IONb] = io;
	++m_IONb;
	m_IONb %= FT900EMU_FT32_MAX_IO_CB; // Safety
}

uint32_t FT32::ioRd(uint32_t io_a, uint32_t io_be)
{
	for (int i = 0; i < m_IONb; ++i)
	{
		if (m_IOFrom[i] <= io_a && io_a < m_IOTo[i])
		{
			return m_IO[i]->ioRd(io_a, io_be);
		}
	}
	printf("Read not handled: %#x (mask: %x)\n", io_a << 2, io_be);
	//FT900EMU_DEBUG_BREAK();
	return 0xDEADBEEF;
}

void FT32::ioWr(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	for (int i = 0; i < m_IONb; ++i)
	{
		// printf("%#x\n", m_IOFrom[i]);
		if (m_IOFrom[i] <= io_a && io_a < m_IOTo[i])
		{
			m_IO[i]->ioWr(io_a, io_be, io_dout);
			return;
		}
	}
	printf("Write not handled: %#x = %u (mask: %x)\n", io_a << 2, io_dout, io_be);
	//FT900EMU_DEBUG_BREAK();
}

FTEMU_FORCE_INLINE uint32_t FT32::readMemU32(uint32_t addr)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		return reinterpret_cast<uint32_t &>(m_Memory[addr]);
	}
	else
	{
		uint32_t b = addr % 4;
		if (!b)
		{
			b <<= 3;
			return ioRd(addr >> 2, 0xFFFFFFFFu);
		}
		else
		{
			// TODO: Unaligned
			printf("Not implemented UNALIGNED READ 32\n");
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE void FT32::writeMemU32(uint32_t addr, uint32_t value)
{
	// printf("######## WRITE: [%#x] <- %#x ########\n", addr, value);
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		reinterpret_cast<uint32_t &>(m_Memory[addr]) = value;
	}
	else
	{
		uint32_t b = addr % 4;
		if (!b)
		{
			b <<= 3;
			ioWr(addr >> 2, 0xFFFFFFFFu, value);
		}
		else
		{
			// TODO: Unaligned
			printf("Not implemented UNALIGNED WRITE 32 [%#x] <- %#x\n", addr, value);
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE uint16_t FT32::readMemU16(uint32_t addr)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		return reinterpret_cast<uint16_t &>(m_Memory[addr]);
	}
	else
	{
		uint32_t b = addr % 4;
		if (b != 3)
		{
			b <<= 3;
			return (ioRd(addr >> 2, 0xFFFFu << b) >> b) & 0xFFFFu;
		}
		else
		{
			// TODO: Unaligned
			printf("Not implemented UNALIGNED READ 16\n");
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE void FT32::writeMemU16(uint32_t addr, uint16_t value)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		reinterpret_cast<uint16_t &>(m_Memory[addr]) = value;
	}
	else
	{
		uint32_t b = addr % 4;
		if (b != 3)
		{
			b <<= 3;
			ioWr(addr >> 2, (0xFFFFu << b), (uint32_t)value << b);
		}
		else
		{
			// TODO: Unaligned
			printf("Not implemented UNALIGNED WRITE 16\n");
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE uint8_t FT32::readMemU8(uint32_t addr)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		return m_Memory[addr];
	}
	else
	{
		printf("READU8: %#x\n", addr);
		uint32_t b = addr % 4;
		b <<= 3;
		return (ioRd(addr >> 2, (0xFFu << b)) >> b) & 0xFFu;
	}
}

FTEMU_FORCE_INLINE void FT32::writeMemU8(uint32_t addr, uint8_t value)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		m_Memory[addr] = value;
	}
	else
	{
		uint32_t b = addr % 4;
		b <<= 3;
		printf("WRITEU8: %#x = %#x\n", addr, value);
		ioWr(addr >> 2, (0xFFu << b), (uint32_t)value << b);
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

FTEMU_FORCE_INLINE void FT32::push(uint32_t v)
{
	uint32_t stackp = (m_Register[FT32_REG_STACK] - 4) & FT32_REG_STACK_MASK;
	writeMemU32(stackp, v);
	m_Register[FT32_REG_STACK] = stackp;
}

FTEMU_FORCE_INLINE uint32_t FT32::pop()
{
	uint32_t stackp = m_Register[FT32_REG_STACK];
	uint32_t r = readMemU32(stackp);
	m_Register[FT32_REG_STACK] = (stackp + 4) & FT32_REG_STACK_MASK;
	return r;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void FT32::call(uint32_t pma)
{
	// VERIFY:
	// When doing 8 or 16 bit operations, are the other bits in the destination register to be preserved, or are they undefined?
	// What's the behaviour of STRCMP etcetera beyond the RAM range?
	// Is MUL signed or unsigned?
	uint32_t cur = pma;
	while (m_Running)
	{
		// Check for IRQ
		uint32_t nirq = m_IRQ->nextInterrupt();
		if (~nirq) call(nirq + 2);

		// printf("R0: %i; R1: %i; R2: %i; R3: %i; R4: %i; R5: %i; R13: %i; R14: %i\n", m_Register[0], m_Register[1], m_Register[2], m_Register[3], m_Register[4], m_Register[5], m_Register[13], m_Register[14]);
		uint32_t inst = m_ProgramMemory[cur & FT32_PM_CUR_MASK];
		// printf("%x    CUR: %u; INST: %#010x\n", cur << 2, cur, inst);
		// if (cur << 2 == 0x2db0) FT900EMU_DEBUG_BREAK(); // end of <ft900_init>
		// if (cur << 2 == 0x3224) FT900EMU_DEBUG_BREAK(); // inside <pad_setter>
		// if (cur << 2 == 0x3234) FT900EMU_DEBUG_BREAK(); // end of <pad_setter>
		if (cur << 2 == 0x31c) printf("*********************MAIN***************************\n");
		if (cur << 2 == 0x3e8) printf("*********************PUTS***************************\n");
		if (cur << 2 == 0x334) printf("*********************PUTS_R***************************\n");
		if (cur << 2 == 0x408) printf("********************* <__sfvwrite_r> ***************************\n");
		if (cur << 2 == 0x4c8) printf("********************* R4... ***************************\n");
		if (cur << 2 == 0x4cc) printf("ok\n");
		if (cur << 2 == 0x17cc) printf("********************* <__swsetup_r> ***************************\n");
		if (cur << 2 == 0xe8) { printf("********************* <exit> ***************************\n"); FT900EMU_DEBUG_BREAK(); }
		if (cur << 2 == 0x303c) printf("/////////////// <ft900_uart_set_baud> ////////////////\n");
		if (cur << 2 == 0x30c8) printf("/////////////// <ft900_uart_activate> ////////////////\n");
		if (cur << 2 == 0x31f4) printf("/////////////// <pad_setter> ////////////////\n");
		if (cur << 2 == 0x3270) printf("/////////////// <attach_interrupt> ////////////////\n");
		if (cur << 2 == 0x3284) printf("/////////////// <ft900_pad_set_function> ////////////////\n");
		if (cur << 2 == 0x3238) printf("/////////////// <streamout_pm_b> ////////////////\n");
		switch (inst & FT32_PATTERNMASK)
		{
			case FT32_PATTERN_TOC:
			case FT32_PATTERN_TOCI:
			{
				uint32_t target = (inst & FT32_TOC_INDIRECT)
					? m_Register[FT32_R2(inst)]
					: FT32_TOC_PA(inst);
				// if (inst & FT32_TOC_INDIRECT) printf("    TOCI %i (R%i)\n", target, FT32_R2(inst));
				// else printf("    TOC %i\n", target);
				// if (cur << 2 == 0xdc) FT900EMU_DEBUG_BREAK();
				bool condition = FT32_TOC_CR_UNCONDITIONAL(inst)
					|| (((m_Register[FT32_TOC_CR_REGISTER(inst)] >> FT32_TOC_CB(inst)) & 1) == FT32_TOC_CV(inst));
				// if (!FT32_TOC_CR_UNCONDITIONAL(inst)) printf("      R%i: %#x, CB: %i, CV: %i\n", FT32_TOC_CR_REGISTER(inst), m_Register[FT32_TOC_CR_REGISTER(inst)], FT32_TOC_CB(inst), FT32_TOC_CV(inst));
				// else printf("      R%i=3\n", FT32_TOC_CR_REGISTER(inst));
				if (condition)
				{
					if (inst & FT32_TOC_CALL) // CALL
					{
						// printf("      CALL\n");
						push(cur << 2);
						call(target);
					}
					else // JUMP
					{
						// printf("      JUMP\n");
						cur = target - 1;
					}
				}
				else
				{
					// printf("      NO COND\n");
				}
				// if (m_Register[13] == 13036 && (cur << 2 == 0x3214)) FT900EMU_DEBUG_BREAK();
				break;
			}
			case FT32_PATTERN_RETURN:
			{
				if (inst & FT32_RETURN_INTMASK)
					m_IRQ->returnInterrupt(); // RETURNI
				pop();
				return;
			}
			case FT32_PATTERN_EXA:
			case FT32_PATTERN_EXI:
			{
				printf("Not implemented EXA/EXI\n");
				FT900EMU_DEBUG_BREAK();
				break;
			}
			case FT32_PATTERN_ALUOP:
			case FT32_PATTERN_CMPOP:
			{
				// printf("    ALU/CMP\n");
				uint32_t r1v = m_Register[FT32_R1(inst)];
				uint32_t rimmv = FT32_RIMM(inst)
					? FT32_RIMM_IMMEDIATE(inst)
					: m_Register[FT32_RIMM_REG(inst)];
				uint32_t result;
				switch (FT32_AL(inst))
				{
					case FT32_ALU_ADD: result = r1v + rimmv; break;
					case FT32_ALU_ROR: result = ror(r1v, rimmv); break;
					case FT32_ALU_SUB: result = r1v - rimmv; break;
					case FT32_ALU_LDL: result = (r1v << 10) | (1023 & rimmv); break;
					case FT32_ALU_AND: result = r1v & rimmv; break;
					case FT32_ALU_OR: result = r1v | rimmv; break;
					case FT32_ALU_XOR: result = r1v ^ rimmv; break;
					case FT32_ALU_XNOR: result = ~(r1v ^ rimmv); break;
					case FT32_ALU_ASHL: result = r1v << rimmv; break;
					case FT32_ALU_LSHR: result = r1v >> rimmv; break;
					case FT32_ALU_ASHR: result = (int32_t)r1v >> rimmv; break;
					case FT32_ALU_BINS: result = bins(r1v, rimmv >> 10, FT32_BINS_WIDTH(rimmv), FT32_BINS_POSITION(rimmv)); break;
					case FT32_ALU_BEXTS: result = nsigned(FT32_BINS_WIDTH(rimmv), r1v >> FT32_BINS_POSITION(rimmv)); break;
					case FT32_ALU_BEXTU: result = nunsigned(FT32_BINS_WIDTH(rimmv), r1v >> FT32_BINS_POSITION(rimmv)); break;
					case FT32_ALU_FLIP: result = flip(r1v, rimmv); break;
					default: printf("Unknown ALU\n"); FT900EMU_DEBUG_BREAK(); break;
				}
				if (FT32_CMP(inst))
				{
					uint32_t dwmask = FT32_DW_MASK[FT32_DW(inst)];
					uint32_t dwsiz = FT32_DW_SIZEU[FT32_DW(inst)];
					int32_t zero = (0 == (result & dwmask));
					int32_t sign = 1 & (result >> dwsiz);
					int32_t ahi = 1 & (r1v >> dwsiz);
					int32_t bhi = 1 & (rimmv >> dwsiz);
					int32_t overflow = (sign != ahi) & (ahi == !bhi);
					int32_t carry;
					int32_t bit = (dwsiz + 1);
					uint64_t ra = r1v & dwmask;
					uint64_t rb = rimmv & dwmask;
					switch (FT32_AL(inst))
					{
						case FT32_ALU_ADD: carry = 1 & ((ra + rb) >> bit); break;
						case FT32_ALU_SUB: carry = 1 & ((ra - rb) >> bit); break;
						default:           carry = 0; break;
					}
					int above = (!carry & !zero);
					int greater = (sign == overflow) & !zero;
					int greatereq = (sign == overflow);
					m_Register[FT32_REG_CMP] =
						(above << 6)
						| (greater << 5)
						| (greatereq << 4)
						| (sign << 3)
						| (overflow << 2)
						| (carry << 1)
						| (zero << 0);
					// printf("      CMP: (%i, %i) R30 <- %#x (res %i)\n", r1v, rimmv, m_Register[FT32_REG_CMP], result & FT32_DW_MASK[FT32_DW(inst)]);
				}
				else
				{
					m_Register[FT32_RD(inst)] = result;
					// printf("      R%i <- %#x\n", FT32_RD(inst), result);
				}
				break;
			}
			case FT32_PATTERN_LDK:
			{
				// Load K20 to [RD]
				// printf("    LDK: R%i <- %i (dw: %i)\n", FT32_RD(inst), FT32_K20(inst), FT32_DW(inst));
				m_Register[FT32_RD(inst)] = FT32_K20(inst);
				break;
			}
			case FT32_PATTERN_LPM:
			{
				// printf("    LPM: %i\n", FT32_PA(inst));
				// Load program memory (int address) [PA] to RD
				m_Register[FT32_RD(inst)] = m_ProgramMemory[FT32_PA(inst)] & FT32_DW_MASK[FT32_DW(inst)];
				FT900EMU_DEBUG_BREAK();
				break;
			}
			case FT32_PATTERN_LPMI:
			{
				// printf("    LPMI: R%i; %u + %i\n", FT32_R1(inst), m_Register[FT32_R1(inst)], FT32_K8(inst));
				// Load program memory (byte address) [[R1] + K8] to RD
				m_Register[FT32_RD(inst)] = reinterpret_cast<uint32_t &>
					(reinterpret_cast<uint8_t *>(m_ProgramMemory)
						[m_Register[FT32_R1(inst)] + FT32_K8(inst)]) & FT32_DW_MASK[FT32_DW(inst)];
				// printf("      = %u\n", m_Register[FT32_RD(inst)]);
				// FT900EMU_DEBUG_BREAK();
				break;
			}
			case FT32_PATTERN_LDA:
			{
				// Load [AA] to RD
				uint32_t addr = FT32_AA(inst);
				switch (FT32_DW(inst))
				{
					case FT32_DW_8: m_Register[FT32_RD(inst)] = readMemU8(addr); break;
					case FT32_DW_16: m_Register[FT32_RD(inst)] = readMemU16(addr); break;
					case FT32_DW_32: m_Register[FT32_RD(inst)] = readMemU32(addr); break;
				}
				break;
			}
			case FT32_PATTERN_LDI:
			{
				// Load [[R1] + K8] to RD
				uint32_t addr = m_Register[FT32_R1(inst)] + FT32_K8(inst);
				switch (FT32_DW(inst))
				{
					case FT32_DW_8: m_Register[FT32_RD(inst)] = readMemU8(addr); break;
					case FT32_DW_16: m_Register[FT32_RD(inst)] = readMemU16(addr); break;
					case FT32_DW_32: m_Register[FT32_RD(inst)] = readMemU32(addr); break;
				}
				break;
			}
			case FT32_PATTERN_STA:
			{
				// Store [RD] to [AA]
				uint32_t addr = FT32_AA(inst);
				switch (FT32_DW(inst))
				{
					case FT32_DW_8: writeMemU8(addr, m_Register[FT32_RD(inst)]); break;
					case FT32_DW_16: writeMemU16(addr, m_Register[FT32_RD(inst)]); break;
					case FT32_DW_32: writeMemU32(addr, m_Register[FT32_RD(inst)]); break;
				}
				break;
			}
			case FT32_PATTERN_STI:
			{
				// Store [RD] to [R1] + K8
				uint32_t addr = m_Register[FT32_RD(inst)] + FT32_K8(inst);
				// printf("STI: (R%i) %u + %i <- R%i\n", FT32_RD(inst), m_Register[FT32_RD(inst)], FT32_K8(inst), FT32_R1(inst));
				switch (FT32_DW(inst))
				{
					case FT32_DW_8: writeMemU8(addr, m_Register[FT32_R1(inst)]); break;
					case FT32_DW_16: writeMemU16(addr, m_Register[FT32_R1(inst)]); break;
					case FT32_DW_32: writeMemU32(addr, m_Register[FT32_R1(inst)]); break;
				}
				break;
			}
			case FT32_PATTERN_PUSH:
			{
				push(m_Register[FT32_R1(inst)]);
				break;
			}
			case FT32_PATTERN_POP:
			{
				m_Register[FT32_RD(inst)] = pop();
				break;
			}
			case FT32_PATTERN_LINK:
			{
				push(m_Register[FT32_RD(inst)]);
				uint32_t stackp = m_Register[FT32_REG_STACK];
				m_Register[FT32_RD(inst)] = stackp;
				m_Register[FT32_REG_STACK] = (stackp - FT32_K16(inst)) & FT32_REG_STACK_MASK;
				break;
			}
			case FT32_PATTERN_UNLINK:
			{
				m_Register[FT32_REG_STACK] = m_Register[FT32_RD(inst)] & FT32_REG_STACK_MASK;
				m_Register[FT32_RD(inst)] = pop();
				break;
			}
			case FT32_PATTERN_FFUOP:
			{
				uint32_t dw = FT32_DW(inst);
				uint32_t r1v = m_Register[FT32_R1(inst)] & FT32_DW_MASK[dw];
				uint32_t rimmv = (FT32_RIMM(inst)
					? FT32_RIMM_IMMEDIATE(inst)
					: m_Register[FT32_RIMM_REG(inst)]) & FT32_DW_MASK[dw];
				switch (FT32_AL(inst))
				{
					case FT32_FFU_UDIV: // Unsigned division
					{
						m_Register[FT32_RD(inst)] = FT32_DW_RES_UNSIGNED(
							FT32_DW_UNSIGNED(r1v, dw) / FT32_DW_UNSIGNED(rimmv, dw), dw);
						break;
					}
					case FT32_FFU_UMOD: // Unsigned mod
					{
						m_Register[FT32_RD(inst)] = FT32_DW_RES_UNSIGNED(
							FT32_DW_UNSIGNED(r1v, dw) % FT32_DW_UNSIGNED(rimmv, dw), dw);
						break;
					}
					case FT32_FFU_DIV: // Signed division
					{
						if (r1v == 0x80000000u && rimmv == 0xFFFFFFFFu)
						{
							m_Register[FT32_RD(inst)] = 0x80000000u;
						}
						else
						{
							m_Register[FT32_RD(inst)] = FT32_DW_RES_SIGNED(
								FT32_DW_SIGNED(r1v, dw) / FT32_DW_SIGNED(rimmv, dw), dw);
						}
						break;
					}
					case FT32_FFU_MOD: // Signed mod
					{
						if (r1v == 0x80000000u && rimmv == 0xffffffffu)
						{
							m_Register[FT32_RD(inst)] = 0;
						}
						else
						{
							m_Register[FT32_RD(inst)] = FT32_DW_RES_SIGNED(
								FT32_DW_SIGNED(r1v, dw) % FT32_DW_SIGNED(rimmv, dw), dw);
						}
						break;
					} // FIXME: These won't work when going outside memory range :)
					case FT32_FFU_STRCMP: m_Register[FT32_RD(inst)] = strcmp((const char *)&m_Memory[r1v], (const char *)&m_Memory[rimmv]); break;
					case FT32_FFU_MEMCPY: memcpy(&m_Memory[FT32_RD(inst)], &m_Memory[r1v], rimmv); break;
					case FT32_FFU_STRLEN: m_Register[FT32_RD(inst)] = strlen((const char *)&m_Memory[r1v]); break;
					case FT32_FFU_MEMSET: memset(&m_Memory[FT32_RD(inst)], r1v, rimmv); break;
					case FT32_FFU_MUL: // Unsigned multiply
					{
						m_Register[FT32_RD(inst)] = FT32_DW_RES_UNSIGNED(
							FT32_DW_UNSIGNED(r1v, dw) * FT32_DW_UNSIGNED(rimmv, dw), dw);
						break;
					}
					case FT32_FFU_MULUH:
					{
						m_Register[FT32_RD(inst)] = FT32_DW_RES_UNSIGNED(
							(uint32_t)((uint64_t)FT32_DW_UNSIGNED(r1v, dw) * (uint64_t)FT32_DW_UNSIGNED(rimmv, dw) >> FT32_DW_SIZE[dw]), dw);
						break;
					}
					case FT32_FFU_STPCPY:
					{
						intptr_t res = (intptr_t)stpcpy((char *)&m_Memory[FT32_RD(inst)], (const char *)&m_Memory[r1v]);
						m_Memory[FT32_RD(inst)] = (uint32_t)(res - (intptr_t)(&m_Memory[0]));
						break;
					}
					case 0xBu:
					{
						printf("Not implemented 0xB\n");
						FT900EMU_DEBUG_BREAK();
						break;
					}
					case FT32_FFU_STREAMIN:
					case FT32_FFU_STREAMINI:
					case FT32_FFU_STREAMOUT:
					case FT32_FFU_STREAMOUTI:
					default:
					{
						printf("Not implemented STREAM\n");
						FT900EMU_DEBUG_BREAK();
						break;
					}
				}
				break;
			}
			default:
			{
				printf("Unknown PATTERN\n");
				FT900EMU_DEBUG_BREAK();
				break;
			}
		}
		++cur;
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace FT900EMU */

/* end of file */
