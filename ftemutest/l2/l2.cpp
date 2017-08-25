/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define TEST_IMAGE "l2/lenaface40.raw"

#define IMAGE_ADDR (0)

int lastfreespace = 0;
void setup()
{
	wr32(REG_PCLK, 5);

	{
		FILE *f = fopen(TEST_IMAGE, "rb");
		if (!f) { printf("Failed to open file\n"); return; }
		uint32_t wp = 0;
		wrstart(IMAGE_ADDR);
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

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(BITMAP_HANDLE(0));
	wr32(BITMAP_SOURCE(0));
	wr32(BITMAP_LAYOUT(L2, 10, 40));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 40, 40));
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
