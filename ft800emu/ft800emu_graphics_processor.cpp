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

struct GraphicsState
{
public:
	GraphicsState()
	{
		Primitive = 0; // Not sure if part of gs
		ColorARGB = 0x00000000;
		PointSize = 16;
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
	}

	int Primitive; // Not sure if part of gs
	argb8888 ColorARGB;
	int PointSize;
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
};

void GraphicsProcessorClass::process(argb8888 *screenArgb8888, uint32_t hsize, uint32_t vsize)
{
	// If a frame is process and there is no clear command, is the tag buffer etc reset or not?
	uint8_t *tag = Memory.getTagBuffer();
	int hsize16 = hsize * 16; // << 4
	int vsize16 = vsize * 16;

	// Swap the display list... Is this done before the frame render or after?
	if (Memory.getRam()[REG_DLSWAP] == SWAP_FRAME)
		Memory.swapDisplayList();

	const uint32_t *displayList = Memory.getDisplayList();
	uint8_t bs[512]; // stencil buffer (per-thread values!) TODO Max line width
	argb8888 bcaa[512][16]; // aa buffer
	bool bcaab[512]; // aa buffer flag
	// TODO: option for multicore rendering
	for (uint32_t y = 0; y < vsize; ++y)
	{
		GraphicsState gs = GraphicsState();
		argb8888 *bc = &screenArgb8888[y * hsize];
		uint8_t *bt = &tag[y * hsize];
		// limits for single core rendering on Intel Core 2 Duo
		// maximum 32 argb8888 memory ops per pixel on average
		// maximum 15360 argb8888 memory ops per line
		int px, py;

		// pre-clear bitmap buffer, but optimize!
		if (!(((displayList[0] & 0xFF000004) == ((FT800EMU_DL_CLEAR << 24) | 0x04))
			|| (((displayList[0] >> 24) == FT800EMU_DL_CLEAR_COLOR_RGB) 
				&& ((displayList[1] & 0xFF000004) == ((FT800EMU_DL_CLEAR << 24) | 0x04)))))
		{
			// about loop+480+480 ops
			for (uint32_t i = 0; i < hsize; ++i)
			{
				bc[i] = 0xFF000000;
				bcaab[i] = false;
			}
		}
		// pre-clear line stencil buffer, but optimize!
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
				case FT800EMU_DL_CLEAR_COLOR_RGB:
					gs.ClearColorARGB = (gs.ClearColorARGB & 0xFF000000) | (v & 0x00FFFFFF);
					break;
				case FT800EMU_DL_TAG:
					gs.Tag = v & 0xFF;
					break;
				case FT800EMU_DL_COLOR_RGB:
					gs.ColorARGB = (gs.ColorARGB & 0xFF000000) | (v & 0x00FFFFFF);
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
				case FT800EMU_DL_BEGIN:
					gs.Primitive = v & 0x0F;
					break;
				case FT800EMU_DL_END:
					gs.Primitive = 0;
					break;
				case FT800EMU_DL_CLEAR:
					// TODO: Create merged versions for optimization to avoid repeated looping...
					if (v & 0x04)
					{
						// clear color buffer (about loop+480 ops)
						for (uint32_t i = 0; i < hsize; ++i)
						{
							// How does alpha work here?
							bc[i] = gs.ClearColorARGB;
							bcaab[i] = false;
						}
					}
					if (v & 0x02)
					{
						// Clear stencil buffer (about loop+480 ops)
						for (uint32_t i = 0; i < hsize; ++i)
						{
							bs[i] = gs.ClearStencil;
						}
					}
					if (gs.TagMask && v & 0x01) // TODO What when clear when clear mask false?
					{
						// Clear tag buffer (about loop+480 ops)
						for (uint32_t i = 0; i < hsize; ++i)
						{
							bt[i] = gs.ClearTag;
						}
					}
					break;
				}
			case FT800EMU_DL_VERTEX2II:
//#define VERTEX2F(x,y) ((1UL<<30)|(((x)&32767UL)<<15)|(((y)&32767UL)<<0))
//#define VERTEX2II(x,y,handle,cell) ((2UL<<30)|(((x)&511UL)<<21)|(((y)&511UL)<<12)|(((handle)&31UL)<<7)|(((cell)&127UL)<<0))
				switch (gs.Primitive)
				{
				case POINTS:
					// Ugly square point rendering  code (it's a square)
					size_t pr = gs.PointSize >> 4;
					px = (v >> 21) & 0xFF;
					py = (v >> 12) & 0xFF;
					if (py - pr <= y && y <= py + pr)
					{
						size_t pxl = px - pr;
						size_t pxr = px + pr;
						for (size_t x = pxl; x <= pxr; ++x)
						{
							bc[x] = gs.ColorARGB;
							bcaab[x] = false;
						}
					}
					break;
				}
				break;
			case FT800EMU_DL_VERTEX2F:
				switch (gs.Primitive)
				{
				case POINTS:
					break;
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
