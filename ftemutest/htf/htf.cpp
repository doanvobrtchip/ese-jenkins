/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <vc4.h>
#include "../ftemutest.h"
#include <stdio.h>

#define RAM_LINETIME_ADDR 0

void setup()
{
	wr32(REG_DLSWAP, DLSWAP_FRAME);
	while (rd32(REG_DLSWAP) != DLSWAP_DONE);

	int wp = rd32(REG_CMD_WRITE);
	int rp = rd32(REG_CMD_READ);
	int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
	int freespace = ((4096 - 4) - fullness);

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
	wr16(31), wr16(OPT_CENTERX);
	wp += wrstr("Test");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr("HSF");
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(31), wr16(OPT_CENTERX);
	wp += wrstr("Test");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr("HSF");
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(26), wr16(0);
	wp += wrstr("FTDI");
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	wp += 152;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);

	wr32(REG_PCLK, 5);

	printf("Begin CMD_HSF ->\n");

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_HSF);
	wr32(420);
	wrend();

	wp += 8;
	wp &= 0xFFF;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);

	printf("<- End CMD_HSF\n");
}

void loop()
{
	int wp = rd32(REG_CMD_WRITE);
	int rp = rd32(REG_CMD_READ);

	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);
}

/* end of file */
