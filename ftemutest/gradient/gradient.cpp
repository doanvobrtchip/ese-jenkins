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

	int freespace = rd32(REG_CMDB_SPACE);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(CLEAR_COLOR_RGB(255,0,0));
	wr32(CLEAR(1, 1, 1));
	wr32(CMD_GRADIENT);
	wr16(94);
	wr16(-10);
	wr32(0x007fff);
	wr16(267);
	wr16(96);
	wr32(0x7fff00);
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();
}

void loop()
{
	while (rd32(REG_CMDB_SPACE) != 4092);
}

/* end of file */
