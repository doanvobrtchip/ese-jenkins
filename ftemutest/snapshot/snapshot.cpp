/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"

#define RAM_ASSETS_OSWALD_LARGE 0
#define RAM_ASSETS_OSWALD_MEDIUM 15000
#define RAM_ASSETS_OSWALD_REGULAR 63000

void setup()
{
	wrstart(RAM_ASSETS_OSWALD_LARGE);
	for (uint32_t i = 0; i < SIZE_ASSETS_OSWALD_LARGE; ++i)
		wr8(assets_oswald_large[i]);
	wrend();

	wrstart(RAM_ASSETS_OSWALD_MEDIUM);
	for (uint32_t i = 0; i < SIZE_ASSETS_OSWALD_MEDIUM; ++i)
		wr8(assets_oswald_medium[i]);
	wrend();

	wrstart(RAM_ASSETS_OSWALD_REGULAR);
	for (uint32_t i = 0; i < SIZE_ASSETS_OSWALD_REGULAR; ++i)
		wr8(assets_oswald_regular[i]);
	wrend();

	wrstart(RAM_DL);
	wr32(BITMAP_HANDLE(0));
	wr32(BITMAP_SOURCE(RAM_ASSETS_OSWALD_LARGE + 148));
	wr32(BITMAP_LAYOUT(L4, 20, 56));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 40, 56));
	wr32(BITMAP_HANDLE(1));
	wr32(BITMAP_SOURCE(RAM_ASSETS_OSWALD_MEDIUM + 148));
	wr32(BITMAP_LAYOUT(L4, 12, 29));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 24, 29));
	wr32(BITMAP_HANDLE(2));
	wr32(BITMAP_SOURCE(RAM_ASSETS_OSWALD_REGULAR + 148));
	wr32(BITMAP_LAYOUT(L4, 8, 17));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 16, 17));
	wrend();

	wr32(REG_DLSWAP, DLSWAP_FRAME);
	while (rd32(REG_DLSWAP) != DLSWAP_DONE);

	int wp = rd32(REG_CMD_WRITE);
	int rp = rd32(REG_CMD_READ);
	int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
	int freespace = ((4096 - 4) - fullness);

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_SETFONT);
	wr32(0);
	wr32(RAM_ASSETS_OSWALD_LARGE);
	wr32(CMD_SETFONT);
	wr32(1);
	wr32(RAM_ASSETS_OSWALD_MEDIUM);
	wr32(CMD_SETFONT);
	wr32(2);
	wr32(RAM_ASSETS_OSWALD_REGULAR);
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	wp += 44;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_DLSTART);
	wr32(CLEAR_COLOR_RGB(59, 149, 217));
	wr32(CLEAR(1, 1, 1));
	wr32(CMD_GRADIENT);
	wr16(203), wr16(375);
	wr8(220), wr8(200), wr8(132), wr8(0);
	wr16(148), wr16(-108);
	wr8(54), wr8(37), wr8(7), wr8(0);
	wr32(COLOR_RGB(0, 0, 0));
	wr32(COLOR_A(64));
	wr32(BEGIN(RECTS));
	wr32(VERTEX2II(0, 0, 0, 0));
	wr32(VERTEX2II(511, 18, 0, 0));
	wr32(COLOR_A(128));
	wr32(VERTEX2II(0, 185, 0, 0));
	wr32(VERTEX2II(511, 313, 0, 0));
	wr32(END());
	wr32(COLOR_RGB(0, 0, 0));
	wr32(COLOR_A(128));
	wr32(CMD_TEXT);
	wr16(242), wr16(70);
	wr16(0), wr16(OPT_CENTERX);
	wp += wrstr("\x01\x02\x0B\x03\n");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(1), wr16(OPT_CENTERX);
	wp += wrstr("Tuesday, June 3rd");
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(0), wr16(OPT_CENTERX);
	wp += wrstr("\x01\x02\x0B\x03\n");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(1), wr16(OPT_CENTERX);
	wp += wrstr("Tuesday, June 3rd");
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(2), wr16(0);
	wp += wrstr("12:30");
	wr32(DISPLAY());
	wr32(CMD_SWAP); 
	// CMD_FGCOLOR(83, 136, 154)
	// CMD_BGCOLOR(16, 28, 33)
	// CMD_TOGGLE(199, 220, 80, 2, OPT_FLAT, 0, "Slide to unlock\xFF")
	// CMD_TEXT(212, 191, 2, 0, "Slide to unlock")
	// CMD_BUTTON(333, 211, 120, 36, 2, OPT_FLAT, "Slide to unlock")* /
	wrend();

	wp += 152;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);

	wr32(REG_PCLK, 5);
}

void loop()
{
	int wp = rd32(REG_CMD_WRITE);
	int rp = rd32(REG_CMD_READ);

	// ...

	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);
}

/* end of file */
