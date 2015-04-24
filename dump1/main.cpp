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
#include <ft800emu_coprocessor.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <ft8xxemu_graphics_driver.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ft800emu_vc.h>

void wr32(size_t address, uint32_t value)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((2 << 6) | ((address >> 16) & 0x3F));
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);

	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 16) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 24) & 0xFF);

	FT800EMU::SPII2C.csHigh();
}

static argb8888 s_Buffer[FT800EMU_SCREEN_WIDTH_MAX * FT800EMU_SCREEN_HEIGHT_MAX + 1];

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char* argv[])
{
	FILE *f = fopen(argv[1], "rb");
    int hsize, vsize;

#ifdef FT810EMU_MODE
	FT800EMU::Memory.begin(FT8XXEMU_EmulatorFT810);
#else
	FT800EMU::Memory.begin(FT8XXEMU_EmulatorFT801);
#endif
    FT800EMU::GraphicsProcessor.begin();
    FT800EMU::GraphicsProcessor.enableMultithread();

    uint8_t *ram = FT800EMU::Memory.getRam();
	argb8888 *buffer = s_Buffer;

    char *a0 = argv[1];
    if (strcmp(a0 + strlen(a0) - 4, ".XBU") == 0) {
        hsize = 480;
        vsize = 272;

        FILE *xbu = fopen(argv[1], "rb");
        f = fopen(argv[2], "wb");

        wr32(REG_HSIZE, hsize);
        wr32(REG_VSIZE, vsize);

        FT800EMU::Coprocessor.begin();
        int i = 0;
        buffer[hsize * vsize] = 1234567;

        while (1) {
            int wp = FT800EMU::Memory.rawReadU32(ram, REG_CMD_WRITE);
            int rp = FT800EMU::Memory.rawReadU32(ram, REG_CMD_READ);
            int fullness = (wp - rp) & 4095;
            if (fullness < 2048) {
                int n = fread(&ram[RAM_CMD + wp], 1, 1024, xbu) & 0xFFFFFFFF;
                // printf("Feed %x to %x\n", n, wp);
                wp = (wp + n) & 4095;
                FT800EMU::Memory.rawWriteU32(ram, REG_CMD_WRITE, wp);
            }
            FT800EMU::Coprocessor.executeManual();
			if (FT800EMU::Memory.getDirectSwapCount() != i) {
                // fprintf(stderr, "%d\n", i);
                if (0) {
                    FT800EMU::Memory.swapDisplayList();
                    FILE *dl = fopen("dl.bin", "wb");
                    fwrite(&ram[RAM_DL], 1, FT800EMU::Memory.rawReadU32(ram, REG_CMD_DL), dl);
                    fclose(dl);
                    FT800EMU::Memory.swapDisplayList();
                }
                FT800EMU::GraphicsProcessor.process(buffer, false, false, hsize, vsize);
                fwrite(buffer, 1, (hsize * vsize * sizeof(buffer[0])), f);
                i++;
            }
            if ((FT800EMU::Memory.getDirectSwapCount() == i) && feof(xbu))
                break;
            // if (i == 300) break;
            assert(buffer[hsize * vsize] == 1234567);
        }
        fclose(f);
    } else {
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
        wr32(REG_DLSWAP, DLSWAP_FRAME);

		int bsize = hsize * vsize;
        FT800EMU::GraphicsProcessor.process(buffer, false, false, hsize, vsize);
        f = fopen(argv[2], "wb");
        fwrite(buffer, 1, sizeof(buffer[0]) * bsize, f);
        fclose(f);
    }
	return 0;
}
