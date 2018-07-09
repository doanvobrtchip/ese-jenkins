/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "../ftemutest.h"
#include <stdio.h>

#include <../../fteditor/constant_mapping.h>
#include <../../fteditor/constant_common.h>

#include <QString>

extern const char *g_Case;
static FILE *s_F = NULL;

void setup()
{
	QByteArray str = (QString("reference/xbu/") + g_Case + ".XBU").toLocal8Bit();
	s_F = fopen(str.data(), "rb");
	if (!s_F) printf("Failed to open XBU file\n");
	else
	{
		printf("Load XBU\n");

		wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE), 480);
		wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE), 272);
		wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PCLK), 5);
	}
}

static int wp = 0;
static int wpr = 0;

static bool okwrite = false;

static int lastround = 0;

void loop()
{
	int regwp = rd32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE));
	int rp = rd32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
	int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
	int freespace = ((4096 - 4) - fullness);

	// printf("rp: %i, wp: %i, wpr: %i, regwp: %i\n", rp, wp, wpr, regwp);
	if (rp == -1)
	{
		printf("rp < 0, error\n");
		printf("Close XBU\n");
		if (fclose(s_F)) printf("Error closing vc1dump file\n");
		s_F = NULL;
	}
	else
	{
		if (freespace)
			// if (freespace >= 2048)
		{
			int freespacediv = freespace >> 2;

			wrstart(FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
			for (int i = 0; i < freespacediv; ++i)
			{
				uint32_t buffer;
				size_t nb = fread(&buffer, 4, 1, s_F);
				if (nb == 1)
				{
					/*if (buffer == CMD_DLSTART) okwrite = true;

					if (okwrite)
					{*/
					wr32(buffer);
					wp += 4;
					//}

					if (buffer == CMD_SWAP)
					{
						wpr = wp;
						wrend();
						wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wpr & 0xFFF));
						wrstart(FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
					}
				}
				else
				{
					printf("Close XBU, nb = %i\n", (int)nb);
					if (fclose(s_F)) printf("Error closing xbu file\n");
					s_F = NULL;
					break;
				}
			}
			wrend();

			if (s_F)
			{
				int wprn = (wp - 128);
				if (wprn > wpr)
				{
					wpr = wprn;
					wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wpr & 0xFFF));
				}
			}
			else
			{
				wpr = wp;
				wr32(FTEDITOR::reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wpr & 0xFFF));
			}
		}
		else
		{
			// FT800EMU::System.delay(1000);
		}
		int newround = wpr / 4096;
		if (lastround != newround)
		{
			// printf("new round\n");
			lastround = newround;
		}
	}
}

void coprocdiff(BT8XXEMU_Emulator *sender, void *context)
{
	setup();
	while (s_F)
	{
		loop();
	}
}

/* end of file */
