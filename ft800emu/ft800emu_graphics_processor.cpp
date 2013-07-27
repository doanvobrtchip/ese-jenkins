/**
 * GraphicsProcessorClass
 * $Id$
 * \file ft800emu_graphics_processor.cpp
 * \brief GraphicsProcessorClass
 * \date 2013-06-22 09:29GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_graphics_processor.h"

// System includes
#include <stdio.h>
#include <math.h>
#include <stack>

// Project includes
#include "ft800emu_memory.h"
#include "ft800emu_graphics_driver.h"
#include "vc.h"

// using namespace ...;

#define FT800EMU_DL_VERTEX2F 1
#define FT800EMU_DL_VERTEX2II 2
#define FT800EMU_DL_DISPLAY 0
#define FT800EMU_DL_BITMAP_SOURCE 1
#define FT800EMU_DL_CLEAR_COLOR_RGB 2
#define FT800EMU_DL_TAG 3
#define FT800EMU_DL_COLOR_RGB 4
#define FT800EMU_DL_BITMAP_HANDLE 5
#define FT800EMU_DL_CELL 6
#define FT800EMU_DL_BITMAP_LAYOUT 7
#define FT800EMU_DL_BITMAP_SIZE 8
#define FT800EMU_DL_ALPHA_FUNC 9
#define FT800EMU_DL_STENCIL_FUNC 10
#define FT800EMU_DL_BLEND_FUNC 11
#define FT800EMU_DL_STENCIL_OP 12
#define FT800EMU_DL_POINT_SIZE 13
#define FT800EMU_DL_LINE_WIDTH 14
#define FT800EMU_DL_CLEAR_COLOR_A 15
#define FT800EMU_DL_COLOR_A 16
#define FT800EMU_DL_CLEAR_STENCIL 17
#define FT800EMU_DL_CLEAR_TAG 18
#define FT800EMU_DL_STENCIL_MASK 19
#define FT800EMU_DL_TAG_MASK 20
#define FT800EMU_DL_BITMAP_TRANSFORM_A 21
#define FT800EMU_DL_BITMAP_TRANSFORM_B 22
#define FT800EMU_DL_BITMAP_TRANSFORM_C 23
#define FT800EMU_DL_BITMAP_TRANSFORM_D 24
#define FT800EMU_DL_BITMAP_TRANSFORM_E 25
#define FT800EMU_DL_BITMAP_TRANSFORM_F 26
#define FT800EMU_DL_SCISSOR_XY 27
#define FT800EMU_DL_SCISSOR_SIZE 28
#define FT800EMU_DL_CALL 29
#define FT800EMU_DL_JUMP 30
#define FT800EMU_DL_BEGIN 31
#define FT800EMU_DL_COLOR_MASK 32
#define FT800EMU_DL_END 33
#define FT800EMU_DL_SAVE_CONTEXT 34
#define FT800EMU_DL_RESTORE_CONTEXT 35
#define FT800EMU_DL_RETURN 36
#define FT800EMU_DL_MACRO 37
#define FT800EMU_DL_CLEAR 38

#define FT800EMU_DEBUG_AA 0
#define FT800EMU_DEBUG_LINES_WITHOUT_ENDINGS 0
#define FT800EMU_DEBUG_DISABLE_OVERLAP 0
#define FT800EMU_DEBUG_LINES_SHIFT_HACK 1
#define FT800EMU_DEBUG_ALPHA_MUL 0
#define FT800EMU_DEBUG_RECTS_MATH 1

namespace FT800EMU {

GraphicsProcessorClass GraphicsProcessor;

namespace {

#pragma region Graphics State

struct GraphicsState
{
public:
	GraphicsState()
	{
		DebugDisplayListIndex = 0;
		ColorARGB = 0xFFFFFFFF;
		PointSize = 16;
		LineWidth = 16;
		ScissorX.I = 0;
		ScissorY.I = 0;
		ScissorWidth = 512;
		ScissorHeight = 512;
		ScissorX2.I = 512;
		ScissorY2.I = 512;
		ClearColorARGB = 0x00000000;
		ClearStencil = 0;
		ClearTag = 0;
		Tag = 0;
		TagMask = true;
		StencilMask = 0xFF;
		StencilFunc = ALWAYS;
		StencilFuncRef = 0x00;
		StencilFuncMask = 0xFF;
		StencilOpPass = KEEP;
		StencilOpFail = KEEP;
		ColorMaskARGB = 0xFFFFFFFF;
		BitmapHandle = 0;
		Cell = 0;
		BitmapTransformA = 256;
		BitmapTransformB = 0;
		BitmapTransformC = 0;
		BitmapTransformD = 0;
		BitmapTransformE = 256;
		BitmapTransformF = 0;
		BlendFuncSrc = SRC_ALPHA;
		BlendFuncDst = ONE_MINUS_SRC_ALPHA;
		AlphaFunc = ALWAYS;
		AlphaFuncRef = 0;
	}
	
	int DebugDisplayListIndex;
	argb8888 ColorARGB;
	int PointSize;
	int LineWidth;
	union { uint32_t U; int I; } ScissorX;
	union { uint32_t U; int I; } ScissorY;
	uint32_t ScissorWidth;
	uint32_t ScissorHeight;
	union { uint32_t U; int I; } ScissorX2; // min(hsize, X + Width)
	union { uint32_t U; int I; } ScissorY2; // Y + Height
	argb8888 ClearColorARGB;
	uint8_t ClearStencil;
	uint8_t ClearTag;
	uint8_t Tag;
	uint8_t StencilMask;
	int StencilFunc; // Not in the programmer guide's graphics state table
	uint8_t StencilFuncRef; // Not in the programmer guide's graphics state table
	uint8_t StencilFuncMask; // Not in the programmer guide's graphics state table
	int StencilOpPass;
	int StencilOpFail;
	bool TagMask;
	argb8888 ColorMaskARGB; // 1,1,1,1 stored as 0xFFFFFFFF
	uint8_t BitmapHandle;
	uint8_t Cell; // Bitmap cell 0-127
	int BitmapTransformA;
	int BitmapTransformB;
	int BitmapTransformC;
	int BitmapTransformD;
	int BitmapTransformE;
	int BitmapTransformF;
	int BlendFuncSrc;
	int BlendFuncDst;
	int AlphaFunc;
	uint8_t AlphaFuncRef;
};

struct BitmapInfo
{
	uint32_t Source;
	int LayoutFormat;
	int LayoutWidth;
	int LayoutStride;
	int LayoutHeight;
	int SizeFilter;
	int SizeWrapX;
	int SizeWrapY;
	int SizeWidth;
	int SizeHeight;
};

BitmapInfo s_BitmapInfo[32];

#pragma endregion

#pragma region Math

__forceinline unsigned int div255(const int &value)
{
	return (value * 257 + 257) >> 16;
}

__forceinline unsigned int mul255div63(const int &value)
{
	return ((value * 16575) + 65) >> 12;
}

__forceinline unsigned int mul255div31(const int &value)
{
	return ((value * 8415) + 33) >> 10;
}

__forceinline unsigned int mul255div3(const int &value)
{
	return value * 85;
}

__forceinline argb8888 mulalpha(const argb8888 &value, const int &alpha)
{
	// todo optimize!
	argb8888 result = div255(((value & 0x00FF0000) >> 16) * alpha);
	result <<= 8;
	result |= div255(((value & 0x0000FF00) >> 8) * alpha);
	result <<= 8;
	result |= div255((value & 0x000000FF) * alpha);
	return result;
}

__forceinline argb8888 mulalpha_argb(const argb8888 &value, const int &alpha)
{
	// todo optimize!
	const argb8888 result = (div255(((value & 0xFF000000) >> 24) * alpha) << 24)
		| (div255(((value & 0x00FF0000) >> 16) * alpha) << 16)
		| (div255(((value & 0x0000FF00) >> 8) * alpha) << 8)
		| (div255((value & 0x000000FF) * alpha) & 0x000000FF);
	return result;
}

__forceinline argb8888 mulalpha_alpha(const argb8888 &value, const int &alpha)
{
	// todo optimize!
	const argb8888 result = (div255(((value & 0xFF000000) >> 24) * alpha) << 24)
		| value & 0x00FFFFFF;
	return result;
}

__forceinline argb8888 mul_argb(const argb8888 &left, const argb8888 &right)
{
	// todo optimize!
	argb8888 result = (div255((left >> 24) * (right >> 24)) << 24)
		| (div255(((left >> 16) & 0xFF) * ((right >> 16) & 0xFF)) << 16)
		| (div255(((left >> 8) & 0xFF) * ((right >> 8) & 0xFF)) << 8)
		| div255((left & 0xFF) * (right & 0xFF));
	return result;
}

__forceinline argb8888 add_argb_safe(const argb8888 &left, const argb8888 &right)
{
	// add rgb and handle overflow
	const argb8888 ag = ((left & 0xFF00FF00) >> 8) + ((right & 0xFF00FF00) >> 8);
	const argb8888 rb = (left & 0x00FF00FF) + (right & 0x00FF00FF);
	const argb8888 agclip = (ag & 0x00FF00FF) << 8;
	const argb8888 rbclip = rb & 0x00FF00FF;
	const argb8888 agover = (ag & 0x01000100) * 0xFF;
	const argb8888 rbover = ((rb & 0x01000100) * 0xFF) >> 8;
	return (agclip | agover) + (rbover | rbclip);
}

#pragma endregion

#pragma region Write Buffer

__forceinline void writeTag(const GraphicsState &gs, uint8_t *bt, int x)
{
	if (gs.TagMask) bt[x] = gs.Tag;
}

__forceinline bool testStencilNoWrite(const GraphicsState &gs, const uint8_t *bs, const int &x)
{
	switch (gs.StencilFunc)
	{
	case NEVER:
		return false;
	case LESS:
		return (bs[x] & gs.StencilFuncMask) < (gs.StencilFuncRef & gs.StencilFuncMask);
	case LEQUAL:
		return (bs[x] & gs.StencilFuncMask) <= (gs.StencilFuncRef & gs.StencilFuncMask);
	case GREATER:
		return (bs[x] & gs.StencilFuncMask) > (gs.StencilFuncRef & gs.StencilFuncMask);
	case GEQUAL:
		return (bs[x] & gs.StencilFuncMask) >= (gs.StencilFuncRef & gs.StencilFuncMask);
	case EQUAL:
		return (bs[x] & gs.StencilFuncMask) == (gs.StencilFuncRef & gs.StencilFuncMask);
	case NOTEQUAL:
		return (bs[x] & gs.StencilFuncMask) != (gs.StencilFuncRef & gs.StencilFuncMask);
	case ALWAYS:
		return true;
	default:
		// error
		printf("Invalid stencil func\n");
		return true;
	}
}

__forceinline bool testStencil(const GraphicsState &gs, uint8_t *bs, const int &x)
{
	bool result = testStencilNoWrite(gs, bs, x);	
	switch (result ? gs.StencilOpPass : gs.StencilOpFail)
	{
	case KEEP:
		break;
	case ZERO:
		bs[x] = (bs[x] & (~gs.StencilMask));
		break;
	case REPLACE:
		bs[x] = (gs.StencilFuncRef & gs.StencilMask) | (bs[x] & (~gs.StencilMask)); // i assume
		break;
	case INCR:
		if (bs[x] < 0xFF) bs[x] = ((bs[x] + 1) & gs.StencilMask) | (bs[x] & (~gs.StencilMask)); // i assume
		break;
	case DECR:
		if (bs[x] > 0x00) bs[x] = ((bs[x] - 1) & gs.StencilMask) | (bs[x] & (~gs.StencilMask)); // i assume
		break;
	case INVERT:
		bs[x] = ((~bs[x]) & gs.StencilMask) | (bs[x] & (~gs.StencilMask));
		break;
	default:
		// error
		printf("Invalid stencil op\n");
		break;
	}
	return result;
}

__forceinline bool testStencil(const GraphicsState &gs, uint8_t *bs, const int &x, const bool &write)
{
	return write ? testStencil(gs, bs, x) : testStencilNoWrite(gs, bs, x);
}

__forceinline argb8888 getPaletted(const uint8_t *ram, const uint8_t &value)
{
	uint32_t result = static_cast<const uint32_t *>(static_cast<const void *>(&ram[RAM_PAL]))[value];
	return result;
}

__forceinline int getAlpha(const int &func, const argb8888 &src, const argb8888 &dst)
{
	switch (func)
	{
	case ZERO:
		return 0;
	case ONE:
		return 255;
	case SRC_ALPHA:
		return src >> 24;
	case DST_ALPHA:
		return dst >> 24;
	case ONE_MINUS_SRC_ALPHA:
		return 255 - (src >> 24);
	case ONE_MINUS_DST_ALPHA:
		return 255 - (dst >> 24);
	}
	printf("Invalid blend func\n");
	return 255;
}

__forceinline bool testAlpha(const GraphicsState &gs, const argb8888 &dst)
{
	switch (gs.AlphaFunc)
	{
	case NEVER:
		return false;
	case LESS:
		return (dst >> 24) < gs.AlphaFuncRef;
	case LEQUAL:
		return (dst >> 24) <= gs.AlphaFuncRef;
	case GREATER:
		return (dst >> 24) > gs.AlphaFuncRef;
	case GEQUAL:
		return (dst >> 24) >= gs.AlphaFuncRef;
	case EQUAL:
		return (dst >> 24) == gs.AlphaFuncRef;
	case NOTEQUAL:
		return (dst >> 24) != gs.AlphaFuncRef;
	case ALWAYS:
		return true;
	}
	printf("Invalid alpha test func\n");
	return true;
}

__forceinline argb8888 blend(const GraphicsState &gs, const argb8888 &src, const argb8888 &dst)
{
	argb8888 result = add_argb_safe(mulalpha_argb(src, getAlpha(gs.BlendFuncSrc, src, dst)), mulalpha_argb(dst, getAlpha(gs.BlendFuncDst, src, dst)));
	return (result & gs.ColorMaskARGB) | (dst & ~gs.ColorMaskARGB);
}

#pragma endregion

#pragma region Primitive: Point

#define FT800EMU_POINT_STENCIL_INCREASE 1

void displayPoint(const GraphicsState &gs, const int ps, const int scx1, const int scy1, const int scx2, const int scy2, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int px, int py)
{
#if FT800EMU_POINT_STENCIL_INCREASE // Add an extra border outside the outer AA for the stencil due to AA differences
	const int stcinc = 8;
	const int x1ps = px + 8 - ps - stcinc; // Top-left inclusive coordinates plus pointsize in 1/16 pixel
	const int y1ps = py + 8 - ps - stcinc;
	const int x2ps = px + 8 + ps + stcinc; // Bottom-right exclusive
	const int y2ps = py + 8 + ps + stcinc;
#else
	const int x1ps = px + 8 - ps; // Top-left inclusive coordinates plus pointsize in 1/16 pixel
	const int y1ps = py + 8 - ps;
	const int x2ps = px + 8 + ps; // Bottom-right exclusive
	const int y2ps = py + 8 + ps;
#endif

	const int x1ps_px = (x1ps >> 4); // Top-left inclusive in screen pixels
	const int y1ps_px = (y1ps >> 4);
	const int x2ps_px = ((x2ps + 15) >> 4); // Bottom-right exclusive in screen pixels
	const int y2ps_px = ((y2ps + 15) >> 4);
	const int xsps_px = x2ps_px - x1ps_px; // Size in screen pixels
	const int ysps_px = y2ps_px - y1ps_px;

	if (max(y1ps_px, scy1) <= y && y < min(y2ps_px, scy2)) // Scissor Y
	{
		const int pssq = ps * ps; // Point size 1/16 squared
		const int psin = ps - 8; // Inner point size 1/16
		const int psout = ps + 8; // Outer
		const int pssqin = (psin) * (psin); // Inner point size 1/16 squared
		const int pssqout = (psout) * (psout); // Outer
#if FT800EMU_POINT_STENCIL_INCREASE
		const int psstc = psout + stcinc;
		const int pssqstc = (psstc) * (psstc);
#endif
		const int y16 = y << 4; // Current Y coordinate in 1/16 pixels

		const int psin256 = psin << 4; // Inner point size 1/256
		
		const int x1ps_px_sc = max(x1ps_px, scx1); // Scissored X1
		const int x2ps_px_sc = min(x2ps_px, scx2); // Scissored X2
		for (int x = x1ps_px_sc; x < x2ps_px_sc; ++x)
		{
			const int x16 = x << 4; // Current X coordinate in 1/16 pixels

			const int dx = x16 - px; // Distance to point center in 1/16 pixels
			const int dy = y16 - py;

			const int distsq = (dx * dx) + (dy * dy);

			if (distsq <= pssqin) // Inside circle
			{
				if (testStencil(gs, bs, x))
				{
					if (psin > 0)
					{
							const argb8888 out = gs.ColorARGB;
							if (testAlpha(gs, out))
							{
								bc[x] = blend(gs, out, bc[x]);
								writeTag(gs, bt, x);
							}
					}
					else // Small point in single pixel
					{
						const int outalpha =  ((gs.ColorARGB >> 24) * ps) >> 3;
						const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
						if (testAlpha(gs, out))
						{
							bc[x] = blend(gs, out, bc[x]);
							writeTag(gs, bt, x);
						}
					}
				}
			}
			else if (distsq <= pssqout) // AA Border
			{
				if (testStencil(gs, bs, x))
				{
					const double dist256sqd = (double)distsq * 256.0; // double.. distsq is 1/16 squared, multiply twice by 16
					const double dist256d = sqrt(dist256sqd); // sqrt..
					const long dist256 = (long)dist256d;
					const int alpha = 256 - max(min(dist256 - psin256, 256), 0);
					const int outalpha = ((gs.ColorARGB >> 24) * alpha) >> 8;
					const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
					if (testAlpha(gs, out))
					{
						bc[x] = blend(gs, out, bc[x]);
						writeTag(gs, bt, x);
					}
				}
			}
#if FT800EMU_POINT_STENCIL_INCREASE
			else if (distsq <= pssqstc)
			{
				testStencil(gs, bs, x);
				const argb8888 out = gs.ColorARGB & 0x00FFFFFF;
				if (testAlpha(gs, out))
				{
					bc[x] = blend(gs, out, bc[x]);
					writeTag(gs, bt, x);
				}
			}
#endif
		}
	}
}

void displayPoint(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int /*hsize*/, int px, int py)
{
	displayPoint(gs, gs.PointSize, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, px, py);	
}

