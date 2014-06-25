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

// io_a is in 32bit indexed address (8bit addr >> 2)
uint32_t FT32IO::ioRd32(uint32_t io_a, uint32_t io_be)
{
	// If only 8bit access is implemented
	uint32_t io_a_b = io_a << 2;
	uint32_t res = 0;
	if (io_be & 0x000000FFu)
		res |= (uint32_t)ioRd8(io_a_b);
	if (io_be & 0x0000FF00u)
		res |= (uint32_t)ioRd8(io_a_b + 1) << 8;
	if (io_be & 0x00FF0000u)
		res |= (uint32_t)ioRd8(io_a_b + 2) << 16;
	if (io_be & 0xFF000000u)
		res |= (uint32_t)ioRd8(io_a_b + 3) << 24;
	return res;
}

void FT32IO::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	// If only 8bit access is implemented
	uint32_t io_a_b = io_a << 2;
	if (io_be & 0x000000FFu)
		ioWr8(io_a_b, io_dout & 0xFFu);
	if (io_be & 0x0000FF00u)
		ioWr8(io_a_b + 1, (io_dout >> 8) & 0xFFu);
	if (io_be & 0x00FF0000u)
		ioWr8(io_a_b + 2, (io_dout >> 16) & 0xFFu);
	if (io_be & 0xFF000000u)
		ioWr8(io_a_b + 3, (io_dout >> 24) & 0xFFu);
}

// io_a is in 8bit indexed address
uint8_t FT32IO::ioRd8(uint32_t io_a)
{
	// If only 32bit access is implemented
	uint32_t b = io_a % 4;
	b <<= 3;
	uint8_t res = (ioRd32(io_a >> 2, (0xFFu << b)) >> b) & 0xFFu;
	return res;
}

void FT32IO::ioWr8(uint32_t io_a, uint8_t io_dout)
{
	// If only 32bit access is implemented
	uint32_t b = io_a % 4;
	b <<= 3;
	ioWr32(io_a >> 2, (0xFFu << b), (uint32_t)io_dout << b);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

FT32::FT32(IRQ *irq) : m_IRQ(irq),
	m_IONb(0)
{
	io(irq);
	memset(m_Memory, 0, FT900EMU_FT32_MEMORY_SIZE);
	m_ProgramMemory[FT900EMU_FT32_PROGRAM_MEMORY_COUNT - 1] = 0x00300023;
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

inline FT32IO *FT32::getIO(uint32_t io_a_32)
{
	for (int i = 0; i < m_IONb; ++i)
	{
		if (m_IOFrom[i] <= io_a_32 && io_a_32 < m_IOTo[i])
		{
			return m_IO[i];
		}
	}
	return NULL;
}

uint32_t FT32::ioRd32(uint32_t io_a, uint32_t io_be)
{
	FT32IO *io = getIO(io_a);
	if (io)
		return io->ioRd32(io_a, io_be);

	printf("Read IO 32 not handled: %#x (mask: %x)\n", io_a << 2, io_be);
	FT900EMU_DEBUG_BREAK();
	return 0xDEADBEEF;
}

void FT32::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	FT32IO *io = getIO(io_a);
	if (io)
		return io->ioWr32(io_a, io_be, io_dout);

	printf("Write IO 32 not handled: %#x = %u (mask: %x)\n", io_a << 2, io_dout, io_be);
	FT900EMU_DEBUG_BREAK();
}

uint8_t FT32::ioRd8(uint32_t io_a)
{
	FT32IO *io = getIO(io_a >> 2);
	if (io)
		return io->ioRd8(io_a);

	printf("Read IO 8 not handled: %#x\n", io_a);
	FT900EMU_DEBUG_BREAK();
	return 0xCC;
}

void FT32::ioWr8(uint32_t io_a, uint8_t io_dout)
{
	FT32IO *io = getIO(io_a >> 2);
	if (io)
		return io->ioWr8(io_a, io_dout);

	printf("Write IO 8 not handled: %#x = %u\n", io_a, (uint32_t)io_dout);
	FT900EMU_DEBUG_BREAK();
}

////////////////////////////////////////////////////////////////////////

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
			return ioRd32(addr >> 2, 0xFFFFFFFFu);
		}
		else
		{
			printf("Not implemented UNALIGNED READ 32\n");
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE void FT32::writeMemU32(uint32_t addr, uint32_t value)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		reinterpret_cast<uint32_t &>(m_Memory[addr]) = value;
	}
	else
	{
		uint32_t b = addr % 4;
		if (!b)
		{
			ioWr32(addr >> 2, 0xFFFFFFFFu, value);
		}
		else
		{
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
			return (ioRd32(addr >> 2, 0xFFFFu << b) >> b) & 0xFFFFu;
		}
		else
		{
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
			ioWr32(addr >> 2, (0xFFFFu << b), (uint32_t)value << b);
		}
		else
		{
			printf("Not implemented UNALIGNED WRITE 16\n");
			FT900EMU_DEBUG_BREAK();
		}
	}
}

