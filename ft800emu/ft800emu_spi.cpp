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
static uint8_t *s_Ram = NULL;
static uint8_t *s_RamCursor = NULL;

void SPIClass::begin()
{
	s_Ram = Memory.getRam();
}

void SPIClass::end()
{
	s_Ram = NULL;
}

void SPIClass::csLow(bool low)
{
	s_CSLow = low;
	s_RamCursor = NULL;
}

void SPIClass::csHigh(bool high)
{
	csLow(!high);
}

void SPIClass::emuSetAddress(size_t address)
{
	s_RamCursor = &s_Ram[address];
}

void SPIClass::emuWriteByte(uint8_t data)
{
	*s_RamCursor = data;
	++s_RamCursor;
}

uint8_t SPIClass::emuReadByte()
{
	uint8_t result = *s_RamCursor;
	++s_RamCursor;
	return result;
}

} /* namespace FT800EMU */

/* end of file */
