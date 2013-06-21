/**
 * FT800SPIClass
 * $Id$
 * \file ft800emu_ft800_spi.cpp
 * \brief FT800SPIClass
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_ft800_spi.h"

// System includes
#include <stdlib.h>
#include <stdio.h>

// Project includes
#include "ft800emu_system.h"

// using namespace ...;

namespace FT800EMU {

FT800SPIClass FT800SPI;

// RAM
static uint8_t s_Ram[8 * 1024 * 1024]; // 8 MiB

void FT800SPIClass::begin()
{
	
}

void FT800SPIClass::end()
{

}

uint8_t *FT800SPIClass::getRam()
{
	return s_Ram;
}


} /* namespace FT800EMU */

/* end of file */
