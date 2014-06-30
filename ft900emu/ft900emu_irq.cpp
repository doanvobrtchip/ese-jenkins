/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_irq.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft900emu_intrin.h"

// using namespace ...;

#define FT900EMU_IRQ_CONTROL 8
#define FT900EMU_IRQ_CONTROL_GLOBALINTMASK 0x80000000u
#define FT900EMU_IRQ_CONTROL_NESTEDINT     0x00000080u
#define FT900EMU_IRQ_CONTROL_NESTEDDEPTH   0x0000000Fu

#define INT_INTERRUPT_NB  32
#define INT_PRIORITY_MASK 0x0F

#define CTRL_IS_GLOBAL_MASK (m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_GLOBALINTMASK)

namespace FT900EMU {

IRQ::IRQ() : m_Lock(0), m_CurrentDepth(0), /*m_CurrentInt(0),*/ m_InterruptCheck(false), m_BuiltinInterrupts(0), m_Interrupts(0), m_FT32(NULL)
{
	// printf(F9ED "+ IRQ :: Init" F9EE);

	_mm_prefetch((const char *)&m_InterruptCheck, _MM_HINT_T0);
	memset(m_Register, 0, FT900EMU_MEMORY_IRQ_COUNT * sizeof(uint32_t));

	softReset();
}

void IRQ::softReset()
{
	for (int i = 0; i < INT_INTERRUPT_NB; ++i)
		m_Register8[i] = i;
	m_Register[FT900EMU_IRQ_CONTROL] = 0x80000000;
}

inline bool IRQ::nestedInterrupt()
{
	return (m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_NESTEDINT) == FT900EMU_IRQ_CONTROL_NESTEDINT;
}

inline uint32_t IRQ::nestedDepth()
{
	return (m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_NESTEDDEPTH) == FT900EMU_IRQ_CONTROL_NESTEDDEPTH;
}

// io_a is addr / 4 (read per 4 bytes)
uint32_t IRQ::ioRd32(uint32_t io_a, uint32_t io_be)
{
	if (io_a < FT900EMU_MEMORY_IRQ_BEGIN32)
	{
		return FT32IO::ioRd32(io_a, io_be);
	}

	uint32_t idx = io_a - FT900EMU_MEMORY_IRQ_START;
	printf(F9ED "+ IRQ :: Read register U32 %i (%#x): %#x" F9EE, idx, idx << 2, m_Register[idx]);
	return m_Register[idx] & io_be;
}

void IRQ::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	if (io_a < FT900EMU_MEMORY_IRQ_BEGIN32)
	{
		FT32IO::ioWr32(io_a, io_be, io_dout);
		return;
	}

	const uint32_t idx = io_a - FT900EMU_MEMORY_IRQ_START;
	uint32_t v = (io_dout & io_be) | (m_Register[idx] & ~io_be);
	const uint32_t diffmask = v ^ m_Register[idx];
	printf(F9ED "+ IRQ :: Write register U32 %i (%#x): %#x [diffmask = %#x]" F9EE, idx, idx << 2, v, diffmask);

	// WRITE
	m_Register[idx] = v;

	if (idx == FT900EMU_IRQ_CONTROL)
	{
		if (CTRL_IS_GLOBAL_MASK)
		{
			lock();
			// Clear interrupts with non-0 priority
			uint32_t interrupts = m_Interrupts;
			for (int ir = 0; ir < INT_INTERRUPT_NB; ++ir)
			{
				if (m_Register8[ir] & INT_PRIORITY_MASK)
				{
					interrupts &= ~(1 << ir);
				}
			}
			m_Interrupts = interrupts;
			unlock();
			m_InterruptCheck = (interrupts || m_BuiltinInterrupts);
		}
	}
}

uint8_t IRQ::ioRd8(uint32_t io_a)
{
	if (io_a >= FT900EMU_MEMORY_IRQ_END8)
	{
		return FT32IO::ioRd8(io_a);
	}

	const uint32_t idx = io_a - FT900EMU_MEMORY_IRQ_START8;
	printf(F9ED "+ IRQ :: Read register U8 %i (%#x): %#x" F9EE, idx, idx << 2, (uint32_t)m_Register8[idx]);
	return m_Register8[idx];
}

