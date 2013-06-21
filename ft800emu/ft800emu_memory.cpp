/**
 * MemoryClass
 * $Id$
 * \file ft800emu_memory.cpp
 * \brief MemoryClass
 * \date 2013-06-21 21:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_memory.h"

// System includes

// Project includes

// using namespace ...;

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint8_t s_Ram[32 * 1024 * 1024]; // 32 MiB // TODO

void MemoryClass::begin()
{
	
}

void MemoryClass::end()
{
	
}

uint8_t *MemoryClass::getRam()
{
	return s_Ram;
}

} /* namespace FT800EMU */

/* end of file */
