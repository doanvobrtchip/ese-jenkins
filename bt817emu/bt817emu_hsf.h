/*
BT817 Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Based on class-hsf.py
Author: Jan Boon <jan@no-break.space>
*/

#ifndef BT817EMU_HSF_H
#define BT817EMU_HSF_H

#include "bt8xxemu.h"

#define FT800EMU_SCREEN_WIDTH_MAX 2048

namespace BT817EMU {
class Memory;

struct HsfRegisters
{
	uint32_t HSize; // UInt
	uint32_t FT1; // UInt, 13 bits
	uint32_t FScale; // Unsigned 0.11 fixed point
	int32_t F00; // Signed 3.11 fixed point, 14 bits
	int32_t F02;
	int32_t F03;
	int32_t F10;
	int32_t F11;
	int32_t F12;
	int32_t F13;
};

struct HsfFbrestab
{
	HsfFbrestab(int y, int v) : V(y), Y(v) { }
	int Y;
	int V;
};

class Hsf
{
private:
	const HsfRegisters *const m_RegHsf;
	HsfFbrestab m_Fbrestab[FT800EMU_SCREEN_WIDTH_MAX]; // Lookup table for source coordinate
	void *m_Cc; // ?

	void fLine(HsfFbrestab *res, const int hsfHSize, const int hsize);

public:
	Hsf(Memory *const memory);

	void init(const uint32_t hsfHSize, const uint32_t hsize);
	void apply(argb8888 *const dst, const uint32_t hsfHSize, const argb8888 *const src, const uint32_t hsize);
};

} /* namespace BT817EMU */

#endif /* #ifndef BT817EMU_HSF_H */

/* end of file */
