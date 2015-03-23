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

	// wr8(REG_ROTATE, 3);

	////////////////////////

	int wp = rd32(REG_CMD_WRITE) & 0xFFF;
	int rp = rd32(REG_CMD_READ) & 0xFFF;

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_CALIBRATE);
	wr32(CMD_SWAP);
	wrend();

	wp += 8;
	wp &= 0xFFF;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);

	////////////////////////

	wrstart(RAM_CMD + (wp & 0xFFF));
	wr32(CMD_SETROTATE);
	wr32(3);
	wrend();

	wp += 8;
	wp &= 0xFFF;
	wr32(REG_CMD_WRITE, wp);
	do
	{
		rp = rd32(REG_CMD_READ);
	} while (wp != rp);
}

int frame = 0;
void loop()
{
	++frame;
	int touchX = (int16_t)rd16(REG_TOUCH_SCREEN_XY + 2);
	int touchY = (int16_t)rd16(REG_TOUCH_SCREEN_XY);
	int tag = rd8(REG_TOUCH_TAG);
	char buffer[1024];
	sprintf(buffer, "REG_TOUCH_SCREEN_XY: %i, %i, TAG: %i", touchX, touchY, tag);

	char bufferb[1024];
	sprintf(bufferb, "%i", frame);

	int wp = rd32(REG_CMD_WRITE) & 0xFFF;
	int rp = rd32(REG_CMD_READ) & 0xFFF;

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
	wp += wrstr("Touch Transform");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr(buffer);
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(31), wr16(OPT_CENTERX);
	wp += wrstr("Touch Transform");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(26), wr16(OPT_CENTERX);
	wp += wrstr(buffer);
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(26), wr16(0);
	wp += wrstr(bufferb);
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

/* end of file */