// Utility for primitives that use points to blend the AA at the edges. 
// Only used for 4 pixels at most in a primitive.
// Only used with point size of 16 or larger.
// Only used on the AA border itself.
__forceinline int getPointAlpha256(const int ps, const int x, const int y, const int px, const int py)
{
	const int pssq = ps * ps; // Point size 1/16 squared
	const int psin = ps - 8; // Inner point size 1/16
	const int y16 = y << 4; // Current Y coordinate in 1/16 pixels
	const int x16 = x << 4; // Current X coordinate in 1/16 pixels

	const int psin256 = psin << 4; // Inner point size 1/256

	const int dx = x16 - px; // Distance to point center in 1/16 pixels
	const int dy = y16 - py;

	const int distsq = (dx * dx) + (dy * dy);

	const double dist256sqd = (double)distsq * 256.0; // double.. distsq is 1/16 squared, multiply twice by 16
	const double dist256d = sqrt(dist256sqd); // sqrt..
	const long dist256 = (long)dist256d;
	const int alpha = 256 - max(min(dist256 - psin256, 256), 0);
	return alpha;
}

#pragma endregion

#pragma region Primitive: Bitmap

__forceinline bool wrap(int &value, const int &max, const int &type)
{
	switch (type)
	{
	case BORDER:
		if (value < 0) return false;
		else if (value >= max) return false;
		break;
	case REPEAT:
		value = (value + max * 512) % max; // + max * 512 necessary to get correct negative behaviour	
		break;
	}
	return true;
}

static const argb8888 s_VGAPalette[] = 
{
	0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF, 
};

__forceinline const uint8_t &bmpSrc8(const uint8_t *ram, const uint32_t srci, const int idx)
{
	const int i = (srci + idx) & 0xFFFFF;
	return ram[i];
}

__forceinline const uint8_t &bmpSrc16(const uint8_t *ram, const uint32_t srci, const int idx)
{
	const int i = (srci + idx) & 0xFFFFE;
	return ram[i];
}

// uses pixel units
__forceinline argb8888 sampleBitmapAt(const uint8_t *ram, const uint32_t srci, int x, int y, const int width, const int height, const int format, const int stride, const int wrapx, const int wrapy)
{
	if (!wrap(x, width, wrapx)) return 0x00000000;
	if (format != BARGRAPH) if (!wrap(y, height, wrapy)) return 0x00000000;
	int py = y * stride;
	switch (format)
	{
	case ARGB1555:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + (x << 1))));
			return (((val >> 15) * 255) << 24) // todo opt
				| (mul255div31((val >> 10) & 0x1F) << 16)
				| (mul255div31((val >> 5) & 0x1F) << 8)
				| mul255div31(val & 0x1F);
		}
	case L1:
		{
			int val = (bmpSrc8(ram, srci, py + (x >> 3)) >> (7 - (x % 8))) & 0x1;
			val *= 255;
			return (val << 24) | 0x00FFFFFF; // todo: check alpha behaviour
		}
	case L4:
		{
			int val = (bmpSrc8(ram, srci, py + (x >> 1)) >> (((x + 1) % 2) << 2)) & 0xF;
			val *= 255;
			val /= 15; // todo opt
			return (val << 24) | 0x00FFFFFF; // todo: check alpha behaviour
		}
	case L8:
		{
			uint8_t val = bmpSrc8(ram, srci, py + x);
			return (val << 24) | 0x00FFFFFF; // todo: check alpha behaviour
		}
	case RGB332:
		{
			uint8_t val = bmpSrc8(ram, srci, py + x);
			return 0xFF000000 // todo opt
				| (((val >> 5) * 255 / 7) << 16)
				| ((((val >> 2) & 0x7) * 255 / 7) << 8)
				| mul255div3(val & 0x3);
		}
	case ARGB2:
		{
			uint8_t val = bmpSrc8(ram, srci, py + x);
			return (((val >> 6) * 255 / 3) << 24) // todo opt
				| (mul255div3((val >> 4) & 0x3) << 16)
				| (mul255div3((val >> 2) & 0x3) << 8)
				| mul255div3(val & 0x3);
		}
	case ARGB4:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + (x << 1))));
			return (((val >> 12) * 255 / 15) << 24) // todo opt
				| ((((val >> 8) & 0xF) * 255 / 15) << 16)
				| ((((val >> 4) & 0xF) * 255 / 15) << 8)
				| ((val & 0xF) * 255 / 15);
		}
	case RGB565:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + (x << 1))));
			return 0xFF000000 // todo opt
				| (mul255div31(val >> 11) << 16)
				| (mul255div63((val >> 5) & 0x3F) << 8)
				| mul255div31(val & 0x1F);
		}
	case PALETTED:
		{
			uint8_t val = bmpSrc8(ram, srci, py + x);
			return getPaletted(ram, val);
		}
	case TEXT8X8:
		{
			const int yn = y >> 3;
			py = yn * stride;
			uint8_t c = bmpSrc8(ram, srci, py + (x >> 3));
			const uint8_t *nsrc = &ram[s_BitmapInfo[16 + ((c & 0x80) >> 7)].Source + (8 * (c & 0x7F))];
			const int pyc = y & 0x7;
			const int xc = x & 0x7;
			const uint32_t val = (nsrc[pyc] >> (7 - xc)) & 0x1;
			return (val * 0xFF000000) | 0x00FFFFFF;
		}
	case TEXTVGA:
		{
			const int yn = y >> 4;
			py = yn * stride;
			uint8_t c = bmpSrc8(ram, srci, py + ((x >> 3) << 1)); // Character
			uint8_t ca = bmpSrc8(ram, srci, py + ((x >> 3) << 1) + 1); // Attribute
			const uint8_t *nsrc = &ram[s_BitmapInfo[18 + ((c & 0x80) >> 7)].Source + (16 * (c & 0x7F))]; // PG says it uses 16 and 17, but reference uses 18 and 19
			const int pyc = y & 0xF;
			const int xc = x & 0x7;
			const uint32_t val = (nsrc[pyc] >> (7 - xc)) & 0x1; // Foreground or background, 1 or 0
			const int vishift = ((1 - val) << 2);
			const int colidx = (ca >> vishift) & 0xF; // Index in 16-color palette
			return (val * 0xFF000000) | s_VGAPalette[colidx];
		}
	case BARGRAPH:
		{
			uint8_t val = bmpSrc8(ram, srci, x);
			if (val < y) 
				return 0xFFFFFFFF;
			else 
				return 0x00FFFFFF; // a black or white transparent pixel? :)
		}
	}
	return 0xFFFF00FF; // invalid format
}

