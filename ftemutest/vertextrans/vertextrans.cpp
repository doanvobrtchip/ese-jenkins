/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

void setup()
{
	wr32(REG_PCLK, 5);

	////////////////////////

	int wp = rd32(REG_CMD_WRITE) & 0xFFF;
	int rp = rd32(REG_CMD_READ) & 0xFFF;

	////////////////////////////

	wr32(REG_MACRO_0, NOP());
	wr32(REG_MACRO_1, NOP());

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_DLSTART);
	wr32(CLEAR_COLOR_RGB(59, 149, 217));
	wr32(CLEAR(1, 1, 1));
	wr32(MACRO(0));
	wr32(MACRO(1));
	wr32(NOP());
	wr32(NOP());
	wr32(NOP());
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
	wp += wrstr("Vertex Translate");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr("Test");
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(31), wr16(OPT_CENTERX);
	wp += wrstr("Vertex Translate");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr("Test");
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(26), wr16(0);
	wp += wrstr("FTDI");
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	wp += 152;
	wp &= 0xFFF;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ) & 0xFFF;
	} while (wp != rp);
}

void loop()
{
	int frame = rd32(REG_FRAMES);
	int offset = ((frame % (512 * 16)) - (256 * 16));

	wr32(REG_MACRO_0, VERTEX_TRANSLATE_X(offset));
	wr32(REG_MACRO_1, VERTEX_TRANSLATE_Y(offset));

	int wp = rd32(REG_CMD_WRITE);
	int rp = rd32(REG_CMD_READ);

	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);
}

/* end of file */
