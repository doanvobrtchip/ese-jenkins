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
#include <vc.h>

uint8_t s_PCM[] = {
#include "pcm.h"
};

uint8_t s_uLaw[] = {
#include "ulaw.h"
};

uint8_t s_ADPCM[] = {
#include "adpcm.h"
};

void wr8(size_t address, uint8_t value)
{
	digitalWrite(9, LOW);

	SPI.transfer((2 << 6) | ((address >> 16) & 0x3F));
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	// SPI.transfer(0x00);

	SPI.transfer(value);

	digitalWrite(9, HIGH);
}

uint8_t rd8(size_t address)
{
	digitalWrite(9, LOW);

	SPI.transfer((address >> 16) & 0x3F);
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	SPI.transfer(0x00);

	uint8_t value;
	value = SPI.transfer(0);

	digitalWrite(9, HIGH);

	return value;
}

uint32_t rd32(size_t address)
{
	digitalWrite(9, LOW);

	SPI.transfer((address >> 16) & 0x3F);
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	SPI.transfer(0x00);

	uint32_t value;
	value = SPI.transfer(0);
	value |= SPI.transfer(0) << 8;
	value |= SPI.transfer(0) << 16;
	value |= SPI.transfer(0) << 24;

	digitalWrite(9, HIGH);
	return value;
}

void wr16(size_t address, uint16_t value)
{
	digitalWrite(9, LOW);

	SPI.transfer((2 << 6) | ((address >> 16) & 0x3F));
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	// SPI.transfer(0x00);

	SPI.transfer(value & 0xFF);
	SPI.transfer((value >> 8) & 0xFF);

	digitalWrite(9, HIGH);
}