// uses 1/(256*16) pixel units, w & h in pixel units
__forceinline argb8888 sampleBitmap(const uint8_t *ram, const uint32_t srci, const int x, const int y, const int width, const int height, const int format, const int stride, const int wrapx, const int wrapy, const int filter)
{
	//return 0xFFFFFF00;
	//switch (filter) NEAREST
	switch (filter)
	{
	case NEAREST:
		{
			int xi = x >> 12;
			int yi = y >> 12;
			return sampleBitmapAt(ram, srci, xi, yi, width, height, format, stride, wrapx, wrapy);
		}
	case BILINEAR:
		{
			int xsep = x & 0xFFF;
			int ysep = y & 0xFFF;
			int xl = x >> 12;
			int yt = y >> 12;
			if (xsep == 0 && ysep == 0)
			{
				return sampleBitmapAt(ram, srci, xl, yt, width, height, format, stride, wrapx, wrapy);
			}
			else if (xsep == 0)
			{
				int yab = ysep >> 4;
				int yat = 255 - yab;
				int yb = yt + 1;
				argb8888 top = sampleBitmapAt(ram, srci, xl, yt, width, height, format, stride, wrapx, wrapy);
				argb8888 btm = sampleBitmapAt(ram, srci, xl, yb, width, height, format, stride, wrapx, wrapy);
				return mulalpha_argb(top, yat) + mulalpha_argb(btm, yab);
			}
			else if (ysep == 0)
			{
				int xar = xsep >> 4;
				int xal = 255 - xar;
				int xr = xl + 1;
				return mulalpha_argb(sampleBitmapAt(ram, srci, xl, yt, width, height, format, stride, wrapx, wrapy), xal) 
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yt, width, height, format, stride, wrapx, wrapy), xar);
			}
			else
			{
				int xar = xsep >> 4;
				int xal = 255 - xar;
				int yab = ysep >> 4;
				int yat = 255 - yab;
				int xr = xl + 1;
				int yb = yt + 1;
				// todo optimize
				argb8888 top = mulalpha_argb(sampleBitmapAt(ram, srci, xl, yt, width, height, format, stride, wrapx, wrapy), xal) 
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yt, width, height, format, stride, wrapx, wrapy), xar);
				argb8888 btm = mulalpha_argb(sampleBitmapAt(ram, srci, xl, yb, width, height, format, stride, wrapx, wrapy), xal) 
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yb, width, height, format, stride, wrapx, wrapy), xar);
				return mulalpha_argb(top, yat) + mulalpha_argb(btm, yab);
			}
		}
	}
	return 0xFFFFFF00; // invalid filter
}

void displayBitmap(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int hsize, int px, int py, int handle, int cell)
{
	// printf("bitmap\n");
	const BitmapInfo &bi = s_BitmapInfo[handle];
	const uint8_t *ram = Memory.getRam();

	int pytop = py; // incl pixel*16 top
	int pybtm = py + (bi.SizeHeight << 4) - 16; // incl pixel*16 btm

	int pytopi = (pytop + 15) >> 4; // (pytop + 8) >> 4 // reference jumps over to the next pixel at +1/16 already
	int pybtmi = (pybtm + 15) >> 4; // (pybtm + 8) >> 4 // +8 jumps over halfway

	if (pytopi <= y && y <= pybtmi)
	{
		int pxlef = px;
		int pxrig = px + (bi.SizeWidth << 4) - 16; // verify if this is the correct behaviour for sizewidth = 0

		int pxlefi = (pxlef + 15) >> 4; // (pxlef + 8) >> 4
		int pxrigi = (pxrig + 15) >> 4; // (pxrig + 8) >> 4

		pxlefi = max((int)gs.ScissorX.I, pxlefi);
		pxrigi = min((int)gs.ScissorX2.I - 1, pxrigi);

		//if (bi.
		int vy = y * 16;
		int ry = vy - py;
		uint32_t sampleSrcPos = bi.Source + (cell * bi.LayoutStride * bi.LayoutHeight);
		// uint8_t *sampleSrc = &Memory.getRam()[sampleSrcPos];
		int sampleFormat = bi.LayoutFormat;
		int sampleWidth = bi.LayoutWidth;
		int sampleHeight = (sampleFormat == TEXT8X8) ? bi.LayoutHeight << 3 : ((sampleFormat == TEXTVGA) ? bi.LayoutHeight << 4 : bi.LayoutHeight);
		int sampleStride = bi.LayoutStride;
		int sampleWrapX = bi.SizeWrapX;
		int sampleWrapY = bi.SizeWrapY;
		int sampleFilter = bi.SizeFilter;
		// pretransform
		int rxtbc = (gs.BitmapTransformB * ry) + (gs.BitmapTransformC << 4);
		int rytef = (gs.BitmapTransformE * ry) + (gs.BitmapTransformF << 4);
		//int sample
		for (int x = pxlefi; x <= pxrigi; ++x)
		{
			if (testStencil(gs, bs, x))
			{
				// relative at 1/16 pixel units
				int vx = x * 16;
				int rx = vx - px;
				// transform with 1/(256*16) pixel units
				int rxt = (gs.BitmapTransformA * rx) + rxtbc;
				int ryt = (gs.BitmapTransformD * rx) + rytef;
				const argb8888 sample = sampleBitmap(ram, sampleSrcPos, rxt, ryt, sampleWidth, sampleHeight, sampleFormat, sampleStride, sampleWrapX, sampleWrapY, sampleFilter);
				// todo tag and stencil // todo multiply by gs.Color // todo ColorMask
				const argb8888 out = mul_argb(sample, gs.ColorARGB);
				if (testAlpha(gs, out))
				{
					bc[x] = blend(gs, out, bc[x]);
					writeTag(gs, bt, x);
				}
			}
		}
	}
}

__forceinline int getLayoutWidth(const int &format, const int &stride)
{
	switch (format)
	{
		case ARGB1555: return stride >> 1;
		case L1: return stride << 3;
		case L4: return stride << 1;
		case L8: return stride;
		case RGB332: return stride;
		case ARGB2: return stride;
		case ARGB4: return stride >> 1;
		case RGB565: return stride >> 1;
		case PALETTED: return stride;
		case TEXT8X8: return stride << 3;
		case TEXTVGA: return stride << 2;
		case BARGRAPH: return stride;
	}
	printf("Invalid bitmap layout\n");
	return stride;
}

#pragma endregion

#pragma region Primitive: Line Strip

struct LineStripDefer
{
	int32_t LeftOuter256, LeftInner256;
	int32_t RightOuter256, RightInner256;
};

struct LineStripState
{
public:
	bool Begin;
	int P1X, P1Y;
	int P2X, P2Y;
	int SL, SR;
	bool WantPoint, WantEndPoint;
	int PointLeft, PointRight;
	
	int DeferCur;
	bool DeferSet[2];
	LineStripDefer Defer[2];
};

__forceinline void renderLineStrip(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const LineStripState &lss)
{
	const LineStripDefer &d = lss.Defer[lss.DeferCur];

	const int leftInc = d.LeftInner256 - d.LeftOuter256;
	const int rightDec = d.RightOuter256 - d.RightInner256;

	const int leftPA = (d.LeftOuter256) >> 8; // inclusive left a
	const int leftPB = ((d.LeftInner256) >> 8) /*+ 1*/; // exclusive left b
	const int rightPA = (d.RightInner256) >> 8; // inclusive right a
	const int rightPB = ((d.RightOuter256) >> 8) /*+ 1*/; // exclusive right b

	if (leftInc)
	{
		const int leftAdd = (65536 / leftInc);
		int leftValue = ((0xFF - (d.LeftOuter256 & 0xFF)) * leftAdd) >> 8;
		for (int x = leftPA; x < leftPB; ++x)
		{
#if FT800EMU_DEBUG_AA
			argb8888 out = 0x80FF0000; // 0x00FFFFFF | (div255(leftValue * 128) << 24); // 0x80FF0000; //
#else
			argb8888 out = mulalpha_alpha(gs.ColorARGB, leftValue);
#endif
			bc[x] = blend(gs, out, bc[x]);

			leftValue = min(255, leftValue + leftAdd); // todo opt
		}
	}

	for (int x = leftPB; x < rightPA; ++x)
	{
#if FT800EMU_DEBUG_AA
		argb8888 out = 0x8000FF00;
#else
		argb8888 out = gs.ColorARGB;
#endif
		bc[x] = blend(gs, out, bc[x]);
	}

	if (rightDec)
	{
		const int rightRem = 65536 / rightDec;
		int rightValue = 0xFF - (((0xFF - (d.RightInner256 & 0xFF)) * rightRem) >> 8);
		for (int x = rightPA; x < rightPB; ++x)
		{
#if FT800EMU_DEBUG_AA
			argb8888 out = 0x800000FF;// 0x00FFFFFF | (div255(rightValue * 128) << 24); //0x800000FF; // 
#else
			argb8888 out = mulalpha_alpha(gs.ColorARGB, rightValue);
#endif
			bc[x] = blend(gs, out, bc[x]);

			rightValue = max(0, rightValue - rightRem); // todo opt
		}
	}

	//argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | alpha << 24;
	//
	/*for (int x = (d.LeftOuter256 >> 8); x < (d.RightOuter256 >> 8); ++x)
	{
		bc[x] = mulalpha(bc[x], (255 - 128)) + mulalpha(gs.ColorARGB, 128);
	}*/
	//bc[(d.LeftOuter256 >> 8)] = 0xFFFF0000;
	//bc[(d.RightOuter256 >> 8)] = 0xFF00FF00;
	
}

__forceinline void deferredLineStrip(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, LineStripState &lss)
{
	int nextDefer = ((lss.DeferCur + 1) % 2);
	if (lss.DeferSet[lss.DeferCur])
	{
		if (lss.DeferSet[nextDefer])
		{
			// verify and resolve overlap
			LineStripDefer &cur = lss.Defer[lss.DeferCur];
			LineStripDefer &nex = lss.Defer[nextDefer];

			// cur [[]]
			// next (())
#if !FT800EMU_DEBUG_DISABLE_OVERLAP
			if (nex.RightInner256 < cur.RightInner256
				&& nex.LeftInner256 > cur.LeftInner256)
			{
				nex.LeftInner256 = nex.RightInner256 = nex.LeftOuter256 = nex.RightOuter256 = 0;
			}
			else if (nex.RightInner256 >= cur.RightInner256
				&& nex.LeftInner256 <= cur.LeftInner256)
			{
				cur.LeftInner256 = cur.RightInner256 = cur.LeftOuter256 = cur.RightOuter256 = 0;
			}
			else if (nex.LeftInner256 <= cur.RightInner256
				&& cur.LeftInner256 <= nex.RightInner256)
			{
				if (nex.LeftInner256 > cur.LeftInner256)
				{
					nex.LeftOuter256 = nex.LeftInner256 = cur.RightOuter256 = cur.RightInner256 = (cur.LeftInner256 & (~0xFF));
				}
				else
				{
					cur.LeftOuter256 = cur.LeftInner256 = nex.RightOuter256 = nex.RightInner256 = (nex.LeftInner256 & (~0xFF));
				}
			}
#endif
			// todo some additional cases where only the aa part overlaps
		}

		renderLineStrip(gs, bc, bs, bt, lss);
		lss.DeferSet[lss.DeferCur] = false;
	}
	lss.DeferCur = nextDefer;
}

