/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.cpp
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
 */

#ifdef FT810EMU_MODE
#include "ft810emu_coprocessor.h"

// System includes
#include <stdio.h>
#include <string.h>
#include <algorithm>

// Project includes
#include "ft800emu_memory.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"

// using namespace ...;

namespace FT810EMU {

#define FT800EMU_COPROCESSOR_DEBUG 0
#define FT800EMU_COPROCESSOR_TRACE 0
#define FT800EMU_COPROCESSOR_TRACEFILE "ft810emu.log"

#define FT800EMU_COPROCESSOR_ROM_SIZE 16384
#define PRODUCT64(a, b) ((int64_t)(int32_t)(a) * (int64_t)(int32_t)(b))
#define isFF(x) (((x) & 0xff) == 0xff)
#define REG(N)    memory32[REG_##N >> 2]

namespace /* anonymous */ {

const uint16_t pgm_rom_ft810[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "resources/crom_ft810.h"
};

const uint32_t crc_table[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t crc_update(uint32_t crc, uint8_t data)
{
	int tbl_idx;
	tbl_idx = crc ^ (data >> (0 * 4));
	crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
	tbl_idx = crc ^ (data >> (1 * 4));
	crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
	return crc;
}

uint32_t crc32(uint32_t crc, uint32_t data)
{
	crc = crc_update(crc, 0xff & (data >> 0));
	crc = crc_update(crc, 0xff & (data >> 8));
	crc = crc_update(crc, 0xff & (data >> 16));
	crc = crc_update(crc, 0xff & (data >> 24));
	return crc;
}

#define SRC(i) src[(i) * stride]
#define DST(i) dst[(i) * stride]

void idct8(idct_t *dst, idct_t *src, size_t stride)
{
#if 0
#define G(X)    (int32_t)((X) * 65536.0)
#define GMUL(a, b)  (PRODUCT64((a), (b)) >> 16)
	idct_t s, t, tmp10, tmp11, tmp12, tmp13, tmp2, tmp3, z1, z2, z3, z4;
	idct_t u, tmp0, tmp1;

	s = GMUL((SRC(2) + SRC(6)), G(0.541196100));

	t = GMUL((SRC(0) + SRC(4)), G(1.0));

	tmp3 = s + GMUL(SRC(2), G(0.765366865));

	tmp10 = t + tmp3;
	tmp13 = t - tmp3;

	t = GMUL((SRC(0) - SRC(4)), G(1.0));
	tmp2 = s + GMUL(SRC(6), G(-1.847759065));

	tmp11 = t + tmp2;
	tmp12 = t - tmp2;

	z3 = SRC(7) + SRC(3);
	z4 = SRC(5) + SRC(1);
	u = GMUL((z3 + z4), G(1.175875602));
	z3 = GMUL(z3, G(-1.961570560)) + u;
	z4 = GMUL(z4, G(-0.390180644)) + u;

	z1 = GMUL((SRC(7) + SRC(1)), G(-0.899976223));
	tmp0 = GMUL(SRC(7), G(0.298631336)) + z1 + z3;
	tmp3 = GMUL(SRC(1), G(1.501321110)) + z1 + z4;

	z2 = GMUL((SRC(5) + SRC(3)), G(-2.562915447));
	tmp1 = GMUL(SRC(5), G(2.053119869)) + z2 + z4;
	tmp2 = GMUL(SRC(3), G(3.072711026)) + z2 + z3;

	DST(0) = (tmp10 + tmp3);
	DST(1) = (tmp11 + tmp2);
	DST(2) = (tmp12 + tmp1);
	DST(3) = (tmp13 + tmp0);
	DST(4) = (tmp13 - tmp0);
	DST(5) = (tmp12 - tmp1);
	DST(6) = (tmp11 - tmp2);
	DST(7) = (tmp10 - tmp3);
#else
	idct_t r[8];
	r[0] = ((256 * SRC(0)) + (355 * SRC(1)) + (334 * SRC(2)) + (301 * SRC(3)) + (256 * SRC(4)) + (201 * SRC(5)) + (138 * SRC(6)) + (70 * SRC(7))) >> 8;
	r[1] = ((256 * SRC(0)) + (301 * SRC(1)) + (138 * SRC(2)) + (-70 * SRC(3)) + (-256 * SRC(4)) + (-355 * SRC(5)) + (-334 * SRC(6)) + (-201 * SRC(7))) >> 8;
	r[2] = ((256 * SRC(0)) + (201 * SRC(1)) + (-138 * SRC(2)) + (-355 * SRC(3)) + (-256 * SRC(4)) + (70 * SRC(5)) + (334 * SRC(6)) + (301 * SRC(7))) >> 8;
	r[3] = ((256 * SRC(0)) + (70 * SRC(1)) + (-334 * SRC(2)) + (-201 * SRC(3)) + (256 * SRC(4)) + (301 * SRC(5)) + (-138 * SRC(6)) + (-355 * SRC(7))) >> 8;
	r[4] = ((256 * SRC(0)) + (-70 * SRC(1)) + (-334 * SRC(2)) + (201 * SRC(3)) + (256 * SRC(4)) + (-301 * SRC(5)) + (-138 * SRC(6)) + (355 * SRC(7))) >> 8;
	r[5] = ((256 * SRC(0)) + (-201 * SRC(1)) + (-138 * SRC(2)) + (355 * SRC(3)) + (-256 * SRC(4)) + (-70 * SRC(5)) + (334 * SRC(6)) + (-301 * SRC(7))) >> 8;
	r[6] = ((256 * SRC(0)) + (-301 * SRC(1)) + (138 * SRC(2)) + (70 * SRC(3)) + (-256 * SRC(4)) + (355 * SRC(5)) + (-334 * SRC(6)) + (201 * SRC(7))) >> 8;
	r[7] = ((256 * SRC(0)) + (-355 * SRC(1)) + (334 * SRC(2)) + (-301 * SRC(3)) + (256 * SRC(4)) + (-201 * SRC(5)) + (138 * SRC(6)) + (-70 * SRC(7))) >> 8;

	DST(0) = r[0];
	DST(1) = r[1];
	DST(2) = r[2];
	DST(3) = r[3];
	DST(4) = r[4];
	DST(5) = r[5];
	DST(6) = r[6];
	DST(7) = r[7];
#endif
}

const int sx[4] = { 0, 1, -2, -1 }; /* 2-bit sign extension */

int addsat(int16_t x, int16_t y)
{
	int s = x + y;
	if (s < 0)
		return 0;
	else if (s > 255)
		return 255;
	else
		return s;
}

} /* anonymous namespace */

uint16_t Ejpg::rgb565(uint8_t y, int8_t Cb, int8_t Cr)
{
	int16_t r, g, b;
#if 0
	r = addsat(GMUL(Cr, G(1.402)), y);
	g = addsat(GMUL(G(-0.34414), Cb) + GMUL(G(-0.71414), Cr), y);
	b = addsat(GMUL(G(1.772), Cb), y);
#else
	r = addsat((Cr * 359) >> 8, y);
	g = addsat((Cb * -88 + Cr * -183) >> 8, y);
	b = addsat((Cb * 454) >> 8, y);
#endif
	uint16_t pix = ((r >> 3) << 11) |
		((g >> 2) << 5) |
		(b >> 3);
	return pix;
}

void Ejpg::reset()
{
	state = 0;
	// FTEMU_printf("%08x\n", G(1.772));
	// FTEMU_printf("%08x\n", G(-0.34414));
	// FTEMU_printf("%08x\n", G(-0.71414));
	// FTEMU_printf("%08x\n", G(1.402   ));
	stim = NULL;
	// stim = fopen("stim", "w");

	bx = 0;
	by = 0;
}
void Ejpg::startblock()
{
	idct_i = 0;
	idct_phase = 0;
	memset(aa, 0, sizeof(aa));
}
void Ejpg::begin()
{
	nbits = 0;
	bits = 0;
	bx = 0;
	by = 0;
	block = 0;
	restart_i = 0;
	expect_ff = 0;
	startblock();
	memset(prevdc, 0, sizeof(prevdc));
	// FTEMU_printf("----------------------\n");
}
void Ejpg::run2(uint8_t *memory8,
	uint16_t *memory16,
	uint32_t options,
	uint32_t dst,
	uint32_t w,
	uint32_t h,
	uint32_t format,
	uint32_t ri,
	uint32_t tq,
	uint32_t tda,
	uint32_t feed)
{
#if FT800EMU_COPROCESSOR_DEBUG
	FTEMU_printf("Ejpg::run2\n");
#endif

	if (stim)
		fprintf(stim, "%08x\n", feed);

	int bs;
	if (format == 0x11112200)
		bs = 6 * 64;
	else if (format == 0x11111100)
		bs = 3 * 64;
	else if (format == 0x11112100)
		bs = 4 * 64;
	else if (format == 0x00001100)
		bs = 1 * 64;
	else
		assert(0);
	// FTEMU_printf("format=%08x state=%d bs=%d\n", format, state, bs);

	int sfeed = (int32_t)feed >> 3;
	if (sfeed < -128)
		feed = 0;
	else if (sfeed > 127)
		feed = 255;
	else
		feed = sfeed + 128;

	if (0 <= state && (state < bs))
		store[state++] = feed;
	if (state == bs)
	{

		int bw, bh;
		uint32_t ex, ey;
		uint8_t y = 0;
		int8_t Cb = 0, Cr = 0;

		switch (format)
		{

		case 0x11112200:
			bw = 16;
			bh = 16;
			break;

		case 0x11112100:
			bw = 16;
			bh = 8;
			break;

		case 0x11111100:
		case 0x00001100:
			bw = 8;
			bh = 8;
			break;

		default:
			bw = 0;
			bh = 0;
		}

		for (int Y = 0; Y < bh; Y++)
			for (int X = 0; X < bw; X++)
			{
				switch (format)
				{

				case 0x11112200:
				{
					int YB = (X >> 3) + 2 * (Y >> 3);
					int cX = X >> 1, cY = Y >> 1;
					y = store[64 * YB + 8 * (Y & 7) + (X & 7)];
					Cb = store[4 * 64 + 8 * cY + cX] - 128;
					Cr = store[5 * 64 + 8 * cY + cX] - 128;
					break;
				}

				case 0x11112100:
				{
					int YB = (X >> 3);
					int cX = X >> 1;
					y = store[64 * YB + 8 * (Y & 7) + (X & 7)];
					Cb = store[2 * 64 + 8 * Y + cX] - 128;
					Cr = store[3 * 64 + 8 * Y + cX] - 128;
					break;
				}

				case 0x11111100:
					y = store[8 * (Y & 7) + (X & 7)];
					Cb = store[1 * 64 + 8 * Y + X] - 128;
					Cr = store[2 * 64 + 8 * Y + X] - 128;
					break;

				case 0x00001100:
					y = store[8 * (Y & 7) + (X & 7)];
					Cb = 0;
					Cr = 0;
					break;

				}

				ex = (bw * bx + X);
				ey = (bh * by + Y);

				if ((ex < w) && (ey < h))
				{
					if (options & OPT_MONO)
					{
						uint32_t a = dst + (ey * w + ex);
						if (a < 0 || a >= (FT800EMU_RAM_SIZE))
						{
							FTEMU_error("Jpeg illegal memory access");
							return;
						}
						memory8[a] = y;
					}
					else
					{
						uint32_t a = dst + 2 * (ey * w + ex);
						if ((a >> 1) < 0 || (a >> 1) >= (FT800EMU_RAM_SIZE >> 1))
						{
							FTEMU_error("Jpeg illegal memory access");
							return;
						}
						memory16[a >> 1] = rgb565(y, Cb, Cr);
					}
				}
			}

		state = 0;
		bx++;
		if (bx == ((w + (bw - 1)) / bw))
		{
			bx = 0;
			by++;
			if (by == ((h + (bh - 1)) / bh))
			{
				// FTEMU_printf("--- finished ---\n");
				// memory8[REG_EJPG_DAT] = 1;
			}
		}
		// FTEMU_printf("bx=%d by=%d\n", bx, by);
	}
}

void Ejpg::idct()
{
	size_t i;
	idct_t xp[64];

	for (i = 0; i < 8; i++)
	{
		idct8(xp + 8 * i, aa + 8 * i, 1);
	}
	for (i = 0; i < 8; i++)
	{
		idct8(aa + i, xp + i, 8);
	}
}
void Ejpg::run(uint8_t *memory8,
	uint16_t *memory16,
	uint32_t options,
	uint32_t dst,
	uint32_t w,
	uint32_t h,
	uint32_t format,
	uint32_t ri,
	uint32_t qt,
	uint32_t tda,
	uint8_t  q[2 * 64],
	int bitsize,
	uint32_t feed)
{
#if FT800EMU_COPROCESSOR_DEBUG
	FTEMU_printf("Ejpg::run\n");
#endif

	// assert((bitsize == 8) | (bitsize == 32));
	if (!((bitsize == 8) | (bitsize == 32)))
	{
		FTEMU_error("Jpeg bitsize invalid (%i)", (int)bitsize);
		return;
	}

	const char *btypes = NULL;
	if (format == 0x11112200)
		btypes = "111123";
	else if (format == 0x11111100)
		btypes = "123";
	else if (format == 0x11112100)
		btypes = "1123";
	else if (format == 0x00001100)
		btypes = "1";
	// assert(btypes != NULL);
	if (!btypes)
	{
		FTEMU_error("Jpeg btypes invalid (NULL)");
		return;
	}

	memory8[REG_EJPG_DAT] = 0;  // not finished yet
	if (bitsize == 8)
	{
		if (feed == 0xff)
		{
			expect_ff = 1;
			return;
		}
		if (expect_ff)
		{
			expect_ff = 0;
			if (feed == 0x00)
				feed = 0xff;
			else {
				// restart marker, next segment, etc. So ignore
				// FTEMU_printf("DISCARD %x\n", feed & 0xff);
				return;
			}
		}
		bits |= ((uint64_t)(feed & 0xff) << (56 - nbits));
		nbits += 8;
	}
	else
	{
		assert(!expect_ff);
		assert(!isFF(feed));
		assert(!isFF(feed >> 8));
		assert(!isFF(feed >> 16));
		assert(!isFF(feed >> 24));

		bits |= ((uint64_t)(feed & 0xff) << (56 - nbits));
		nbits += 8;
		bits |= ((uint64_t)((feed >> 8) & 0xff) << (56 - nbits));
		nbits += 8;
		bits |= ((uint64_t)((feed >> 16) & 0xff) << (56 - nbits));
		nbits += 8;
		bits |= ((uint64_t)((feed >> 24) & 0xff) << (56 - nbits));
		nbits += 8;
	}
	assert(nbits <= 64);

	while (1)
	{
		blk_type = btypes[block] - '0';

		// FTEMU_printf("\n%2d: bits=%016llx\n", nbits, bits);

		uint8_t da8 = tda >> (8 * blk_type);  // DC in bit 4, AC in bit 0
		// FTEMU_printf("blk_type = %d %08x %02x\n", blk_type, tda, da8);
		size_t ht;
		int isdc = idct_i == 0;
		if (isdc)
			ht = REG_EJPG_HT + 64 * (da8 >> 4);
		else
			ht = REG_EJPG_HT + 64 * (2 | (da8 & 1));
		// FTEMU_printf("ht = %x\n", ht);
		if (ht < 0 || ht >= (FT800EMU_RAM_SIZE))
		{
			FTEMU_error("Jpeg illegal memory access");
			return;
		}
		uint32_t *pht = (uint32_t*)&memory8[ht];
		size_t codesize;
		uint16_t b = ~0;
		for (codesize = 1; codesize <= std::min(nbits, (size_t)16); codesize++, pht++)
		{
			b = (bits >> (64 - codesize)) & 0xFFFF;
			b &= (1 << codesize) - 1;
			uint16_t hi = *pht >> 16;
			// FTEMU_printf("[%d: b=%x hi=%x] ", codesize, b, hi);
			// FTEMU_printf("%2lu: %08x %d\n", codesize, *pht, b);
			if (b < hi)
				break;
		}
		if (codesize > nbits)
			return; // not enough bits yet

		// have CODESIZE bits in B
		// FTEMU_printf("codesize %lu\n", codesize);
		// FTEMU_printf("symbol at %u\n", 0xff & (b + *pht));
		uint8_t sym_pos = 0xff & (b + *pht);
		size_t symbols;
		if (isdc)
			symbols = REG_EJPG_DCC + 12 * (da8 >> 4);
		else
			symbols = REG_EJPG_ACC + 256 * (da8 & 1);
		if ((symbols + sym_pos) < 0 || (symbols + sym_pos) >= (FT800EMU_RAM_SIZE))
		{
			FTEMU_error("Jpeg illegal memory access");
			return;
		}
		uint8_t symbol = memory8[symbols + sym_pos];
		// FTEMU_printf("symbol %02x\n", symbol);

		size_t dmsize = symbol & 0xf;
		if (nbits < (codesize + dmsize))
			return; // not enough bits yet

		assert(dmsize < 12);
		int16_t dm = (bits >> (64 - codesize - dmsize)) & 0xFFFF;
		dm &= (1 << dmsize) - 1;
		if (dm < (1 << (dmsize - 1)))
			dm = (-(1 << dmsize) ^ dm) + 1;

		// uint32_t consume = bits >> (64 - (codesize + dmsize));
#if 0
		FTEMU_printf("DEBUG: %08x\n", 0x80000000 | symbol);
		if (symbol)
			FTEMU_printf("DEBUG: %08x\n", dm);
#endif
		// FTEMU_printf("%02x,%04x,%02x\n", symbol, 0xffff & dm);

		nbits -= (codesize + dmsize);
		bits <<= (codesize + dmsize);
		run1(memory8,
			memory16,
			options,
			dst,
			w,
			h,
			format,
			ri,
			qt,
			tda,
			q,
			btypes,
			symbol, dm);
	}
	return;
}
void Ejpg::run1(uint8_t *memory8,
	uint16_t *memory16,
	uint32_t options,
	uint32_t dst,
	uint32_t w,
	uint32_t h,
	uint32_t format,
	uint32_t ri,
	uint32_t qt,
	uint32_t tda,
	uint8_t  q[2 * 64],
	const char *btypes,
	uint8_t symbol,
	int16_t dm)
{
#if FT800EMU_COPROCESSOR_DEBUG
	FTEMU_printf("Ejpg::run1\n");
#endif

	// FTEMU_printf("idct_i=%d, symbol %02x\n", idct_i, symbol);
	// FTEMU_printf("SYMBOL %02x,%04x,%02x\n", symbol, 0xffff & dm, idct_i);
	uint8_t runlength = (symbol >> 4) & 15;
	if ((idct_i > 0) && (symbol & 255) == 0)
	{
		idct_i = 64;
	}
	else
	{
		idct_i += runlength;
		size_t qtab = 1 & (qt >> (8 * blk_type));
		uint8_t qi = q[64 * qtab + idct_i];
		// FTEMU_printf("--> %x (q=%d)\n", feed, q[64 * qtab + idct_i]);
		int16_t f = dm;
		if (idct_i == 0)
		{
			// FTEMU_printf("DC %04x+%04x=%04x\n", 0xffff & prevdc[blk_type], 0xffff & f, 0xffff & (f + prevdc[blk_type]));
			f += prevdc[blk_type];
			prevdc[blk_type] = f;
		}
		int16_t feed = f * qi;

		size_t zz[64] = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };
		// FTEMU_printf("Write %04x*%02x=%04x to %02x\n", dm & 0xffff, qi, feed & 0xffff, zz[idct_i]);
		if (idct_i >= 64)
		{
			FTEMU_error("Jpeg idct_i out of bounds (%i)", (int)idct_i);
		}
		else
		{
			aa[zz[idct_i++]] = feed;
		}
	}

