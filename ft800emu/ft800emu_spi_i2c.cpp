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

// Project includes
#include "ft800emu_memory.h"

// using namespace ...;

namespace FT800EMU {

SPII2CClass SPI;

static bool s_CSLow = false;
static size_t s_Cursor = 0;

void SPII2CClass::begin()
{

}

void SPII2CClass::end()
{

}

void SPII2CClass::csLow(bool low)
{
	s_CSLow = low;
	s_Cursor = 0;
}

void SPII2CClass::csHigh(bool high)
{
	csLow(!high);
}

void SPII2CClass::mcuSetAddress(size_t address)
{
	s_Cursor = address;
}

void SPII2CClass::mcuWriteByte(uint8_t data)
{
	Memory.mcuWrite(s_Cursor, data);
	++s_Cursor;
}

uint8_t SPII2CClass::mcuReadByte()
{
	uint8_t result = Memory.mcuRead(s_Cursor);
	++s_Cursor;
	return result;
}

} /* namespace FT800EMU */

/* end of file */
