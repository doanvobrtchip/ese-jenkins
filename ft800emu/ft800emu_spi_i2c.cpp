/**
 * SPII2CClass
 * $Id$
 * \file ft800emu_spi_i2c.cpp
 * \brief SPII2CClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_spi_i2c.h"

// System includes
#include <stdio.h>

// Project includes
#include "ft800emu_memory.h"
#include "vc.h"

// using namespace ...;

namespace FT800EMU {

SPII2CClass SPII2C;

static bool s_CSLow = false;

static uint32_t s_RWBuffer;
static uint32_t s_RWBufferStage;

enum SPII2CState
{
	SPII2CIdle, 
	SPII2CNotImplemented, 
	SPII2CInvalidState, 
	SPII2CReadAddress, 
	SPII2CWriteAddress, 
	SPII2CRead, 
	SPII2CWrite, 
};
static SPII2CState s_State;
static uint32_t s_Cursor;
static int s_Stage = 0;

void SPII2CClass::begin()
{
	s_State = SPII2CIdle;
}

void SPII2CClass::end()
{

}

void SPII2CClass::csLow(bool low)
{
	s_CSLow = low;

	if (s_RWBufferStage)
	{
		// printf("Non-32bit write size, cursor %i\n", s_Cursor);

		while (s_RWBufferStage != 32)
		{
			uint8_t data = Memory.rawReadU8(Memory.getRam(), s_Cursor);
			s_RWBuffer |= (data << s_RWBufferStage);
			s_RWBufferStage += 8;
			++s_Cursor;
		}

		// printf("Write to %i, %i\n", (s_Cursor - 4), s_RWBuffer);
		Memory.mcuWriteU32(s_Cursor - 4, s_RWBuffer);

		s_RWBuffer = 0;
		s_RWBufferStage = 0;
	}

	// Reset state
	s_State = SPII2CIdle;
	s_Cursor = 0;
	s_Stage = 0;
}

void SPII2CClass::csHigh(bool high)
{
	csLow(!high);
}

uint8_t SPII2CClass::transfer(uint8_t data)
{
	if (s_CSLow)
	{
		switch (s_State)
		{
		case SPII2CIdle:
			switch ((data & 0xC0) >> 6)
			{
			case 0: // READ
				s_State = SPII2CReadAddress;
				s_Cursor = (data & 0x3F);
				// printf("SPI/I2C: Begin read\n");
				break;
			case 1: // COMMAND
				s_State = SPII2CNotImplemented;
				// printf("SPI/I2C: Command received\n");
				break;
			case 2: // WRITE
				s_State = SPII2CWriteAddress;
				s_Cursor = (data & 0x3F);
				// printf("SPI/I2C: Begin write\n");
				break;
			case 3: // INVALID
				s_State = SPII2CInvalidState;
				// printf("SPI/I2C: Invalid request\n");
				break;
			}
			break;
		case SPII2CReadAddress:
			if (s_Stage < 2)
			{
				++s_Stage;
				s_Cursor <<= 8;
				s_Cursor |= data;
			}
			else
			{
				// Dummy byte
				s_State = SPII2CRead;
				// printf("SPI/I2C: Address %d\n", s_Cursor);
				uint32_t aligned = s_Cursor & ~0x3;
				if (aligned != s_Cursor)
				{
					// printf("Non-aligned read\n");
					s_RWBuffer = Memory.mcuReadU32(aligned);
					uint32_t misaligned = s_Cursor - aligned;
					s_RWBuffer >>= (8 * misaligned);
				}
				else
				{					
					s_RWBuffer = 0;
				}
			}
			break;
		case SPII2CWriteAddress:
			if (s_Stage < 2)
			{
				++s_Stage;
				s_Cursor <<= 8;
				s_Cursor |= data;
			}
			else
			{
				// Dummy byte
				s_State = SPII2CWrite;
				// printf("SPI/I2C: Address %d\n", s_Cursor);
				if (s_Cursor % 4)
				{
					// printf("Non-aligned write address %d\n", s_Cursor);
					s_RWBufferStage = s_Cursor % 4;
					// printf("align to address %d\n", s_Cursor - (s_Cursor % 4));
					s_RWBuffer = Memory.rawReadU32(Memory.getRam(), s_Cursor - (s_Cursor % 4));
					// printf("rwbuffer %d\n", s_RWBuffer);
					s_RWBufferStage *= 8;
				}
				else
				{
					s_RWBuffer = 0;
					s_RWBufferStage = 0;
				}
			}
			break;
		case SPII2CRead:
			{
				// printf("Read\n");
				
				if (!(s_Cursor % 4))
				{
					s_RWBuffer = Memory.mcuReadU32(s_Cursor);
					// printf("Read U32 %d (%d)\n", s_Cursor, s_RWBuffer);
				}
				
				uint8_t result = s_RWBuffer & 0xFF;
				s_RWBuffer >>= 8;
				
				++s_Cursor;
				return result;
			}
			break;
		case SPII2CWrite:
			{
				// Memory.mcuWrite(s_Cursor, data);
				
				s_RWBuffer |= (data << s_RWBufferStage);
				s_RWBufferStage += 8;

				if (s_RWBufferStage == 32)
				{
					// printf("write %i\n", (s_Cursor - 3));

					if ((s_Cursor - 3) % 4)
						printf("Non-aligned write %d\n", s_Cursor);

					Memory.mcuWriteU32(s_Cursor - 3, s_RWBuffer);
					s_RWBuffer = 0;
					s_RWBufferStage = 0;
				}

				if (s_Cursor == RAM_CMD + 4095) 
				{
					// printf("Cursor wrap to RAM_CMD\n");
					s_Cursor = RAM_CMD;
				}
				else ++s_Cursor;
			}
			break;
		}
	}
	return 0;
}

} /* namespace FT800EMU */

/* end of file */