void IRQ::ioWr8(uint32_t io_a, uint8_t io_dout)
{
	if (io_a >= FT900EMU_MEMORY_IRQ_END8)
	{
		FT32IO::ioWr8(io_a, io_dout);
	}

	const uint32_t idx = io_a - FT900EMU_MEMORY_IRQ_START8;
	printf(F9ED "+ IRQ :: Write register U8 %i (%#x): %#x" F9EE, idx, idx << 2, (uint32_t)io_dout);

	// WRITE
	m_Register8[idx] = io_dout;
}

void IRQ::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_IRQ_START;
	to = FT900EMU_MEMORY_IRQ_START + FT900EMU_MEMORY_IRQ_COUNT;
}

// Called by a module to trigger an interrupt
void IRQ::interrupt(uint32_t irq)
{
	if (irq >= FT900EMU_BUILTIN_IRQ_INDEX)
	{
		lock();
		m_BuiltinInterrupts |= (1 << (irq - FT900EMU_BUILTIN_IRQ_INDEX));
		unlock();
		// Turn on next check
		m_InterruptCheck = true;
		m_FT32->wake();
	}
	else if ((!CTRL_IS_GLOBAL_MASK) || ((m_Register8[irq] & INT_PRIORITY_MASK) == 0)) // Only allow if not masked or priority 0
	{
		lock();
		m_Interrupts |= (1 << irq);
		unlock();
		// Turn on next check
		m_InterruptCheck = true;
		m_FT32->wake();
	}
	else
	{
		printf(F9ED "Interrupt requested, but global interrupt mask on (%#x)" F9EE, m_Register[FT900EMU_IRQ_CONTROL]);
	}
}

// Called by FT32 to check if there's an interrupt, ~0 if none
uint32_t IRQ::nextInterrupt()
{
	// Don't check again until a change is notified
	m_InterruptCheck = false;

	// printf(F9ED "+ Check interrupt +" F9EE);

	// Check if depth is ok
	if ((nestedInterrupt() && m_CurrentDepth <= nestedDepth())
		|| m_CurrentDepth == 0 || m_BuiltinInterrupts) // Always allow emulator interrupts
	{
		uint32_t irq = ~0;
		lock();
		uint32_t builtinint = m_BuiltinInterrupts;
		if (builtinint)
		{
			printf(F9ED "+ Builtin interrupt +" F9EE);

			int idx = 0;
			while (!(builtinint & (1 << idx))) ++idx;
			builtinint &= ~(1 << idx);
			m_BuiltinInterrupts = builtinint;
			irq = FT900EMU_BUILTIN_IRQ_INDEX + idx;
			m_Priority.push_back(0);
			++m_CurrentDepth;
		}
		else
		{
			uint32_t ints = m_Interrupts;
			if (ints)
			{
				// printf(F9ED "+ Device interrupt %#x +" F9EE, ints);

				uint8_t priority = m_Priority.size() ? m_Priority.back() : INT_INTERRUPT_NB;
				// Find highest priority interrupt
				uint32_t idx = 0;
				uint8_t primax = priority; // Need higher priority than current
				uint32_t idxmax = ~0;
				uint32_t lim = (ints & 0xFFFF0000) ? 32 : 16; // Optimize away the last 16 bits
				if (!(ints & 0x0000FFFF)) idx += 16; // Optimize away the first 16 bits
				else if (!(ints & 0x000000FF)) idx += 8; // Or optimize away the first 8 bits
				for (; idx < lim; ++idx)
				{
					if ((ints >> idx) & 1)
					{
						uint8_t pri = m_Register8[idx] & INT_PRIORITY_MASK;
						if (pri < primax)
						{
							primax = pri;
							idxmax = idx;
						}
					}
				}
				// printf("interrupt %i\n", idxmax);
				if (~idxmax)
				{
					m_Interrupts &= ~(1 << idxmax);
					m_Priority.push_back(primax);
					++m_CurrentDepth;
					// printf("interrupt %i\n", idxmax);
					irq = idxmax;
				}
			}
		}
		unlock();
		return irq;
	}
	return ~0;
}

// Called by FT32 when interrupt call returns
void IRQ::returnInterrupt()
{
	// printf("Return interrupt\n");

	if (m_CurrentDepth == 0)
	{
		printf(F9ED "Invalid RETURNI outside interrupt" F9EE);
	}
	else
	{
		--m_CurrentDepth;
		m_Priority.pop_back();
	}

	// Check to see if there's a new one waiting
	m_InterruptCheck = true;
}

void IRQ::lock()
{
	SDL_AtomicLock(&m_Lock);
}

void IRQ::unlock()
{
	SDL_AtomicUnlock(&m_Lock);
}

} /* namespace FT900EMU */

/* end of file */
