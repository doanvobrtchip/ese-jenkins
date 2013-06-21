/**
 * SPIClass
 * $Id$
 * \file ft800emu_spi.cpp
 * \brief SPIClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_spi.h"

// System includes

// Project includes
#include "ft800emu_memory.h"

// using namespace ...;

namespace FT800EMU {

SPIClass SPI;

static bool s_CSLow = false;
static size_t s_Cursor = 0;

void SPIClass::begin()
{

}

void SPIClass::end()
{

}

void SPIClass::csLow(bool low)
{
	s_CSLow = low;
	s_Cursor = 0;
}

void SPIClass::csHigh(bool high)
{
	csLow(!high);
}

void SPIClass::mcuSetAddress(size_t address)
{
	s_Cursor = address;
}

void SPIClass::mcuWriteByte(uint8_t data)
{
	Memory.mcuWrite(s_Cursor, data);
	++s_Cursor;
}

uint8_t SPIClass::mcuReadByte()
{
	uint8_t result = Memory.mcuRead(s_Cursor);
	++s_Cursor;
	return result;
}

} /* namespace FT800EMU */

/* end of file */
