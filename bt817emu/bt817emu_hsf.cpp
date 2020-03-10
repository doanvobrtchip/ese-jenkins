/*
BT817 Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Based on class-hsf.py
Author: Jan Boon <jan@no-break.space>
*/

#pragma warning(push)
// #pragma warning(disable : 26495) // Uninitialized member variables

#include "bt817emu_hsf.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"
#include "ft800emu_memory.h"
#include <memory.h>
#include <algorithm>
#include <assert.h>

namespace BT817EMU {

const int c_Prec = 11;

Hsf::Hsf(Memory *memory)
    : m_RegHsf(static_cast<HsfRegisters *>(static_cast<void *>(memory->getRam() + REG_HSF_HSIZE)))
{
	// ...
}

void Hsf::fLine(HsfFbrestab *res, const int hsfHSize, const int hsize) // (x1, y1)
{
	int de = hsize - hsfHSize; // Delta (hsize example 862 - render pixels, hsfHSize example 800 - display pixels)
	int e = 0;
	int y = 0;
	int hx1 = hsfHSize >> 1; // Half hsfHSize
	int regFt1 = m_RegHsf->FT1;
	for (int x = 0; x < hsfHSize; ++x)
	{
		int v = ((e * regFt1) + (1 << (c_Prec - 1))) >> c_Prec;
		res[x] = HsfFbrestab(y, v);
		e += de;
		y += 1;
		if (e > hx1)
		{
			y += 1;
			e -= hsfHSize;
		}
	}
}

void Hsf::init(const uint32_t hsfHSize, const uint32_t hsize)
{
	fLine(m_Fbrestab, hsfHSize, hsize); // Initialize table ?
}

} /* namespace BT817EMU */

#pragma warning(pop)
#pragma warning(disable : 26495)

/* end of file */
