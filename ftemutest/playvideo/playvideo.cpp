/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define TEST_FILE "c:/temp/mjpegtestb.avi"
static FILE *s_F = NULL;

void setup()
{
	wr32(REG_PCLK, 5);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_MEDIAFIFO);
	wr32(0);
	wr32(0x100000);
	wrend();

	s_F = fopen(TEST_FILE, "rb");
	if (!s_F) printf("Failed to open AVI file\n");
	uint32_t wp = 0;
	wrstart(0);
	for (;;)
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
		wp += 4;
		if (wp == 0x100000)
			break;
	}
	wrend();

	printf("Size: %u\n", wp);
	wr32(REG_MEDIAFIFO_WRITE, wp);

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_PLAYVIDEO);
	wr32(OPT_FULLSCREEN | OPT_MEDIAFIFO);
	wrend();
}

int lastfreespace = 0;
int lastmediafiforead = 0;
void loop()
{
	int freespace = rd32(REG_CMDB_SPACE);
	if (freespace != lastfreespace)
	{
		printf("REG_CMDB_SPACE: %i\n", freespace);
		lastfreespace = freespace;
		if (freespace & 1)
			printf("Coprocessor flags error\n");
	}
	int fiforead = rd32(REG_MEDIAFIFO_READ);
	if (lastmediafiforead != fiforead)
	{
		printf("REG_MEDIAFIFO_READ: %i\n", fiforead);
		printf("REG_MEDIAFIFO_WRITE: %i\n", rd32(REG_MEDIAFIFO_WRITE));
		lastmediafiforead = fiforead;
	}
	/*if (s_F)
	{
		int freespace = rd32(REG_CMDB_SPACE);
		if (freespace != lastfreespace)
		{
			printf("Freespace: %i\n", freespace);
			lastfreespace = freespace;
		}
		if (freespace & 1)
		{
			printf("Coprocessor returned error\n");
			if (fclose(s_F)) printf("Error closing AVI file\n");
			s_F = NULL;
		}
		else if (freespace)
		{
			wrstart(REG_CMDB_WRITE);
			for (int i = 0; i < freespace; i += 8)
			{
				uint32_t buffer;
				size_t nb = fread(&buffer, 4, 1, s_F);
				if (nb == 1)
				{
					wr32(buffer);
				}
				else
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
	}*/
}

/* end of file */
