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
#define FT800EMU_ROM_INDEX 0xC0000 //(RAM_DL - FT800EMU_ROM_SIZE)

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint8_t s_Ram[4 * 1024 * 1024]; // 4 MiB
static uint32_t s_DisplayList[FT800EMU_DISPLAY_LIST_SIZE];

__forceinline void MemoryClass::rawWriteU32(size_t address, uint32_t data)
{
	rawWriteU32(s_Ram, address, data);
}

__forceinline uint32_t MemoryClass::rawReadU32(size_t address)
{
	return rawReadU32(s_Ram, address);
}

void MemoryClass::begin()
{
	FILE *f;
	f = fopen(FT800EMU_ROM_FILE, "rb");
	if (!f) printf("Failed to open ROM file\n");
	else
	{
		size_t s = fread(&s_Ram[FT800EMU_ROM_INDEX], 1, FT800EMU_ROM_SIZE, f);
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
	rawWriteU32(REG_VOFFSET, 12);
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

const uint32_t *MemoryClass::getDisplayList()
{
	return s_DisplayList;
}

void MemoryClass::mcuWrite(size_t address, uint8_t data)
{
	// switches for 1 byte regs
	// least significant byte
	if (address % 4 == 0)
	{
		switch (address)
		{
		case REG_DLSWAP:
			if (data == SWAP_FRAME && s_Ram[REG_PCLK] == 0)
			{
				swapDisplayList();
				data = 0;
			}
			break;
		}
	}
	/*
	switch (address & 0xFFFFFFFC)
	{
	case REG_...:
		...
		break;
	}
	*/
	s_Ram[address] = data;
}

void MemoryClass::swapDisplayList()
{
	// Should this literally swap or is a copy good enough?
	memcpy(static_cast<void *>(s_DisplayList), static_cast<void *>(&s_Ram[RAM_DL]), sizeof(s_DisplayList) * sizeof(s_DisplayList[0]));
}

uint8_t MemoryClass::mcuRead(size_t address)
{
	return s_Ram[address];
}

} /* namespace FT800EMU */

/* end of file */
