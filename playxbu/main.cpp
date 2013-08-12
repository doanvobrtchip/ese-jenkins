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

#include <ft800emu_keyboard_keys.h>
#include <ft800emu_emulator.h>
#include <ft800emu_keyboard.h>
#include <ft800emu_system.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <stdio.h>
#include <vc.h>

#define FT800EMU_XBU_FILE "../reference/xbu/3D.XBU"

void swrbegin(size_t address)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((2 << 6) | ((address >> 16) & 0x3F));
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);
	FT800EMU::SPII2C.transfer(0x00);
}

void swr8(uint8_t value)
{
	FT800EMU::SPII2C.transfer(value);
}

void swr16(uint16_t value)
{
	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
}

void swr32(uint32_t value)
{
	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 16) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 24) & 0xFF);
}

void swrend()
{
	FT800EMU::SPII2C.csHigh();
}

void wr32(size_t address, uint32_t value)
{
	swrbegin(address);
	swr32(value);
	swrend();
}

uint32_t rd32(size_t address)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((address >> 16) & 0x3F);
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);
	FT800EMU::SPII2C.transfer(0x00);

	uint32_t value;
	value = FT800EMU::SPII2C.transfer(0);
	value |= FT800EMU::SPII2C.transfer(0) << 8;
	value |= FT800EMU::SPII2C.transfer(0) << 16;
	value |= FT800EMU::SPII2C.transfer(0) << 24;

	FT800EMU::SPII2C.csHigh();
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

void loop()
{
	if (!s_F)
	{
		FT800EMU::System.delay(10);
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
						printf("Close XBU, nb = %i\n", nb);
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
				printf("new round\n");
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
}

void keyboard()
{

}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	FT800EMU::EmulatorParameters params;
	params.Setup = setup;
	params.Loop = loop;
	params.Flags = FT800EMU::EmulatorEnableKeyboard | FT800EMU::EmulatorEnableMouse | FT800EMU::EmulatorEnableDebugShortkeys | FT800EMU::EmulatorEnableCoprocessor;
	params.Keyboard = keyboard;
	FT800EMU::Emulator.run(params);
	return 0;
}
