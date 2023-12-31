/*
BT8XX Emulator Samples
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#define BT815EMU_MODE

#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>
#include <ft800emu_vc.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define BTDUMP_FILE argv[1]
#define BTDUMP_FALLBACK "C:/source/ft800emu/reference/vc3test/traces/test_astc_layout_0.vc1dump"
#define BTFLASH_DATA_FILE L"C:/source/ft800emu/reference/vc3roms/stdflash.bin"

extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long Timeout);

void swrbegin(BT8XXEMU_Emulator *emulator, size_t address)
{
	BT8XXEMU_chipSelect(emulator, 1);

	BT8XXEMU_transfer(emulator, (2 << 6) | ((address >> 16) & 0x3F));
	BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, address & 0xFF);
	// BT8XXEMU_transfer(0x00);
}

void swr8(BT8XXEMU_Emulator *emulator, uint8_t value)
{
	BT8XXEMU_transfer(emulator, value);
}

void swr16(BT8XXEMU_Emulator *emulator, uint16_t value)
{
	BT8XXEMU_transfer(emulator, value & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
}

void swr32(BT8XXEMU_Emulator *emulator, uint32_t value)
{
	BT8XXEMU_transfer(emulator, value & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 24) & 0xFF);
}

void swrend(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMU_chipSelect(emulator, 0);
}

void wr32(BT8XXEMU_Emulator *emulator, size_t address, uint32_t value)
{
	swrbegin(emulator, address);
	swr32(emulator, value);
	swrend(emulator);
}

uint32_t rd32(BT8XXEMU_Emulator *emulator, size_t address)
{
	BT8XXEMU_chipSelect(emulator, 1);

	BT8XXEMU_transfer(emulator, (address >> 16) & 0x3F);
	BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, address & 0xFF);
	BT8XXEMU_transfer(emulator, 0x00);

	uint32_t value;
	value = BT8XXEMU_transfer(emulator, rand() & 0xFF);
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 8;
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 16;
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 24;

	BT8XXEMU_chipSelect(emulator, 0);
	return value;
}

void flush(BT8XXEMU_Emulator *emulator)
{
	while (rd32(emulator, REG_CMD_READ) != rd32(emulator, REG_CMD_WRITE));
}

static int32_t s_HSize, s_VSize;
static volatile bool s_DlSwapDone = false;
static bool s_DlSkippedOne = false;
static volatile bool s_GraphicsDone = false;
static const char *s_OutFileName = NULL;

int graphics(BT8XXEMU_Emulator *sender, void *context, int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags)
{
	if (s_DlSwapDone && hsize && vsize && hsize == s_HSize && vsize == s_VSize)
	{
		s_DlSkippedOne = true;
	}
	if (s_DlSkippedOne && hsize && vsize && (flags == BT8XXEMU_FrameBufferComplete))
	{
		int bsize = hsize * vsize;
		FILE *f = NULL;
		while (!(f = fopen(s_OutFileName, "wb"))) Sleep(1);
		while (!fwrite(buffer, 1, sizeof(buffer[0]) * bsize, f)) Sleep(1);
		fflush(f);
		fclose(f);
		s_GraphicsDone = true;
	}
	return 1;
}

int main(int argc, char* argv[])
{
	s_OutFileName = argc > 2 ? argv[2] : NULL;
	FILE *f = fopen(argc > 1 ? BTDUMP_FILE : BTDUMP_FALLBACK, "rb");
	bool wait = false;

	printf("%s\n\n", BT8XXEMU_version());

	BT8XXEMU_FlashParameters flashParams;
	BT8XXEMU_Flash_defaults(BT8XXEMU_VERSION_API, &flashParams);
	wcscpy(flashParams.DataFilePath, BTFLASH_DATA_FILE);
	flashParams.StdOut = true;
	BT8XXEMU_Flash *flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);

	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, BT8XXEMU_EmulatorBT815);
	params.Flags |= BT8XXEMU_EmulatorEnableStdOut;

	params.Flash = flash;

	if (s_OutFileName)
	{
		params.Graphics = graphics;
	}

	BT8XXEMU_Emulator *emulator = NULL;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &emulator, &params);
	uint8_t *ram = BT8XXEMU_getRam(emulator);

    const char *a0 = argc > 1 ? BTDUMP_FILE : BTDUMP_FALLBACK;
    if (strcmp(a0 + strlen(a0) - 4, ".XBU") == 0) {
		/*
        hsize = 480;
        vsize = 272;

        FILE *xbu = fopen(BTDUMP_FILE, "rb");
        f = fopen(argv[2], "wb");

        wr32(emulator, REG_HSIZE, hsize);
        wr32(emulator, REG_VSIZE, vsize);

        int i = 0;
        buffer[hsize * vsize] = 1234567;

        while (1) {
            int wp = rd32(emulator, REG_CMD_WRITE);
            int rp = rd32(emulator, REG_CMD_READ);
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
		*/
	}
	else
	{
		if (!f) printf("Failed to open vc1dump file\n");
		else
		{
			const size_t headersz = 6;
			uint32_t header[headersz];
			size_t s = fread(header, sizeof(uint32_t), headersz, f);
			if (s != headersz) printf("Incomplete vc1dump header\n");
			else if (header[0] == 100)
			{
				s_HSize = header[1];
				s_VSize = header[2];
				wr32(emulator, REG_HSIZE, s_HSize);
				wr32(emulator, REG_VSIZE, s_VSize);
				wr32(emulator, REG_PCLK, 5);
				wr32(emulator, REG_MACRO_0, header[3]);
				wr32(emulator, REG_MACRO_1, header[4]);;

				s = fread(&ram[RAM_G], 1, 262144, f);
				if (s != 262144) printf("Incomplete vc1dump RAM_G\n");
				else
				{
#ifndef RAM_PAL
#define RAM_PAL (RAM_G + 262144)
#endif
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
			else if (header[0] == 110)
			{
				s_HSize = header[1];
				s_VSize = header[2];
				wr32(emulator, REG_HSIZE, s_HSize);
				wr32(emulator, REG_VSIZE, s_VSize);
				wr32(emulator, REG_PCLK, 5);
				wr32(emulator, REG_MACRO_0, header[3]);
				wr32(emulator, REG_MACRO_1, header[4]);;

				if (!s_HSize || !s_VSize)
				{
					printf("Invalid size\n");
				}
				else
				{
					s = fread(&ram[RAM_G], 1, 1048576, f);
					if (s != 1048576) printf("Incomplete vc1dump RAM_G\n");
					else
					{
						s = fread(&ram[RAM_DL], 1, 8192, f);
						if (s != 8192) printf("Incomplete vc1dump RAM_DL\n");
						else
						{
							printf("Loaded vc1dump file\n");
							wait = true;
						}
					}
				}
			}
			else
			{
				printf("Invalid header version %i\n", header[0]);
			}
			if (fclose(f)) printf("Error closing vc1dump file\n");
		}
		wr32(emulator, REG_DLSWAP, DLSWAP_FRAME);

		if (wait)
		{
			while ((rd32(emulator, REG_DLSWAP) != DLSWAP_DONE) && BT8XXEMU_isRunning(emulator));
			s_DlSwapDone = true;
			while (!s_GraphicsDone && BT8XXEMU_isRunning(emulator)) flush(emulator);
		}
    }

	BT8XXEMU_stop(emulator);
	BT8XXEMU_destroy(emulator);
	
	BT8XXEMU_Flash_destroy(flash);

	return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
