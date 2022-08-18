/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

void setup()
{
	wr32(REG_PCLK, 5);

	int freespace = rd32(REG_CMDB_SPACE);
	printf("Freespace: %i\n", freespace);
}

int frame = 0;
void loop()
{
	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(CLEAR_COLOR_RGB(59, 149, 217));
	wr32(CLEAR(1, 1, 1));
	wr32(NOP()); 
	wr32(NOP()); 
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
	wrstr("CMDB");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(26), wr16(OPT_CENTERX);
	wrstr("Test REG_CMDB_WRITE");
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(31), wr16(OPT_CENTERX);
	wrstr("CMDB");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(26), wr16(OPT_CENTERX);
	wrstr("Test REG_CMDB_WRITE");
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(26), wr16(0);
	wrstr("FTDI");
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	while (rd32(REG_CMDB_SPACE) != 4092);
}

/* end of file */
