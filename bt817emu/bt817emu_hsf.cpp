/*
BT817 Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Based on class-hsf.py
Author: Jan Boon <jan@no-break.space>
*/

#pragma warning(push)
#pragma warning(disable : 26495) // Uninitialized member variables

#include "bt817emu_hsf.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"
#include "ft800emu_memory.h"
#include <memory.h>
#include <algorithm>
#include <assert.h>

namespace BT817EMU {

const int c_Prec = 11;

BT8XXEMU_FORCE_INLINE int fixMul(const int x, const int y)
{
	const long long p = (long long)x * (long long)y;
	const long long a = (1 << (c_Prec - 1));
	return (int)((p + a) >> c_Prec);
}

Hsf::Hsf(Memory *memory)
    : m_RegHsf(static_cast<HsfRegisters *>(static_cast<void *>(memory->getRam() + REG_HSF_HSIZE)))
{
	// ...
}

void Hsf::fLine(HsfTable *const res, const int hsfHSize, const int hsize) // (x1, y1)
{
	int de = hsize - hsfHSize; // Delta (hsize example 862 - render pixels, hsfHSize example 800 - display pixels)
	int e = 0;
	int y = 0;
	int hx1 = hsfHSize >> 1; // Half hsfHSize
	int regFt1 = m_RegHsf->FT1;
	for (int x = 0; x < hsfHSize; ++x)
	{
		int v = ((e * regFt1) + (1 << (c_Prec - 1))) >> c_Prec;
		res[x] = HsfTable(y, v);
		e += de;
		y += 1;
		if (e > hx1)
		{
			y += 1;
			e -= hsfHSize;
		}
	}
}

void Hsf::fCalculateContrib2(HsfTable *const res, const int dstX, const int hsize)
{
	int center = m_Fbrestab[dstX].Y; // Source center
	int ee = m_Fbrestab[dstX].V; // Signed fixed point N.11
	int edge = hsize - 1; // Source edge
	int fScale = m_RegHsf->FScale;
	res[0] = HsfTable(std::max(0, (center - 2)), fMitchell2(ee + (fScale << 1)));
	res[1] = HsfTable(std::max(0, (center - 1)), fMitchell2(ee + fScale));
	res[2] = HsfTable((center), fMitchell2(abs(ee)));
	res[3] = HsfTable(std::min(edge, (center + 1)), fMitchell2((fScale - ee)));
	res[4] = HsfTable(std::min(edge, (center + 2)), fMitchell2(((fScale << 1) - ee)));
}

int Hsf::fMitchell2(int t)
{
	// assert(t >= 0);
	int t2 = fixMul(t, t);
	int t3 = fixMul(t, t2);
	if (t < (1 << c_Prec)) // 1.0
	{
		// print('--->', F03 * t3)
		return fixMul(m_F03, t3)
		    + fixMul(m_F02, t2)
		    + m_F00;
	}
	else if (t < (2 << c_Prec)) // 2.0
	{
		return fixMul(m_F13, t3)
		    + fixMul(m_F12, t2)
		    + fixMul(m_F11, t)
		    + m_F10;
	}
	return 0;
}

void Hsf::init(const uint32_t hsfHSize, const uint32_t hsize)
{
	// Extend fixed point sign
	m_F00 = ((int)m_RegHsf->F00 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F02 = ((int)m_RegHsf->F02 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F03 = ((int)m_RegHsf->F03 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F10 = ((int)m_RegHsf->F10 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F11 = ((int)m_RegHsf->F11 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F12 = ((int)m_RegHsf->F12 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);
	m_F13 = ((int)m_RegHsf->F13 << (sizeof(int) * 8 - 14)) >> (sizeof(int) * 8 - 14);

	// Initialize tables
	fLine(m_Fbrestab, hsfHSize, hsize);
	for (int j = 0; j < (int)hsfHSize; ++j)
		fCalculateContrib2(m_Cc[j], j, hsize);
}
void Hsf::apply(argb8888 *const dst, const uint32_t hsfHSize, const argb8888 *const src, const uint32_t hsize)
{
	uint8_t *dst8 = reinterpret_cast<uint8_t *>(dst);
	int dst8Sz = hsfHSize << 2;
	const uint8_t *src8 = reinterpret_cast<const uint8_t *>(src);
	int src8Sz = hsize << 2;

	for (int x = 0; x < (int)hsfHSize; ++x)
	{
		for (int c = 0; c < 4; ++c)
		{
#if 0
			// Verify m_Fbrestab
			const int y = m_Fbrestab[x].Y;
			dst8[x * 4 + c] = src8[y * 4 + c];
#else
			// Apply filter
			const HsfTable *ct = m_Cc[x];
			int res = 1 << (c_Prec - 1); // Signed fixed point N.11, .5 for rounding
			for (int j = 0; j < 5; ++j)
			{
				int i = ct[j].Y;
				int w = ct[j].V;
				res += src8[i * 4 + c] * w; // Add weighted (filtered) value
			}
			dst8[x * 4 + c] = (uint8_t)std::min(std::max(0, res >> c_Prec), 255);
#endif
		}
	}
}

} /* namespace BT817EMU */

#pragma warning(pop)
#pragma warning(disable : 26495)

/* end of file */
