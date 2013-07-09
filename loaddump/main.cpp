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

#include <ft800emu_emulator.h>
#include <ft800emu_system.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <stdio.h>
#include <vc.h>

void wr32(size_t address, uint32_t value)
{
	FT800EMU::SPII2C.csLow();
	FT800EMU::SPII2C.mcuSetAddress(address);
	FT800EMU::SPII2C.mcuWriteByte(value & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 8) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 16) & 0xFF);
	FT800EMU::SPII2C.mcuWriteByte((value >> 24) & 0xFF);
	FT800EMU::SPII2C.csHigh();
}

void setup()
{
	/*  4      version (always decimal 100)
  4      width of screen in pixels (1-512)
  4      height of screen in pixels (1-512)
  4      contents of REG_MACRO_0
  4      contents of REG_MACRO_1
  4      Expected CRC32 of image
  2**18  Main RAM contents
  2**10  Palette RAM contents
  2**13  Display list contents*/

	FILE *f;
	f = fopen("../reference/dumps/test_autumn.0.vc1dump", "rb");
	// f = fopen("../reference/dumps/test_bm_xform_rot.0.vc1dump", "rb");
	if (!f) printf("Failed to open vc1dump file\n");
	else
	{
		const size_t headersz = 4 * 6;
		uint8_t header[headersz];
		size_t s = fread(header, 1, headersz, f);
		if (s != headersz) printf("Incomplete vc1dump header\n");
		else
		{
			// todo set regs
			uint8_t *ram = FT800EMU::Memory.getRam();
			s = fread(&ram[RAM_G], 1, 262144, f);
			if (s != 262144) printf("Incomplete vc1dump RAM_G\n");
			else
			{
				s = fread(&ram[RAM_PAL], 1, 1024, f);
				if (s != 1024) printf("Incomplete vc1dump RAM_PAL\n");
				else
				{
					s = fread(&ram[RAM_DL], 1, 8192, f);
					if (s != 8192) printf("Incomplete vc1dump RAM_DL\n");
					else printf("Loaded vc1dump file\n");
				}
			}
		}
		if (fclose(f)) printf("Error closing vc1dump file\n");
	}
	wr32(REG_DLSWAP, SWAP_FRAME);
	wr32(REG_PCLK, 5);
}

void loop()
{
	FT800EMU::System.delay(10);
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	FT800EMU::Emulator.run(setup, loop, 
		FT800EMU::EmulatorEnableKeyboard
		);
	return 0;
}
