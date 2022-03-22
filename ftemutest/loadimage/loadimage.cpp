/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <vc2.h>
#include "../ftemutest.h"
#include <stdio.h>

#define TEST_FILE "lenna256.jpg"
static FILE *s_F = NULL;

void setup()
{
	wr32(REG_PCLK, 5);

	s_F = fopen(TEST_FILE, "rb");
	if (!s_F) printf("Failed to open file\n");

	wrstart(REG_CMDB_WRITE);
	wr32(CMD_DLSTART);
	wr32(CMD_LOADIMAGE);
	wr32(0);
	wr32(0);
	wrend();
}

int lastfreespace = 0;
void loop()
{
	int freespace = rd32(REG_CMDB_SPACE);
	if (freespace != lastfreespace)
	{
		printf("Freespace: %i\n", freespace);
		lastfreespace = freespace;
	}
	if (s_F)
	{
		if (freespace & 1)
		{
			printf("Coprocessor returned error\n");
			if (fclose(s_F)) printf("Error closing file\n");
			s_F = NULL;
		}
		else if (freespace >= 1024)
		{
			wrstart(REG_CMDB_WRITE);
			for (int i = 0; i < freespace; i += 4)
			{
				uint32_t buffer;
				size_t nb = fread(&buffer, 1, 4, s_F);
				if (nb > 0)
				{
					wr32(buffer);
				}
				if (nb != 4)
				{
					printf("Close file, nb = %i\n", (int)nb);
					if (fclose(s_F)) printf("Error closing file\n");
					s_F = NULL;
					wrend();
					while (rd32(REG_CMDB_SPACE) != 4092);
					wrstart(REG_CMDB_WRITE);
					wr32(BEGIN(BITMAPS));
					wr32(VERTEX2II(0, 0, 0, 0));
					wr32(END());
					wr32(DISPLAY());
					wr32(CMD_SWAP);
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
}

/* end of file */
