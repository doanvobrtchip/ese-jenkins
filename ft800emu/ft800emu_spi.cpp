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

void SPIClass::begin()
{
	
}

void SPIClass::end()
{
	
}

void SPIClass::csLow(bool low)
{
	s_CSLow = low;
}

void SPIClass::csHigh(bool high)
{
	s_CSLow = !high;
}

void SPIClass::writeAddress(unsigned int address)
{
	
}

void SPIClass::writeByte(uint8_t data)
{
	
}

void SPIClass::readAddress(unsigned int address)
{
	
}

uint8_t SPIClass::readByte()
{
	
}

} /* namespace FT800EMU */

/* end of file */
