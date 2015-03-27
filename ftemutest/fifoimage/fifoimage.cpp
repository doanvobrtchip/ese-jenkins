/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define TEST_FILE "redglasses730.jpg"
static FILE *s_F = NULL;

#define FIFO_SIZE (131072)
#define FIFO_ADDR (0x100000 - FIFO_SIZE)

int lastfreespace = 0;
void setup()
{
	wr32(REG_PCLK, 5);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_MEDIAFIFO);
	wr32(FIFO_ADDR);
	wr32(FIFO_SIZE);
	wrend();

	s_F = fopen(TEST_FILE, "rb");
	if (!s_F) { printf("Failed to open file\n"); return; }
	
	uint32_t wp = 0;
	wrstart(FIFO_ADDR);
	for (;;)
	{
		uint32_t buffer;
		size_t nb = fread(&buffer, 1, 4, s_F);
		if (nb > 0)
		{
			wr32(buffer);
		}
		wp += 4;
		if (nb != 4 || wp >= FIFO_SIZE)
		{
			printf("Close file, nb = %i, wp = %i\n", (int)nb, (int)wp);
			if (fclose(s_F)) printf("Error closing file\n");
			s_F = NULL;
			break;
		}
	}
	wrend();

	printf("Size: %u\n", wp);
	wr32(REG_MEDIAFIFO_WRITE, wp);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(CMD_LOADIMAGE);
	wr32(0);
	wr32(OPT_MEDIAFIFO | OPT_MONO);
	wrend();

	for (;; )
	{
		int freespace = rd32(REG_CMDB_SPACE);
		if (freespace != lastfreespace)
		{
			printf("REG_CMDB_SPACE: %i\n", freespace);
			lastfreespace = freespace;
			if (freespace & 1)
				printf("Coprocessor flags error\n");
		}
		if (freespace == 4092)
			break;
	}

	wrstart(REG_CMDB_WRITE);
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
