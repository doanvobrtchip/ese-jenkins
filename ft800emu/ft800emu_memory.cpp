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
#include <stdio.h>

// Project includes

// using namespace ...;

#define D_FT800EMU_ROM_FILE "../reference/ROM"
#define D_FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint8_t s_Ram[32 * 1024 * 1024]; // 32 MiB // TODO
static uint8_t s_Rom[D_FT800EMU_ROM_SIZE];

void MemoryClass::begin()
{
	FILE *f;
	f = fopen(D_FT800EMU_ROM_FILE, "rb");
	if (!f) printf("Failed to open ROM file\n");
	else
	{
		size_t s = fread(s_Rom, 1, D_FT800EMU_ROM_SIZE, f);
		if (s != D_FT800EMU_ROM_SIZE) printf("Incomplete ROM file\n");
		else printf("Loaded ROM file\n");
		if (fclose(f)) printf("Error closing ROM file\n");
	}
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
