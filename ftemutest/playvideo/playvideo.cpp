/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

//#define TEST_FILE "sampleapp/Test/chickens-4-short.avi"
#define TEST_FILE "c:/temp/catf.avi"
static FILE *s_F = NULL;

#define FIFO_SIZE (262144)
#define FIFO_ADDR (0x100000 - FIFO_SIZE)

#define TEST_A 0
#define TEST_FIFO 0

void setup()
{
	wr32(REG_PCLK, 5);

#if TEST_FIFO
	wrstart(REG_CMDB_WRITE);
	wr32(CMD_MEDIAFIFO);
	wr32(FIFO_ADDR);
	wr32(FIFO_SIZE);
	wrend();
	wr32(0, 1);
#endif

	s_F = fopen(TEST_FILE, "rb");
	if (!s_F) printf("Failed to open AVI file\n");
#if TEST_FIFO
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
		if (nb != 4 || wp >= FIFO_SIZE - 4)
		{
			printf("Close AVI, nb = %i\n", (int)nb);
			if (fclose(s_F)) printf("Error closing AVI file\n");
			s_F = NULL;
			break;
		}
	}
	wrend();

	printf("Size: %u\n", wp);
	wr32(REG_MEDIAFIFO_WRITE, wp);
#endif

	wrstart(REG_CMDB_WRITE);
	wr32(CLEAR_COLOR_RGB(0, 128, 255));
	wr32(CLEAR(1, 1, 1));
#if TEST_A
	wr32(BITMAP_HANDLE(0));
	wr32(BITMAP_SOURCE(4));
	wr32(BITMAP_LAYOUT(RGB565, 115*2, 80));
	wr32(BITMAP_SIZE(NEAREST, BORDER, BORDER, 115, 80));
	wr32(BEGIN(BITMAPS));
	wr32(VERTEX2II(0, 0, 0, 0));
	wr32(END());
#endif
	wr32(DISPLAY());
	wr32(CMD_SWAP);

#if TEST_A
	wr32(CMD_VIDEOSTART);
#else
	wr32(CMD_PLAYVIDEO);
#if TEST_FIFO
	wr32(OPT_FULLSCREEN | OPT_MEDIAFIFO /*| OPT_NOTEAR*/);
#else
	wr32(OPT_FULLSCREEN | OPT_SOUND /*| OPT_NOTEAR*/);
#endif
#endif

	wrend();

	/*wr8(REG_VOL_SOUND, 64);
	wr16(REG_SOUND, ((68 << 8) + 0x46));
	wr8(REG_PLAY, 1);*/
}

int lastfreespace = 0;
int lastmediafiforead = 0;
void loop()
{
	int freespace = rd32(REG_CMDB_SPACE);
	if (freespace != lastfreespace)
	{
		//printf("REG_CMDB_SPACE: %i\n", freespace);
		lastfreespace = freespace;
		if (freespace & 1)
			printf("Coprocessor flags error\n");
	}

#if TEST_A
	if (freespace == 4092)
	{
		if (rd32(0) != 0)
		{
			printf("Frame req\n");
			wrstart(REG_CMDB_WRITE);
			wr32(CMD_VIDEOFRAME);
			wr32(4);
			wr32(0);
			wrend();
		}
	}
#endif

	int fiforead = rd32(REG_MEDIAFIFO_READ);
	if (lastmediafiforead != fiforead)
	{
		// printf("REG_MEDIAFIFO_READ: %i\n", fiforead);
		// printf("REG_MEDIAFIFO_WRITE: %i\n", rd32(REG_MEDIAFIFO_WRITE));
		lastmediafiforead = fiforead;
	}
#if !TEST_FIFO
	if (s_F)
	{
		int freespace = rd32(REG_CMDB_SPACE);
		if (freespace != lastfreespace)
		{
			//printf("Freespace: %i\n", freespace);
			lastfreespace = freespace;
		}
		if (freespace & 1)
		{
			printf("Coprocessor returned error\n");
			if (fclose(s_F)) printf("Error closing AVI file\n");
			s_F = NULL;
		}
		else if (freespace >= 1024)
		{
			wrstart(REG_CMDB_WRITE);
			for (int i = 0; i < freespace; i += 8)
			{
				uint32_t buffer;
				size_t nb = fread(&buffer, 1, 4, s_F);
				if (nb > 0)
				{
					wr32(buffer);
				}
				if (nb != 4)
				{
					printf("Close AVI, nb = %i\n", (int)nb);
					if (fclose(s_F)) printf("Error closing AVI file\n");
					s_F = NULL;
					break;
				}
			}
			wrend();
		}
	}
	else
	{
		while (rd32(REG_CMDB_SPACE) != 4092);
	}
#endif
}

/* end of file */