void displayLineStrip(GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, LineStripState &lss, const int &p2x, const int &p2y)
{
	if (lss.Begin)
	{
		lss.P2X = p2x;
		lss.P2Y = p2y;
		lss.Begin = false;
		lss.SL = 0;
		lss.SR = hsize;
		lss.DeferCur = 0;
		lss.DeferSet[0] = false;
		lss.DeferSet[1] = false;
		return;
	}
	int p1x = lss.P1X = lss.P2X;
	int p1y = lss.P1Y = lss.P2Y;
	lss.P2X = p2x;
	lss.P2Y = p2y;

	// draw opening or connecting sphere part
	// todo

	// draw line
	int pointleft = gs.ScissorX.I, pointright = gs.ScissorX2.I;
	int nextpointleft = gs.ScissorX.I, nextpointright = gs.ScissorX2.I;
	int ytop = (min(p1y, p2y) - ((p2x - p1x) ? gs.LineWidth : 0) - 8) >> 4;
	int ybtm = (max(p1y, p2y) + ((p2x - p1x) ? gs.LineWidth : 0) + 8) >> 4;
	if (ytop <= y && y <= ybtm)
	{
		int32_t p1x256 = p1x << 4;
		int32_t p1y256 = p1y << 4;
		int32_t p2x256 = p2x << 4;
		int32_t p2y256 = p2y << 4;
		int32_t pdx256 = p2x256 - p1x256;
		int32_t pdy256 = p2y256 - p1y256;
		int32_t pdx256a = abs(pdx256);
		int32_t pdy256a = abs(pdy256);
		int64_t pd256sq = (pdx256 * pdx256) + (pdy256 * pdy256);
		int32_t pd256 = (int32_t)(sqrt((double)pd256sq)); // line len
		int32_t r256 = gs.LineWidth << 4;
		
		// find center point
		int32_t y256 = y << 8;
		int64_t x256;
		if (pdy256 == 0)
		{
			// special case, horizontal line
			x256 = (p1x256 + p2x256) >> 1; // not really necessary
		}
		else
		{
			// general case, diagonal line
			x256 = ((y256 - p1y256) * pdx256 / pdy256) + p1x256;
		}

		// draw center point for test
		/*int xtc = x256 >> 8;
		bc[xtc] = gs.ColorARGB;*/

		// find edges, sides
		int32_t xlout256, xlin256, xrout256, xrin256;
		if (pdy256a == 0)
		{
			// special case, horizontal line
			xlout256 = xlin256 = 0;
			xrout256 = xrin256 = (hsize << 8);
		}
		else
		{
			// general case, diagonal line
			int64_t outerdist256 = (r256 + 128) * pd256 / pdy256a;
			xlout256 = (int32_t)min(max(x256 - outerdist256, 0), (hsize << 8));
			xrout256 = (int32_t)min(max(x256 + outerdist256, 0), (hsize << 8));
			int64_t innerdist256 = (r256 - 128) * pd256 / pdy256a;
			xlin256 = (int32_t)min(max(x256 - innerdist256, 0), (hsize << 8));
			xrin256 = (int32_t)min(max(x256 + innerdist256, 0), (hsize << 8));
			// test...
			/*int xtl = (x256 - outerdist256) >> 8;
			int xtr = (x256 + outerdist256) >> 8;
			bc[xtl] = 0xFF80FF00;
			bc[xtr] = 0xFF00FF80;*/
		}

		// find edges, begin and end
		int32_t xleft256, xright256;
		if (pdx256 == 0)
		{
			// special case, vertical line
			xleft256 = 0;
			xright256 = (hsize << 9);
		}
		else
		{
			// general case, diagonal line
			//xleft256 = min(p1x256, p2x256) + ((y256 - min(p1y256, p2y256)) * pd256 / pdx256a);
			int32_t x1t256 = p1x256 + ((y256 - p1y256) * -pdy256 / pdx256);
			int32_t x2t256 = p2x256 + ((y256 - p2y256) * -pdy256 / pdx256);
			xleft256 = min(max(min(x1t256, x2t256), 0), (hsize << 8));
			xright256 = min(max(max(x1t256, x2t256), 0), (hsize << 8));
			// test
			/*int lxl = xleft256 >> 8;
			bc[lxl] = 0xFFFF0000;
			int lxr = xright256 >> 8;
			bc[lxr] = 0xFF0000FF;*/
		}

		int32_t lsci = max(gs.ScissorX.I, xleft256 >> 8) << 8;
		int32_t rsci = min(gs.ScissorX2.I, xright256 >> 8) << 8;
		int32_t lsslo256 = min(max(xlout256, lsci), rsci);
		int32_t lssli256 = min(max(xlin256, lsci), rsci);
		int32_t lssro256 = min(max(xrout256, lsci), rsci);
		int32_t lssri256 = min(max(xrin256, lsci), rsci);

		int nextDefer = ((lss.DeferCur + 1) % 2);
		lss.Defer[nextDefer].LeftOuter256 = lsslo256;
		lss.Defer[nextDefer].LeftInner256 = lssli256;
		lss.Defer[nextDefer].RightOuter256 = lssro256;
		lss.Defer[nextDefer].RightInner256 = lssri256;
		lss.DeferSet[nextDefer] = true;
				
		// test fill
		for (int x = max(max(gs.ScissorX.I, xleft256 >> 8), xlout256 >> 8); x <= min(min(gs.ScissorX2.I, xright256 >> 8), xrout256 >> 8); ++x)
		{
			//bc[x] = mulalpha(bc[x], (255 - 128)) + mulalpha(gs.ColorARGB, 128);
		}
		
		if (pdx256 > 0)
		{
			// line going right
			pointright = lsci >> 8;
			nextpointleft = rsci >> 8;
		}
		else if (pdx256 < 0)
		{
			// line going left
			pointleft = rsci >> 8;
			nextpointright = lsci >> 8;
		}
	}

	// draw point
	bool nextwantpoint = (p2x - p1x) != 0 || !(ytop <= y && y <= ybtm); // vertical
#if !FT800EMU_DEBUG_LINES_WITHOUT_ENDINGS
	if (lss.WantPoint && nextwantpoint)
	{
#if FT800EMU_DEBUG_LINES_SHIFT_HACK
		displayPoint(gs, gs.LineWidth, max(pointleft, lss.PointLeft), gs.ScissorY.I, min(pointright, lss.PointRight), gs.ScissorY2.I, bc, bs, bt, y, p1x - 16, p1y);  // hack ls shift (- 16)
#else
		displayPoint(gs, gs.LineWidth, max(pointleft, lss.PointLeft), gs.ScissorY.I, min(pointright, lss.PointRight), gs.ScissorY2.I, bc, bs, bt, y, p1x, p1y);
#endif
		
		lss.WantEndPoint = true;
	}
#endif

	// data for next point
	lss.WantPoint = nextwantpoint; // vertical
	lss.PointLeft = nextpointleft;
	lss.PointRight = nextpointright;
	
	deferredLineStrip(gs, bc, bs, bt, lss);
}

void beginLineStrip(LineStripState &lss)
{
	lss.Begin = true;
	lss.WantPoint = true;
	lss.WantEndPoint = false;
	lss.PointLeft = 0;
	lss.PointRight = FT800EMU_WINDOW_WIDTH_MAX;
}

void endLineStrip(GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, LineStripState &lss)
{
#if !FT800EMU_DEBUG_LINES_WITHOUT_ENDINGS
	if (lss.WantPoint && lss.WantEndPoint)
	{
#if FT800EMU_DEBUG_LINES_SHIFT_HACK
		displayPoint(gs, gs.LineWidth, lss.PointLeft, gs.ScissorY.I, lss.PointRight, gs.ScissorY2.I, bc, bs, bt, y, lss.P2X - 16, lss.P2Y);  // hack ls shift (- 16)
#else
		displayPoint(gs, gs.LineWidth, lss.PointLeft, gs.ScissorY.I, lss.PointRight, gs.ScissorY2.I, bc, bs, bt, y, lss.P2X, lss.P2Y);
#endif
	}
#endif

	deferredLineStrip(gs, bc, bs, bt, lss);

	// draw closing sphere cap
	// todo
}

void resetLineStrip(GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, LineStripState &lss)
{
	if (!lss.Begin)
	{
		int px = lss.P2X;
		int py = lss.P2Y;
		endLineStrip(gs, bc, bs, bt, y, hsize, lss);
		beginLineStrip(lss);
		displayLineStrip(gs, bc, bs, bt, y, hsize, lss, px, py);
	}
}

#pragma endregion

#pragma region Primitive: Rects

#define FT800EMU_RECTS_FT800_COORDINATES 1

struct RectsState
{
public:
	RectsState() : Set(false) { }
	bool Set;
	int X1, Y1;
};