	if (idct_i == 64)
	{
		// FTEMU_printf("BLOCK\n");
		// FTEMU_printf("%d    %x %s %x %d\n", blk_type, format, btypes, qt, qtab);
		block = (block + 1) % strlen(btypes);
		idct();
		for (size_t i = 0; i < 64; i++)
			run2(memory8,
			memory16,
			options,
			dst,
			w,
			h,
			format,
			ri,
			qt,
			tda,
			aa[i]);
		startblock();
		if (block == 0)
		{
			if (++restart_i == ri)
			{
				// FTEMU_printf("RESTART %d\n", ri);
				restart_i = 0;
				memset(prevdc, 0, sizeof(prevdc));

				// FTEMU_printf("gulp %d\n", nbits & 7);
				while (nbits & 7)
				{
					// FTEMU_printf("\n%2d: bits=%016llx\n", nbits, bits);
					assert(1 & (bits >> 63));
					nbits--;
					bits <<= 1;
				}
			}
		}
	}
}

BT8XXEMU_FORCE_INLINE void Coprocessor::cpureset()
{
	pc = 0;
	dsp = rsp = 0;
	t = 0;
}

BT8XXEMU_FORCE_INLINE void Coprocessor::push(int v) // push v on the data stack
{
	dsp = 31 & (dsp + 1);
	d[dsp] = t;
	t = v;
}

BT8XXEMU_FORCE_INLINE int Coprocessor::pop() // pop value from the data stack and return it
{
	int v = t;
	t = d[dsp];
	dsp = 31 & (dsp - 1);
	return v;
}

#if FT800EMU_COPROCESSOR_TRACE
static FILE *trace = NULL;
#endif

Coprocessor::Coprocessor(FT8XXEMU::System *system, Memory *memory, const char *romFilePath, BT8XXEMU_EmulatorMode mode)
	: m_System(system), m_Memory(memory)
{
	ejpg.setSystem(system);
	if (romFilePath)
	{
		FILE *f;
		f = fopen(romFilePath, "rb");
		if (!f) FTEMU_error("Failed to open coprocessor ROM file");
		else
		{
			size_t s = fread(j1boot, 1, FT800EMU_COPROCESSOR_ROM_SIZE, f);
			if (s != FT800EMU_COPROCESSOR_ROM_SIZE) FTEMU_error("Incomplete coprocessor ROM file");
			else FTEMU_message("Loaded coprocessor ROM file");
			if (fclose(f)) FTEMU_error("Error closing coprocessor ROM file");
		}
	}
	else
	{
		memcpy(j1boot, pgm_rom_ft810, sizeof(pgm_rom_ft810));
	}

#if FT800EMU_COPROCESSOR_TRACE
	trace = fopen(FT800EMU_COPROCESSOR_TRACEFILE, "w");
	if (!trace) FTEMU_printf("Failed to create trace file\n");
#endif

	ejpg.reset();
	cpureset();

	// memcpy(romtop, j1boot + 0x3c00, 0x800); ?

	// REG(EJPG_READY) = 1;
	m_Memory->rawWriteU32(m_Memory->getRam(), REG_EJPG_READY, 1);
}

// template <bool singleFrame>
void Coprocessor::execute()
{
	// if (!singleFrame)
	m_Running = true;

	uint16_t _pc;
	uint32_t _t;


	do
	{
		if (m_Memory->coprocessorGetReset())
		{
			ejpg.reset();
			cpureset();
			//FTEMU_printf("RESET COPROCESSOR\n");
			FT8XXEMU::System::delay(1);
			continue;
		}
		uint32_t regRomsubSel = m_Memory->rawReadU32(m_Memory->getRam(), REG_ROMSUB_SEL);
		// uint32_t regJ1Cold = m_Memory->rawReadU32(m_Memory->getRam(), REG_J1_COLD);

#if FT800EMU_COPROCESSOR_DEBUG
		//FTEMU_printf("pc: %i, 0x%x\n", (int)pc, (int)pc);
		switch (pc)
		{
		case 0x04CB:
			FTEMU_printf("COPROCESSOR: throw\n");
			break;
		case 0x2D5A:
			FTEMU_printf("func:playvideo\n");
			FTEMU_printf("\tCALL mjpeg-DHT\n");
			break;
		case 0x2D5B:
			FTEMU_printf("\tCALL jpgavi-common\n");
			break;
		case 0x2D5C:
			FTEMU_printf("\tCALL avi-prime\n");
			break;
		case 0x2D5D:
			FTEMU_printf("\tCALL avi-chunk\n");
			break;
		case 0x2D64:
			FTEMU_printf("\tJUMP avi-cleanup\n");
			break;
		/*case 0x2C3A:
			FTEMU_printf("\t0x2C3A N==T (N: 0x%x, T: 0x%x)\n", (unsigned int)d[dsp], (unsigned int)t);
			break;
		case 0x2C3C:
			FTEMU_printf("\t0x2C3C\n");
			break;
		case 0x2C70:
			FTEMU_printf("\t0x2C70\n");
			break;
		case 0x2C75:
			FTEMU_printf("\t0x2C75 N==T (N: 0x%x, T: 0x%x)\n", (unsigned int)d[dsp], (unsigned int)t);
			break;
		case 0x2C77:
			FTEMU_printf("\t0x2C77\n");
			break;*/
		}
#endif
		uint16_t insn = ((regRomsubSel == 3) && (pc >= 0x3c00))
			? m_Memory->rawReadU16(m_Memory->getRam(), RAM_ROMSUB + ((pc - 0x3c00) * 2))
			: j1boot[pc];
		_pc = pc + 1;

#if FT800EMU_COPROCESSOR_DEBUG
		if (pc == 0x3fff)
			FTEMU_printf("NEW ENTRYPOINT\n");
#endif

		switch (insn >> 13)
		{
		case 4:
		case 5: // literal
			push(insn & 0x3fff);
			break;
		case 0: // jump
		case 1:
			_pc = insn & 0x3fff;
			break;
		case 7: // conditional jump
			if (pop() == 0)
				_pc = (pc & 0x2000) | (insn & 0x1fff);
			break;
		case 2: // call
		case 3:
			rsp = 31 & (rsp + 1);
			r[rsp] = _pc << 1;
#if FT800EMU_COPROCESSOR_DEBUG
			FTEMU_printf("(call) r[rsp] = r[%i] = %i\n", (uint32_t)rsp, (uint32_t)r[rsp]);
#endif
			_pc = insn & 0x3fff;
			break;
		case 6: // ALU
			if (insn & 0x80)
			{ /* R->PC */
				if ((r[rsp] & 1) || (r[rsp] > 32767))
				{
					FTEMU_warning("Coprocessor invalid return address [%d] %08x", rsp, r[rsp]);
				}
				_pc = r[rsp] >> 1;
			}
			uint32_t n = d[dsp];
			switch ((insn >> 8) & 0x1f)
			{
			case 0x00:  _t = t; break;
			case 0x01:  _t = n; break;
			case 0x02:  _t = t + n; break;
			case 0x03:  _t = t & n; break;
			case 0x04:  _t = t | n; break;
			case 0x05:  _t = t ^ n; break;
			case 0x06:  _t = ~t; break;
			case 0x07:
				_t = -(t == n);
#if FT800EMU_COPROCESSOR_DEBUG
				FTEMU_printf("\tN==T (N: 0x%x, T: 0x%x)\n", (unsigned int)n, (unsigned int)t);
#endif
				break;
			case 0x08:  _t = -((int32_t)n < (int32_t)t); break;
			case 0x09:  _t = ((int32_t)(n)) >> t; break;
			case 0x0a:  _t = t - 1; break;
			case 0x0b:  _t = r[rsp]; break;
			case 0x0c:  _t = memrd; break;
			case 0x0d:  _t = t * n; break;
			case 0x0e:  _t = (n << 14) | (t & 0x3fff); break;
			case 0x0f:  _t = -(n < t); break;
			case 0x10:  _t = t + 4;
				m_Memory->coprocessorWriteU32(REG_CRC, crc32(m_Memory->coprocessorReadU32(REG_CRC), memrd)); break;
			case 0x11:  _t = n << t; break;
			case 0x12:  _t = /*(int8_t)*/m_Memory->coprocessorReadU8(t); /*memory8[t]*/; break;
			case 0x13:  assert((t & 1) == 0);
				_t = /*(int16_t)*/m_Memory->coprocessorReadU16(t); /*memory16[t >> 1];*/ break;
			case 0x14:  _t = PRODUCT64(t, n) >> 32; break;
			case 0x15:  _t = PRODUCT64(t, n) >> 16; break;
			case 0x16:  _t = (t & ~0xfff) | ((t + 4) & 0xfff); break;
			case 0x17:  _t = n - t; break;
			case 0x18:  _t = t + 1; break;
			case 0x19:  assert((t & 1) == 0);
				_t = (int16_t)m_Memory->coprocessorReadU16(t); break;
			case 0x1a:  { uint32_t sum32 = t + n; _t = ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); } break;
			case 0x1b:  switch (t >> 12) {
			case 0: _t = t | RAM_REG; break;
			case 1: _t = t | RAM_J1RAM; break;
			default:;
				// selfassert(0, __LINE__);
			}
						break;
			case 0x1c:  _t = t + 2; break;
			case 0x1d:  _t = t << 1; break;
			case 0x1e:  _t = t + 4; break;
			case 0x1f:  _t = !(isFF(memrd) | isFF(memrd >> 8) | isFF(memrd >> 16) | isFF(memrd >> 24));
				// FTEMU_printf("xmit %08x\n", memrd);
				if (_t)
				{
					uint8_t *memory8 = m_Memory->getRam();
					uint16_t *memory16 = static_cast<uint16_t *>(static_cast<void *>(memory8));
					uint32_t *memory32 = static_cast<uint32_t *>(static_cast<void *>(memory8));
					ejpg.run(memory8,
						memory16,
						REG(EJPG_OPTIONS),
						REG(EJPG_DST),
						REG(EJPG_W),
						REG(EJPG_H),
						REG(EJPG_FORMAT),
						REG(EJPG_RI),
						REG(EJPG_TQ),
						REG(EJPG_TDA),
						&memory8[REG_EJPG_Q],
						32,
						memrd);
				}
			}
			dsp = 31 & (dsp + sx[insn & 3]);
			rsp = 31 & (rsp + sx[(insn >> 2) & 3]);

			switch (7 & (insn >> 4)) {
			case 0:
				break;
			case 1:
				d[dsp] = t;
				break;
			case 2:
				r[rsp] = t;
#if FT800EMU_COPROCESSOR_DEBUG
				FTEMU_printf("(r[rsp] = t) r[rsp] = r[%i] = %i\n", (uint32_t)rsp, (uint32_t)r[rsp]);
#endif
				break;
			case 3:
				break;
			case 4:
				// selfassert((t & 3) == 0, __LINE__);
				if (t == 0x600000)
				{
				// 	FTEMU_printf("DEBUG: %08x\n", n);
				// 	PyList_Append(debugs, PyLong_FromUnsignedLong(n));
					break;
				}
				// assert(t < MEMSIZE);
				// assert((t < (1024 * 1024)) | (t >= RAM_DL));
				// if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
				// 	written[(t - RAM_J1RAM) >> 2] = 1;
				m_Memory->coprocessorWriteU32(t, n); // memory32[t >> 2] = n;
				if (t == REG_EJPG_FORMAT)
				{
					// FTEMU_printf("EJPG Begin\n");
					ejpg.begin();
				}
				assert(t != REG_EJPG_DAT);
				if (t == REG_J1_INT)
				{
					m_Memory->coprocessorWriteU32(REG_INT_FLAGS,
						m_Memory->coprocessorReadU32(REG_INT_FLAGS) | (n << 5));
					// REG(INT_FLAGS) |= (n << 5);
				}
				break;
			case 5:
				// assert((t & 1) == 0);
				// assert(t < MEMSIZE);
				// assert((t < (1024 * 1024)) | (t >= RAM_DL));
				// if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
				// 	written[(t - RAM_J1RAM) >> 2] = 1;
				m_Memory->coprocessorWriteU16(t, n); // memory16[t >> 1] = n;
				break;
			case 6:
				// selfassert(t < MEMSIZE, __LINE__);
				// selfassert((t < (1024 * 1024)) | (t >= RAM_DL), __LINE__);
				m_Memory->coprocessorWriteU8(t, n); // memory8[t] = n;
				//if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
				//	written[(t - RAM_J1RAM) >> 2] = 1;
				if (t == REG_EJPG_DAT)
				{
					// FTEMU_printf("EJPG Data memrd %u\n", (unsigned int)memrd);
					uint8_t *memory8 = m_Memory->getRam();
					uint16_t *memory16 = static_cast<uint16_t *>(static_cast<void *>(memory8));
					uint32_t *memory32 = static_cast<uint32_t *>(static_cast<void *>(memory8));
					ejpg.run(memory8,
						memory16,
						REG(EJPG_OPTIONS),
						REG(EJPG_DST),
						REG(EJPG_W),
						REG(EJPG_H),
						REG(EJPG_FORMAT),
						REG(EJPG_RI),
						REG(EJPG_TQ),
						REG(EJPG_TDA),
						&memory8[REG_EJPG_Q],
						8,
						n);
				}
				break;
			case 7:
				// selfassert(t < MEMSIZE, __LINE__);
				// if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
				// 	selfassert(written[(t - RAM_J1RAM) >> 2], __LINE__);
				memrd = m_Memory->coprocessorReadU32(t);
				break;
			}
			t = _t;
			break;
		}
		pc = _pc;
		/*maskregs();
		if (memory8[REG_DLSWAP] != 0) {
			memory8[REG_DLSWAP] = 0;
		}*/

#if FT800EMU_COPROCESSOR_TRACE
		fprintf(trace, "pc=%04x t=%08x insn=%04x\n", pc, t, insn);
#if FT800EMU_COPROCESSOR_DEBUG
		FTEMU_printf("pc=%04x t=%08x insn=%04x\n", pc, t, insn);
#endif
#endif

		//REG(CLOCK)++;

		/*compute_space();
		write_checks();*/

		// increment REG_FRAMES for animation
		/*if (++screenclocks > screensize) {
			screenclocks = 0;
			REG(FRAMES)++;
		}*/
		//REG(RASTERY) = (screenclocks > (REG(PCLK) * REG(HCYCLE) * REG(VSIZE))) ? -1 : 0;

		// support touch_fake() scheme
		/*if (REG(TOUCH_MODE) != touchflip) {
			touchflip = REG(TOUCH_MODE);
			uint32_t xy = ((0xffff & REG(TOUCH_TRANSFORM_A)) << 16) | REG(TOUCH_CHARGE);
			REG(TOUCH_SCREEN_XY) = xy;
			REG(TOUCH_RAW_XY) = xy;
		}*/
		/*size_t t0 = memory8[REG_CLOCK] & HMASK;
		history[t0].pc = pc;
		history[t0].rsp = rsp;
		if (pc > 16383) {
			FTEMU_printf("PC escape\n");
			for (size_t i = ((t0 + 1) & HMASK); i != t0; i = (i + 1) & HMASK)
				FTEMU_printf("%04X rsp=%d\n", history[i].pc, history[i].rsp);
			FTEMU_printf("Final PC %04X rsp=%d\n", pc, rsp);
		}
		assert(pc < 16384);*/

	} while (m_Running);


