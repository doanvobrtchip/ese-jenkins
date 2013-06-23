/**
 * app.cpp
 * $Id$
 * \file app.cpp
 * \brief app.cpp
 * \date 2013-06-21 21:51GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <SPI.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <vc.h>

void wr32(size_t address, uint32_t value)
{
	FT800EMU::SPII2C.csLow();
	FT800EMU::SPII2C.mcuSetAddress(address);
	FT800EMU::SPII2C.mcuWriteByte(value & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 8) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 16) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 24) & 0xFF);
	FT800EMU::SPII2C.csHigh();
}

static size_t dli;
static void dl(uint32_t cmd)
{
	wr32(dli, cmd);
	dli += 4;
}

void setup()
{
	dli = RAM_DL;
	// dl(CLEAR_COLOR_RGB(0, 64, 128));
	dl(CLEAR(1, 1, 1)); // clear screen
	dl(BEGIN(BITMAPS)); // start drawing bitmaps
	dl(VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
	dl(VERTEX2II(244, 110, 31, 'T')); // ascii T
	dl(VERTEX2II(270, 110, 31, 'D')); // ascii D
	dl(VERTEX2II(299, 110, 31, 'I')); // ascii I
	dl(END());
	dl(COLOR_RGB(160, 22, 22)); // change color to red
	dl(POINT_SIZE(320)); // set point size to 20 pixels in radius
	dl(BEGIN(POINTS)); // start drawing points
	dl(VERTEX2II(192, 133, 0, 0)); // red point
	dl(END());
	dl(DISPLAY()); // display the image
	wr32(REG_DLSWAP, SWAP_FRAME);
	wr32(REG_PCLK, 5);
}

void loop()
{
	delay(10); // let's be nice on the cpu today :)
	// wr32(REG_DLSWAP, SWAP_FRAME);
}
