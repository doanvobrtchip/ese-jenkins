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
#include <ft800emu_inttypes.h>
#include <vc.h>

void wr32(size_t address, uint32_t value)
{
	/*FT800EMU::SPII2C.csLow();
	FT800EMU::SPII2C.mcuSetAddress(address);
	FT800EMU::SPII2C.mcuWriteByte(value & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 8) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 16) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 24) & 0xFF);
	FT800EMU::SPII2C.csHigh();*/

	digitalWrite(9, LOW);

	SPI.transfer((2 << 6) | ((address >> 16) & 0x3F));
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	SPI.transfer(0x00);

	SPI.transfer(value & 0xFF);
	SPI.transfer((value >> 8) & 0xFF);
	SPI.transfer((value >> 16) & 0xFF);
	SPI.transfer((value >> 24) & 0xFF);

	digitalWrite(9, HIGH);
}

static size_t dli;
static void dl(uint32_t cmd)
{
	wr32(dli, cmd);
	dli += 4;
}

inline uint32_t transformvalue(double d)
{
	int result = (int)(d * 256.0);
	if (result < 0) result = (0x10000 + result) | 0x10000;
	return ((uint32_t)result) & 0x1FFFF;
}

void setup()
{
	dli = RAM_DL;
	// dl(CLEAR_COLOR_RGB(0, 64, 128));
	dl(CLEAR(1, 1, 1)); // clear screen

	dl(BEGIN(LINES));
	dl(LINE_WIDTH(160));
	dl(COLOR_RGB(105, 238, 100));
	dl(VERTEX2F(2367, 58));
	dl(VERTEX2F(8223, 409));
	dl(END());

	dl(TAG(25));

	dl(BEGIN(BITMAPS)); // start drawing bitmaps
	dl(BITMAP_TRANSFORM_A(transformvalue(cos(0.5))));
	dl(BITMAP_TRANSFORM_B(transformvalue(-sin(0.5))));
	dl(BITMAP_TRANSFORM_D(transformvalue(sin(0.5))));
	dl(BITMAP_TRANSFORM_E(transformvalue(cos(0.5))));
	dl(VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
	dl(BITMAP_TRANSFORM_A(256));
	dl(BITMAP_TRANSFORM_B(0));
	dl(BITMAP_TRANSFORM_D(0));
	dl(BITMAP_TRANSFORM_E(256));
	dl(VERTEX2II(244, 110, 31, 'T')); // ascii T
	dl(VERTEX2II(270, 110, 31, 'D')); // ascii D
	dl(VERTEX2II(299, 110, 31, 'I')); // ascii I
	dl(END());
	dl(COLOR_RGB(160, 22, 22)); // change color to red
	dl(POINT_SIZE(320)); // set point size to 20 pixels in radius
	dl(BEGIN(POINTS)); // start drawing points
	dl(VERTEX2II(192, 133, 0, 0)); // red point
	dl(END());



	dl(COLOR_A(255));




	/*// ** SMALL POINTS X **
	dl(COLOR_RGB(255, 255, 255));
	dl(BEGIN(POINTS));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 8; ++x)
		{
			dl(POINT_SIZE(x));
			dl(VERTEX2F(x * 32 + y, y * 16, 0, 0));
		}	
	}
	dl(END());
	// ** SMALL POINTS X ***/

	// ** SMALL POINTS Y **
	/*dl(COLOR_RGB(255, 255, 255));
	dl(BEGIN(POINTS));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 8; ++x)
		{
			dl(POINT_SIZE(x));
			dl(VERTEX2F(y * 16, x * 32 + y));
		}	
	}
	dl(END());*/
	// ** SMALL POINTS Y **

	dl(TAG(50));

	// ** RECT TEST **
	dl(COLOR_A(128));

	dl(LINE_WIDTH(160));
	dl(BEGIN(RECTS));
	dl(VERTEX2F(162, 160));
	dl(VERTEX2F(164+16+24, 324));
	dl(END());

	dl(COLOR_RGB(0, 128, 255));
	dl(POINT_SIZE(28));
	dl(BEGIN(POINTS));
	//dl(VERTEX2F(160, 324));
	dl(END());
	// ** RECT TEST **


	dl(LINE_WIDTH(0));
	dl(BEGIN(RECTS));

	/*
	dl(COLOR_RGB(255, 128, 0));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F(x * 16, y * 16));
			dl(VERTEX2F(x * 16 + x, y * 16 + y));
		}
	}
	
	dl(COLOR_RGB(0, 128, 255));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F(16 * 17 + x * 16 + x, y * 16 + y));
			dl(VERTEX2F(16 * 17 + x * 16 + 16, y * 16 + 16));
		}
	}*/

	/*
	dl(COLOR_RGB(0, 255, 128));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F(x * 16, y * 32));
			dl(VERTEX2F(x * 16 + x, y * 32 + y + 16));
		}
	}

	dl(COLOR_RGB(128, 0, 255));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F((16 * 17) + x * 16 + x, y * 32 + y));
			dl(VERTEX2F((16 * 17) + x * 16 + 16, y * 32 + 32));
		}
	}*/
/*
	

	dl(COLOR_RGB(0, 255, 128));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F(x * 32, y * 16));
			dl(VERTEX2F(x * 32 + x + 16, y * 16 + y));
		}
	}

	dl(COLOR_RGB(128, 0, 255));
	for (int y = 0; y <= 16; ++y)
	{
		for (int x = 0; x <= 16; ++x)
		{
			dl(VERTEX2F(x * 32 + x, (16 * 17) + y * 16 + y));
			dl(VERTEX2F(x * 32 + 16 + 16, (16 * 17) + y * 16 + 16));
		}
	}*/


	dl(END());



 // LINE_STRIP

dl(COLOR_A(128));
dl( COLOR_RGB(255, 168, 64) );
dl( BEGIN(LINE_STRIP) );
dl(LINE_WIDTH(160));
dl( VERTEX2F(5 * 16, 5 * 16) );
dl( VERTEX2F(50 * 16, 30 * 16) );
dl( VERTEX2F(63 * 16, 50 * 16) );
dl( VERTEX2F(73 * 16, 100 * 16) );
dl( VERTEX2F(73 * 16, 200 * 16) );
dl( VERTEX2F(150 * 16, 200 * 16) );
dl( VERTEX2F(200 * 16, 150 * 16) );
dl( VERTEX2F(150 * 16, 100 * 16) );
dl( VERTEX2F(160 * 16, 50 * 16) );
dl( VERTEX2F(240 * 16, 70 * 16) );
dl( VERTEX2F(250 * 16, 100 * 16) );
dl( VERTEX2F(350 * 16, 200 * 16) );
	dl(END());
/*
	dl(COLOR_A(128));
dl( COLOR_RGB(0, 128, 255) );
dl( BEGIN(LINES) );
dl(LINE_WIDTH(5));
dl( VERTEX2F(5 * 16, 5 * 16) );
dl( VERTEX2F(50 * 16, 30 * 16) );
dl( VERTEX2F(63 * 16, 50 * 16) );
dl( VERTEX2F(73 * 16, 100 * 16) );

dl( VERTEX2F(200 * 16, 150 * 16) );
dl( VERTEX2F(150 * 16, 100 * 16) );
dl( VERTEX2F(160 * 16, 50 * 16) );
dl( VERTEX2F(240 * 16, 70 * 16) );
dl( VERTEX2F(250 * 16, 100 * 16) );
dl( VERTEX2F(350 * 16, 200 * 16) );
	dl(END());

/*
dl( BEGIN(LINES) );

dl( VERTEX2F(73 * 16, 200 * 16) );
dl( VERTEX2F(150 * 16, 200 * 16) );

dl( VERTEX2F(200 * 16, 200 * 16) );
dl( VERTEX2F(300 * 16, 201 * 16) );

	dl(END());*/

dl( BEGIN(RECTS) );

dl( VERTEX2F(400 * 16, 73 * 16) );
dl( VERTEX2F(400 * 16, 150 * 16) );

dl( VERTEX2F(400 * 16, 170 * 16) );
dl( VERTEX2F(401 * 16, 250 * 16) );

	dl(END());


/*
dl( COLOR_RGB(128, 0, 0) );
dl( POINT_SIZE(5 * 16) );
dl( BEGIN(POINTS) );
dl( VERTEX2F(30 * 16,17 * 16) );
dl( COLOR_RGB(0, 128, 0) );
dl( POINT_SIZE(8 * 16) );
dl( VERTEX2F(90 * 16, 17 * 16) );
dl( COLOR_RGB(0, 0, 128) );
dl( POINT_SIZE(10 * 16) );
dl( VERTEX2F(30 * 16, 51 * 16) );
dl( COLOR_RGB(128, 128, 0) );
dl( POINT_SIZE(13 * 16) );
dl( VERTEX2F(90 * 16, 51 * 16) );
dl(END());*/
/*
dl(SCISSOR_XY(40, 20)); // Scissor rectangle top left at (40, 20)
dl(SCISSOR_SIZE(40, 40)); // Scissor rectangle is 40 x 40 pixels
dl(CLEAR_COLOR_RGB(255, 255, 0)); // Clear to yellow
dl(CLEAR(1, 1, 1));

dl( BEGIN(POINTS) );
dl( COLOR_RGB(255, 0, 128) );
dl( POINT_SIZE(5 * 16) );
for (int i = 0; i < 500; ++i)
{
	dl(VERTEX2F(random(4096 * 2), random(4096)));
}
dl(END());
*//*
dl( COLOR_RGB(255, 255, 255) );
dl( STENCIL_OP(INCR, INCR) );
dl( POINT_SIZE(760) );
dl( BEGIN(POINTS) );
dl( VERTEX2II(50, 60, 0, 0) );
dl( VERTEX2II(110, 60, 0, 0) );
dl( STENCIL_FUNC(EQUAL, 2, 255) );
dl( COLOR_RGB(100, 0, 0) );
dl( VERTEX2II(80, 60, 0, 0) );*/

	dl(DISPLAY()); // display the image

	wr32(REG_DLSWAP, SWAP_FRAME);
	wr32(REG_PCLK, 5);
}

void loop()
{
	delay(10); // let's be nice on the cpu today :)
	// wr32(REG_DLSWAP, SWAP_FRAME);
}
