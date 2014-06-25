/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft900emu_irq.h"

// System includes
#include <stdio.h>
#include <string.h>
#include <sched.h>

// Project includes

// using namespace ...;

#define FT900EMU_IRQ_CONTROL 8
#define FT900EMU_IRQ_CONTROL_GLOBALINTMASK 0x80000000u
#define FT900EMU_IRQ_CONTROL_NESTEDINT     0x00000080u
#define FT900EMU_IRQ_CONTROL_NESTEDDEPTH   0x0000000Fu

namespace FT900EMU {

IRQ::IRQ() : m_Lock(0), m_CurrentDepth(0), m_CurrentInt(0), m_InterruptCheck(false)
{
	printf(F9ED "+ IRQ :: Init" F9EE);
	softReset();
}

void IRQ::softReset()
{
	memset(m_Register, 0, FT900EMU_MEMORY_IRQ_COUNT * sizeof(uint32_t));
}

inline bool IRQ::globalInterruptMask()
{
	return m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_GLOBALINTMASK;
}

inline bool IRQ::nestedInterrupt()
{
	return m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_NESTEDINT;
}

inline uint32_t IRQ::nestedDepth()
{
	return m_Register[FT900EMU_IRQ_CONTROL] & FT900EMU_IRQ_CONTROL_NESTEDDEPTH;
}

// io_a is addr / 4 (read per 4 bytes)
uint32_t IRQ::ioRd32(uint32_t io_a, uint32_t io_be)
{
	uint idx = io_a - FT900EMU_MEMORY_IRQ_START;
	printf(F9ED "+ IRQ :: Read register %i (%#x): %#x" F9EE, idx, idx << 2, m_Register[idx]);
	return m_Register[idx] & io_be;
}

void IRQ::ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
{
	const uint32_t idx = io_a - FT900EMU_MEMORY_IRQ_START;
	uint32_t v = (io_dout & io_be) | (m_Register[idx] & ~io_be);
	const uint32_t diffmask = v ^ m_Register[idx];
	printf(F9ED "+ IRQ :: Write register %i (%#x): %#x [diffmask = %#x]" F9EE, idx, idx << 2, v, diffmask);

	// WRITE
	m_Register[idx] = v;

	if (idx == FT900EMU_IRQ_CONTROL)
	{
		if (!globalInterruptMask())
		{
			// Clear interrupts
			lock();
			m_Waiting.clear();
			unlock();
			m_InterruptCheck = false;
		}
	}
}

void IRQ::ioGetRange(uint32_t &from, uint32_t &to)
{
	from = FT900EMU_MEMORY_IRQ_START;
	to = FT900EMU_MEMORY_IRQ_START + FT900EMU_MEMORY_IRQ_COUNT;
}

// Called by a module to trigger an interrupt
void IRQ::interrupt(uint32_t irq)
{
	if (globalInterruptMask())
	{
		// Enter sorted into list
		lock();
		// TODO: Implement priority
		if (globalInterruptMask()) // Must recheck in case of threaded change since lock
		{
			m_Waiting.push_back(irq);
		}
		unlock();
		// Turn on next check
		m_InterruptCheck = true;
	}
	else
	{
		printf(F9ED "Interrupt requested, but global interrupt mask off" F9EE);
	}
}

// Called by FT32 to check if there's an interrupt, ~0 if none
uint32_t IRQ::nextInterruptInternal()
{
	// Don't check again until a change is notified
	m_InterruptCheck = false;

	printf(F9ED "+ Check interrupt +" F9EE);

	// Check if depth is ok
	if ((nestedInterrupt() && m_CurrentDepth <= nestedDepth())
		|| m_CurrentDepth == 0) //
	{
		uint32_t irq = ~0;
		lock();
		// Pop from list
		if (!m_Waiting.empty())
		{
			irq = m_Waiting.front();
			// TODO: Check if priority vs currently running interrupt is ok
			m_Waiting.pop_front();
			++m_CurrentDepth;
		}
		unlock();
		return irq;
	}
	return ~0;
}

// Called by FT32 when interrupt call returns
void IRQ::returnInterrupt()
{
	if (m_CurrentDepth == 0)
	{
		printf(F9ED "Invalid RETURNI outside interrupt" F9EE);
	}
	else
	{
		--m_CurrentDepth;
	}

	// Check to see if there's a new one waiting
	m_InterruptCheck = true;
}

void IRQ::lock()
{
	while (__sync_lock_test_and_set(&m_Lock, 1)) sched_yield();
}

void IRQ::unlock()
{
	__sync_lock_release(&m_Lock);
}

} /* namespace FT900EMU */

/* end of file */
