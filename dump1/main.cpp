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
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <stdio.h>
#include <vc.h>

void wr32(size_t address, uint32_t value)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((2 << 6) | ((address >> 16) & 0x3F));
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);
	FT800EMU::SPII2C.transfer(0x00);

	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 16) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 24) & 0xFF);

	FT800EMU::SPII2C.csHigh();
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char* argv[])
{
	FILE *f = fopen(argv[1], "rb");
    int hsize, vsize;

    FT800EMU::MemoryClass::begin();
    FT800EMU::GraphicsProcessor.begin();

	if (!f) printf("Failed to open vc1dump file\n");
	else
	{
		const size_t headersz = 6;
		uint32_t header[headersz];
		size_t s = fread(header, sizeof(uint32_t), headersz, f);
		if (header[0] != 100) printf("Invalid header version %i\n", header[0]);
		else
		{
            hsize = header[1];
            vsize = header[2];
			wr32(REG_HSIZE, hsize);
			wr32(REG_VSIZE, vsize);
			wr32(REG_MACRO_0, header[3]);
			wr32(REG_MACRO_1, header[4]);
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
		}
		if (fclose(f)) printf("Error closing vc1dump file\n");
	}
	wr32(REG_DLSWAP, SWAP_FRAME);

    argb8888 buffer[hsize*vsize];
    FT800EMU::GraphicsProcessor.process(buffer, false, hsize, vsize);
    f = fopen(argv[2], "wb");
    fwrite(buffer, 1, sizeof(buffer), f);
    fclose(f);
	return 0;
}
