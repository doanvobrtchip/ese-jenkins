/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

void setup()
{
	wr32(REG_PCLK, 5);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(CMD_SKETCH);
	wr16(0); wr16(0);
	wr16(480); wr16(480);
	wr32(0);
	wr32(L1);
	wr32(BITMAP_HANDLE(0));
	wr32(BITMAP_SOURCE(0));
	wr32(BITMAP_LAYOUT(L1, 60, 272));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 480, 272));
	wr32(BEGIN(BITMAPS));
	wr32(VERTEX2II(0, 0, 0, 0));
	wr32(END());
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();
}

void loop()
{
	while (rd32(REG_CMDB_SPACE) != 4092);
}

/* end of file */
