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

	FILE *f = NULL;
	f = fopen("../reference/dumps/test_autumn.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_bm_xform_rot.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_bm_params.0.vc1dump", "rb"); // middle bar not ok
	// f = fopen("../reference/dumps/test_bm_params.1.vc1dump", "rb"); // middle bar not ok
	// f = fopen("../reference/dumps/test_bm_wrap.0.vc1dump", "rb"); // ok, but does not match the 'undefined behaviour' of npot bitmap
	// f = fopen("../reference/dumps/test_formats.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_bilinear_fmts.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_alpha_comparisons.0.vc1dump", "rb");
	// f = fopen("../reference/dumps/test_blending.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_bm_cell_handle.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_linestrip_changes.0.vc1dump", "rb");
	// f = fopen("../reference/dumps/test_blend_illegal.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_bm_subpixel.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_bilinear_simple.0.vc1dump", "rb"); // verify: what does bitmap size width = 0 mean precisely
	// f = fopen("../reference/dumps/test_bm_draw_source_1.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_bm_height.0.vc1dump", "rb"); // ng
	// f = fopen("../reference/dumps/test_bm_odd.0.vc1dump", "rb"); // ng
	// f = fopen("../reference/dumps/test_bm_xform_zoom.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_bm_xy.0.vc1dump", "rb");
	// f = fopen("../reference/dumps/test_font_aa.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_font_prop.0.vc1dump", "rb"); // seems ok, not sure about blue bg coming through on display?
	// f = fopen("../reference/dumps/test_format_bargraph.0.vc1dump", "rb"); // todo
	f = fopen("../reference/dumps/test_format_paletted.0.vc1dump", "rb"); // ok
	// f = fopen("../reference/dumps/test_formats_16.0.vc1dump", "rb"); // text todo
	// f = fopen("../reference/dumps/test_mem_exhaustive_b.3.vc1dump", "rb");
	// f = fopen("../reference/dumps/test_modulate_256.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_nonpow2.0.vc1dump", "rb"); // undefined behaviour
	// f = fopen("../reference/dumps/test_points_increase.0.vc1dump", "rb"); // seems ok, todo check sizes
	// f = fopen("../reference/dumps/test_points_modulate.0.png", "rb"); // ng, invisible?
	// f = fopen("../reference/dumps/test_points_offscreen.0.vc1dump", "rb"); // ng, todo
	// f = fopen("../reference/dumps/test_points_subpixel.0.vc1dump", "rb"); // verify
	// f = fopen("../reference/dumps/test_points_visit.0.vc1dump", "rb"); // vefrify
	// f = fopen("../reference/dumps/test_ram.0.vc1dump", "rb"); // seems ok
	// f = fopen("../reference/dumps/test_mem_exhaustive.0.vc1dump", "rb"); // looks ok
	// f = fopen("../reference/dumps/test_scissor_connected.0.vc1dump", "rb"); // todo
	// f = fopen("../reference/dumps/test_scissor_max.0.vc1dump", "rb"); // todo, partially ok
	// f = fopen("../reference/dumps/test_stencil_ops.0.vc1dump", "rb"); // todo, not ok, and runs dead slow
	if (!f) printf("Failed to open vc1dump file\n");
	else
	{
		const size_t headersz = 6;
		uint32_t header[headersz];
		size_t s = fread(header, sizeof(uint32_t), headersz, f);
		if (header[0] != 100) printf("Invalid header version");
		else
		{
			wr32(REG_HSIZE, header[1]);
			wr32(REG_VSIZE, header[2]);
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