FTEMU_FORCE_INLINE uint8_t FT32::readMemU8(uint32_t addr)
{
	if (addr < FT900EMU_FT32_MEMORY_SIZE)
	{
		//printf("######## READ8: [%#x] <- %#x (%i) ########\n", addr, (uint32_t)m_Memory[addr], (uint32_t)m_Memory[addr]);
		return m_Memory[addr];
	}
	else
	{
		return ioRd8(addr);
		//printf("READU8: %#x\n", addr);
		uint32_t b = addr % 4;
		b <<= 3;
		uint8_t res = (ioRd32(addr >> 2, (0xFFu << b)) >> b) & 0xFFu;
		//printf("  U8: %#x\n", res);
		return res;
	}
}

static bool inprintf = false;
static int wrc = 0;
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
		//printf("WRITEU8: %#x = %#x\n", addr, (uint32_t)value);
		ioWr32(addr >> 2, (0xFFu << b), (uint32_t)value << b);
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
	uint32_t cur = pma;
	while (m_Running)
	{
		// Check for IRQ
		uint32_t nirq = m_IRQ->nextInterrupt();
		if (~nirq) call(nirq + 2);

		uint32_t inst = m_ProgramMemory[cur & FT32_PM_CUR_MASK];
		//printf("%x    CUR: %u; INST: %#010x\n", cur << 2, cur, inst);

		switch (inst & FT32_PATTERNMASK)
		{
			case FT32_PATTERN_TOC:
			case FT32_PATTERN_TOCI:
			{
				uint32_t target = (inst & FT32_TOC_INDIRECT)
					? (m_Register[FT32_R2(inst)] >> 2)
					: FT32_TOC_PA(inst);
				bool condition = FT32_TOC_CR_UNCONDITIONAL(inst)
					|| (((m_Register[FT32_TOC_CR_REGISTER(inst)] >> FT32_TOC_CB(inst)) & 1) == FT32_TOC_CV(inst));
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
					 //printf("      NO COND\n");
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
				//printf("> ALU/CMP.%c\n", FT32_DW(inst) == FT32_DW_8 ? 'b' : (FT32_DW(inst) ==  FT32_DW_16 ? 's' : 'l'));
				uint32_t r1v = m_Register[FT32_R1(inst)];
				uint32_t rimmv = FT32_RIMM(inst)
					? FT32_RIMM_IMMEDIATE(inst)
					: m_Register[FT32_RIMM_REG(inst)];
				uint32_t result;
				switch (FT32_AL(inst))
				{
					case FT32_ALU_ADD: /*printf("  | ADD\n");*/ result = r1v + rimmv; break;
					case FT32_ALU_ROR: /*printf("  | ROR\n");*/ result = ror(r1v, rimmv); break;
					case FT32_ALU_SUB: /*printf("  | SUB\n");*/ result = r1v - rimmv; break;
					case FT32_ALU_LDL: /*printf("  | LDL\n");*/ result = (r1v << 10) | (1023 & rimmv); break;
					case FT32_ALU_AND: /*printf("  | AND\n");*/ result = r1v & rimmv; break;
					case FT32_ALU_OR: /*printf("  | OR\n");*/ result = r1v | rimmv; break;
					case FT32_ALU_XOR: /*printf("  | XOR\n");*/ result = r1v ^ rimmv; break;
					case FT32_ALU_XNOR: /*printf("  | XNOR\n");*/ result = ~(r1v ^ rimmv); break;
					case FT32_ALU_ASHL: /*printf("  | ASHL\n");*/ result = r1v << rimmv; break;
					case FT32_ALU_LSHR: /*printf("  | LSHR\n");*/ result = r1v >> rimmv; break;
					case FT32_ALU_ASHR: /*printf("  | ASHR\n");*/ result = (int32_t)r1v >> rimmv; break;
					case FT32_ALU_BINS: /*printf("  | BINS\n");*/ result = bins(r1v, rimmv >> 10, FT32_BINS_WIDTH(rimmv), FT32_BINS_POSITION(rimmv)); break;
					case FT32_ALU_BEXTS: /*printf("  | BEXTS\n");*/ result = nsigned(FT32_BINS_WIDTH(rimmv), r1v >> FT32_BINS_POSITION(rimmv)); /*printf("BEXTS: %i; %i -> %i\n", r1v, rimmv, result);*/ break;
					case FT32_ALU_BEXTU: /*printf("  | BEXTU\n");*/ result = nunsigned(FT32_BINS_WIDTH(rimmv), r1v >> FT32_BINS_POSITION(rimmv)); /*printf("BEXTU: %i; %i -> %u\n", r1v, rimmv, result);*/ break;
					case FT32_ALU_FLIP: /*printf("  | FLIP\n");*/ result = flip(r1v, rimmv); break;
					default: printf("Unknown ALU\n"); FT900EMU_DEBUG_BREAK(); break;
				}
				//printf("  | %i = %i ; %i\n", result, r1v, rimmv);
				if (FT32_CMP(inst))
				{
					//printf("  | CMP\n");
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
					//printf("  | ALU\n");
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
				// printf("(%#x) LDA: addr %u // inst %#x\n", cur << 2, addr, inst);
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
				//printf("(%#x) LDI: (R%i) %u + %i (%u, %#x)\n", cur << 2, FT32_R1(inst), m_Register[FT32_R1(inst)], FT32_K8(inst), addr, addr);
				//if (FT32_DW(inst) == FT32_DW_32) printf("-4 = %#x, +4 = %#x\n", readMemU32(addr - 4), readMemU32(addr + 4));
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
				//printf("(%#x) STI: (R%i) %u + %i (%u) <- R%i (%i)\n", cur << 2, FT32_RD(inst), m_Register[FT32_RD(inst)], FT32_K8(inst), addr, FT32_R1(inst), m_Register[FT32_R1(inst)]);
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
				//printf("PUSH R%i (was %#x)\n", FT32_R1(inst), m_Register[FT32_R1(inst)]);
				push(m_Register[FT32_R1(inst)]);
				break;
			}
			case FT32_PATTERN_POP:
			{
				m_Register[FT32_RD(inst)] = pop();
				//printf("POP R%i (now %#x)\n", FT32_RD(inst), m_Register[FT32_RD(inst)]);
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
				// printf("> FFUOP.%c\n", dw == FT32_DW_8 ? 'b' : (dw ==  FT32_DW_16 ? 's' : 'l'));
				uint32_t r1v = m_Register[FT32_R1(inst)];
				uint32_t rimmv = (FT32_RIMM(inst)
					? FT32_RIMM_IMMEDIATE(inst)
					: m_Register[FT32_RIMM_REG(inst)]);
				switch (FT32_AL(inst))
				{
					case FT32_FFU_UDIV: // Unsigned division
					{
						// printf("  | UDIV\n");
						m_Register[FT32_RD(inst)] = r1v / rimmv;
						break;
					}
					case FT32_FFU_UMOD: // Unsigned mod
					{
						// printf("  | UMOD\n");
						m_Register[FT32_RD(inst)] = r1v % rimmv;
						break;
					}
					case FT32_FFU_DIV: // Signed division
					{
						// printf("  | DIV\n");
						if (r1v == 0x80000000u && rimmv == 0xFFFFFFFFu)
						{
							m_Register[FT32_RD(inst)] = 0x80000000u;
						}
						else
						{
							m_Register[FT32_RD(inst)] = (uint32_t)((int32_t)r1v / (int32_t)rimmv);
						}
						break;
					}
					case FT32_FFU_MOD: // Signed mod
					{
						// printf("  | MOD\n");
						if (r1v == 0x80000000u && rimmv == 0xffffffffu)
						{
							m_Register[FT32_RD(inst)] = 0;
						}
						else
						{
							m_Register[FT32_RD(inst)] = (uint32_t)((int32_t)r1v % (int32_t)rimmv);
						}
						break;
					} // FIXME:  :)
					case FT32_FFU_STRCMP:
					{
						 /*printf("  | STRCMP\n");*/
						// FIXME: This won't work when going outside memory range
						m_Register[FT32_RD(inst)] = strcmp((const char *)&m_Memory[r1v], (const char *)&m_Memory[rimmv]);
						break;
					}
					case FT32_FFU_MEMCPY:
					{
						/*printf("  | MEMCPY\n");*/
						// FIXME: This won't work when going outside memory range
						memcpy(&m_Memory[m_Register[FT32_RD(inst)]], &m_Memory[r1v], rimmv);
						break;
					}
					case FT32_FFU_STRLEN:
					{
						/*printf("  | STRLEN\n");*/
						// FIXME: This won't work when going outside memory range
						m_Register[FT32_RD(inst)] = strlen((const char *)&m_Memory[r1v]);
						/* printf("**** STRLEN %#x (%c): %i\n", r1v, m_Memory[r1v], m_Register[FT32_RD(inst)]); */
						break;
					}
					case FT32_FFU_MEMSET:
					{
						/*printf("  | MEMSET\n");*/
						// FIXME: This won't work when going outside memory range
						memset(&m_Memory[m_Register[FT32_RD(inst)]], r1v, rimmv);
						break;
					}
					case FT32_FFU_MUL: // Unsigned? multiply
					{
						//printf("  | MUL\n");
						m_Register[FT32_RD(inst)] = r1v * rimmv;
						break;
					}
					case FT32_FFU_MULUH:
					{
						//printf("  | MULUH\n");
						m_Register[FT32_RD(inst)] = (uint32_t)(((uint64_t)r1v * (uint64_t)rimmv) >> FT32_DW_SIZE[dw]);
						break;
					}
					case FT32_FFU_STPCPY:
					{
						//printf("  | STPCPY\n");
						// FIXME: This won't work when going outside memory range
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
					{
						uint32_t rdv = m_Register[FT32_RD(inst)];
						switch (dw)
						{
						case FT32_DW_8:
							for (uint32_t i = 0; i < rimmv; ++i)
								writeMemU8(rdv + i, readMemU8(r1v));
							break;
						default:
							printf("Not implemented STREAMIN dw: %i\n", dw);
							FT900EMU_DEBUG_BREAK();
							break;
						}
						break;
					}
					case FT32_FFU_STREAMOUT:
					{
						uint32_t rdv = m_Register[FT32_RD(inst)];
						switch (dw)
						{
						case FT32_DW_8:
							for (uint32_t i = 0; i < rimmv; ++i)
								writeMemU8(rdv, readMemU8(r1v + i));
							break;
						default:
							printf("Not implemented STREAMOUT dw: %i\n", dw);
							FT900EMU_DEBUG_BREAK();
							break;
						}
						break;
					}
					case FT32_FFU_STREAMINI:
					{
						printf("Not implemented STREAMINI\n");
						FT900EMU_DEBUG_BREAK();
						break;
					}
					case FT32_FFU_STREAMOUTI:
					{
						printf("Not implemented STREAMOUTI\n");
						FT900EMU_DEBUG_BREAK();
						break;
					}
					default:
					{
						printf("Not implemented FFUOP\n");
						FT900EMU_DEBUG_BREAK();
						break;
					}
				}
				//printf("  | %i = %i ; %i\n", m_Register[FT32_RD(inst)], r1v, rimmv);
				// strict support test ->
				switch (FT32_AL(inst))
				{
				case FT32_FFU_MEMSET:
				case FT32_FFU_STRLEN:
				case FT32_FFU_STRCMP:
				case FT32_FFU_STPCPY:
				case FT32_FFU_MEMCPY:
					// DW is ignored
					break;
				case FT32_FFU_UMOD:
				case FT32_FFU_UDIV:
				case FT32_FFU_DIV:
				case FT32_FFU_MOD:
				case FT32_FFU_MUL:
				case FT32_FFU_MULUH:
					// Only allow these ops on 32 bit
					if (dw == FT32_DW_32) break;
					goto defaultlabel;
				default:
				defaultlabel:
					printf("Not implemented FFUOP (%i, dw %i)\n", FT32_AL(inst), dw);
					FT900EMU_DEBUG_BREAK();
					break;
				}
				// strict test <-
				break;
			}
			default:
			{
				printf("Unknown PATTERN\n");
				FT900EMU_DEBUG_BREAK();
				break;
			}
		}
		//printf("-> RD: R%i: %i (%#x)\n", FT32_RD(inst), m_Register[FT32_RD(inst)], m_Register[FT32_RD(inst)]);
		++cur;
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace FT900EMU */

/* end of file */