void displayRects(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, RectsState &rs, const int xp, const int yp)
{
	if (!rs.Set)
	{
		rs.X1 = xp;
		rs.Y1 = yp;
		rs.Set = true;
		return;
	}

#if FT800EMU_RECTS_FT800_COORDINATES // Coordinate correction for the drawing code
	const int x1r = rs.X1 + 8; // Coordinates in 1/16 pixel
	const int y1r = rs.Y1 + 8;
	const int x2r = xp + 8;
	const int y2r = yp + 8;
	const int x1 = min(x1r, x2r);
	const int x2 = max(x1r, x2r);
	const int y1 = min(y1r, y2r);
	const int y2 = max(y1r, y2r);
#else // Test version for the drawing code
	const int x1 = rs.X1; // Coordinates in 1/16 pixel
	const int y1 = rs.Y1;
	const int x2 = xp;
	const int y2 = yp;
#endif
	rs.Set = false;

	const int lw = gs.LineWidth; // Linewidth in 1/16 pixel

	const int x1lw = x1 - lw; // Coordinates plus linewidth in 1/16 pixel
	const int y1lw = y1 - lw;
	const int x2lw = x2 + lw;
	const int y2lw = y2 + lw;
	const int xslw = x2lw - x1lw;
	const int yslw = y2lw - y1lw;

	const int x1lw_px = (x1lw >> 4); // Top-left inclusive in screen pixels
	const int y1lw_px = (y1lw >> 4);
	const int x2lw_px = ((x2lw + 15) >> 4); // Bottom-right exclusive in screen pixels
	const int y2lw_px = ((y2lw + 15) >> 4);
	const int xslw_px = x2lw_px - x1lw_px; // Size in screen pixels
	const int yslw_px = y2lw_px - y1lw_px;

	if (max(y1lw_px, gs.ScissorY.I) <= y && y < min(y2lw_px, gs.ScissorY2.I)) // Scissor Y
	{	
		// Notes:
		// Need special handling for x2 - x1 < 16 and y2 - y1 < 16 (multiple overlap in single pixel line), handle explicitly

		if (xslw == 0)
		{
			// Zero width
		}
		else if (yslw == 0)
		{
			// Zero height
		}
		else if (xslw_px == 1 && yslw_px == 1) // Display a single pixel (ignores impact of rounded corners)
		{
			if (x1lw_px >= gs.ScissorX.I && x2lw_px <= gs.ScissorX2.I) // Scissor X
			{
				const int x = x1lw_px;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int dxs = x2lw - x1lw; // Width in 1/16 pixel
					const int dys = y2lw - y1lw; // Height in 1/16 pixel
#if FT800EMU_DEBUG_RECTS_MATH
					{
						const int dxl_ = x1lw & 0xF; // Left coordinate relative to pixel in 1/16
						const int dyt_ = y1lw & 0xF; // Top
						const int dxr_ = x2lw - (x1lw_px << 4); // Right
						const int dyb_ = y2lw - (y1lw_px << 4); // Bottom
						const int dxs_ = dxr_ - dxl_; // Width in 1/16 pixel
						const int dys_ = dyb_ - dyt_; // Height in 1/16 pixel
						if (dxs_ != dxs) printf("Rect 11 width error\n");
						if (dys_ != dys) printf("Rect 11 height error\n");
					}
#endif
					const int surf = dxs * dys; // Surface of the rectangle in scale 256
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8; // Alpha 0-255
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
		else if (xslw_px == 1) // Single pixel width rects (ignores impact of rounded corners)
		{
			if (x1lw_px >= gs.ScissorX.I && x2lw_px <= gs.ScissorX2.I) // Scissor X
			{
				const int x = x1lw_px;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int dxs = x2lw - x1lw; // Width in 1/16 pixel
					const int dyt = y1lw & 0xF; // Top coordinate in 1/16 relative to top pixel // Todo: Can be moved inside y condition in some way
					const int dyb = y2lw - ((y2lw_px - 1) << 4); // Bottom coordinate in 1/16 relative to bottom pixel // Todo: Can be moved inside y condition in some way
#if FT800EMU_DEBUG_RECTS_MATH
					{
						if (dyb < 1) printf("Rect pixelwidth dyb < 1\n");
						if (dyb > 16) printf("Rect pixelwidth dyb > 16\n");
						if (dxs < 1) printf("Rect pixelwidth dxs < 1\n");
						if (dxs > 16) printf("Rect pixelwidth dxs > 16\n");
					}
#endif
					int alpha; // Alpha 0-255
					if (y == y1lw_px && dyt > 0) // Top pixel, not fully filled
					{
						const int surf = dxs * (16 - dyt);
						alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					}
					else if (y == (y2lw_px - 1) && dyb < 16) // Bottom pixel, not fully filled
					{
						const int surf = dxs * dyb;
						alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					}
					else // Any other pixel
					{
						alpha = ((gs.ColorARGB >> 24) * dxs) >> 4;
					}
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
		else if (yslw_px == 1) // Single pixel height rects (ignores impact of rounded corners)
		{
			const int dys = y2lw - y1lw; // Height in 1/16 pixel
			int x1lw_px_sc = max(x1lw_px, gs.ScissorX.I); // Scissored X1
			int x2lw_px_sc = min(x2lw_px, gs.ScissorX2.I); // Scissored X2
			const int dxl = x1lw & 0xF; // Left coordinate in 1/16 relative to top pixel
			const int dxr = x2lw - ((x2lw_px - 1) << 4); // Right coordinate in 1/16 relative to bottom pixel
#if FT800EMU_DEBUG_RECTS_MATH
			{
				if (dxr < 1) printf("Rect pixelheight dxr < 1\n");
				if (dxr > 16) printf("Rect pixelheight dxr > 16\n");
				if (dys < 1) printf("Rect pixelheight dys < 1\n");
				if (dys > 16) printf("Rect pixelheight dys > 16\n");
			}
#endif
			if (x1lw_px == x1lw_px_sc && dxl > 0) // Draw the left pixel if not fully on
			{
				const int x = x1lw_px_sc;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int surf = (16 - dxl) * dys;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
				++x1lw_px_sc;
			}
			if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
			{
				const int x = x2lw_px_sc - 1;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int surf = dxr * dys;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
				--x2lw_px_sc;
			}
			for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
			{
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int alpha = ((gs.ColorARGB >> 24) * dys) >> 4;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
		else if (lw < 16)
		{
			// Need special handling for gs.LineWidth < 16 (multiple overlap in single pixel line), ignore rounded corners
			// Size is > 1px guaranteed here
			int x1lw_px_sc = max(x1lw_px, gs.ScissorX.I); // Scissored X1
			int x2lw_px_sc = min(x2lw_px, gs.ScissorX2.I); // Scissored X2
			const int dxl = x1lw & 0xF; // Left coordinate in 1/16 relative to left pixel
			const int dxr = x2lw - ((x2lw_px - 1) << 4); // Right coordinate in 1/16 relative to right pixel
			const int dyt = y1lw & 0xF; // Top coordinate in 1/16 relative to top pixel // Todo: Can be moved inside y condition in some way
			const int dyb = y2lw - ((y2lw_px - 1) << 4); // Bottom coordinate in 1/16 relative to bottom pixel // Todo: Can be moved inside y condition in some way
			int rowfill; // Fill by row scale 16
			if (y == y1lw_px && dyt > 0) // Top row, not fully filled
			{
				rowfill = (16 - dyt);
			}
			else if (y == (y2lw_px - 1) && dyb < 16) // Bottom row, not fully filled
			{
				rowfill = dyb;
			}
			else // Any other row
			{
				rowfill = 16;
			}
			if (x1lw_px == x1lw_px_sc && dxl > 0) // Draw the left pixel if not fully on
			{
				const int x = x1lw_px_sc;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int surf = (16 - dxl) * rowfill;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
				++x1lw_px_sc;
			}
			if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
			{
				const int x = x2lw_px_sc - 1;
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const int surf = dxr * rowfill;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
				--x2lw_px_sc;
			}
			if (rowfill < 16) // Top or bottom row
			{
				for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
				{
					if (testStencil(gs, bs, x)) // Test and write the stencil buffer
					{
						const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 4;
						const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
						if (testAlpha(gs, out)) // Test alpha
						{
							bc[x] = blend(gs, out, bc[x]); // Write color
							writeTag(gs, bt, x); // Write tag
						}
					}
				}
			}
			else // Any other row
			{
				for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
				{
					if (testStencil(gs, bs, x)) // Test and write the stencil buffer
					{
						const argb8888 out = gs.ColorARGB;
						if (testAlpha(gs, out)) // Test alpha
						{
							bc[x] = blend(gs, out, bc[x]); // Write color
							writeTag(gs, bt, x); // Write tag
						}
					}
				}
			}
		}
		else if (x2 - x1 == 0 && y2 - y1 == 0) // Rectangle that's actually a point
		{
			displayPoint(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, x1 - 8, y1 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
		}
		else
		{			
			// Divide into 9 parts
			// Alias the coordinates for clarity
			const int ax1 = x1lw;
			const int ax2 = x1;
			const int bx1 = x1;
			const int bx2 = x2;
			const int cx1 = x2;
			const int cx2 = x2lw;
			const int ay1 = y1lw;
			const int ay2 = y1;
			const int by1 = y1;
			const int by2 = y2;
			const int cy1 = y2;
			const int cy2 = y2lw;

			// Get the used pixel positions
			const int ax1_px = x1lw_px;
			const int ax2_px = ((ax2 + 15) >> 4);
			const int bx1_px = (bx1 >> 4);
			const int bx2_px = ((bx2 + 15) >> 4);
			const int cx1_px = (cx1 >> 4);
			const int cx2_px = x2lw_px;
			const int ay1_px = y1lw_px;
			const int ay2_px = ((ay2 + 15) >> 4);
			const int by1_px = (by1 >> 4);
			const int by2_px = ((by2 + 15) >> 4);
			const int cy1_px = (cy1 >> 4);
			const int cy2_px = y2lw_px;
			
			// 0-16 how much to blend with the points
			int blendtop;
			int blendbottom;
			// Alpha 256 for blended point AA region
			int alphatopleft;
			int alphatopright;
			int alphabottomleft;
			int alphabottomright;
			if (y < by1_px) // Top point
			{
				blendtop = 16;
				blendbottom = 0;
			}
			else if (y < ay2_px) 
			{
				if (y >= cy1_px) // Shared boundery top, center, bottom
				{
					blendtop = ay2 - ((ay2_px - 1) << 4);
					blendbottom = by2 - ((by2_px - 1) << 4);
					blendbottom = 16 - blendbottom;
#if FT800EMU_DEBUG_RECTS_MATH
					if (blendtop + blendbottom >= 16) printf("Full rect shared blend bad math (blendtop + blendbottom >= 16)\n");
#endif
				}
				else // Shared boundary between top and center
				{
					blendtop = ay2 - ((ay2_px - 1) << 4);
					blendbottom = 0;
					alphabottomleft = 0;
					alphabottomright = 0;
				}
			}
			else if (y < cy1_px) // Center area
			{
				blendtop = 0;
				blendbottom = 0;
			}
			else if (y < by2_px) // Shared boundary between center and bottom
			{
				blendbottom = by2 - ((by2_px - 1) << 4);
				blendbottom = 16 - blendbottom;
				blendtop = 0;
				alphatopleft = 0;
				alphatopright = 0;
			}
			else if (y < cy2_px) // Bottom point
			{
				blendtop = 0;
				blendbottom = 16;
			}

			if (blendtop == 16) // Top
			{
				if (x2 - x1 == 0) // Vertical line
				{				
					displayPoint(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by1 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
				}
				else
				{
					const int bx1_px_sc_r = min(bx1_px, gs.ScissorX2.I); // Scissored
					const int bx2_px_sc_l = max(bx2_px, gs.ScissorX.I); // Scissored
					// Render top-left and top-right corner
					displayPoint(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, bx1_px_sc_r, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by1 - 8);
					displayPoint(gs, gs.LineWidth, bx2_px_sc_l, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx2 - 8, by1 - 8);
					const int dyt = y1lw & 0xF; // Top coordinate in 1/16 relative to top pixel
					if (y == y1lw_px) // && dyt > 0) // Top row
					{
						int bx1_px_sc_l = max(bx1_px, gs.ScissorX.I); // Scissored
						int bx2_px_sc_r = min(bx2_px, gs.ScissorX2.I); // Scissored
						const int rowfill = (16 - dyt) * 16; // Alpha 256
						if (ax2_px - 1 == cx1_px) // Triple blend
						{
							const int x = cx1_px;
							if (bx1_px_sc_l <= x && x < bx2_px_sc_r) // Check scissor
							{
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendleft - blendright;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by1 - 8);
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by1 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}								
							}
						}
						else
						{
							if (bx1_px_sc_l == bx1_px && ax2_px - 1 == bx1_px) // Left pixel overlaps
							{
								const int x = bx1_px;
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									const int blendinv = 16 - blendleft;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by1 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}
								++bx1_px_sc_l;
							}

							if (bx2_px_sc_r == bx2_px && bx2_px - 1 == cx1_px) // Right pixel overlaps
							{
								const int x = cx1_px;
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendright;
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by1 - 8);
									const int alphaval = (rowfill * blendinv) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}
								--bx2_px_sc_r;
							}

							// Draw the rest of this top row
							{
								const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 8;
								const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);

								for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
								{
									if (testStencil(gs, bs, x)) // Test and write the stencil buffer
									{
										if (testAlpha(gs, out)) // Test alpha
										{
											bc[x] = blend(gs, out, bc[x]); // Write color
											writeTag(gs, bt, x); // Write tag
										}
									}
								}
							}
						}
					}
					else
					{
						// Fill between the points
						const int bx1_px_sc_l = max(bx1_px_sc_r, gs.ScissorX.I); // Scissored
						const int bx2_px_sc_r = min(bx2_px_sc_l, gs.ScissorX2.I); // Scissored
						for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
						{
							if (testStencil(gs, bs, x)) // Test and write the stencil buffer
							{
								if (testAlpha(gs, gs.ColorARGB)) // Test alpha
								{
									bc[x] = blend(gs, gs.ColorARGB, bc[x]); // Write color
									writeTag(gs, bt, x); // Write tag
								}
							}
						}
					}
				}
			}
			else if (blendbottom == 16)
			{
				if (x2 - x1 == 0) // Vertical line
				{
					displayPoint(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by2 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
				}
				else
				{
					const int bx1_px_sc_r = min(bx1_px, gs.ScissorX2.I); // Scissored X2
					const int bx2_px_sc_l = max(bx2_px, gs.ScissorX.I); // Scissored X1
					// Render bottom-left and bottom-right corner
					displayPoint(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, bx1_px_sc_r, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by2 - 8);
					displayPoint(gs, gs.LineWidth, bx2_px_sc_l, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx2 - 8, by2 - 8);
					const int dyb = y2lw - ((y2lw_px - 1) << 4); // Bottom coordinate in 1/16 relative to bottom pixel
					if (y == (y2lw_px - 1)) // && dyb < 16) // Bottom row
					{
						int bx1_px_sc_l = max(bx1_px, gs.ScissorX.I); // Scissored
						int bx2_px_sc_r = min(bx2_px, gs.ScissorX2.I); // Scissored
						const int rowfill = dyb * 16; // Alpha 256
						if (ax2_px - 1 == cx1_px) // Triple blend
						{
							const int x = cx1_px;
							if (bx1_px_sc_l <= x && x < bx2_px_sc_r) // Check scissor
							{
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendleft - blendright;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by2 - 8);
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by2 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}
							}
						}
						else
						{
							if (bx1_px_sc_l == bx1_px && ax2_px - 1 == bx1_px) // Left pixel overlaps
							{
								const int x = bx1_px;
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									const int blendinv = 16 - blendleft;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by2 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}
								++bx1_px_sc_l;
							}

							if (bx2_px_sc_r == bx2_px && bx2_px - 1 == cx1_px) // Right pixel overlaps
							{
								const int x = cx1_px;
								if (testStencil(gs, bs, x)) // Test and write the stencil buffer
								{
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendright;
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by2 - 8);
									const int alphaval = (rowfill * blendinv) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									if (testAlpha(gs, out)) // Test alpha
									{
										bc[x] = blend(gs, out, bc[x]); // Write color
										writeTag(gs, bt, x); // Write tag
									}
								}
								--bx2_px_sc_r;
							}

							// Draw the rest of this bottom row
							{
								const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 8;
								const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);

								for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
								{
									if (testStencil(gs, bs, x)) // Test and write the stencil buffer
									{
										if (testAlpha(gs, out)) // Test alpha
										{
											bc[x] = blend(gs, out, bc[x]); // Write color
											writeTag(gs, bt, x); // Write tag
										}
									}
								}
							}
						}
					}
					else
					{
						// Fill between the points
						const int bx1_px_sc_l = max(bx1_px_sc_r, gs.ScissorX.I); // Scissored
						const int bx2_px_sc_r = min(bx2_px_sc_l, gs.ScissorX2.I); // Scissored
						for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
						{
							if (testStencil(gs, bs, x)) // Test and write the stencil buffer
							{
								if (testAlpha(gs, gs.ColorARGB)) // Test alpha
								{
									bc[x] = blend(gs, gs.ColorARGB, bc[x]); // Write color
									writeTag(gs, bt, x); // Write tag
								}
							}
						}
					}
				}
			}
			else
			{
				int x1lw_px_sc = max(x1lw_px, gs.ScissorX.I); // Scissored X1
				int x2lw_px_sc = min(x2lw_px, gs.ScissorX2.I); // Scissored X2
				const int dxl = x1lw & 0xF; // Left coordinate in 1/16 relative to left pixel
				const int dxr = x2lw - ((x2lw_px - 1) << 4); // Right coordinate in 1/16 relative to right pixel

				if (blendtop == 0 && blendbottom == 0) // Center only
				{
					if (x1lw_px == x1lw_px_sc && dxl > 0) // Draw the left pixel if not fully on
					{
						const int x = x1lw_px_sc;
						if (testStencil(gs, bs, x)) // Test and write the stencil buffer
						{
							const int surf = (16 - dxl);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 4;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							if (testAlpha(gs, out)) // Test alpha
							{
								bc[x] = blend(gs, out, bc[x]); // Write color
								writeTag(gs, bt, x); // Write tag
							}
						}
						++x1lw_px_sc;
					}
					if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
					{
						const int x = x2lw_px_sc - 1;
						if (testStencil(gs, bs, x)) // Test and write the stencil buffer
						{
							const int surf = dxr;
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 4;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							if (testAlpha(gs, out)) // Test alpha
							{
								bc[x] = blend(gs, out, bc[x]); // Write color
								writeTag(gs, bt, x); // Write tag
							}
						}
						--x2lw_px_sc;
					}
				}
				else // Draw blended left and right pixels
				{						
					if (blendtop > 0)
					{
						// Calculate border alphas for top point
						alphatopleft = getPointAlpha256(lw, x1lw_px_sc, y, bx1 - 8, by1 - 8);
						alphatopright = getPointAlpha256(lw, x2lw_px_sc - 1, y, bx2 - 8, by1 - 8);
					}
					
					if (blendbottom > 0)
					{
						// Calculate border alphas for bottom point
						alphabottomleft = getPointAlpha256(lw, x1lw_px_sc, y, bx1 - 8, by2 - 8);
						alphabottomright = getPointAlpha256(lw, x2lw_px_sc - 1, y, bx2 - 8, by2 - 8);
					}

#if FT800EMU_DEBUG_RECTS_MATH
					{
						if (x1lw_px_sc == x2lw_px_sc) printf("Full rect lr blend x1lw_px_sc == x2lw_px_sc\n");
					}
#endif
					const int blendc = (16 - blendtop - blendbottom) * 16;
					// Left
					if (x1lw_px == x1lw_px_sc) // Check scissor
					{
						const int x = x1lw_px_sc;
						if (testStencil(gs, bs, x)) // Test and write the stencil buffer
						{
							const int surf = ((16 - dxl) * blendc) + (alphatopleft * blendtop) + (alphabottomleft * blendbottom);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 12;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							if (testAlpha(gs, out)) // Test alpha
							{
								bc[x] = blend(gs, out, bc[x]); // Write color
								writeTag(gs, bt, x); // Write tag
							}
						}
						++x1lw_px_sc;
					}
					// Right
					if (x2lw_px == x2lw_px_sc) // Check scissor
					{
						const int x = x2lw_px_sc - 1;
						if (testStencil(gs, bs, x)) // Test and write the stencil buffer
						{
							const int surf = (dxr * blendc) + (alphatopright * blendtop) + (alphabottomright * blendbottom);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 12;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							if (testAlpha(gs, out)) // Test alpha
							{
								bc[x] = blend(gs, out, bc[x]); // Write color
								writeTag(gs, bt, x); // Write tag
							}
						}
						--x2lw_px_sc;
					}
				}
				// The rest of the pixels
				for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
				{
					if (testStencil(gs, bs, x)) // Test and write the stencil buffer
					{
						const argb8888 out = gs.ColorARGB;
						if (testAlpha(gs, out)) // Test alpha
						{
							bc[x] = blend(gs, out, bc[x]); // Write color
							writeTag(gs, bt, x); // Write tag
						}
					}
				}
			}
		}
	}
}

