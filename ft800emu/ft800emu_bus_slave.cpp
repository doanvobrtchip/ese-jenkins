/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

// #include <...>
#include "ft800emu_bus_slave.h"

// System includes
#include <stdio.h>

// Project includes
#include "ft8xxemu_system.h"
#include "ft800emu_memory.h"
#include "ft800emu_vc.h"

// using namespace ...;

namespace FT800EMU {

#define FT800EMU_DUMMY_WRITE 0

BusSlave::BusSlave(FT8XXEMU::System *system, Memory *memory) : m_System(system), m_Memory(memory)
{
	
}

BusSlave::~BusSlave()
{

}

void BusSlave::cs(bool cs)
{
	if (cs == m_CS)
		return;
	
	m_CS = cs;

	if (m_RWBufferStage)
	{
		// FTEMU_printf("Non-32bit write size, cursor %i\n", m_Cursor);

		while (m_RWBufferStage != 32)
		{
			uint8_t data = m_Memory->rawReadU8(m_Memory->getRam(), m_Cursor);
			m_RWBuffer |= (data << m_RWBufferStage);
			m_RWBufferStage += 8;
			++m_Cursor;
		}

		// FTEMU_printf("Write to %i, %i\n", (m_Cursor - 4), m_RWBuffer);
		m_Memory->mcuWriteU32(m_Cursor - 4, m_RWBuffer);

		m_RWBuffer = 0;
		m_RWBufferStage = 0;
	}

	// Reset state
	m_State = BusSlaveIdle;
	m_Cursor = 0;
	m_Stage = 0;
}

uint8_t BusSlave::transfer(uint8_t data)
{
	if (m_CS)
	{
		switch (m_State)
		{
		case BusSlaveIdle:
			switch ((data & 0xC0) >> 6)
			{
			case 0: // READ
				m_State = BusSlaveReadAddress;
				m_Cursor = (data & 0x3F);
				// FTEMU_printf("SPI/I2C: Begin read\n");
				break;
			case 1: // COMMAND
				m_State = BusSlaveNotImplemented;
				// FTEMU_printf("SPI/I2C: Command received\n");
				break;
			case 2: // WRITE
				m_State = BusSlaveWriteAddress;
				m_Cursor = (data & 0x3F);
				// FTEMU_printf("SPI/I2C: Begin write\n");
				break;
			case 3: // INVALID
				m_State = BusSlaveInvalidState;
				// FTEMU_printf("SPI/I2C: Invalid request\n");
				break;
			}
			break;
		case BusSlaveReadAddress:
			if (m_Stage < 2)
			{
				++m_Stage;
				m_Cursor <<= 8;
				m_Cursor |= data;
			}
			else
			{
				// Dummy byte
				m_State = BusSlaveRead;
				// FTEMU_printf("SPI/I2C: Address %d\n", m_Cursor);
				uint32_t aligned = m_Cursor & ~0x3;
				if (aligned != m_Cursor)
				{
					// FTEMU_printf("Non-aligned read\n");
					m_RWBuffer = m_Memory->mcuReadU32(aligned);
					uint32_t misaligned = m_Cursor - aligned;
					m_RWBuffer >>= (8 * misaligned);
				}
				else
				{
					m_RWBuffer = 0;
				}
			}
			break;
		case BusSlaveWriteAddress:
			if (m_Stage < 2)
			{
				++m_Stage;
				m_Cursor <<= 8;
				m_Cursor |= data;
			}
			else
			{
				// Dummy byte
				m_State = BusSlaveWrite;
				// FTEMU_printf("SPI/I2C: Address %d\n", m_Cursor);
				if (m_Cursor % 4)
				{
					// FTEMU_printf("Non-aligned write address %d\n", m_Cursor);
					m_RWBufferStage = m_Cursor % 4;
					// FTEMU_printf("align to address %d\n", m_Cursor - (m_Cursor % 4));
					m_RWBuffer = m_Memory->rawReadU32(m_Memory->getRam(), m_Cursor - (m_Cursor % 4));
					// FTEMU_printf("rwbuffer %d\n", m_RWBuffer);
					m_RWBufferStage *= 8;
					// mask away bytes that will be written
					// FTEMU_printf("> %i", m_Cursor);
					for (int i = m_RWBufferStage; i < (4 * 8); i += 8)
					{
						uint32_t mask = ~(0xFF << i);
						// FTEMU_printf(" | %#010x & %#010x (%i)", m_RWBuffer, mask, i);
						m_RWBuffer = m_RWBuffer & mask;
					}
					// FTEMU_printf("\n");
					// FTEMU_printf(">>> %#010x\n", m_RWBuffer);
				}
				else
				{
					m_RWBuffer = 0;
					m_RWBufferStage = 0;
				}
				m_WriteStartAddr = m_Cursor;
#if !FT800EMU_DUMMY_WRITE
				return transfer(data);
#endif
			}
			break;
		case BusSlaveRead:
			{
				// FTEMU_printf("Read\n");

				if (!(m_Cursor % 4))
				{
					m_RWBuffer = m_Memory->mcuReadU32(m_Cursor);
					// FTEMU_printf("Read U32 %d (%d)\n", m_Cursor, m_RWBuffer);
				}

				uint8_t result = m_RWBuffer & 0xFF;
				m_RWBuffer >>= 8;

				++m_Cursor;
				return result;
			}
			break;
		case BusSlaveWrite:
			{
				// m_Memory->mcuWrite(m_Cursor, data);

				m_RWBuffer |= (data << m_RWBufferStage);
				m_RWBufferStage += 8;

				if (m_RWBufferStage == 32)
				{
					// FTEMU_printf("write %i\n", (m_Cursor - 3));

					if ((m_Cursor - 3) % 4)
						FTEMU_warning("Non-aligned write %d", m_Cursor);

					m_Memory->mcuWriteU32(m_Cursor - 3, m_RWBuffer);
					m_RWBuffer = 0;
					m_RWBufferStage = 0;
				}

				if (m_Cursor == RAM_CMD + 4095 && m_WriteStartAddr >= RAM_CMD)
				{
					// FTEMU_printf("Cursor wrap to RAM_CMD\n");
					m_Cursor = RAM_CMD;
				}
#ifdef FT810EMU_MODE
				else if (m_Cursor == REG_CMDB_WRITE + 3)
				{
					// Cursor wrap to REG_CMDB_WRITE
					m_Cursor = REG_CMDB_WRITE;
				}
#endif
				else ++m_Cursor;
			}
			break;
		}
	}
	return 0;
}

} /* namespace FT800EMU */

/* end of file */