	/*if (!singleFrame)
		s_Running = true;

		int _pc, _t, n;
		int insn;

		int swapped = 0;
		int starve = 0;
		do {
		if (m_Memory->coprocessorGetReset())
		{
		pc = 0;
		//FTEMU_printf("RESET COPROCESSOR\n");
		FT8XXEMU::System.delay(1);
		continue;
		}
		insn = pgm[pc];
		// FTEMU_printf("PC=%04x %04x\n", pc, insn);
		// if (pc == 0x1BA6) FTEMU_printf("COMMAND [%03x] %08x\n", m_Memory->coprocessorReadU32(REG_CMD_READ), t);
		if (singleFrame)
		{
		if (pc == 0x0980) { // cmd.has1
		// 0x1090f8 is the location in coprocessor private RAM where the read pointer is cached.
		int rp = m_Memory->coprocessorReadU32(0x1090f8);
		// FTEMU_printf("cmd.has1 %x %x\n", m_Memory->coprocessorReadU32(REG_CMD_WRITE), rp);
		starve = m_Memory->coprocessorReadU32(REG_CMD_WRITE) == rp;
		}
		}
		_pc = pc + 1;
		if (insn & 0x8000) { // literal
		push(insn & 0x7fff);
		} else {
		#define F8CP_TARGET (insn & 0x1fff)
		#define F8CP_INSN_6_4 (7 & (insn >> 4))

		switch (insn >> 13) {
		case 0: // jump
		_pc = F8CP_TARGET;
		break;
		case 1: // conditional jump
		if (pop() == 0)
		_pc = F8CP_TARGET;
		break;
		case 2: // call
		rsp = 31 & (rsp + 1);
		r[rsp] = _pc << 1;
		_pc = F8CP_TARGET;
		break;
		case 3: // ALU
		if (insn & (1 << 7)) /* R->PC * /
		_pc = r[rsp] >> 1;
		n = d[dsp];
		uint64_t product = (int64_t)(int32_t)t * (int64_t)(int32_t)n;
		uint32_t sum32 = t + n;

		switch ((insn >> 8) & 31) {
		case 0:     _t = t; break;
		case 1:     _t = n; break;
		case 2:     _t = t + n; break;
		case 3:     _t = t & n; break;
		case 4:     _t = t | n; break;
		case 5:     _t = t ^ n; break;
		case 6:     _t = ~t; break;
		case 7:     _t = -(t == n); break;
		case 8:     _t = -((int32_t)n < (int32_t)t); break;
		case 9:     _t = n >> t; break;
		case 10:    _t = t - 1; break;
		case 11:    _t = r[rsp]; break;
		case 12:    _t = m_Memory->coprocessorReadU32(t & ~3); /*FTEMU_printf("rd[%x] = %x\n", t, _t);* / break;
		case 13:    _t = product; break;
		case 14:    _t = (n << 15) | (t & 0x7fff); break;
		case 15:    _t = -(n < t); break;
		case 16:    assert(0);
		case 17:    _t = n << t; break;
		case 18:    _t = m_Memory->coprocessorReadU8(t); /*FTEMU_printf("rd8[%x] = %x\n", t, _t);* / break;
		case 19:    _t = m_Memory->coprocessorReadU16(t & ~1); break;
		case 20:    _t = product >> 32; break;
		case 21:    _t = product >> 16; break;
		case 22:    _t = t == r[rsp]; break;
		case 23:    _t = n - t; break;
		case 24:    _t = t + 1; break;
		case 25:    _t = (int16_t)m_Memory->coprocessorReadU16(t); break;
		case 26:    _t = (sum32 & 0x80000000) ? 0 : ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); break;
		case 27:    _t = (0x00109 << 12) | (t & 0xfff); break;
		case 28:    _t = t + 2; break;
		case 29:    _t = t << 1; break;
		case 30:    _t = t + 4; break;
		case 31:    assert(0);
		}
		dsp = 31 & (dsp + sx[insn & 3]);
		rsp = 31 & (rsp + sx[(insn >> 2) & 3]);

		switch (F8CP_INSN_6_4)
		{
		case 1: // T_N
		d[dsp] = t;
		break;
		case 2: // T_R
		r[rsp] = t;
		break;
		case 3: // Rinc
		++r[rsp];
		break;
		case 4: // write32
		// FTEMU_printf("wr[%x] <= %x\n", t, n);
		m_Memory->coprocessorWriteU32(t, n);
		if (singleFrame)
		{
		if (t == REG_DLSWAP)
		swapped = 1;
		}
		break;
		case 5: // write16
		m_Memory->coprocessorWriteU16(t, n);
		break;
		case 6: // write8
		m_Memory->coprocessorWriteU8(t, n);
		break;
		case 7: // read
		// no-op
		break;
		}

		t = _t;
		break;
		}
		}
		pc = _pc;
		// fflush(stdout);
		} while (singleFrame ? (!swapped && !starve) : s_Running);
		// FTEMU_printf("coprocessor done\n");*/
}

void Coprocessor::executeManual()
{
	// execute<true>();
}

void Coprocessor::executeEmulator()
{
	// execute<false>();
	execute();
}

void Coprocessor::stopEmulator()
{
	m_Running = false;
}

Coprocessor::~Coprocessor()
{

}

} /* namespace FT810EMU */
#endif /* #ifdef FT810EMU_MODE */

/* end of file */
