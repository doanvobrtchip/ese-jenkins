/**
 * main.cpp
 * $Id$
 * \file main.cpp
 * \brief main.cpp
 * \date 2013-07-09 07:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <Windows.h>

#include <bt8xxemu.h>
#include <stdio.h>
#include <ft800emu_vc.h>


#define FT800EMU_XBU_FILE "xbu/SCATTER.XBU"

BT8XXEMU_Emulator *s_Emulator;

void swrbegin(size_t address)
{
	BT8XXEMU_cs(s_Emulator, 1);

	BT8XXEMU_transfer(s_Emulator, (2 << 6) | ((address >> 16) & 0x3F));
	BT8XXEMU_transfer(s_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(s_Emulator, address & 0xFF);
	// BT8XXEMU_transfer(0x00);
}

void swr8(uint8_t value)
{
	BT8XXEMU_transfer(s_Emulator, value);
}

void swr16(uint16_t value)
{
	BT8XXEMU_transfer(s_Emulator, value & 0xFF);
	BT8XXEMU_transfer(s_Emulator, (value >> 8) & 0xFF);
}

void swr32(uint32_t value)
{
	BT8XXEMU_transfer(s_Emulator, value & 0xFF);
	BT8XXEMU_transfer(s_Emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(s_Emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(s_Emulator, (value >> 24) & 0xFF);
}

void swrend()
{
	BT8XXEMU_cs(s_Emulator, 0);
}

void wr32(size_t address, uint32_t value)
{
	swrbegin(address);
	swr32(value);
	swrend();
}

uint32_t rd32(size_t address)
{
	BT8XXEMU_cs(s_Emulator, 1);

	BT8XXEMU_transfer(s_Emulator, (address >> 16) & 0x3F);
	BT8XXEMU_transfer(s_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(s_Emulator, address & 0xFF);
	BT8XXEMU_transfer(s_Emulator, 0x00);

	uint32_t value;
	value = BT8XXEMU_transfer(s_Emulator, 0);
	value |= BT8XXEMU_transfer(s_Emulator, 0) << 8;
	value |= BT8XXEMU_transfer(s_Emulator, 0) << 16;
	value |= BT8XXEMU_transfer(s_Emulator, 0) << 24;

	BT8XXEMU_cs(s_Emulator, 0);
	return value;
}

static FILE *s_F = NULL;

void setup()
{
	s_F = fopen(FT800EMU_XBU_FILE, "rb");
	if (!s_F) printf("Failed to open XBU file\n");
	else
	{
		printf("Load XBU\n");

		wr32(REG_HSIZE, 480);
		wr32(REG_VSIZE, 272);
		wr32(REG_PCLK, 5);
		// if (fclose(s_F)) printf("Error closing vc1dump file\n");

		/*
		swrbegin(RAM_CMD);
		swr32(CMD_DLSTART);
		swr32(CMD_SWAP);
		swr32(CMD_DLSTART);
		swr32(CLEAR_COLOR_RGB(0, 32, 64));
		swr32(CLEAR(1, 1, 1));
		swr32(TAG(1));
		swr32(CMD_BUTTON);
		swr16(10);
		swr16(10);
		swr16(160);
		swr16(24);
		swr16(26);
		swr16(0);
		swr8('B');
		swr8('a');
		swr8('r');
		swr8(0);
		swr32(CMD_SWAP);
		swrend();

		wr32(REG_CMD_WRITE, (6 * 4) + (4 * 4) + 4 + 4);*/
	}
}

static int wp = 0;
static int wpr = 0;

static bool okwrite = false;

static int lastround = 0;

bool loop()
{
	if (!s_F)
	{
		Sleep(10);
		return false;
	}
	else
	{
		int regwp = rd32(REG_CMD_WRITE);
		int rp = rd32(REG_CMD_READ);
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

				swrbegin(RAM_CMD + (wp & 0xFFF));
				for (int i = 0; i < freespacediv; ++i)
				{
					uint32_t buffer;
					size_t nb = fread(&buffer, 4, 1, s_F);
					if (nb == 1)
					{
						/*if (buffer == CMD_DLSTART) okwrite = true;

						if (okwrite)
						{*/
							swr32(buffer);
							wp += 4;
						//}

						if (buffer == CMD_SWAP)
						{
							wpr = wp;
							swrend();
							wr32(REG_CMD_WRITE, (wpr & 0xFFF));
							swrbegin(RAM_CMD + (wp & 0xFFF));
						}
					}
					else
					{
						printf("Close XBU, nb = %i\n", (int)nb);
						if (fclose(s_F)) printf("Error closing vc1dump file\n");
						s_F = NULL;
						break;
					}
				}
				swrend();

				if (s_F)
				{
					int wprn = (wp - 128);
					if (wprn > wpr)
					{
						wpr = wprn;
						wr32(REG_CMD_WRITE, (wpr & 0xFFF));
					}
				}
				else
				{
					wpr = wp;
					wr32(REG_CMD_WRITE, (wpr & 0xFFF));
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


		/*
		int tag = rd32(REG_TOUCH_TAG);
		if (tag == 1)
		{
			swrbegin(RAM_CMD + wp);
			swr32(CMD_DLSTART);
			swr32(CMD_SPINNER);
			swr16(80);
			swr16(60);
			swr16(0);
			swr16(0);
			swrend();

			wr32(REG_CMD_WRITE, (wp + 4 + 4 + (2 * 4)) & 0xFFF);

			/*swrbegin(RAM_CMD + wp);
			swr32(CMD_LOGO);
			swrend();

			wr32(REG_CMD_WRITE, (wp + 4) & 0xFFF);

			FT800EMU::System.delay(3000);*/
		//}
	}

	return s_F != NULL;
}

void keyboard()
{

}

static bool s_Closed = false;
void close()
{
	s_Closed = true;
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	SetProcessAffinityMask(GetCurrentProcess(), 3);
	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params,
#ifdef FT810EMU_MODE
		BT8XXEMU_EmulatorFT810
#else
		BT8XXEMU_EmulatorFT801
#endif
	);
	memset(&params, 0, sizeof(BT8XXEMU_EmulatorParameters));
	params.Close = close;
	params.Flags =
		BT8XXEMU_EmulatorEnableKeyboard
		| BT8XXEMU_EmulatorEnableMouse
		| BT8XXEMU_EmulatorEnableDebugShortkeys
		| BT8XXEMU_EmulatorEnableCoprocessor
		| BT8XXEMU_EmulatorEnableGraphicsMultithread
		| BT8XXEMU_EmulatorEnableStdOut
		| BT8XXEMU_EmulatorEnableMainPerformance
		;
	params.ReduceGraphicsThreads = 2;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &s_Emulator, &params);
	setup();
	while (!s_Closed)
		loop();
	BT8XXEMU_destroy(s_Emulator);
	s_Emulator = NULL;
	return 0;
}
