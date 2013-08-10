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
#include <string.h>

// Project includes
#include "vc.h"

// using namespace ...;

#define FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB
#define FT800EMU_ROM_INDEX 0xC0000 //(RAM_DL - FT800EMU_ROM_SIZE)

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint8_t s_Ram[4 * 1024 * 1024]; // 4 MiB
static uint32_t s_DisplayListA[FT800EMU_DISPLAY_LIST_SIZE];
static uint32_t s_DisplayListB[FT800EMU_DISPLAY_LIST_SIZE];
static uint32_t *s_DisplayListActive = s_DisplayListA;
static uint32_t *s_DisplayListFree = s_DisplayListB;

static int s_DirectSwapCount;

int MemoryClass::getDirectSwapCount()
{
	return s_DirectSwapCount;
}

template<typename T>
FT800EMU_FORCE_INLINE void MemoryClass::actionWrite(const size_t address, T &data)
{
	// switches for 1 byte regs
	// least significant byte
	if (address % 4 == 0)
	{
		switch (address)
		{
		case REG_DLSWAP:
			if (data == DLSWAP_FRAME && s_Ram[REG_PCLK] == 0)
			{
				// Direct swap
				swapDisplayList();
				data = 0;
				++s_DirectSwapCount;
			}
			break;
		}
	}
}

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU32(size_t address, uint32_t data)
{
    actionWrite(address, data);
	rawWriteU32(s_Ram, address, data);
}

FT800EMU_FORCE_INLINE uint32_t MemoryClass::rawReadU32(size_t address)
{
	return rawReadU32(s_Ram, address);
}

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU16(size_t address, uint16_t data)
{
    actionWrite(address, data);
	rawWriteU16(s_Ram, address, data);
}

FT800EMU_FORCE_INLINE uint16_t MemoryClass::rawReadU16(size_t address)
{
	return rawReadU16(s_Ram, address);
}

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU8(size_t address, uint8_t data)
{
    actionWrite(address, data);
	rawWriteU8(s_Ram, address, data);
}

FT800EMU_FORCE_INLINE uint8_t MemoryClass::rawReadU8(size_t address)
{
	return rawReadU8(s_Ram, address);
}

static const uint8_t rom[FT800EMU_ROM_SIZE] = {
#include "rom.h"
};

void MemoryClass::begin()
{
    memcpy(&s_Ram[FT800EMU_ROM_INDEX], rom, sizeof(rom));

	s_DirectSwapCount = 0;

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
	rawWriteU32(REG_TAG_X, 0);
	rawWriteU32(REG_TAG_Y, 0);
	rawWriteU32(REG_TOUCH_RZTHRESH, 0xFFFF);
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
	return s_DisplayListActive;
}

void MemoryClass::mcuWrite(size_t address, uint8_t data)
{

	rawWriteU8(address, data);
}

void MemoryClass::swapDisplayList()
{
	memcpy(static_cast<void *>(s_DisplayListFree), static_cast<void *>(&s_Ram[RAM_DL]), sizeof(s_DisplayListA));
	memcpy(static_cast<void *>(&s_Ram[RAM_DL]), static_cast<void *>(s_DisplayListActive), sizeof(s_DisplayListA));
	uint32_t *active = s_DisplayListFree;
	s_DisplayListFree = s_DisplayListActive;
	s_DisplayListActive = active;
}

uint8_t MemoryClass::mcuRead(size_t address)
{
	return s_Ram[address];
}

void MemoryClass::coprocessorWrite(size_t address, uint8_t data)
{
	s_Ram[address] = data;
}

uint8_t MemoryClass::coprocessorRead(size_t address)
{
	return s_Ram[address];
}

void MemoryClass::coprocessorWriteU32(size_t address, uint32_t data)
{
	rawWriteU32(address, data);
}

uint32_t MemoryClass::coprocessorReadU32(size_t address)
{
	return rawReadU32(address);
}

void MemoryClass::coprocessorWriteU16(size_t address, uint16_t data)
{
	rawWriteU16(address, data);
}

uint16_t MemoryClass::coprocessorReadU16(size_t address)
{
	return rawReadU16(address);
}

void MemoryClass::coprocessorWriteU8(size_t address, uint8_t data)
{
	rawWriteU8(address, data);
}

uint8_t MemoryClass::coprocessorReadU8(size_t address)
{
	return rawReadU8(address);
}

} /* namespace FT800EMU */

/* end of file */
