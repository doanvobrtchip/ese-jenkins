/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define RAM_G_SIZE (1024 * 1024L)

#define TEST_FILE "logo.png"
#define TEST_WIDTH 292
#define TEST_HEIGHT 103
#define TEST_SIZE (TEST_WIDTH * TEST_HEIGHT * 2)
int lastfreespace = 0;

void loadimage(int handle, int addr, int x, int y)
{
	printf("Handle: %i\n", handle);
	FILE *f = fopen(TEST_FILE, "rb");
	if (!f)
		printf("Failed to open file\n");
	else
	{
		wrstart(REG_CMDB_WRITE);
		wr32(BITMAP_HANDLE(handle));
		wr32(CMD_LOADIMAGE);
		wr32(addr);
		wr32(0);
		wrend();
		while (f)
		{
			int freespace = rd32(REG_CMDB_SPACE);
			if (freespace != lastfreespace)
			{
				// printf("Freespace: %i\n", freespace);
				lastfreespace = freespace;
			}
			if (freespace & 1)
			{
				printf("Coprocessor returned error\n");
				if (fclose(f))
					printf("Error closing file\n");
				f = NULL;
			}
			else if (freespace >= 1024)
			{
				wrstart(REG_CMDB_WRITE);
				for (int i = 0; i < freespace; i += 4)
				{
					uint32_t buffer;
					size_t nb = fread(&buffer, 1, 4, f);
					if (nb > 0)
					{
						wr32(buffer);
					}
					if (nb != 4)
					{
						printf("Close file, nb = %i\n", (int)nb);
						if (fclose(f))
							printf("Error closing file\n");
						f = NULL;
						wrend();
						while (rd32(REG_CMDB_SPACE) != 4092)
							;
						break;
					}
				}
				wrend();
			}
		}
		wrstart(REG_CMDB_WRITE);
		wr32(BEGIN(BITMAPS));
		wr32(VERTEX2II(x, y, handle, 0));
		wr32(END());
		wrend();
	}
}

void setup()
{
	wr32(REG_PCLK, 5);

	printf("Using '%s', w: %i, h: %i, expected size: %i\n",
	    TEST_FILE, TEST_WIDTH, TEST_HEIGHT, TEST_SIZE);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wrend();

	loadimage(1, 0, 0, 0);
	loadimage(3, TEST_SIZE, TEST_WIDTH, 0);
	loadimage(5, RAM_G_SIZE - TEST_SIZE - (TEST_SIZE / 2), 0, TEST_HEIGHT);

	wrstart(REG_CMDB_WRITE);
	wr32(DISPLAY());
	wr32(CMD_SWAP);
	wrend();
}

void loop()
{
	int freespace = rd32(REG_CMDB_SPACE);
	if (freespace != lastfreespace)
	{
		printf("Freespace: %i\n", freespace);
		lastfreespace = freespace;
	}
	while (rd32(REG_CMDB_SPACE) != 4092)
		;
}

/* end of file */