void wr32(size_t address, uint32_t value)
{
	digitalWrite(9, LOW);

	SPI.transfer((2 << 6) | ((address >> 16) & 0x3F));
	SPI.transfer((address >> 8) & 0xFF);
	SPI.transfer(address & 0xFF);
	// SPI.transfer(0x00);

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

int s_Memory[64 * 1024];
void writeMemory(int index, int value)
{
	s_Memory[index] = value;
}

int *s_MemoryPtr;
void writeMemoryPtrInit()
{
	s_MemoryPtr = new int[64 * 1024];
}

void writeMemoryPtr(int index, int value)
{
	s_MemoryPtr[index] = value;
}

class MemoryWriter
{
public:
	void init();
	void write(int index, int value);
	int *m_Memory;
};
class MemoryWriterDirect
{
public:
	void init();
	void write(int index, int value);
	int m_MemoryTest[64 * 1024];
	int m_Memory[64 * 1024];
};

void MemoryWriter::init()
{
	m_Memory = new int[64 * 1024];
}

void MemoryWriter::write(int index, int value)
{
	m_Memory[index] = value;
}

void MemoryWriterDirect::init()
{

}

void MemoryWriterDirect::write(int index, int value)
{
	m_Memory[index] = value;
}

class MemoryWriterStatic
{
public:
	static void writeMemory(int index, int value);
	static void writeMemoryPtr(int index, int value);
};

void MemoryWriterStatic::writeMemory(int index, int value)
{
	s_Memory[index] = value;
}

void MemoryWriterStatic::writeMemoryPtr(int index, int value)
{
	s_MemoryPtr[index] = value;
}

void setup()
{
	// test per byte access
	for (uint8_t i = 0; i < 32; ++i)
	{
		wr8(i, i);
	}
	for (uint8_t i = 0; i < 32; ++i)
	{
		uint8_t v = rd8(i);
		if (v == i) printf("OK (%i)\n", v);
		else printf("FAIL (%i)\n", v);
	}
	// test per byte access backwards
	for (uint8_t i = 63; i >= 32; --i)
	{
		wr8(i, i);
	}
	for (uint8_t i = 63; i >= 32; --i)
	{
		uint8_t v = rd8(i);
		if (v == i) printf("OK (%i)\n", v);
		else printf("FAIL (%i)\n", v);
	}
	// test per byte access
	for (uint8_t i = 0; i < 32; ++i)
	{
		wr8(i, i * 2);
	}
	for (uint8_t i = 0; i < 32; ++i)
	{
		uint8_t v = rd8(i);
		if (v == i * 2) printf("OK (%i)\n", v);
		else printf("FAIL (%i)\n", v);
	}
	// test per byte access backwards
	for (uint8_t i = 63; i >= 32; --i)
	{
		wr8(i, i * 2);
	}
	for (uint8_t i = 63; i >= 32; --i)
	{
		uint8_t v = rd8(i);
		if (v == i * 2) printf("OK (%i)\n", v);
		else printf("FAIL (%i)\n", v);
	}




	// AUDIO TEST

	wr8(REG_VOL_SOUND, 64);
	wr16(REG_SOUND, ((68 << 8) + 0x46));
	wr8(REG_PLAY, 1);

	for (int i = 0; i < sizeof(s_ADPCM); ++i)
	{
		wr8(i, s_ADPCM[i]);
	}
	wr32(REG_PLAYBACK_START, 0);
	wr32(REG_PLAYBACK_LENGTH, sizeof(s_ADPCM));
	wr32(REG_PLAYBACK_FREQ, 44100);
	wr32(REG_PLAYBACK_FORMAT, 2);
	wr32(REG_PLAYBACK_LOOP, 0);
	wr32(REG_PLAYBACK_PLAY, 1);
	wr32(REG_VOL_PB, 255);



	dli = RAM_DL;

	dl(CLEAR_COLOR_RGB(0, 64, 128));
	dl(CLEAR(1, 1, 1)); // clear screen

	dl(TAG(200));

	dl(BEGIN(LINES));
	dl(LINE_WIDTH(8));
	dl(VERTEX2F(4191, 2524));
	dl(VERTEX2F(4185, 2548));
	//dl(VERTEX2F(2367, 58));
	//dl(VERTEX2F(8223, 409));

	/*
+  2a3: 1f000003   BEGIN prim=3
+  2a4: 0e000008   LINE_WIDTH width=8
+  2a5: 48e18c3f   VERTEX2F x=4547 y=3135
+  2a6: 48d58c39   VERTEX2F x=4523 y=3129

+  2a7: 48e10b3d   VERTEX2F x=4546 y=2877
+  2a8: 48d50b44   VERTEX2F x=4522 y=2884

+  2a9: 489f8a5c   VERTEX2F x=4415 y=2652
+  2aa: 48968a6e   VERTEX2F x=4397 y=2670

+  2ab: 482f89dc   VERTEX2F x=4191 y=2524
+  2ac: 482c89f4   VERTEX2F x=4185 y=2548

+  2ad: 47ae89dd   VERTEX2F x=3933 y=2525
+  2ae: 47b209f5   VERTEX2F x=3940 y=2549
+  2af: 473e0a60   VERTEX2F x=3708 y=2656
+  2b0: 47470a72   VERTEX2F x=3726 y=2674
+  2b1: 46fe0b40   VERTEX2F x=3580 y=2880
+  2b2: 470a0b46   VERTEX2F x=3604 y=2886
+  2b3: 46fe8c42   VERTEX2F x=3581 y=3138
+  2b4: 470a8c3b   VERTEX2F x=3605 y=3131 */
	dl(END());

	dl(TAG(0));

/*
	dl(BEGIN(LINES));
	dl(LINE_WIDTH(160));
	dl(COLOR_RGB(105, 238, 100));
	dl(VERTEX2F(2367, 58));
	dl(VERTEX2F(8223, 409));
	dl(END());

	dl(TAG(25));
*/
	dl(TAG(10));
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
	dl(TAG(50));
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
/*
	dl(TAG(50));*/
/*
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
*/
/*
	dl(LINE_WIDTH(0));
	dl(BEGIN(RECTS));
*/
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

  dl(TAG(42));
  dl(COLOR_RGB(255, 128, 0));
  dl(BEGIN(RECTS));
  dl(VERTEX2II(10, 10, 0, 0));
  dl(VERTEX2II(50, 50, 0, 0));
  dl(END());
  dl(TAG(0));

/*
	dl(END());
	*


*/
 // LINE_STRIP
/*
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
	dl(END());*/
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
/*
dl( BEGIN(RECTS) );

dl( VERTEX2F(400 * 16, 73 * 16) );
dl( VERTEX2F(400 * 16, 150 * 16) );

dl( VERTEX2F(400 * 16, 170 * 16) );
dl( VERTEX2F(401 * 16, 250 * 16) );

	dl(END());

*/
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

	wr32(REG_DLSWAP, DLSWAP_FRAME);
	wr32(REG_PCLK, 5);

	//wr32(REG_ROTATE, 1);
	wr32(REG_PWM_DUTY, 64);

	/*wr32(RAM_CMD, CMD_CALIBRATE);
	wr32(REG_CMD_WRITE, 4);*/
}

void loop()
{
	delay(10); // let's be nice on the cpu today :)
	// wr32(REG_DLSWAP, SWAP_FRAME);
	/*uint32_t t = rd32(REG_TOUCH_RZ);
	if (t != 32767)
	{
		printf("touch: %i\n", t);
	}*/
}
