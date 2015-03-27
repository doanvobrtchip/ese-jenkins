/*
* Copyright (C) 2015  Future Technology Devices International Ltd
* Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

// NOTE: SETFONT2 uses the address in the font raw header
#define FONT_ADDR 63000
#define FONT_BMP_ADDR FONT_ADDR + 148

#define START_CHAR 33

void setup()
{
	wr32(REG_PCLK, 5);

	//

	{
		FILE *f = fopen("oswald_regular.raw", "rb");
		if (!f) { printf("Failed to open file\n"); return; }
		uint32_t wp = 0;
		wrstart(FONT_ADDR);
		for (;;)
		{
			uint32_t buffer;
			size_t nb = fread(&buffer, 1, 4, f);
			if (nb > 0)
			{
				wr32(buffer);
			}
			wp += 4;
			if (nb != 4)
			{
				printf("Close file, nb = %i, wp = %i\n", (int)nb, (int)wp);
				if (fclose(f)) printf("Error closing file\n");
				f = NULL;
				break;
			}
		}
		wrend();
	}

	/*for (int i = 0; i < (128 - START_CHAR); ++i) //(int i = (128 - START_CHAR - 1); i >= 0; --i)
	{
		// shift indices
		// wr8(FONT_ADDR + i + START_CHAR, rd32(FONT_ADDR + i));
		wr8(FONT_ADDR + i, rd8(FONT_ADDR + i + START_CHAR));
	}*/
	for (int i = (128 - START_CHAR - 2); i >= 0; --i)
	{
		// shift indices
		wr8(FONT_ADDR + i + START_CHAR, rd8(FONT_ADDR + i));
	}
	wr32(FONT_ADDR + 144, FONT_BMP_ADDR);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(BITMAP_HANDLE(1));
	wr32(BITMAP_SOURCE(FONT_BMP_ADDR));
	wr32(BITMAP_LAYOUT(L4, 8, 17));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 16, 17));
	/*wr32(CMD_SETFONT);
	wr32(1);
	wr32(FONT_ADDR);*/
	wr32(CMD_SETFONT2);
	wr32(1);
	wr32(FONT_ADDR);
	wr32(START_CHAR);
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	while (rd32(REG_CMDB_SPACE) != 4092);

	//

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
	wrstr("Font");
	wr32(CMD_TEXT);
	wr16(241), wr16(121);
	wr16(1), wr16(OPT_CENTERX);
	for (int i = 1; i < 128; ++i)
	{
		wr8(START_CHAR + i);
	}
	wr8(0);
	wr32(COLOR_RGB(255, 255, 255));
	wr32(COLOR_A(255));
	wr32(CMD_TEXT);
	wr16(240), wr16(60);
	wr16(31), wr16(OPT_CENTERX);
	wrstr("Font");
	wr32(CMD_TEXT);
	wr16(240), wr16(116);
	wr16(1), wr16(OPT_CENTERX);
	// wrstr("Test CMD_SETFONT2");
	for (int i = 1; i < 128; ++i)
	{
		wr8(START_CHAR + i);
	}
	wr8(0);
	wr32(CMD_TEXT);
	wr16(2), wr16(1);
	wr16(1), wr16(0);
	wrstr("FTDI Test CMD_SETFONT2");
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();

	while (rd32(REG_CMDB_SPACE) != 4092);
}

void loop()
{
	while (rd32(REG_CMDB_SPACE) != 4092);
}

/* end of file */