#pragma endregion

#pragma region Primitive: Edge Strip

// Mimic strange clipping behaviour
// This clips to the outer value transformed to pixels directly
// Only works for L and R
#define FT800EMU_EDGE_STRIP_CLIPPING_BEHAVIOUR 0

struct EdgeStripState
{
public:
	EdgeStripState() : Set(false) { }
	bool Set;
	int X1, Y1;
};

__forceinline int findx(const int &x1, const int &x2, const int &y1, const int &y2, const int &y)
{
	const int xd = x2 - x1;
	const int yd = y2 - y1;
	const int yr = y - y1;
	return x1 + (xd * yr / yd);
}

void displayEdgeStripL(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, EdgeStripState &ess, const int xp, const int yp)
{
	if (!ess.Set)
	{
		ess.X1 = xp;
		ess.Y1 = yp;
		ess.Set = true;
		return;
	}

	// Interpret coordinates
	const int y1r = ess.Y1;
	const int y2r = yp;
	if (y1r == y2r) return; // No op
	const int x1r = ess.X1;
	const int x2r = xp;
	const int y1 = min(y1r, y2r);
	const int y2 = max(y1r, y2r);
	const int x1 = y1 == y1r ? x1r : x2r;
	const int x2 = y2 == y2r ? x2r : x1r;

	// Store coordinates for next call
	ess.X1 = xp;
	ess.Y1 = yp;

	// Get pixel positions
	const int y1_px = ((y1 + 15) >> 4); // Y Inclusive
	const int y2_px = ((y2 + 15) >> 4); // Y Exclusive
	
	// Render
	if (max(y1_px, gs.ScissorY.I) <= y && y < min(y2_px, gs.ScissorY2.I))
	{
		// Get boundary
		const int yv = (y << 4); // Y value 16
		const int xv = findx(x1, x2, y1, y2, yv); // X value 16
		const int xv_px = xv >> 4; // ((xv + 15) >> 4); // X pixel exclusive		

		const int left_sc = gs.ScissorX.I;
#if FT800EMU_EDGE_STRIP_CLIPPING_BEHAVIOUR
		const int xm_px = (max(x1, x2) - 16) >> 4;
		const int right_sc = min(xm_px, min(xv_px, gs.ScissorX2.I));
#else
		const int right_sc = min(xv_px, gs.ScissorX2.I);
#endif

		for (int x = left_sc; x < right_sc; ++x)
		{
			if (testStencil(gs, bs, x)) // Test and write the stencil buffer
			{
				const argb8888 out = gs.ColorARGB;
				if (testAlpha(gs, out)) // Test alpha
				{
					bc[x] = blend(gs, out, bc[x]); // Write color
					writeTag(gs, bt, x); // Write tag
				}
			}
		}
	}
}

void displayEdgeStripR(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, EdgeStripState &ess, const int xp, const int yp)
{
	if (!ess.Set)
	{
		ess.X1 = xp;
		ess.Y1 = yp;
		ess.Set = true;
		return;
	}

	// Interpret coordinates
	const int y1r = ess.Y1;
	const int y2r = yp;
	if (y1r == y2r) return; // No op
	const int x1r = ess.X1;
	const int x2r = xp;
	const int y1 = min(y1r, y2r);
	const int y2 = max(y1r, y2r);
	const int x1 = y1 == y1r ? x1r : x2r;
	const int x2 = y2 == y2r ? x2r : x1r;

	// Store coordinates for next call
	ess.X1 = xp;
	ess.Y1 = yp;

	// Get pixel positions
	const int y1_px = ((y1 + 15) >> 4); // Y Inclusive
	const int y2_px = ((y2 + 15) >> 4); // Y Exclusive
	
	// Render
	if (max(y1_px, gs.ScissorY.I) <= y && y < min(y2_px, gs.ScissorY2.I))
	{
		// Get boundary
		const int yv = (y << 4); // Y value 16
		const int xv = findx(x1, x2, y1, y2, yv); // X value 16
		const int xv_px = ((xv) >> 4); // ((xv + 15) >> 4); // X pixel exclusive		

#if FT800EMU_EDGE_STRIP_CLIPPING_BEHAVIOUR
		const int xm_px = (max(x1, x2) - 16) >> 4;
		const int left_sc = max(min(xm_px, xv_px), gs.ScissorX.I);
#else
		const int left_sc = max(xv_px, gs.ScissorX.I);
#endif
		const int right_sc = gs.ScissorX2.I;

		for (int x = left_sc; x < right_sc; ++x)
		{
			if (testStencil(gs, bs, x)) // Test and write the stencil buffer
			{
				const argb8888 out = gs.ColorARGB;
				if (testAlpha(gs, out)) // Test alpha
				{
					bc[x] = blend(gs, out, bc[x]); // Write color
					writeTag(gs, bt, x); // Write tag
				}
			}
		}
	}
}

