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

__forceinline void MemoryClass::rawWriteU32(size_t address, uint32_t data)
{
	rawWriteU32(s_Ram, address, data);
}

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

	rawWriteU32(REG_ID, 0x7C);
	rawWriteU32(REG_FRAMES, 0); // Frame counter - is this updated before or after frame render?
	rawWriteU32(REG_CLOCK, 0);
	rawWriteU32(REG_FREQUENCY, 48000000);
	rawWriteU32(REG_RENDERMODE, 0);
	rawWriteU32(REG_SNAPY, 0);
	rawWriteU32(REG_SNAPSHOT, 0);
	rawWriteU32(REG_CPURESET, 0);
	rawWriteU32(REG_TAP_CRC, 0);
	rawWriteU32(REG_TAP_MASK, ~0);
	rawWriteU32(REG_HCYCLE, 548);
	rawWriteU32(REG_HOFFSET, 43);
	rawWriteU32(REG_HSIZE, 480);
	rawWriteU32(REG_HSYNC0, 0);
	rawWriteU32(REG_HSYNC1, 41);
	rawWriteU32(REG_VCYCLE, 292);
	rawWriteU32(REG_VOFFSET, 0);
	rawWriteU32(REG_VSIZE, 272);
	rawWriteU32(REG_VSYNC0, 0);
	rawWriteU32(REG_VSYNC1, 10);
	rawWriteU32(REG_DLSWAP, 0);
	rawWriteU32(REG_ROTATE, 0);
	rawWriteU32(REG_OUTBITS, 0x1B6);
	rawWriteU32(REG_DITHER, 1);
	rawWriteU32(REG_SWIZZLE, 0);
	rawWriteU32(REG_CSPREAD, 1);
	rawWriteU32(REG_PCLK_POL, 0);	
	rawWriteU32(REG_PCLK, 0);
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
	/*switch (address & 0xFFFFFFFC)
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
	}*/
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
