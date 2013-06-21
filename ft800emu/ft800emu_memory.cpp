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
#include "ft800emu_system_windows.h"

// System includes
#include <stdio.h>
#include <WinBase.h>

// Project includes
#include "vc.h"

// using namespace ...;

#define FT800EMU_ROM_FILE "../reference/ROM"
#define FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint8_t s_Ram[32 * 1024 * 1024]; // 32 MiB // TODO
static uint8_t s_Rom[FT800EMU_ROM_SIZE];

static LONG s_TouchedResolutionRegisters = 0;

void MemoryClass::begin()
{
	FILE *f;
	f = fopen(FT800EMU_ROM_FILE, "rb");
	if (!f) printf("Failed to open ROM file\n");
	else
	{
		size_t s = fread(s_Rom, 1, FT800EMU_ROM_SIZE, f);
		if (s != FT800EMU_ROM_SIZE) printf("Incomplete ROM file\n");
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

void MemoryClass::mcuWrite(size_t address, uint8_t data)
{
	switch (address & 0xFFFFFFFC)
	{
	case REG_PCLK:
	case REG_PCLK_POL:
	case REG_HCYCLE:
	case REG_HOFFSET:
	case REG_HSIZE:
	case REG_HSYNC0:
	case REG_HSYNC1:
	case REG_VCYCLE:
	case REG_VOFFSET:
	case REG_VSIZE:
	case REG_VSYNC0:
	case REG_VSYNC1:
		s_TouchedResolutionRegisters = 1;
		break;
	}
	s_Ram[address] = data;
}

uint8_t MemoryClass::mcuRead(size_t address)
{
	return s_Ram[address];
}

bool MemoryClass::untouchResolutionRegisters()
{
	return InterlockedExchange(&s_TouchedResolutionRegisters, 0);
}

} /* namespace FT800EMU */

/* end of file */