void displayEdgeStripA(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, EdgeStripState &ess, const int xp, const int yp)
{
	if (!ess.Set)
	{
		ess.X1 = xp;
		ess.Y1 = yp;
		ess.Set = true;
		return;
	}

	// Interpret coordinates
	const int x1r = ess.X1;
	const int x2r = xp;
	if (x1r == x2r) return; // No op
	const int y1r = ess.Y1;
	const int y2r = yp;
	const int x1 = min(x1r, x2r);
	const int x2 = max(x1r, x2r);
	const int y1 = x1 == x1r ? y1r : y2r;
	const int y2 = x2 == x2r ? y2r : y1r;

	// Store coordinates for next call
	ess.X1 = xp;
	ess.Y1 = yp;

	
	// Render
	const int ym = max(y1, y2);
	const int ym_px = (ym + 15) >> 4; // Clip
	if (gs.ScissorY.I <= y && y < min(ym_px, gs.ScissorY2.I)) // Scissor
	{
		// Get pixel positions
		const int x1_px = ((x1) >> 4); // X Inclusive
		const int x2_px = ((x2) >> 4); // X Exclusive

		const int x1_sc = max(x1_px, gs.ScissorX.I);
		const int x2_sc = min(x2_px, gs.ScissorX2.I);

		if (y1 - y2 != 0)
		{
			const int yv = (y << 4); // Y value 16
			const int xv = findx(x1, x2, y1, y2, yv); // X value 16
			const int xv_px = ((xv) >> 4); // X pixel exclusive or inclusive?

			const int left_sc = y1 >= y2 ? x1_sc : max(xv_px, x1_sc);
			const int right_sc = y1 <= y2 ? x2_sc : min(xv_px, x2_sc);

			for (int x = left_sc; x < right_sc; ++x)
			{
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const argb8888 out = gs.ColorARGB; // x == left_sc ? 0xFFFF8000 : x == right_sc - 1 ? 0xFF00FF80 : gs.ColorARGB;
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
		else
		{
			for (int x = x1_sc; x < x2_sc; ++x)
			{
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const argb8888 out = gs.ColorARGB;
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
	}
}

void displayEdgeStripB(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, EdgeStripState &ess, const int xp, const int yp)
{

	if (!ess.Set)
	{
		ess.X1 = xp;
		ess.Y1 = yp;
		ess.Set = true;
		return;
	}

	// Interpret coordinates
	const int x1r = ess.X1;
	const int x2r = xp;
	if (x1r == x2r) return; // No op
	const int y1r = ess.Y1;
	const int y2r = yp;
	const int x1 = min(x1r, x2r);
	const int x2 = max(x1r, x2r);
	const int y1 = x1 == x1r ? y1r : y2r;
	const int y2 = x2 == x2r ? y2r : y1r;

	// Store coordinates for next call
	ess.X1 = xp;
	ess.Y1 = yp;
		
	// Render
	const int ym = min(y1, y2);
	const int ym_px = (ym + 16) >> 4; // Clip
	if (max(ym_px, gs.ScissorY.I) <= y && y < gs.ScissorY2.I) // Scissor
	{
		// Get pixel positions
		const int x1_px = ((x1) >> 4); // X Inclusive
		const int x2_px = ((x2) >> 4); // X Exclusive

		const int x1_sc = max(x1_px, gs.ScissorX.I);
		const int x2_sc = min(x2_px, gs.ScissorX2.I);

		if (gs.DebugDisplayListIndex == 0x1b5)
		{
			printf("ah");
		}

		if ((y1 - y2) != 0)
		{
			const int yv = (y << 4); // Y value 16
			const int xv = findx(x1, x2, y1, y2, yv); // X value 16
			const int xv_px = ((xv) >> 4); // X pixel exclusive or inclusive?

			const int left_sc = y1 <= y2 ? x1_sc : max(xv_px, x1_sc);
			const int right_sc = y1 >= y2 ? x2_sc : min(xv_px, x2_sc);;

			for (int x = left_sc; x < right_sc; ++x)
			{
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const argb8888 out = gs.ColorARGB;
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
		else
		{
			for (int x = x1_sc; x < x2_sc; ++x)
			{
				if (testStencil(gs, bs, x)) // Test and write the stencil buffer
				{
					const argb8888 out = gs.ColorARGB;
					if (testAlpha(gs, out)) // Test alpha
					{
						bc[x] = blend(gs, out, bc[x]); // Write color
						writeTag(gs, bt, x); // Write tag
					}
				}
			}
		}
	}
}

#pragma endregion

}

static int s_DebugMode;
static int s_DebugMultiplier = 1;

void GraphicsProcessorClass::begin()
{
	s_DebugMode = FT800EMU_DEBUGMODE_NONE;
	uint8_t *ram = Memory.getRam();
	uint32_t fi = Memory.rawReadU32(ram, FT800EMU_ROM_FONTINFO);
	printf("Font index: %u\n", fi);
	for (int i = 0; i < 16; ++i)
	{
		int ir = i + 16;
		uint32_t bi = (i * 148) + fi;
		uint32_t format =  Memory.rawReadU32(ram, bi + 128);
		uint32_t stride =  Memory.rawReadU32(ram, bi + 132);
		uint32_t width =  Memory.rawReadU32(ram, bi + 136);
		uint32_t height =  Memory.rawReadU32(ram, bi + 140);
		uint32_t data =  Memory.rawReadU32(ram, bi + 144);
		printf("Font[%i] -> Format: %u, Stride: %u, Width: %u, Height: %u, Data: %u\n", ir, format, stride, width, height, data);

		s_BitmapInfo[ir].Source = data;
		s_BitmapInfo[ir].LayoutFormat = format;
		s_BitmapInfo[ir].LayoutStride = stride;
		s_BitmapInfo[ir].LayoutHeight = height;
		s_BitmapInfo[ir].LayoutWidth = getLayoutWidth(format, stride);
		s_BitmapInfo[ir].SizeFilter = ir < 25 ? NEAREST : BILINEAR; // i assume
		s_BitmapInfo[ir].SizeWrapX = BORDER;
		s_BitmapInfo[ir].SizeWrapY = BORDER;
		s_BitmapInfo[ir].SizeWidth = width;
		s_BitmapInfo[ir].SizeHeight = height;
	}
}

void GraphicsProcessorClass::end()
{
	
}

void GraphicsProcessorClass::process(argb8888 *screenArgb8888, bool upsideDown, uint32_t hsize, uint32_t vsize)
{
	// Swap the display list... Is this done before the frame render or after?
	uint8_t *ram = Memory.getRam();
	if (ram[REG_DLSWAP] == SWAP_FRAME)
		Memory.swapDisplayList();

	const uint32_t *displayList = Memory.getDisplayList();
	uint8_t bt[FT800EMU_WINDOW_WIDTH_MAX]; // tag buffer (per thread value)
	uint8_t bs[FT800EMU_WINDOW_WIDTH_MAX]; // stencil buffer (per-thread values!)
	// TODO: option for multicore rendering
	for (uint32_t y = 0; y < vsize; ++y)
	{
		LineStripState lss = LineStripState();
		RectsState rs = RectsState();
		EdgeStripState ess = EdgeStripState();
		bool begun = false;
		int primitive = 0;
		GraphicsState gs = GraphicsState();
		std::stack<GraphicsState> gsstack;
		std::stack<int> callstack;
		gs.ScissorX2.I = min((int)hsize, gs.ScissorX2.I);
		argb8888 *bc = &screenArgb8888[(upsideDown ? (vsize - y - 1) : y) * hsize];
		// limits for single core rendering on Intel Core 2 Duo
		// maximum 32 argb8888 memory ops per pixel on average
		// maximum 15360 argb8888 memory ops per line

		// pre-clear bitmap buffer, but optimize! (don't clear if the user already does it)
		if (!(((displayList[0] & 0xFF000004) == ((FT800EMU_DL_CLEAR << 24) | 0x04))
			|| (((displayList[0] >> 24) == FT800EMU_DL_CLEAR_COLOR_RGB) 
				&& ((displayList[1] & 0xFF000004) == ((FT800EMU_DL_CLEAR << 24) | 0x04)))))
		{
			// about loop+480 ops
			for (uint32_t i = 0; i < hsize; ++i)
			{
				bc[i] = 0xFF000000;
			}
		}
		// pre-clear line stencil buffer, but optimize! (don't clear if the user already does it)
		if (!(((displayList[0] & 0xFF000002) == ((FT800EMU_DL_CLEAR << 24) | 0x02))
			|| (((displayList[0] >> 24) == FT800EMU_DL_CLEAR_COLOR_RGB) 
				&& ((displayList[1] & 0xFF000002) == ((FT800EMU_DL_CLEAR << 24) | 0x02)))))
		{
			// about loop+480 ops
			for (uint32_t i = 0; i < hsize; ++i)
			{
				bs[i] = 0;
			}
		}
		// pre-clear line tag buffer, but optimize! (don't clear if the user already does it)
		if (!(((displayList[0] & 0xFF000001) == ((FT800EMU_DL_CLEAR << 24) | 0x01))
			|| (((displayList[0] >> 24) == FT800EMU_DL_CLEAR_COLOR_RGB) 
				&& ((displayList[1] & 0xFF000001) == ((FT800EMU_DL_CLEAR << 24) | 0x01)))))
		{
			// about loop+480 ops
			for (uint32_t i = 0; i < hsize; ++i)
			{
				bt[i] = 0;
			}
		}

		// run display list
		for (size_t c = 0; c < FT800EMU_DISPLAY_LIST_SIZE; ++c)
		{
			gs.DebugDisplayListIndex = c;
			uint32_t v = displayList[c]; // (up to 2048 ops)
EvaluateDisplayListValue:
			switch (v >> 30)
			{
			case 0:
				if (begun && primitive == LINE_STRIP)
				{
					switch (v >> 24)
					{
					case FT800EMU_DL_END:
					case FT800EMU_DL_CALL:
					case FT800EMU_DL_RETURN:
					case FT800EMU_DL_JUMP:
					case FT800EMU_DL_MACRO:
						break;
					case FT800EMU_DL_BEGIN:
						endLineStrip(gs, bc, bs, bt, y, hsize, lss);
						break;
					default:
						resetLineStrip(gs, bc, bs, bt, y, hsize, lss);
						break;
					}
				}
				switch (v >> 24)
				{
				case FT800EMU_DL_DISPLAY:
					goto DisplayListDisplay;
				case FT800EMU_DL_BITMAP_SOURCE:
					s_BitmapInfo[gs.BitmapHandle].Source = v & 0xFFFFF;
					break;
				case FT800EMU_DL_CLEAR_COLOR_RGB:
					gs.ClearColorARGB = (gs.ClearColorARGB & 0xFF000000) | (v & 0x00FFFFFF);
					break;
				case FT800EMU_DL_TAG:
					gs.Tag = v & 0xFF;
					break;
				case FT800EMU_DL_COLOR_RGB:
					gs.ColorARGB = (gs.ColorARGB & 0xFF000000) | (v & 0x00FFFFFF);
					break;
				case FT800EMU_DL_BITMAP_HANDLE:
					gs.BitmapHandle = v & 0x1F;
					break;
				case FT800EMU_DL_CELL:
					gs.Cell = v & 0x7F;
					break;
				case FT800EMU_DL_BITMAP_LAYOUT: 
					{
						BitmapInfo &bi = s_BitmapInfo[gs.BitmapHandle];
						const int format = (v >> 19) & 0x1F;
						bi.LayoutFormat = format;
						int stride = (v >> 9) & 0x3FF;
						if (stride == 0) { /*if (y == 0) printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 1024; } // correct behaviour is probably 'infinite'?
						bi.LayoutStride = stride;
						bi.LayoutHeight = v & 0x1FF;
						if (bi.LayoutHeight == 0) { /*if (y == 0) printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 512; } // correct behaviour is probably 'infinite'?
						bi.LayoutWidth = getLayoutWidth(format, stride);
					}
					break;
				case FT800EMU_DL_BITMAP_SIZE:
					s_BitmapInfo[gs.BitmapHandle].SizeFilter = (v >> 20) & 0x1;
					s_BitmapInfo[gs.BitmapHandle].SizeWrapX = (v >> 19) & 0x1;
					s_BitmapInfo[gs.BitmapHandle].SizeWrapY = (v >> 18) & 0x1;
					s_BitmapInfo[gs.BitmapHandle].SizeWidth = (v >> 9) & 0x1FF;
					if (s_BitmapInfo[gs.BitmapHandle].SizeWidth == 0) s_BitmapInfo[gs.BitmapHandle].SizeWidth = 512; // verify
					s_BitmapInfo[gs.BitmapHandle].SizeHeight = v & 0x1FF;
					if (s_BitmapInfo[gs.BitmapHandle].SizeHeight== 0) s_BitmapInfo[gs.BitmapHandle].SizeHeight = 512; // vefiry
					break;
				case FT800EMU_DL_ALPHA_FUNC:
					gs.AlphaFunc = (v >> 8) & 0x07;
					gs.AlphaFuncRef = v & 0xFF;
					break;
				case FT800EMU_DL_STENCIL_FUNC:
					gs.StencilFunc = (v >> 16) & 0x7;
					gs.StencilFuncRef = (v >> 8) & 0xFF;
					gs.StencilFuncMask = v & 0xFF;
					break;
				case FT800EMU_DL_BLEND_FUNC:
					gs.BlendFuncSrc = (v >> 3) & 0x7;
					gs.BlendFuncDst = v & 0x7;
					break;
				case FT800EMU_DL_STENCIL_OP:
					gs.StencilOpFail = (v >> 3) & 0x7;
					gs.StencilOpPass = v & 0x7;
					break;
				case FT800EMU_DL_POINT_SIZE:
					gs.PointSize = v & 0x1FFF;
					break;
				case FT800EMU_DL_LINE_WIDTH:
					gs.LineWidth = v & 0xFFF;
					break;
				case FT800EMU_DL_CLEAR_COLOR_A:
					gs.ClearColorARGB = (gs.ClearColorARGB & 0x00FFFFFF) | (v << 24);
					break;
				case FT800EMU_DL_COLOR_A:
					gs.ColorARGB = (gs.ColorARGB & 0x00FFFFFF) | (v << 24);
					break;
				case FT800EMU_DL_CLEAR_STENCIL:
					gs.ClearStencil = v & 0xFF;
					break;
				case FT800EMU_DL_CLEAR_TAG:
					gs.ClearTag = v & 0xFF;
					break;
				case FT800EMU_DL_STENCIL_MASK:
					gs.StencilMask = v & 0xFF;
					break;
				case FT800EMU_DL_TAG_MASK:
					gs.TagMask = (v & 0x01) != 0;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_A:
					gs.BitmapTransformA = v & 0xFFFF; // 8.8 signed
					if (v & 0x10000) gs.BitmapTransformA = gs.BitmapTransformA - 0x10000;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_B:
					gs.BitmapTransformB = v & 0xFFFF;
					if (v & 0x10000) gs.BitmapTransformB = gs.BitmapTransformB - 0x10000;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_C: // 15.8 signed
					gs.BitmapTransformC = v & 0x7FFFFF;
					if (v & 0x800000) gs.BitmapTransformC = gs.BitmapTransformC - 0x800000;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_D:
					gs.BitmapTransformD = v & 0xFFFF;
					if (v & 0x10000) gs.BitmapTransformD = gs.BitmapTransformD - 0x10000;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_E:
					gs.BitmapTransformE = v & 0xFFFF;
					if (v & 0x10000) gs.BitmapTransformE = gs.BitmapTransformE - 0x10000;
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_F:
					gs.BitmapTransformF = v & 0x7FFFFF;
					if (v & 0x800000) gs.BitmapTransformF = gs.BitmapTransformF - 0x800000;
					break;
				case FT800EMU_DL_SCISSOR_XY:
					gs.ScissorY.U = v & 0x1FF;
					gs.ScissorX.U = (v >> 9) & 0x1FF;
					gs.ScissorX2.U = min(hsize, gs.ScissorX.I + gs.ScissorWidth);
					gs.ScissorY2.U = gs.ScissorY.I + gs.ScissorHeight;
					break;
				case FT800EMU_DL_SCISSOR_SIZE:
					gs.ScissorHeight = v & 0x3FF;
					gs.ScissorWidth = (v >> 10) & 0x3FF;
					gs.ScissorX2.U = min(hsize, gs.ScissorX.I + gs.ScissorWidth);
					gs.ScissorY2.U = gs.ScissorY.I + gs.ScissorHeight;
					break;
				case FT800EMU_DL_CALL:
					callstack.push(c);
					c = (v & 0xFFFF) - 1;
					break;
				case FT800EMU_DL_JUMP:
					c = (v & 0xFFFF) - 1;
					break;
				case FT800EMU_DL_BEGIN:
					primitive = v & 0x0F;
					// if (begun) printf("Double begin\n");
					begun = true;
					switch (primitive)
					{
					case LINE_STRIP:
						beginLineStrip(lss);
						break;
					case EDGE_STRIP_R:
					case EDGE_STRIP_L:
					case EDGE_STRIP_A:
					case EDGE_STRIP_B:
						ess.Set = false;
						break;
					case RECTS:
						rs.Set = false;
						break;
					}
					break;
				case FT800EMU_DL_COLOR_MASK:
					gs.ColorMaskARGB = (((v >> 1) & 0x1) * 0xFF)
						| (((v >> 2) & 0x1) * 0xFF00)
						| (((v >> 3) & 0x1) * 0xFF0000)
						| ((v & 0x1) * 0xFF000000);
					break;
				case FT800EMU_DL_END:
					switch (primitive)
					{
					case LINE_STRIP:
						endLineStrip(gs, bc, bs, bt, y, hsize, lss);
						break;
					}
					primitive = 0;
					begun = false;
					break;
				case FT800EMU_DL_SAVE_CONTEXT:
					gsstack.push(gs);
					break;
				case FT800EMU_DL_RESTORE_CONTEXT:
					gs = gsstack.top();
					gsstack.pop();
					break;
				case FT800EMU_DL_RETURN:
					c = callstack.top();
					callstack.pop();
					break;
				case FT800EMU_DL_MACRO:
					v = Memory.rawReadU32(ram, REG_MACRO_0 + (4 * (v & 0x01)));
					// What happens when the macro macros itself? :)
					goto EvaluateDisplayListValue;
					break;
				case FT800EMU_DL_CLEAR:
					if (y >= gs.ScissorY.U && y < gs.ScissorY2.U)
					{
						if (v & 0x04)
						{
							// clear color buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX.U; i < gs.ScissorX2.U; ++i)
							{
								bc[i] = (gs.ClearColorARGB & gs.ColorMaskARGB) | (bc[i] & (~gs.ColorMaskARGB));
							}
						}
						if (v & 0x02)
						{
							// Clear stencil buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX.U; i < gs.ScissorX2.U; ++i)
							{
								bs[i] = (gs.ClearStencil & gs.StencilMask) | (bs[i] & (~gs.StencilMask));
							}
						}
						if (gs.TagMask && v & 0x01)
						{
							// Clear tag buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX.U; i < gs.ScissorX2.U; ++i)
							{
								bt[i] = gs.ClearTag;
							}
						}
					}
					break;
				default:
					printf("%i: Invalid display list entry %i\n", c, (v >> 24));
				}
				break;
			case FT800EMU_DL_VERTEX2II:
				if (y >= gs.ScissorY.U && y < gs.ScissorY2.U)
				{
					int px = ((v >> 21) & 0x1FF) * 16;
					int py = ((v >> 12) & 0x1FF) * 16;
					switch (primitive)
					{
					case BITMAPS:
						displayBitmap(gs, bc, bs, bt, y, hsize, 
							px, 
							py,
							((v >> 7) & 0x1F),
							v & 0x7F);
						break;
					case POINTS:
						displayPoint(gs, bc, bs, bt, y, hsize, 
							px, 
							py);
						break;
					case LINE_STRIP:
						displayLineStrip(gs, bc, bs, bt, y, hsize, lss, 
							px
#if FT800EMU_DEBUG_LINES_SHIFT_HACK
								+ 16
#endif
								, 
							py);
						break;
					case EDGE_STRIP_R:
						displayEdgeStripR(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_L:
						displayEdgeStripL(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_A:
						displayEdgeStripA(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_B:
						displayEdgeStripB(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case RECTS:
						displayRects(gs, bc, bs, bt, y, hsize, rs, 
							px, 
							py);
						break;
					}
				}
				break;
			case FT800EMU_DL_VERTEX2F:				
				if (y >= gs.ScissorY.U && y < gs.ScissorY2.U)
				{
					int px = (v >> 15) & 0x3FFF;
					if ((v >> 15) & 0x4000) px = px - 0x4000;
					int py = (v)  & 0x3FFF;
					if ((v) & 0x4000) py = py - 0x4000;
					switch (primitive)
					{
					case BITMAPS:
						displayBitmap(gs, bc, bs, bt, y, hsize, 
							px, 
							py, 
							gs.BitmapHandle, 
							gs.Cell);
						break;
					case POINTS:
						displayPoint(gs, bc, bs, bt, y, hsize, 
							px, 
							py);
						break;
					case LINE_STRIP:
						displayLineStrip(gs, bc, bs, bt, y, hsize, lss,  
							px
#if FT800EMU_DEBUG_LINES_SHIFT_HACK
								+ 16
#endif
								, 
							py);
						break;
					case EDGE_STRIP_R:
						displayEdgeStripR(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_L:
						displayEdgeStripL(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_A:
						displayEdgeStripA(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case EDGE_STRIP_B:
						displayEdgeStripB(gs, bc, bs, bt, y, hsize, ess, 
							px, 
							py);
						break;
					case RECTS:
						displayRects(gs, bc, bs, bt, y, hsize, rs, 
							px, 
							py);
						break;
					}
				}
				break;
			}
		}
DisplayListDisplay:
		;
#if FT800EMU_DEBUG_ALPHA_MUL
		for (uint32_t x = 0; x < hsize; ++x)
		{
			bc[x] = mulalpha(bc[x], bc[x] >> 24) | 0xFF000000;
		}
#endif
		if (s_DebugMode)
		{
			switch (s_DebugMode)
			{
			case FT800EMU_DEBUGMODE_ALPHA:
				for (uint32_t x = 0; x < hsize; ++x)
				{
					int v = bc[x] >> 24;
					v *= s_DebugMultiplier;
					bc[x] = 0xFF000000 | (v << 16) | (v << 8) | (v);
				}
				break;
			case FT800EMU_DEBUGMODE_TAG:
				for (uint32_t x = 0; x < hsize; ++x)
				{
					int v = bt[x];
					v *= s_DebugMultiplier;
					bc[x] = 0xFF000000 | (v << 16) | (v << 8) | (v);
				}
				break;
			case FT800EMU_DEBUGMODE_STENCIL:
				for (uint32_t x = 0; x < hsize; ++x)
				{
					int v = bs[x];
					v *= s_DebugMultiplier;
					bc[x] = 0xFF000000 | (v << 16) | (v << 8) | (v);
				}
				break;
			}
		}
	}
}
	
void GraphicsProcessorClass::setDebugMode(int debugMode)
{
	s_DebugMode = debugMode;
}

int GraphicsProcessorClass::getDebugMode()
{
	return s_DebugMode;
}

void GraphicsProcessorClass::setDebugMultiplier(int debugMultiplier)
{
	s_DebugMultiplier = debugMultiplier;
}

int GraphicsProcessorClass::getDebugMultiplier()
{
	return s_DebugMultiplier;
}

} /* namespace FT800EMU */

/* end of file */
