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

// Project includes
#include "ft800emu_memory.h"
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

namespace FT800EMU {

GraphicsProcessorClass GraphicsProcessor;

namespace {

struct GraphicsState
{
public:
	GraphicsState()
	{
		Primitive = 0; // Not sure if part of gs
		ColorARGB = 0xFF000000; // Default alpha 255?
		PointSize = 16;
		ScissorX = 0;
		ScissorY = 0;
		ScissorWidth = 512;
		ScissorHeight = 512;
		ScissorX2 = 512;
		ScissorY2 = 512;
		ClearColorARGB = 0xFF000000; // Not found in the programmer guide
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
	}

	int Primitive; // Not sure if part of gs
	argb8888 ColorARGB;
	int PointSize;
	uint32_t ScissorX;
	uint32_t ScissorY;
	uint32_t ScissorWidth;
	uint32_t ScissorHeight;
	uint32_t ScissorX2; // min(hsize, X + Width)
	uint32_t ScissorY2; // Y + Height
	argb8888 ClearColorARGB; // Not in the programmer guide's graphics state table
	uint8_t ClearStencil; // PG says "Stencil value" instead of "Stencil clear value"...?
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
};

struct BitmapInfo
{
	uint32_t Source;
	int LayoutFormat;
	int LayoutStride;
	int LayoutHeight;
	int SizeFilter;
	int SizeWrapX;
	int SizeWrapY;
	int SizeWidth;
	int SizeHeight;
};

BitmapInfo s_BitmapInfo[32];

__forceinline unsigned int div255(int value)
{
	return (value * 257 + 257) >> 16;
}

__forceinline argb8888 mulalpha(argb8888 value, int alpha)
{
	// todo optimize!
	argb8888 result = div255(((value & 0x00FF0000) >> 16) * alpha);
	result <<= 8;
	result |= div255(((value & 0x0000FF00) >> 8) * alpha);
	result <<= 8;
	result |= div255((value & 0x000000FF) * alpha);
	return result;
}

__forceinline void writeTag(const GraphicsState &gs, uint8_t *bt, int x)
{
	if (gs.TagMask) bt[x] = gs.Tag;
}

__forceinline bool testStencil(const GraphicsState &gs, uint8_t *bs, int x)
{
	bool result;
	switch (gs.StencilFunc)
	{
	case NEVER:
		result = false;
		break;
	case LESS:
		result = (bs[x] & gs.StencilFuncMask) < (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case LEQUAL:
		result = (bs[x] & gs.StencilFuncMask) <= (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case GREATER:
		result = (bs[x] & gs.StencilFuncMask) > (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case GEQUAL:
		result = (bs[x] & gs.StencilFuncMask) >= (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case EQUAL:
		result = (bs[x] & gs.StencilFuncMask) == (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case NOTEQUAL:
		result = (bs[x] & gs.StencilFuncMask) != (gs.StencilFuncRef & gs.StencilFuncMask);
		break;
	case ALWAYS:
		result = true;
		break;
	default:
		// error
		printf("Invalid stencil func");
		result = true;
	}
	switch (result ? gs.StencilOpPass : gs.StencilOpFail)
	{
	case KEEP:
		break;
	case ZERO:
		bs[x] = 0;
		break;
	case REPLACE:
		bs[x] = gs.StencilFuncRef; // i assume
		break;
	case INCR:
		if (bs[x] < 0xFF) ++bs[x]; // i assume
		break;
	//case INCR_WRAP:
	//	++bs[x];
	//	break;
	case DECR:
		if (bs[x] > 0x00) --bs[x]; // i assume
		break;
	//case DECR_WRAP:
	//	--bs[x];
	//	break;
	case INVERT:
		bs[x] = ~bs[x];
		break;
	default:
		// error
		printf("Invalid stencil op");
		break;
	}
	return result;
}

__forceinline argb8888 getPaletted(uint8_t *ram, uint8_t value)
{
	uint32_t result = static_cast<uint32_t *>(static_cast<void *>(&ram[RAM_PAL]))[value];
	return (result >> 8) & (result << 24); // really, RGBA?
}

void displayPoint(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int hsize, int px, int py)
{
	int yy = y * 16;
	int r = gs.PointSize;
	int rsq = r * r;

	int pytop = py - r; // incl pixel*16 top
	int pybtm = py + r - 1; // incl pixel*16 btm

	int pytopi = (pytop + 8) >> 4;
	int pybtmi = (pybtm + 8) >> 4;

	if (pytopi <= y && y <= pybtmi)
	{
		int pxlef = px - r;
		int pxrig = px + r - 1;

		int pxlefi = (pxlef + 8) >> 4;
		int pxrigi = (pxrig + 8) >> 4;

		pxlefi = max((int)gs.ScissorX, pxlefi);
		pxrigi = min((int)gs.ScissorX2 - 1, pxrigi);

		int border = 16 * r;
		int border2sqrt = (int)sqrtf((float)(border * 2)); // sqrt :(
		for (int x = pxlefi; x <= pxrigi; ++x)
		{
			// todo optimize! (works fine for 500 average sized points at full fps)
			int xx = x * 16;
			int xdist = xx - px;
			int ydist = yy - py;
			int distctr = (xdist * xdist) + (ydist * ydist);
			int distouter = distctr - border;
			int distinner = distctr + border;
			if (distinner < rsq)
			{
				if (testStencil(gs, bs, x))
				{
					int alpha = gs.ColorARGB >> 24;
					bc[x] = mulalpha(bc[x], (255 - alpha)) + mulalpha(gs.ColorARGB, alpha);
					writeTag(gs, bt, x);
				}
			}
			else if (distouter < rsq)
			{
				if (testStencil(gs, bs, x))
				{
					int alpha = gs.ColorARGB >> 24;
					alpha *= (int)sqrtf((float)(rsq - distouter)); // sqrt :(
					alpha /= border2sqrt;
					bc[x] = mulalpha(bc[x], (255 - alpha)) + mulalpha(gs.ColorARGB, alpha);
					writeTag(gs, bt, x);
				}
			}
		}
	}
}

}

void GraphicsProcessorClass::begin()
{
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
	// If a frame is process and there is no clear command, is the tag buffer etc reset or not?
	uint8_t *tag = Memory.getTagBuffer();
	// int hsize16 = hsize * 16; // << 4
	// int vsize16 = vsize * 16;

	// Swap the display list... Is this done before the frame render or after?
	if (Memory.getRam()[REG_DLSWAP] == SWAP_FRAME)
		Memory.swapDisplayList();

	const uint32_t *displayList = Memory.getDisplayList();
	uint8_t bs[512]; // stencil buffer (per-thread values!) TODO Max line width
	// TODO: option for multicore rendering
	for (uint32_t y = 0; y < vsize; ++y)
	{
		GraphicsState gs = GraphicsState();
		gs.ScissorX2 = min((int)hsize, gs.ScissorX2);
		argb8888 *bc = &screenArgb8888[(upsideDown ? (vsize - y - 1) : y) * hsize];
		uint8_t *bt = &tag[y * hsize];
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

		// run display list
		for (size_t c = 0; c < FT800EMU_DISPLAY_LIST_SIZE; ++c)
		{
			uint32_t v = displayList[c]; // (up to 2048 ops)
			switch (v >> 30)
			{
			case 0:
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
					s_BitmapInfo[gs.BitmapHandle].LayoutFormat = (v >> 19) & 0x1F;
					s_BitmapInfo[gs.BitmapHandle].LayoutStride = (v >> 9) & 0x3FF;
					s_BitmapInfo[gs.BitmapHandle].LayoutHeight = v & 0x1FF;
					break;
				case FT800EMU_DL_STENCIL_FUNC:
					gs.StencilFunc = (v >> 16) & 0x7;
					gs.StencilFuncRef = (v >> 8) & 0xFF;
					gs.StencilFuncMask = v & 0xFF;
					break;
				case FT800EMU_DL_STENCIL_OP:
					gs.StencilOpFail = (v >> 3) & 0x7;
					gs.StencilOpPass = v & 0x7;
					break;
				case FT800EMU_DL_POINT_SIZE:
					gs.PointSize = v & 0x1FFF;
					break;
				case FT800EMU_DL_CLEAR_COLOR_A:
					gs.ClearColorARGB = (gs.ClearColorARGB & 0x00FFFFFF) | (v & 0xFF000000);
					break;
				case FT800EMU_DL_COLOR_A:
					gs.ColorARGB = (gs.ColorARGB & 0x00FFFFFF) | (v & 0xFF000000);
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
				case FT800EMU_DL_SCISSOR_XY:
					gs.ScissorY = v & 0x1FF;
					gs.ScissorX = (v >> 9) & 0x1FF;
					gs.ScissorX2 = min(hsize, gs.ScissorX + gs.ScissorWidth);
					gs.ScissorY2 = gs.ScissorY + gs.ScissorHeight;
					break;
				case FT800EMU_DL_SCISSOR_SIZE:
					gs.ScissorHeight = v & 0x3FF;
					gs.ScissorWidth = (v >> 10) & 0x3FF;
					gs.ScissorX2 = min(hsize, gs.ScissorX + gs.ScissorWidth);
					gs.ScissorY2 = gs.ScissorY + gs.ScissorHeight;
					break;
				case FT800EMU_DL_BEGIN:
					gs.Primitive = v & 0x0F;
					break;
				case FT800EMU_DL_COLOR_MASK:
					gs.ColorMaskARGB = (((v >> 1) & 0x1) * 0xFF)
						| (((v >> 2) & 0x1) * 0xFF00)
						| (((v >> 3) & 0x1) * 0xFF0000)
						| ((v & 0x1) * 0xFF000000);
					break;
				case FT800EMU_DL_END:
					gs.Primitive = 0;
					break;
				case FT800EMU_DL_CLEAR:
					if (y >= gs.ScissorY && y < gs.ScissorY2)
					{
						if (v & 0x04)
						{
							// clear color buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX; i < gs.ScissorX2; ++i)
							{
								bc[i] = gs.ClearColorARGB;
							}
						}
						if (v & 0x02)
						{
							// Clear stencil buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX; i < gs.ScissorX2; ++i)
							{
								bs[i] = gs.ClearStencil;
							}
						}
						if (gs.TagMask && v & 0x01) // TODO What when clear when clear mask false?
						{
							// Clear tag buffer (about loop+480 ops)
							for (uint32_t i = gs.ScissorX; i < gs.ScissorX2; ++i)
							{
								bt[i] = gs.ClearTag;
							}
						}
					}
					break;
				}
				break;
			case FT800EMU_DL_VERTEX2II:
				if (y >= gs.ScissorY && y < gs.ScissorY2)
				{
					switch (gs.Primitive)
					{
					case POINTS:
						displayPoint(gs, bc, bs, bt, y, hsize, 
							((v >> 21) & 0xFF) * 16, 
							((v >> 12) & 0xFF) * 16);
						break;
					}
				}
				break;
			case FT800EMU_DL_VERTEX2F:				
				if (y >= gs.ScissorY && y < gs.ScissorY2)
				{
					switch (gs.Primitive)
					{
					case POINTS:
						displayPoint(gs, bc, bs, bt, y, hsize, 
							((v >> 15) & 0x7FFF), 
							(v & 0x7FFF));
						break;
					}
				}
				break;
			}
		}
DisplayListDisplay:
		;
	}
}

} /* namespace FT800EMU */

/* end of file */
