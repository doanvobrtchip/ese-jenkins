/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define TEST_IMAGE "redglasses.paletted.raw"
#define TEST_PALETTE "redglasses.paletted.lut.raw"

#define IMAGE_ADDR (0)
#define PAL_ADDR (131072)

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

	{
		FILE *f = fopen(TEST_PALETTE, "rb");
		if (!f) { printf("Failed to open file\n"); return; }
		uint32_t wp = 0;
		wrstart(PAL_ADDR);
		for (;;)
		{
			uint32_t buffer;
			size_t nb = fread(&buffer, 1, 4, f);
			if (nb > 0)
			{
#if 0
				wr32(buffer);
#elif 0
				uint16_t r = ((buffer >> 3) & 0x1F)
					| (((buffer >> 10) & 0x3F) << 5)
					| (((buffer >> 19) & 0x1F) << 11);
				wr16(r);
#else
				uint16_t r = ((buffer >> 4) & 0xF)
					| (((buffer >> 12) & 0xF) << 4)
					| (((buffer >> 20) & 0xF) << 8)
					| (((buffer >> 28) & 0xF) << 12);
				wr16(r);
#endif
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
#if 0
	wr32(BITMAP_LAYOUT(PALETTED8, 360, 238));
#elif 0
	wr32(BITMAP_LAYOUT(PALETTED565, 360, 238));
#else
	wr32(BITMAP_LAYOUT(PALETTED4444, 360, 238));
#endif
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 360, 238));
	wr32(PALETTE_SOURCE(PAL_ADDR));
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
