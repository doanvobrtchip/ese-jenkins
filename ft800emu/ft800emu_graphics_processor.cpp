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

// Enable or disable debug messages
#define FT800EMU_DEBUG 0

#ifndef FTEMU_CMAKE
#	define FTEMU_SSE41 0
#endif
#define FTEMU_SSE41_INSTRUCTIONS FTEMU_SSE41
#define FTEMU_SSE41_INSTRUCTIONS_ALL 0 // Slightly slower code, no performance improvement, just for testing
#define FTEMU_SSE41_INSTRUCTIONS_USE FTEMU_SSE41 // Faster code, up to 40% faster depending on the used features (slower in debug, though)

#define FT800EMU_LIMIT_JUMP_LOOP 1
#define FT800EMU_LIMIT_JUMP_LOOP_COUNT (FT800EMU_DISPLAY_LIST_SIZE * 16)

// System includes
#include <vector>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stack>
#include <sstream>
#if FTEMU_SSE41_INSTRUCTIONS
#	include <xmmintrin.h>
#	include <emmintrin.h>
#	include <smmintrin.h>
#endif
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
#	include <SDL_thread.h>
#	include <SDL_assert.h>
#else
#	ifdef WIN32
#		include "ft8xxemu_system_windows.h"
#	endif
#endif

// Project includes
#include "ft8xxemu_inttypes.h"
#include "ft8xxemu_system.h"
#include "ft8xxemu_thread_state.h"
#include "ft8xxemu_window_output.h"
#include "ft8xxemu_minmax.h"
#include "ft800emu_memory.h"
#include "ft800emu_touch.h"
#include "ft800emu_vc.h"

// using namespace ...;

#if defined(FTEMU_SDL2)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, name, data)
#elif defined(FTEMU_SDL)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, data)
#endif

#ifdef FT810EMU_MODE
#define FT810EMU_SWAPXY_PARAM , const bool swapXY
#define FT810EMU_SWAPXY , swapXY
#define FT810EMU_SWAPXY_FALSE , false
#else
#define FT810EMU_SWAPXY_PARAM
#define FT810EMU_SWAPXY
#define FT810EMU_SWAPXY_FALSE
#endif

namespace FT800EMU {

GraphicsProcessorClass GraphicsProcessor;

namespace {

#pragma region Graphics State

struct GraphicsState
{
public:
	BT8XXEMU_FORCE_INLINE GraphicsState(
#ifdef FT810EMU_MODE
		const bool swapXY
#endif
		)
	{
		DebugDisplayListIndex = 0;
		ColorARGB = 0xFFFFFFFF;
		PointSize = 16;
		LineWidth = 16;
		ScissorX.I = 0;
		ScissorY.I = 0;
		ScissorWidth = FT800EMU_SCREEN_WIDTH_MAX;
		ScissorHeight = FT800EMU_SCREEN_HEIGHT_MAX;
		ScissorX2.I = FT800EMU_SCREEN_WIDTH_MAX;
		ScissorY2.I = FT800EMU_SCREEN_HEIGHT_MAX;
		ClearColorARGB = 0x00000000;
		ClearStencil = 0;
		ClearTag = 0;
		Tag = 255;
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
#ifdef FT810EMU_MODE
		if (swapXY)
		{
			BitmapTransformA = 0;
			BitmapTransformB = 256;
			BitmapTransformC = 0;
			BitmapTransformD = 256;
			BitmapTransformE = 0;
			BitmapTransformF = 0;
		}
		else
#endif
		{
			BitmapTransformA = 256;
			BitmapTransformB = 0;
			BitmapTransformC = 0;
			BitmapTransformD = 0;
			BitmapTransformE = 256;
			BitmapTransformF = 0;
		}
		BlendFuncSrc = SRC_ALPHA;
		BlendFuncDst = ONE_MINUS_SRC_ALPHA;
		AlphaFunc = ALWAYS;
		AlphaFuncRef = 0;
#ifdef FT810EMU_MODE
		VertexFormatShift = 0;
		VertexTranslateX = 0;
		VertexTranslateY = 0;
		PaletteSource = 0;
#endif
	}

	int DebugDisplayListIndex;
	argb8888 ColorARGB;
	int PointSize;
	int LineWidth;
	union { uint32_t U; int I; } ScissorX; // NOTE: Scissor already swapped during XY swapped render
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
#ifdef FT810EMU_MODE
	int VertexFormatShift; // = (4 - VertexFormat)
	int VertexTranslateX;
	int VertexTranslateY;
	int PaletteSource;
#endif
};

BitmapInfo s_BitmapInfoMain[32];

bool s_RegPwmDutyEmulation = false;

uint32_t s_DebugTraceX = 0;
uint32_t s_DebugTraceLine = 0;
int *s_DebugTraceStack = NULL;
int s_DebugTraceStackMax = 0;
int *s_DebugTraceStackSize = NULL;

bool s_ThreadPriorityRealtime = true;

#pragma endregion

#pragma region Math

#if FTEMU_SSE41_INSTRUCTIONS

#ifdef WIN32
#	define FT800EMU_STATIC_M128I(FTNAME) static const __m128i FTNAME
#	define FT800EMU_STATIC_M128I_CAST(FTNAME) &FTNAME
#else
#	define FT800EMU_STATIC_M128I(FTNAME) static const int8_t FTNAME[16] __attribute__((aligned(16)))
#	define FT800EMU_STATIC_M128I_CAST(FTNAME) static_cast<const __m128i *>(static_cast<const void *>(&FTNAME))
#endif

// Here's a nice reference table with all the intrinsics listed:
// http://www.taffysoft.com/pages/20120418-01.html

BT8XXEMU_FORCE_INLINE __m128i to_m128i(const argb8888 &value)
{
	const register __m128i mzero = _mm_setzero_si128();
	register __m128i result = _mm_set1_epi32(value);
	result = _mm_unpacklo_epi8(result, mzero);
	result = _mm_unpacklo_epi8(result, mzero);
	return result;
}

BT8XXEMU_FORCE_INLINE argb8888 to_argb8888(const __m128i &value)
{
	register __m128i result = _mm_packus_epi16(value, value);
	result = _mm_packus_epi16(result, result);
	return _mm_cvtsi128_si32(result);
}

BT8XXEMU_FORCE_INLINE __m128i div255(const __m128i &value)
{
	// There exists no integer division in SSE so we have to use these division tricks anyways.
	FT800EMU_STATIC_M128I(cm257) = { 1,1,0,0, 1,1,0,0, 1,1,0,0, 1,1,0,0 };
	const register __m128i m257 =  _mm_load_si128(FT800EMU_STATIC_M128I_CAST(cm257));
	register __m128i result = _mm_mullo_epi32(value, m257);
	result = _mm_add_epi32(result, m257);
	result = _mm_srli_epi32(result, 16);
	return result;
}

BT8XXEMU_FORCE_INLINE __m128i mulalpha_argb(const __m128i &value, const int &alpha)
{
	const register __m128i malpha = _mm_set1_epi32(alpha);
	register __m128i result = _mm_mullo_epi32(value, malpha);
	result = div255(result);
	return result;
}

BT8XXEMU_FORCE_INLINE __m128i mul_argb(const __m128i &left, const __m128i &right)
{
	register __m128i result = _mm_mullo_epi32(left, right);
	result = div255(result);
	return result;
}

BT8XXEMU_FORCE_INLINE __m128i add_argb_safe(const __m128i &left, const __m128i &right)
{
	FT800EMU_STATIC_M128I(cmmax) = { ~0,0,0,0, ~0,0,0,0, ~0,0,0,0, ~0,0,0,0 };
	const register __m128i mmax = _mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmmax));
	register __m128i result = _mm_add_epi32(left, right);
	result = _mm_min_epi32(result, mmax);
	return result;
}

BT8XXEMU_FORCE_INLINE __m128i getAlphaSplat(const int &func, const __m128i &src, const __m128i &dst)
{
	FT800EMU_STATIC_M128I(cmone) = { 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 };
	FT800EMU_STATIC_M128I(cmmax) = { ~0,0,0,0, ~0,0,0,0, ~0,0,0,0, ~0,0,0,0 };
	switch (func)
	{
	case ZERO:
		return _mm_setzero_si128();
	case ONE:
		return _mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmone));
	case SRC_ALPHA:
		return _mm_shuffle_epi32(src, _MM_SHUFFLE(3, 3, 3, 3)); // test
	case DST_ALPHA:
		return _mm_shuffle_epi32(dst, _MM_SHUFFLE(3, 3, 3, 3)); // test
	case ONE_MINUS_SRC_ALPHA:
		return _mm_sub_epi32(_mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmmax)), _mm_shuffle_epi32(src, _MM_SHUFFLE(3, 3, 3, 3)));
	case ONE_MINUS_DST_ALPHA:
		return _mm_sub_epi32(_mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmmax)), _mm_shuffle_epi32(dst, _MM_SHUFFLE(3, 3, 3, 3)));
	}
	FTEMU_printf("Invalid blend func (sse)\n");
	if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid blend func (sse)");
	return _mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmone));
}


#endif

BT8XXEMU_FORCE_INLINE unsigned int div255(const int &value)
{
	return (value * 257 + 257) >> 16;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div63(const int &value)
{
	return ((value * 16575) + 65) >> 12;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div31(const int &value)
{
	return ((value * 8415) + 33) >> 10;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div15(const int &value)
{
	return ((value * 4335) + 17) >> 8;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div7(const int &value)
{
	// Slightly noticeable loss of accuracy but also noticably faster.
	return ((value * 2295) + 9) >> 6;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div3(const int &value)
{
	return value * 85;
}

BT8XXEMU_FORCE_INLINE argb8888 mulalpha128_argb(const argb8888 &value, const int &alpha)
{
	const argb8888 result = (((((value & 0xFF000000) >> 24) * alpha) >> 7) << 24)
		| (((((value & 0x00FF0000) >> 16) * alpha) >> 7) << 16)
		| (((((value & 0x0000FF00) >> 8) * alpha) >> 7) << 8)
		| ((((value & 0x000000FF) * alpha) >> 7) & 0x000000FF);
	return result;
}

BT8XXEMU_FORCE_INLINE argb8888 mulalpha_argb(const argb8888 &value, const int &alpha)
{
#if FTEMU_SSE41_INSTRUCTIONS_ALL
	return to_argb8888(mulalpha_argb(to_m128i(value), alpha));
#else
	const argb8888 result = (div255(((value & 0xFF000000) >> 24) * alpha) << 24)
		| (div255(((value & 0x00FF0000) >> 16) * alpha) << 16)
		| (div255(((value & 0x0000FF00) >> 8) * alpha) << 8)
		| (div255((value & 0x000000FF) * alpha) & 0x000000FF);
	return result;
#endif
}

BT8XXEMU_FORCE_INLINE argb8888 mul_argb(const argb8888 &left, const argb8888 &right)
{
#if FTEMU_SSE41_INSTRUCTIONS_ALL
	return to_argb8888(mul_argb(to_m128i(left), to_m128i(right)));
#else
	argb8888 result = (div255((left >> 24) * (right >> 24)) << 24)
		| (div255(((left >> 16) & 0xFF) * ((right >> 16) & 0xFF)) << 16)
		| (div255(((left >> 8) & 0xFF) * ((right >> 8) & 0xFF)) << 8)
		| div255((left & 0xFF) * (right & 0xFF));
	return result;
#endif
}

BT8XXEMU_FORCE_INLINE argb8888 add_argb_safe(const argb8888 &left, const argb8888 &right)
{
#if FTEMU_SSE41_INSTRUCTIONS_ALL
	return to_argb8888(add_argb_safe(to_m128i(left), to_m128i(right)));
#else
	// add rgb and handle overflow
	const argb8888 ag = ((left & 0xFF00FF00) >> 8) + ((right & 0xFF00FF00) >> 8);
	const argb8888 rb = (left & 0x00FF00FF) + (right & 0x00FF00FF);
	const argb8888 agclip = (ag & 0x00FF00FF) << 8;
	const argb8888 rbclip = rb & 0x00FF00FF;
	const argb8888 agover = (ag & 0x01000100) * 0xFF;
	const argb8888 rbover = ((rb & 0x01000100) * 0xFF) >> 8;
	return (agclip | agover) + (rbover | rbclip);
#endif
}

#pragma endregion

#pragma region Write Buffer

BT8XXEMU_FORCE_INLINE void writeTag(const GraphicsState &gs, uint8_t *bt, int x)
{
	if (gs.TagMask) bt[x] = gs.Tag;
}

BT8XXEMU_FORCE_INLINE bool testStencilNoWrite(const GraphicsState &gs, const uint8_t *bs, const int &x)
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
		FTEMU_printf("Invalid stencil func\n");
		if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid stencil func");
		return true;
	}
}

BT8XXEMU_FORCE_INLINE bool testStencil(const GraphicsState &gs, uint8_t *bs, const int &x)
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
		else bs[x] = (bs[x] & (~gs.StencilMask));
		break;
	case DECR:
		if (bs[x] > 0x00) bs[x] = ((bs[x] - 1) & gs.StencilMask) | (bs[x] & (~gs.StencilMask)); // i assume
		else bs[x] = (bs[x] | gs.StencilMask);
		break;
	case INVERT:
		bs[x] = ((~bs[x]) & gs.StencilMask) | (bs[x] & (~gs.StencilMask));
		break;
	default:
		// error
		FTEMU_printf("Invalid stencil op\n");
		if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid stencil op");
		break;
	}
	return result;
}

BT8XXEMU_FORCE_INLINE bool testStencil(const GraphicsState &gs, uint8_t *bs, const int &x, const bool &write)
{
	return write ? testStencil(gs, bs, x) : testStencilNoWrite(gs, bs, x);
}

#ifdef FT810EMU_MODE

BT8XXEMU_FORCE_INLINE argb8888 getPaletted8(const uint8_t *ram, const uint8_t &value, const int paletteSource)
{
	uint8_t val = reinterpret_cast<const uint8_t *>(&reinterpret_cast<const uint32_t *>(&ram[paletteSource])[value])[0];
	return val << ((paletteSource & 3) << 3);
}

BT8XXEMU_FORCE_INLINE argb8888 getPaletted565(const uint8_t *ram, const uint8_t &value, const int paletteSource)
{
	uint16_t val = static_cast<const uint16_t *>(static_cast<const void *>(&ram[paletteSource]))[value];
	return 0xFF000000 // todo opt
		| (mul255div31(val >> 11) << 16)
		| (mul255div63((val >> 5) & 0x3F) << 8)
		| mul255div31(val & 0x1F);
}

BT8XXEMU_FORCE_INLINE argb8888 getPaletted4444(const uint8_t *ram, const uint8_t &value, const int paletteSource)
{
	uint16_t val = static_cast<const uint16_t *>(static_cast<const void *>(&ram[paletteSource]))[value];
	return (mul255div15(val >> 12) << 24)
		| (mul255div15((val >> 8) & 0xF) << 16)
		| (mul255div15((val >> 4) & 0xF) << 8)
		| (mul255div15(val & 0xF));
}

#else

BT8XXEMU_FORCE_INLINE argb8888 getPaletted(const uint8_t *ram, const uint8_t &value)
{
	return static_cast<const uint32_t *>(static_cast<const void *>(&ram[RAM_PAL]))[value];
}

#endif

BT8XXEMU_FORCE_INLINE bool testAlpha(const GraphicsState &gs, const argb8888 &dst)
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
	FTEMU_printf("Invalid alpha test func\n");
	if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid alpha test func");
	return true;
}

BT8XXEMU_FORCE_INLINE int getAlpha(const int &func, const argb8888 &src, const argb8888 &dst)
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
	FTEMU_printf("Invalid blend func\n");
	if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid blend func");
	return 255;
}

BT8XXEMU_FORCE_INLINE argb8888 blend(const GraphicsState &gs, const argb8888 &src, const argb8888 &dst)
{
#if FTEMU_SSE41_INSTRUCTIONS_USE // > 10% faster
	FT800EMU_STATIC_M128I(cmmax) = { ~0,0,0,0, ~0,0,0,0, ~0,0,0,0, ~0,0,0,0 };
	const register __m128i mmax = _mm_load_si128(FT800EMU_STATIC_M128I_CAST(cmmax));
	const register __m128i msrc = to_m128i(src);
	const register __m128i mdst = to_m128i(dst);
	const register __m128i srca = getAlphaSplat(gs.BlendFuncSrc, msrc, mdst);
	const register __m128i dsta = getAlphaSplat(gs.BlendFuncDst, msrc, mdst);
	register __m128i result = div255(_mm_add_epi32(_mm_mullo_epi32(msrc, srca), _mm_mullo_epi32(mdst, dsta)));
	result = _mm_min_epi32(result, mmax);
	return (to_argb8888(result) & gs.ColorMaskARGB) | (dst & ~gs.ColorMaskARGB);
#else
	argb8888 result = add_argb_safe(mulalpha_argb(src, getAlpha(gs.BlendFuncSrc, src, dst)), mulalpha_argb(dst, getAlpha(gs.BlendFuncDst, src, dst)));
	return (result & gs.ColorMaskARGB) | (dst & ~gs.ColorMaskARGB);
#endif

}

template <bool debugTrace>
BT8XXEMU_FORCE_INLINE void processPixel(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int x, const argb8888 out)
{
	if (testAlpha(gs, out))
	{
		if (testStencil(gs, bs, x))
		{
			bc[x] = blend(gs, out, bc[x]);
			writeTag(gs, bt, x);

			// Conditionally compiled debug functionality, will only be called on the line that is being debugged.
			if (debugTrace)
			{
				// Check the point to be traced.
				if (x == s_DebugTraceX)
				{
					if (*s_DebugTraceStackSize < s_DebugTraceStackMax)
					{
						s_DebugTraceStack[*s_DebugTraceStackSize] = s_DebugTraceLine;
						++(*s_DebugTraceStackSize);
					}
				}
			}
		}
	}
}

#pragma endregion

#pragma region Primitive: Point

#define FT800EMU_POINT_STENCIL_INCREASE 1

template <bool debugTrace>
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
				if (psin > 0)
				{
					const argb8888 out = gs.ColorARGB;
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
				}
				else // Small point in single pixel
				{
					const int outalpha =  ((gs.ColorARGB >> 24) * ps) >> 3;
					const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
				}
			}
			else if (distsq <= pssqout) // AA Border
			{
				const double dist256sqd = (double)distsq * 256.0; // double.. distsq is 1/16 squared, multiply twice by 16
				const double dist256d = sqrt(dist256sqd); // sqrt..
				const long dist256 = (long)dist256d;
				const int alpha = 256 - max(min(dist256 - psin256, 256), 0);
				const int outalpha = ((gs.ColorARGB >> 24) * alpha) >> 8;
				const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
				processPixel<debugTrace>(gs, bc, bs, bt, x, out);
			}
#if FT800EMU_POINT_STENCIL_INCREASE
			else if (distsq <= pssqstc)
			{
				const argb8888 out = gs.ColorARGB & 0x00FFFFFF;
				processPixel<debugTrace>(gs, bc, bs, bt, x, out);
			}
#endif
		}
	}
}

template <bool debugTrace>
void displayPoint(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int /*hsize*/, int px, int py)
{
	displayPoint<debugTrace>(gs, gs.PointSize, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, px, py);
}

// Utility for primitives that use points to blend the AA at the edges.
// Only used for 4 pixels at most in a primitive.
// Only used with point size of 16 or larger.
// Only used on the AA border itself.
BT8XXEMU_FORCE_INLINE int getPointAlpha256(const int ps, const int x, const int y, const int px, const int py)
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

BT8XXEMU_FORCE_INLINE bool wrap(int &value, const int &max, const int &type)
{
	switch (type)
	{
	case BORDER:
		if (value < 0) return false;
		else if (value >= max) return false;
		break;
	case REPEAT:
		value = value & (max - 1);
		break;
	}
	return true;
}

static const argb8888 s_VGAPalette[] =
{
	0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};

BT8XXEMU_FORCE_INLINE const uint8_t &bmpSrc8(const uint8_t *ram, const uint32_t srci, const int idx)
{
	const int i = (srci + idx) & FT800EMU_ADDR_MASK;
	return ram[i];
}

BT8XXEMU_FORCE_INLINE const uint8_t &bmpSrc16(const uint8_t *ram, const uint32_t srci, const int idx)
{
	const int i = (srci + idx) & (FT800EMU_ADDR_MASK - 1);
	return ram[i];
}

// uses pixel units
#if FT810EMU_MODE
BT8XXEMU_FORCE_INLINE argb8888 sampleBitmapAt(const uint8_t *ram, const uint32_t srci, int x, int y, const int height, const int format, const int stride, const int wrapx, const int wrapy, const BitmapInfo *const bitmapInfo, const int paletteSource)
#else
BT8XXEMU_FORCE_INLINE argb8888 sampleBitmapAt(const uint8_t *ram, const uint32_t srci, int x, int y, const int height, const int format, const int stride, const int wrapx, const int wrapy, const BitmapInfo *const bitmapInfo)
#endif
{
    int xo;   // x byte offset
	switch (format)
	{
	case ARGB1555:
	case ARGB4:
	case RGB565:
		xo = x << 1;
		break;
	case RGB332:
	case ARGB2:
	case PALETTED:
#ifdef FT810EMU_MODE
	case PALETTED565:
	case PALETTED4444:
	case PALETTED8:
#endif
	case L8:
	case BARGRAPH:
		xo = x;
		break;
	case L1:
	case TEXT8X8:
		xo = x >> 3;
		break;
	case TEXTVGA:
		xo = (x >> 3) << 1;
		break;
#ifdef FT810EMU_MODE
	case L2:
		xo = x >> 2;
		break;
#endif
	case L4:
		xo = x >> 1;
		break;
	}

	if (!wrap(xo, stride, wrapx)) return 0x00000000;
	if (format != BARGRAPH) if (!wrap(y, height, wrapy)) return 0x00000000;
	int py = y * stride;
	switch (format)
	{
	case ARGB1555:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + xo)));
			return (((val >> 15) * 255) << 24)
				| (mul255div31((val >> 10) & 0x1F) << 16)
				| (mul255div31((val >> 5) & 0x1F) << 8)
				| mul255div31(val & 0x1F);
		}
	case L1:
		{
			int val = (bmpSrc8(ram, srci, py + xo) >> (7 - (x % 8))) & 0x1;
			val *= 255;
			return (val << 24) | 0x00FFFFFF;
		}
#ifdef FT810EMU_MODE
	case L2:
		{
			int val = (bmpSrc8(ram, srci, py + xo) >> ((3 - (x % 4)) << 1)) & 0x3;
			val = mul255div3(val);
			return (val << 24) | 0x00FFFFFF;
		}
#endif
	case L4:
		{
			int val = (bmpSrc8(ram, srci, py + xo) >> (((x + 1) % 2) << 2)) & 0xF;
			val = mul255div15(val);
			return (val << 24) | 0x00FFFFFF;
		}
	case L8:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return (val << 24) | 0x00FFFFFF;
		}
	case RGB332:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return 0xFF000000
				| (mul255div7(val >> 5) << 16)
				| (mul255div7((val >> 2) & 0x7) << 8)
				| mul255div3(val & 0x3);
		}
	case ARGB2:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return (((val >> 6) * 255 / 3) << 24)
				| (mul255div3((val >> 4) & 0x3) << 16)
				| (mul255div3((val >> 2) & 0x3) << 8)
				| mul255div3(val & 0x3);
		}
	case ARGB4:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + xo)));
			return (mul255div15(val >> 12) << 24)
				| (mul255div15((val >> 8) & 0xF) << 16)
				| (mul255div15((val >> 4) & 0xF) << 8)
				| (mul255div15(val & 0xF));
		}
	case RGB565:
		{
			uint16_t val = *static_cast<const uint16_t *>(static_cast<const void *>(&bmpSrc16(ram, srci, py + xo)));
			return 0xFF000000 // todo opt
				| (mul255div31(val >> 11) << 16)
				| (mul255div63((val >> 5) & 0x3F) << 8)
				| mul255div31(val & 0x1F);
		}
	case PALETTED:
		{
#ifdef FT810EMU_MODE
			FTEMU_printf("PALETTED is not a valid format\n");
			return 0xFFFF00FF; // invalid format
#else
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return getPaletted(ram, val);
#endif
		}
	case TEXT8X8:
		{
			const int yn = y >> 3;
			py = yn * stride;
			uint8_t c = bmpSrc8(ram, srci, py + xo);
			const uint8_t *nsrc = &ram[bitmapInfo[16 + ((c & 0x80) >> 7)].Source + (8 * (c & 0x7F))];
			const int pyc = y & 0x7;
			const int xc = x & 0x7;
			const uint32_t val = (nsrc[pyc] >> (7 - xc)) & 0x1;
			return (val * 0xFF000000) | 0x00FFFFFF;
		}
	case TEXTVGA:
		{
			const int yn = y >> 4;
			py = yn * stride;
			uint8_t c = bmpSrc8(ram, srci, py + xo); // Character
			uint8_t ca = bmpSrc8(ram, srci, py + xo + 1); // Attribute
			const uint8_t *nsrc = &ram[bitmapInfo[18 + ((c & 0x80) >> 7)].Source + (16 * (c & 0x7F))]; // PG says it uses 16 and 17, but reference uses 18 and 19
			const int pyc = y & 0xF;
			const int xc = x & 0x7;
			const uint32_t val = (nsrc[pyc] >> (7 - xc)) & 0x1; // Foreground or background, 1 or 0
			const int vishift = ((1 - val) << 2);
			const int colidx = (ca >> vishift) & 0xF; // Index in 16-color palette
			return (val * 0xFF000000) | s_VGAPalette[colidx];
		}
	case BARGRAPH:
		{
			uint8_t val = bmpSrc8(ram, srci, xo);
			if (val < y)
				return 0xFFFFFFFF;
			else
				return 0x00FFFFFF; // a black or white transparent pixel? :)
		}
#ifdef FT810EMU_MODE
	case PALETTED565:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return getPaletted565(ram, val, paletteSource);
		}
	case PALETTED4444:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return getPaletted4444(ram, val, paletteSource);
		}
	case PALETTED8:
		{
			uint8_t val = bmpSrc8(ram, srci, py + xo);
			return getPaletted8(ram, val, paletteSource);
		}
#endif
	}
	return 0xFFFF00FF; // invalid format
}

// uses 1/(256*16) pixel units, w & h in pixel units
#ifdef FT810EMU_MODE
BT8XXEMU_FORCE_INLINE argb8888 sampleBitmap(const uint8_t *ram, const uint32_t srci, const int x, const int y, const int width, const int height, const int format, const int stride, const int wrapx, const int wrapy, const int filter, const BitmapInfo *const bitmapInfo, const int paletteSource)
#else
BT8XXEMU_FORCE_INLINE argb8888 sampleBitmap(const uint8_t *ram, const uint32_t srci, const int x, const int y, const int width, const int height, const int format, const int stride, const int wrapx, const int wrapy, const int filter, const BitmapInfo *const bitmapInfo)
#endif
{
#ifdef FT810EMU_MODE
#define FT800_SAMPLE_AT_PARAMS height, format, stride, wrapx, wrapy, bitmapInfo, paletteSource
#else
#define FT800_SAMPLE_AT_PARAMS height, format, stride, wrapx, wrapy, bitmapInfo
#endif
	//return 0xFFFFFF00;
	//switch (filter) NEAREST
	switch (filter)
	{
	case NEAREST:
		{
			int xi = x >> 12;
			int yi = y >> 12;
			return sampleBitmapAt(ram, srci, xi, yi, FT800_SAMPLE_AT_PARAMS);
		}
	case BILINEAR:
		{
			int xsep = x & 0xFFF;
			int ysep = y & 0xFFF;
			int xl = x >> 12;
			int yt = y >> 12;
			if (xsep == 0 && ysep == 0)
			{
				return sampleBitmapAt(ram, srci, xl, yt, FT800_SAMPLE_AT_PARAMS);
			}
			else if (xsep == 0)
			{
				int yab = ysep >> 4;
				int yat = 255 - yab;
				int yb = yt + 1;
				argb8888 top = sampleBitmapAt(ram, srci, xl, yt, FT800_SAMPLE_AT_PARAMS);
				argb8888 btm = sampleBitmapAt(ram, srci, xl, yb, FT800_SAMPLE_AT_PARAMS);
				return mulalpha_argb(top, yat) + mulalpha_argb(btm, yab);
			}
			else if (ysep == 0)
			{
				int xar = xsep >> 4;
				int xal = 255 - xar;
				int xr = xl + 1;
				return mulalpha_argb(sampleBitmapAt(ram, srci, xl, yt, FT800_SAMPLE_AT_PARAMS), xal)
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yt, FT800_SAMPLE_AT_PARAMS), xar);
			}
			else
			{
				int xar = xsep >> 4;
				int xal = 255 - xar;
				int yab = ysep >> 4;
				int yat = 255 - yab;
				int xr = xl + 1;
				int yb = yt + 1;
#if FTEMU_SSE41_INSTRUCTIONS_USE
				const __m128i tl = to_m128i(sampleBitmapAt(ram, srci, xl, yt, FT800_SAMPLE_AT_PARAMS));
				const __m128i tr = to_m128i(sampleBitmapAt(ram, srci, xr, yt, FT800_SAMPLE_AT_PARAMS));
				const __m128i bl = to_m128i(sampleBitmapAt(ram, srci, xl, yb, FT800_SAMPLE_AT_PARAMS));
				const __m128i br = to_m128i(sampleBitmapAt(ram, srci, xr, yb, FT800_SAMPLE_AT_PARAMS));
				const __m128i top = _mm_add_epi32(mulalpha_argb(tl, xal), mulalpha_argb(tr, xar));
				const __m128i btm = _mm_add_epi32(mulalpha_argb(bl, xal), mulalpha_argb(br, xar));
				const __m128i result = _mm_add_epi32(mulalpha_argb(top, yat), mulalpha_argb(btm, yab));
				return to_argb8888(result);
#else
				argb8888 top = mulalpha_argb(sampleBitmapAt(ram, srci, xl, yt, FT800_SAMPLE_AT_PARAMS), xal)
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yt, FT800_SAMPLE_AT_PARAMS), xar);
				argb8888 btm = mulalpha_argb(sampleBitmapAt(ram, srci, xl, yb, FT800_SAMPLE_AT_PARAMS), xal)
					+ mulalpha_argb(sampleBitmapAt(ram, srci, xr, yb, FT800_SAMPLE_AT_PARAMS), xar);
				return mulalpha_argb(top, yat) + mulalpha_argb(btm, yab);
#endif
			}
		}
	}
	return 0xFFFFFF00; // invalid filter
}

template <bool debugTrace>
void displayBitmap(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, int y, int hsize, int px, int py, int handle, int cell, 
#ifdef FT810EMU_MODE
	bool swapXY, // Used to swap width and height from bitmapinfo
#endif
	BitmapInfo *const bitmapInfo)
{
    // if (y != 22) return;
	// FTEMU_printf("bitmap\n");
	const BitmapInfo &bi = bitmapInfo[handle];
	const uint8_t *ram = Memory.getRam();

#ifdef FT810EMU_MODE
	int sizeHeight = swapXY ? bi.SizeWidth : bi.SizeHeight;
#else
	int sizeHeight = bi.SizeHeight;
#endif

	int pytop = py; // incl pixel*16 top
	int pybtm = py + (sizeHeight << 4) - 16; // incl pixel*16 btm

	int pytopi = (pytop + 15) >> 4; // (pytop + 8) >> 4 // reference jumps over to the next pixel at +1/16 already
	int pybtmi = (pybtm + 15) >> 4; // (pybtm + 8) >> 4 // +8 jumps over halfway

	if (max(pytopi, gs.ScissorY.I) <= y && y <= min(pybtmi, gs.ScissorY2.I - 1)) // Scissor Y
	{
#ifdef FT810EMU_MODE
		int sizeWidth = swapXY ? bi.SizeHeight : bi.SizeWidth;
#else
		int sizeWidth = bi.SizeWidth;
#endif

		int pxlef = px;
		int pxrig = px + (sizeWidth << 4) - 16; // verify if this is the correct behaviour for sizewidth = 0

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
#if FT810EMU_MODE
		int paletteSource = gs.PaletteSource;
#endif
		// pretransform
		int rxtbc = (gs.BitmapTransformB * ry) + (gs.BitmapTransformC << 4);
		int rytef = (gs.BitmapTransformE * ry) + (gs.BitmapTransformF << 4);
		//int sample
		for (int x = pxlefi; x <= pxrigi; ++x)
		{
			// relative at 1/16 pixel units
			int vx = x * 16;
			int rx = vx - px;
			// transform with 1/(256*16) pixel units
			int rxt = (gs.BitmapTransformA * rx) + rxtbc;
			int ryt = (gs.BitmapTransformD * rx) + rytef;
#if FT810EMU_MODE
			const argb8888 sample = sampleBitmap(ram, sampleSrcPos, rxt, ryt, sampleWidth, sampleHeight, sampleFormat, sampleStride, sampleWrapX, sampleWrapY, sampleFilter, bitmapInfo, paletteSource);
#else
			const argb8888 sample = sampleBitmap(ram, sampleSrcPos, rxt, ryt, sampleWidth, sampleHeight, sampleFormat, sampleStride, sampleWrapX, sampleWrapY, sampleFilter, bitmapInfo);
#endif
			// todo tag and stencil // todo multiply by gs.Color // todo ColorMask
			const argb8888 out = mul_argb(sample, gs.ColorARGB);
			processPixel<debugTrace>(gs, bc, bs, bt, x, out);
		}
	}
}

BT8XXEMU_FORCE_INLINE int getLayoutWidth(const int &format, const int &stride)
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
#ifdef FT810EMU_MODE
		case PALETTED565: return stride;
		case PALETTED4444: return stride;
		case PALETTED8: return stride;
		case L2: return stride << 2;
#endif
	}
	FTEMU_printf("Invalid bitmap layout\n");
	if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid bitmap layout");
	return stride;
}

#pragma endregion

#pragma region Primitive: Rects

#define FT800EMU_DEBUG_RECTS_MATH 0
#define FT800EMU_RECTS_FT800_COORDINATES 1

struct VertexState
{
public:
	VertexState() : Set(false) { }
	bool Set;
	int X1, Y1;
};

template <bool debugTrace>
void displayRects(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &rs, const int x1, const int y1, const int x2, const int y2)
{
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
					if (dxs_ != dxs) FTEMU_printf("Rect 11 width error\n");
					if (dys_ != dys) FTEMU_printf("Rect 11 height error\n");
				}
#endif
				const int surf = dxs * dys; // Surface of the rectangle in scale 256
				const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8; // Alpha 0-255
				const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
				processPixel<debugTrace>(gs, bc, bs, bt, x, out);
			}
		}
		else if (xslw_px == 1) // Single pixel width rects (ignores impact of rounded corners)
		{
			if (x1lw_px >= gs.ScissorX.I && x2lw_px <= gs.ScissorX2.I) // Scissor X
			{
				const int x = x1lw_px;
				const int dxs = x2lw - x1lw; // Width in 1/16 pixel
				const int dyt = y1lw & 0xF; // Top coordinate in 1/16 relative to top pixel // Todo: Can be moved inside y condition in some way
				const int dyb = y2lw - ((y2lw_px - 1) << 4); // Bottom coordinate in 1/16 relative to bottom pixel // Todo: Can be moved inside y condition in some way
#if FT800EMU_DEBUG_RECTS_MATH
				{
					if (dyb < 1) FTEMU_printf("Rect pixelwidth dyb < 1\n");
					if (dyb > 16) FTEMU_printf("Rect pixelwidth dyb > 16\n");
					if (dxs < 1) FTEMU_printf("Rect pixelwidth dxs < 1\n");
					if (dxs > 16) FTEMU_printf("Rect pixelwidth dxs > 16\n");
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
				processPixel<debugTrace>(gs, bc, bs, bt, x, out);
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
				if (dxr < 1) FTEMU_printf("Rect pixelheight dxr < 1\n");
				if (dxr > 16) FTEMU_printf("Rect pixelheight dxr > 16\n");
				if (dys < 1) FTEMU_printf("Rect pixelheight dys < 1\n");
				if (dys > 16) FTEMU_printf("Rect pixelheight dys > 16\n");
			}
#endif
			if (x1lw_px_sc < x2lw_px_sc)
			{
				if (x1lw_px == x1lw_px_sc && dxl > 0) // Draw the left pixel if not fully on
				{
					const int x = x1lw_px_sc;
					const int surf = (16 - dxl) * dys;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					++x1lw_px_sc;
				}
				if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
				{
					const int x = x2lw_px_sc - 1;
					const int surf = dxr * dys;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					--x2lw_px_sc;
				}
				for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
				{
					const int alpha = ((gs.ColorARGB >> 24) * dys) >> 4;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
				}
			}
		}
		else if (lw < 16)
		{
			// Need special handling for gs.LineWidth < 16 (multiple overlap in single pixel line), ignore rounded corners
			// Size is > 1px guaranteed here
			int x1lw_px_sc = max(x1lw_px, gs.ScissorX.I); // Scissored X1
			int x2lw_px_sc = min(x2lw_px, gs.ScissorX2.I); // Scissored X2
			if (x1lw_px_sc < x2lw_px_sc)
			{
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
					const int surf = (16 - dxl) * rowfill;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					++x1lw_px_sc;
				}
				if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
				{
					const int x = x2lw_px_sc - 1;
					const int surf = dxr * rowfill;
					const int alpha = ((gs.ColorARGB >> 24) * surf) >> 8;
					const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
					processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					--x2lw_px_sc;
				}
				if (rowfill < 16) // Top or bottom row
				{
					for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
					{
						const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 4;
						const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
						processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					}
				}
				else // Any other row
				{
					for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
					{
						const argb8888 out = gs.ColorARGB;
						processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					}
				}
			}
		}
		else if (x2 - x1 == 0 && y2 - y1 == 0) // Rectangle that's actually a point
		{
			displayPoint<debugTrace>(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, x1 - 8, y1 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
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
					if (blendtop + blendbottom >= 16) FTEMU_printf("Full rect shared blend bad math (blendtop + blendbottom >= 16)\n");
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
					displayPoint<debugTrace>(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by1 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
				}
				else
				{
					const int bx1_px_sc_r = min(bx1_px, gs.ScissorX2.I); // Scissored
					const int bx2_px_sc_l = max(bx2_px, gs.ScissorX.I); // Scissored
					// Render top-left and top-right corner
					displayPoint<debugTrace>(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, bx1_px_sc_r, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by1 - 8);
					displayPoint<debugTrace>(gs, gs.LineWidth, bx2_px_sc_l, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx2 - 8, by1 - 8);
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
								int blendleft = ax2 - ((ax2_px - 1) << 4);
								int blendright = bx2 - ((bx2_px - 1) << 4);
								blendright = 16 - blendright;
								const int blendinv = 16 - blendleft - blendright;
								const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by1 - 8);
								const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by1 - 8);
								const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft) + (alpharight * blendright);
								const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
								const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
								processPixel<debugTrace>(gs, bc, bs, bt, x, out);
							}
						}
						else
						{
							if (bx1_px_sc_l < bx2_px_sc_r)
							{
								if (bx1_px_sc_l == bx1_px && ax2_px - 1 == bx1_px) // Left pixel overlaps
								{
									const int x = bx1_px;
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									const int blendinv = 16 - blendleft;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by1 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									processPixel<debugTrace>(gs, bc, bs, bt, x, out);
									++bx1_px_sc_l;
								}

								if (bx2_px_sc_r == bx2_px && bx2_px - 1 == cx1_px) // Right pixel overlaps
								{
									const int x = cx1_px;
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendright;
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by1 - 8);
									const int alphaval = (rowfill * blendinv) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									processPixel<debugTrace>(gs, bc, bs, bt, x, out);
									--bx2_px_sc_r;
								}

								// Draw the rest of this top row
								{
									const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 8;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);

									for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
									{
										processPixel<debugTrace>(gs, bc, bs, bt, x, out);
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
							processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
						}
					}
				}
			}
			else if (blendbottom == 16)
			{
				if (x2 - x1 == 0) // Vertical line
				{
					displayPoint<debugTrace>(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by2 - 8); // -8 correction due to shifted coordinate space used for easier rectangle AA.
				}
				else
				{
					const int bx1_px_sc_r = min(bx1_px, gs.ScissorX2.I); // Scissored X2
					const int bx2_px_sc_l = max(bx2_px, gs.ScissorX.I); // Scissored X1
					// Render bottom-left and bottom-right corner
					displayPoint<debugTrace>(gs, gs.LineWidth, gs.ScissorX.I, gs.ScissorY.I, bx1_px_sc_r, gs.ScissorY2.I, bc, bs, bt, y, bx1 - 8, by2 - 8);
					displayPoint<debugTrace>(gs, gs.LineWidth, bx2_px_sc_l, gs.ScissorY.I, gs.ScissorX2.I, gs.ScissorY2.I, bc, bs, bt, y, bx2 - 8, by2 - 8);
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
								int blendleft = ax2 - ((ax2_px - 1) << 4);
								int blendright = bx2 - ((bx2_px - 1) << 4);
								blendright = 16 - blendright;
								const int blendinv = 16 - blendleft - blendright;
								const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by2 - 8);
								const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by2 - 8);
								const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft) + (alpharight * blendright);
								const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
								const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
								processPixel<debugTrace>(gs, bc, bs, bt, x, out);
							}
						}
						else
						{
							if (bx1_px_sc_l < bx2_px_sc_r)
							{
								if (bx1_px_sc_l == bx1_px && ax2_px - 1 == bx1_px) // Left pixel overlaps
								{
									const int x = bx1_px;
									int blendleft = ax2 - ((ax2_px - 1) << 4);
									const int blendinv = 16 - blendleft;
									const int alphaleft = getPointAlpha256(gs.LineWidth, x, y, bx1 - 8, by2 - 8);
									const int alphaval = (rowfill * blendinv) + (alphaleft * blendleft);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									processPixel<debugTrace>(gs, bc, bs, bt, x, out);
									++bx1_px_sc_l;
								}

								if (bx2_px_sc_r == bx2_px && bx2_px - 1 == cx1_px) // Right pixel overlaps
								{
									const int x = cx1_px;
									int blendright = bx2 - ((bx2_px - 1) << 4);
									blendright = 16 - blendright;
									const int blendinv = 16 - blendright;
									const int alpharight = getPointAlpha256(gs.LineWidth, x, y, bx2 - 8, by2 - 8);
									const int alphaval = (rowfill * blendinv) + (alpharight * blendright);
									const int alpha = ((gs.ColorARGB >> 24) * alphaval) >> 12;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
									processPixel<debugTrace>(gs, bc, bs, bt, x, out);
									--bx2_px_sc_r;
								}

								// Draw the rest of this bottom row
								{
									const int alpha = ((gs.ColorARGB >> 24) * rowfill) >> 8;
									const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);

									for (int x = bx1_px_sc_l; x < bx2_px_sc_r; ++x)
									{
										processPixel<debugTrace>(gs, bc, bs, bt, x, out);
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
							processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
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

				if (x1lw_px_sc < x2lw_px_sc)
				{
					if (blendtop == 0 && blendbottom == 0) // Center only
					{
						if (x1lw_px == x1lw_px_sc && dxl > 0) // Draw the left pixel if not fully on
						{
							const int x = x1lw_px_sc;
							const int surf = (16 - dxl);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 4;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
							++x1lw_px_sc;
						}
						if (x2lw_px == x2lw_px_sc && dxr < 16) // Draw the right pixel if not fully on
						{
							const int x = x2lw_px_sc - 1;
							const int surf = dxr;
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 4;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
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
							if (x1lw_px_sc == x2lw_px_sc) FTEMU_printf("Full rect lr blend x1lw_px_sc == x2lw_px_sc\n");
						}
	#endif
						const int blendc = (16 - blendtop - blendbottom) * 16;
						// Left
						if (x1lw_px == x1lw_px_sc) // Check scissor
						{
							const int x = x1lw_px_sc;
							const int surf = ((16 - dxl) * blendc) + (alphatopleft * blendtop) + (alphabottomleft * blendbottom);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 12;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
							++x1lw_px_sc;
						}
						// Right
						if (x2lw_px == x2lw_px_sc) // Check scissor
						{
							const int x = x2lw_px_sc - 1;
							const int surf = (dxr * blendc) + (alphatopright * blendtop) + (alphabottomright * blendbottom);
							const int alpha = ((gs.ColorARGB >> 24) * surf) >> 12;
							const argb8888 out = (gs.ColorARGB & 0x00FFFFFF) | (alpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
							--x2lw_px_sc;
						}
					}
					// The rest of the pixels
					for (int x = x1lw_px_sc; x < x2lw_px_sc; ++x) // Draw the rest of the pixels
					{
						processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
					}
				}
			}
		}
	}
}

template <bool debugTrace>
void displayRects(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &rs, const int xp, const int yp)
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

	displayRects<debugTrace>(gs, bc, bs, bt, y, hsize, rs, x1, y1, x2, y2);
}

#pragma endregion

#pragma region Primitive: Edge Strip

// Mimic strange clipping behaviour
// This clips to the outer value transformed to pixels directly
// Only works for L and R
#define FT800EMU_EDGE_STRIP_CLIPPING_BEHAVIOUR 0

BT8XXEMU_FORCE_INLINE int findx(const int &x1, const int &x2, const int &y1, const int &y2, const int &y)
{
	const int xd = x2 - x1;
	const int yd = y2 - y1;
	const int yr = y - y1;
	return x1 + (xd * yr / yd);
}

template <bool debugTrace>
void displayEdgeStripL(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &ess, const int xp, const int yp)
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
	if (y1r == y2r) { // No op
        ess.X1 = xp;
        ess.Y1 = yp;
        return;
    }
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
			processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
		}
	}
}

template <bool debugTrace>
void displayEdgeStripR(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &ess, const int xp, const int yp)
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
	if (y1r == y2r) { // No op
        ess.X1 = xp;
        ess.Y1 = yp;
        return;
    }
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
			processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
		}
	}
}

template <bool debugTrace>
void displayEdgeStripA(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &ess, const int xp, const int yp)
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
	if (x1r == x2r) { // No op
        ess.X1 = xp;
        ess.Y1 = yp;
        return;
    }
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
				processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
			}
		}
		else
		{
			for (int x = x1_sc; x < x2_sc; ++x)
			{
				processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
			}
		}
	}
}

template <bool debugTrace>
void displayEdgeStripB(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &ess, const int xp, const int yp)
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
	if (x1r == x2r) { // Nop op
        ess.X1 = xp;
        ess.Y1 = yp;
        return;
    }
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
	const int ym_px = (ym + 15) >> 4; // Clip
	if (max(ym_px, gs.ScissorY.I) <= y && y < gs.ScissorY2.I) // Scissor
	{
		// Get pixel positions
		const int x1_px = ((x1) >> 4); // X Inclusive
		const int x2_px = ((x2) >> 4); // X Exclusive

		const int x1_sc = max(x1_px, gs.ScissorX.I);
		const int x2_sc = min(x2_px, gs.ScissorX2.I);

		/*if (gs.DebugDisplayListIndex == 0x1b5)
		{
			FTEMU_printf("ah");
		}*/

		if ((y1 - y2) != 0)
		{
			const int yv = (y << 4); // Y value 16
			const int xv = findx(x1, x2, y1, y2, yv); // X value 16
			const int xv_px = ((xv) >> 4); // X pixel exclusive or inclusive?

			const int left_sc = y1 <= y2 ? x1_sc : max(xv_px, x1_sc);
			const int right_sc = y1 >= y2 ? x2_sc : min(xv_px, x2_sc);;

			for (int x = left_sc; x < right_sc; ++x)
			{
				processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
			}
		}
		else
		{
			for (int x = x1_sc; x < x2_sc; ++x)
			{
				processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
			}
		}
	}
}

#pragma endregion

#pragma region Primitive: Lines

#define FT800EMU_LINE_DEBUG_DRAW 0
#define FT800EMU_LINE_DEBUG_DRAW_FILL 0
#define FT800EMU_LINE_DEBUG_DRAW_SPECIAL 0
#define FT800EMU_LINE_DEBUG_MATH 0

BT8XXEMU_FORCE_INLINE int findxrel(const int &x1, const int &xd, const int &y1, const int &yd, const int &y)
{
	const int yr = y - y1;
	return x1 + ((int64_t)xd * (int64_t)yr / (int64_t)yd);
}

template <bool debugTrace>
void displayLines(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &rs, const int x1, const int y1, const int x2, const int y2)
{
	const int lw = gs.LineWidth; // Linewidth in 1/16 pixel
	const int y1lw = y1 - lw; // Y coordinates plus linewidth in 1/16 pixel
	const int y2lw = y2 + lw;
	const int y1lw_px = ((y1lw + 8) >> 4); // Top inclusive in screen pixels
	const int y2lw_px = (((y2lw + 8) + 15) >> 4); // Bottom exclusive in screen pixels

	const int scy1 = gs.ScissorY.I;
	const int scy2 = gs.ScissorY2.I;

	if (max(y1lw_px, scy1) <= y && y < min(y2lw_px, scy2)) // Scissor Y
	{
		if (x1 - x2 == 0 && y1 - y2 == 0) // This line is a point
		{
			// Use point rendering
			displayPoint<debugTrace>(gs, lw, gs.ScissorX.I, scy1, gs.ScissorX2.I, scy2, bc, bs, bt, y, x1, y1);
		}
		if (x1 - x2 == 0 || y1 - y2 == 0) // Horizontal or vertical lines
		{
			// Just use the rects optimized codepath for this
			// FTEMU_printf("Not implemented horizontal or vertical line\n");
			displayRects<debugTrace>(gs, bc, bs, bt, y, hsize, rs, min(x1, x2) + 8, y1 + 8, max(x1, x2) + 8, y2 + 8);
		}
		else
		{
			const int scx1 = gs.ScissorX.I;
			const int scx2 = gs.ScissorX2.I;

			// Horizontal slice size factor based on the direction is Length / Y-dist
			// Use 256 scale coordinates (prefix with q, because nothing uses q yet)
			const int qx1 = x1 << 4; // Top-left, 256 scale
			const int qy1 = y1 << 4;
			const int qx2 = x2 << 4; // Bottom-right, 256 scale
			const int qy2 = y2 << 4;
			const int qxd = qx2 - qx1; // X-dist, 256 scale
			const int qyd = qy2 - qy1; // Y-dist, 256 scale
			const int64_t qlensq = ((int64_t)qxd * (int64_t)qxd) + ((int64_t)qyd * (int64_t)qyd); // Length squared, 256*256 scale
			const int qlen = (int)sqrt((double)qlensq); // Length, 256 scale
			// Use (... * qlen) / qyd

			// Current Y coord in 256 scale
			const int qy = y << 8;
			// Line center x coord in 256 scale
			const int qx = findxrel(qx1, qxd, qy1, qyd, qy);

#if FT800EMU_LINE_DEBUG_DRAW
			{
				const int x = qx >> 8;
				if (x > 0) bc[x] = 0xFFFF0080;
			}
#endif

			if (lw > 8) // Lines wider than a pixel
			{
				// Line width boundaries in 256 scale
				const int qlw = lw << 4; // Line width 256 scale
				const int qwdo = ((int64_t)(qlw + 128) * (int64_t)qlen) / (int64_t)qyd; // Distance from center for outer boundary (always positive)
				const int qwdi = ((int64_t)(qlw - 128) * (int64_t)qlen) / (int64_t)qyd; // Distance from center for inner boundary (always positive for lines wider than a pixel)
	#if FT800EMU_LINE_DEBUG_MATH
				if (qwdo <= 0) FTEMU_printf("qwdo <= 0\n");
				if (qwdi <= 0) FTEMU_printf("qwdi <= 0\n");
	#endif
				const int qw1o = qx - qwdo; // Left outer linewidth boundary in 256 scale
				const int qw1i = qx - qwdi; // Left inner linewidth boundary in 256 scale
				const int qw2i = qx + qwdi; // Right inner linewidth boundary in 256 scale
				const int qw2o = qx + qwdo; // Right outer linewidth boundary in 256 scale

	#if FT800EMU_LINE_DEBUG_DRAW
				{
					int x = qw1o >> 8;
					if (x > 0) bc[x] = 0xFF0080FF;
					x = qw1i >> 8;
					if (x > 0) bc[x] = 0xFF00FF80;
					x = qw2i >> 8;
					if (x > 0) bc[x] = 0xFF00FF80;
					x = qw2o >> 8;
					if (x > 0) bc[x] = 0xFF0080FF;
				}
	#endif

				// Convert to screen pixels
				const int w1o = (qw1o + 255) >> 8; // Left outer linewidth boundary
				const int w1i = (qw1i + 255) >> 8; // Left inner linewidth boundary
				const int w2i = (qw2i + 255) >> 8; // Right inner linewidth boundary
				const int w2o = (qw2o + 255) >> 8; // Right outer linewidth boundary

				// Find the length boundaries
				const int ql1r = findxrel(qx1, qyd, qy1, -qxd, qy); // Length boundary for top-left point in 256 scale
				const int ql2r = findxrel(qx2, qyd, qy2, -qxd, qy); // Length boundary for bottom-right point in 256 scale
				const int ql1 = min(ql1r, ql2r);
				const int ql2 = max(ql1r, ql2r);
				const int qlaa = (128 * qlen) / abs(qxd); // AA distance
				const int ql1o = ql1 - qlaa; // Outer and inner length boundaries
				const int ql1i = ql1 + qlaa;
				const int ql2i = ql2 - qlaa;
				const int ql2o = ql2 + qlaa;

#if FT800EMU_LINE_DEBUG_DRAW
				{
					int x = ql1o >> 8;
					if (x > 0) bc[x] = 0xFFFF8000;
					x = ql1i >> 8;
					if (x > 0) bc[x] = 0xFF80FF00;
					x = ql2i >> 8;
					if (x > 0) bc[x] = 0xFF80FF00;
					x = ql2o >> 8;
					if (x > 0) bc[x] = 0xFFFF8000;
				}
#endif

				// Convert to screen pixels
				const int l1o = (ql1o + 255) >> 8; // Left outer length boundary
				const int l1i = (ql1i + 255) >> 8; // Left inner length boundary
				const int l2i = (ql2i + 255) >> 8; // Right inner length boundary
				const int l2o = (ql2o + 255) >> 8; // Right outer length boundary

				// Draw the left AA
				{
					const int left_sc = max(scx1, max(w1o, l1o)); // Left included
					const int right_sc = min(scx2, min(w1i, l2o));  // Right excluded

					for (int x = left_sc; x < right_sc; ++x)
					{
#if FT800EMU_LINE_DEBUG_DRAW_FILL
						bc[x] = 0xFFFF0000;
#endif

						const int qxc = x << 8; // Current x in 256 scale
						const int alphawidth = findx(0, 256, qw1o, qw1i, qxc); // Alpha 256

						int blendleft;
						int alphaleft;
						int blendright;
						int alpharight;

						if (x < l1i) // Blend with left length boundary and circle
						{
#if FT800EMU_LINE_DEBUG_DRAW_SPECIAL
							bc[x] = 0xFFFF00FF;
#endif
							blendleft = findx(256, 0, ql1o, ql1i, qxc);
							alphaleft = blendleft * getPointAlpha256(lw, x, y, x1 < x2 ? x1 : x2, x1 < x2 ? y1 : y2);
						}
						else
						{
							blendleft = 0;
							alphaleft = 0;
						}
						if (x >= l2i) // Blend with right length boundary and circle
						{
#if FT800EMU_LINE_DEBUG_DRAW_SPECIAL
							bc[x] = 0xFFFF00FF;
#endif
							blendright = findx(0, 256, ql2i, ql2o, qxc);
							alpharight = blendright * getPointAlpha256(lw, x, y, x1 < x2 ? x2 : x1, x1 < x2 ? y2 : y1);
						}
						else
						{
							blendright = 0;
							alpharight = 0;
						}

						const int alpharest = 256 - blendleft - blendright;

						const int outalpha = ((gs.ColorARGB >> 24) * ((alphawidth * alpharest) + (alphaleft) + (alpharight))) >> 16;
						const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
						processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					}
				}

				// Draw the filler
				{
					const int left_sc = max(scx1, max(w1i, l1o)); // Left included
					const int right_sc = min(scx2, min(w2i, l2o));  // Right excluded

					for (int x = left_sc; x < right_sc; ++x)
					{
						processPixel<debugTrace>(gs, bc, bs, bt, x, gs.ColorARGB);
					}
				}

				// Draw right-side AA
				{
					const int left_sc = max(scx1, max(w2i, l1o)); // Left included
					const int right_sc = min(scx2, min(w2o, l2o));  // Right excluded

					for (int x = left_sc; x < right_sc; ++x)
					{
						const int qxc = x << 8; // Current x in 256 scale
						const int alphawidth = findx(256, 0, qw2i, qw2o, qxc); // Alpha 256

						int blendleft;
						int alphaleft;
						int blendright;
						int alpharight;

						if (x < l1i) // Blend with left length boundary and circle
						{
#if FT800EMU_LINE_DEBUG_DRAW_SPECIAL
							bc[x] = 0xFFFF00FF;
#endif
							blendleft = findx(256, 0, ql1o, ql1i, qxc);
							alphaleft = blendleft * getPointAlpha256(lw, x, y, x1 < x2 ? x1 : x2, x1 < x2 ? y1 : y2);
						}
						else
						{
							blendleft = 0;
							alphaleft = 0;
						}
						if (x >= l2i) // Blend with right length boundary and circle
						{
#if FT800EMU_LINE_DEBUG_DRAW_SPECIAL
							bc[x] = 0xFFFF00FF;
#endif
							blendright = findx(0, 256, ql2i, ql2o, qxc);
							alpharight = blendright * getPointAlpha256(lw, x, y, x1 < x2 ? x2 : x1, x1 < x2 ? y2 : y1);
						}
						else
						{
							blendright = 0;
							alpharight = 0;
						}

						const int alpharest = 256 - blendleft - blendright;

						const int outalpha = ((gs.ColorARGB >> 24) * ((alphawidth * alpharest) + (alphaleft) + (alpharight))) >> 16;
						const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
						processPixel<debugTrace>(gs, bc, bs, bt, x, out);
					}
				}

				// Draw the points
				if (ql2r > ql1r)
				{
					displayPoint<debugTrace>(gs, lw, scx1, scy1, min(l1o, scx2), scy2, bc, bs, bt, y, x1, y1);
					displayPoint<debugTrace>(gs, lw, max(l2o, scx1), scy1, scx2, scy2, bc, bs, bt, y, x2, y2);
				}
				else
				{
					displayPoint<debugTrace>(gs, lw, max(l2o, scx1), scy1, scx2, scy2, bc, bs, bt, y, x1, y1);
					displayPoint<debugTrace>(gs, lw, scx1, scy1, min(l1o, scx2), scy2, bc, bs, bt, y, x2, y2);
				}
			}
			else // Line width a pixel or less
			{
				// Slightly different handling, extend the line length by the line width, and don't draw any points.
				// Use the minimum of the two AA borders as the alpha, multiply with length AA border if necessary.
				// Render as if 1 pixel width and multiply the alpha to reduce.

				// Line width boundaries in 256 scale
				const int qwdo = ((int64_t)(256) * (int64_t)qlen) / (int64_t)qyd; // Distance from center for outer boundary (always positive)
#if FT800EMU_LINE_DEBUG_MATH
				if (qwdo <= 0) FTEMU_printf("qwdo <= 0\n");
#endif
				const int qw1o = qx - qwdo; // Left outer linewidth boundary in 256 scale
				const int qwi = qx; // Inner linewidth boundary in 256 scale
				const int qw2o = qx + qwdo; // Right outer linewidth boundary in 256 scale

				// Convert to screen pixels
				const int w1o = (qw1o + 255) >> 8; // Left outer linewidth boundary
				const int w1i = (qwi + 255) >> 8; // Inner linewidth boundary
				const int w2o = (qw2o + 255) >> 8; // Right outer linewidth boundary

				const int qlw = lw << 4; // Line width 256 scale
				const int qxext = (qxd * qlw) / qlen; // Extension in X direction
				const int qyext = (qyd * qlw) / qlen; // Extension in Y direction

				// Find the length boundaries
				const int ql1r = findxrel(qx1, qyd, qy1, -qxd, qy); // Length boundary for top-left point in 256 scale
				const int ql2r = findxrel(qx2, qyd, qy2, -qxd, qy); // Length boundary for bottom-right point in 256 scale
				const int ql1 = min(ql1r, ql2r);
				const int ql2 = max(ql1r, ql2r);
				const int qlaa = (128 * qlen) / abs(qxd); // AA distance
				const int ql1o = ql1 - qlaa; // Outer and inner length boundaries
				const int ql1i = ql1 + qlaa;
				const int ql2i = ql2 - qlaa;
				const int ql2o = ql2 + qlaa;

				// Convert to screen pixels
				const int l1o = (ql1o + 255) >> 8; // Left outer length boundary
				const int l1i = (ql1i + 255) >> 8; // Left inner length boundary
				const int l2i = (ql2i + 255) >> 8; // Right inner length boundary
				const int l2o = (ql2o + 255) >> 8; // Right outer length boundary

				// Draw AA
				{
					const int left_sc = max(scx1, max(w1o, l1o)); // Left included
					const int right_sc = min(scx2, min(w2o, l2o));  // Right excluded

					for (int x = left_sc; x < right_sc; ++x)
					{
						const int qxc = x << 8; // Current x in 256 scale
						const int alphawidthleft = findx(0, 256, qw1o, qwi, qxc); // Alpha 256
						const int alphawidthright = findx(256, 0, qwi, qw2o, qxc); // Alpha 256
						const int alphawidth = min(alphawidthleft, alphawidthright);

						if (x >= l1i && x < l2i) // No blending with outer ends
						{
							// lw = 3, alphawidth = 8
							const int outalpha = ((gs.ColorARGB >> 24) * lw * alphawidth) >> 11;
							const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
						}
						else
						{
							const int alphalenleft = findx(0, 256, ql1o, ql1i, qxc);
							const int alphalenright = findx(256, 0, ql2i, ql2o, qxc);
							const int alphalen = min(alphalenleft, alphalenright);

							const int outalpha = ((gs.ColorARGB >> 24) * lw * alphawidth * alphalen) >> 19;
							const argb8888 out = gs.ColorARGB & 0x00FFFFFF | (outalpha << 24);
							processPixel<debugTrace>(gs, bc, bs, bt, x, out);
						}
					}
				}
			}
		}
	}
}

template <bool debugTrace>
void displayLines(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &rs, const int xp, const int yp)
{
	if (!rs.Set)
	{
		rs.X1 = xp;
		rs.Y1 = yp;
		rs.Set = true;
		return;
	}

	const int x1r = rs.X1; // Coordinates in 1/16 pixel
	const int y1r = rs.Y1;
	const int x2r = xp;
	const int y2r = yp;
	const int y1 = min(y1r, y2r); // Simplify line coordinates from top to bottom
	const int y2 = max(y1r, y2r);
	const int x1 = y1 == y1r ? x1r : x2r;
	const int x2 = y2 == y2r ? x2r : x1r;
	rs.Set = false;

	displayLines<debugTrace>(gs, bc, bs, bt, y, hsize, rs, x1, y1, x2, y2);
}

#pragma endregion

#pragma region Primitive: Line Strip

template <bool debugTrace>
void displayLineStrip(const GraphicsState &gs, argb8888 *bc, uint8_t *bs, uint8_t *bt, const int y, const int hsize, VertexState &rs, const int xp, const int yp)
{
	if (!rs.Set)
	{
		rs.X1 = xp;
		rs.Y1 = yp;
		rs.Set = true;
		return;
	}

	const int x1r = rs.X1; // Coordinates in 1/16 pixel
	const int y1r = rs.Y1;
	const int x2r = xp;
	const int y2r = yp;
	const int y1 = min(y1r, y2r); // Simplify line coordinates from top to bottom
	const int y2 = max(y1r, y2r);
	const int x1 = y1 == y1r ? x1r : x2r;
	const int x2 = y2 == y2r ? x2r : x1r;
	rs.X1 = xp;
	rs.Y1 = yp;

	displayLines<debugTrace>(gs, bc, bs, bt, y, hsize, rs, x1, y1, x2, y2);
}

#pragma endregion

}

static int s_DebugMode;
static int s_DebugMultiplier;
static int s_DebugLimiter;
static bool s_DebugLimiterEffective;
static int s_DebugLimiterIndex;

static int s_ThreadCount;

void GraphicsProcessorClass::begin()
{
	s_DebugMode = FT800EMU_DEBUGMODE_NONE;
	s_DebugMultiplier = 1;
	s_DebugLimiter = 0;
	s_DebugLimiterEffective = false;
	s_DebugLimiterIndex = 0;

	s_RegPwmDutyEmulation = false;
	s_ThreadCount = 1;

	uint8_t *ram = Memory.getRam();
	uint32_t fi = Memory.rawReadU32(ram, FT800EMU_ROM_FONTINFO);
	if (FT800EMU_DEBUG) FTEMU_printf("Font index: %u\n", fi);
	for (int i = 0; i < 16; ++i)
	{
		int ir = i + 16;
		uint32_t bi = (i * 148) + fi;
		uint32_t format =  Memory.rawReadU32(ram, bi + 128);
		uint32_t stride =  Memory.rawReadU32(ram, bi + 132);
		uint32_t width =  Memory.rawReadU32(ram, bi + 136);
		uint32_t height =  Memory.rawReadU32(ram, bi + 140);
		uint32_t data =  Memory.rawReadU32(ram, bi + 144);
		if (FT800EMU_DEBUG) FTEMU_printf("Font[%i] -> Format: %u, Stride: %u, Width: %u, Height: %u, Data: %u\n", ir, format, stride, width, height, data);

		s_BitmapInfoMain[ir].Source = data;
		s_BitmapInfoMain[ir].LayoutFormat = format;
		s_BitmapInfoMain[ir].LayoutStride = stride;
		s_BitmapInfoMain[ir].LayoutHeight = height;
		s_BitmapInfoMain[ir].LayoutWidth = getLayoutWidth(format, stride);
		s_BitmapInfoMain[ir].SizeFilter = ir < 25 ? NEAREST : BILINEAR; // i assume
		s_BitmapInfoMain[ir].SizeWrapX = BORDER;
		s_BitmapInfoMain[ir].SizeWrapY = BORDER;
		s_BitmapInfoMain[ir].SizeWidth = width;
		s_BitmapInfoMain[ir].SizeHeight = height;
	}
}

struct ThreadInfo
{
public:
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	volatile bool Running;
	SDL_Thread *Thread;
	SDL_sem *StartSem;
	SDL_sem *EndSem;
#else
#	ifdef WIN32
	volatile bool Running;
	HANDLE Thread;
	HANDLE StartEvent;
	HANDLE EndEvent;
#	endif
#endif
	argb8888 *ScreenArgb8888;
	bool UpsideDown;
	bool Mirrored;
#ifdef FT810EMU_MODE
	bool SwapXY;
#endif
	uint32_t HSize;
	uint32_t VSize;
	uint32_t YIdx;
	uint32_t YInc;
	BitmapInfo Bitmap[32];
};

std::vector<ThreadInfo> s_ThreadInfos;

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
int launchGraphicsProcessorThread(void *startInfo);
#else
#	ifdef WIN32
DWORD WINAPI launchGraphicsProcessorThread(void *startInfo);
#	endif
#endif

void resizeThreadInfos(int size)
{
	if (s_ThreadInfos.size() == size)
		return;

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	for (size_t i = 0; i < s_ThreadInfos.size(); ++i)
	{
		s_ThreadInfos[i].Running = false;
		FTEMU_printf("Stop graphics thread: 1+%u\n", (uint32_t)i);
		SDL_SemPost(s_ThreadInfos[i].StartSem);
		SDL_WaitThread(s_ThreadInfos[i].Thread, NULL);
		s_ThreadInfos[i].Thread = NULL;
		SDL_DestroySemaphore(s_ThreadInfos[i].StartSem);
		s_ThreadInfos[i].StartSem = NULL;
		SDL_DestroySemaphore(s_ThreadInfos[i].EndSem);
		s_ThreadInfos[i].EndSem = NULL;
	}
#else
#	ifdef WIN32
	for (size_t i = 0; i < s_ThreadInfos.size(); ++i)
	{
		s_ThreadInfos[i].Running = false;
		SetEvent(s_ThreadInfos[i].StartEvent);
		WaitForSingleObject(s_ThreadInfos[i].Thread, INFINITE);
		CloseHandle(s_ThreadInfos[i].Thread);
		CloseHandle(s_ThreadInfos[i].StartEvent);
		CloseHandle(s_ThreadInfos[i].EndEvent);
	}
#	endif
#endif
	s_ThreadInfos.resize(size);
	for (size_t i = 0; i < s_ThreadInfos.size(); ++i)
	{
		memcpy(&s_ThreadInfos[i].Bitmap, &s_BitmapInfoMain, sizeof(s_BitmapInfoMain));
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
		s_ThreadInfos[i].Running = true;
		s_ThreadInfos[i].StartSem = SDL_CreateSemaphore(0);
		s_ThreadInfos[i].EndSem = SDL_CreateSemaphore(0);
		std::stringstream threadName;
		threadName << std::string("FT800EMU::GPU::") << i;
		std::string threadNameStr = threadName.str();
		FTEMU_printf("Start graphics thread: 1+%u\n", (uint32_t)i);
		s_ThreadInfos[i].Thread = SDL_CreateThreadFT(launchGraphicsProcessorThread, threadNameStr.c_str(), static_cast<void *>(&s_ThreadInfos[i]));
#else
#	ifdef WIN32
		s_ThreadInfos[i].Running = true;
		s_ThreadInfos[i].StartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		s_ThreadInfos[i].EndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		s_ThreadInfos[i].Thread = CreateThread(NULL, 0, launchGraphicsProcessorThread, static_cast<void *>(&s_ThreadInfos[i]), 0, NULL);
#	endif
#endif
	}
}

void GraphicsProcessorClass::end()
{
	resizeThreadInfos(0);
}

void GraphicsProcessorClass::enableRegPwmDutyEmulation(bool enabled)
{
	s_RegPwmDutyEmulation = enabled;
}

void GraphicsProcessorClass::enableMultithread(bool enabled)
{
	if (enabled)
	{
		s_ThreadCount = FT8XXEMU::System.getCPUCount();
	}
	else
	{
		s_ThreadCount = 1;
	}
	FTEMU_printf("Graphics processor threads: %i\n", s_ThreadCount);
	resizeThreadInfos(s_ThreadCount - 1);
}

void GraphicsProcessorClass::reduceThreads(int nb)
{
	s_ThreadCount = max(1, s_ThreadCount - nb);
	FTEMU_printf("Graphics processor threads: %i\n", s_ThreadCount);
	resizeThreadInfos(s_ThreadCount - 1);
}

// Sign-extend the n-bit value v
#define SIGNED_N(v, n) \
    (((int32_t)((v) << (32-(n)))) >> (32-(n)))

namespace {

void processBlankDL(BitmapInfo *const bitmapInfo)
{
	uint8_t *const ram = Memory.getRam();
	const uint32_t *displayList = Memory.getDisplayList();

	GraphicsState gs = GraphicsState(
#ifdef FT810EMU_MODE
		false
#endif
		);
	std::stack<GraphicsState> gsstack;
	std::stack<int> callstack;

	int loopCount = 0;

	// run display list
	for (size_t c = 0; c < FT800EMU_DISPLAY_LIST_SIZE; ++c)
	{
#if FT800EMU_LIMIT_JUMP_LOOP
		if (loopCount > FT800EMU_LIMIT_JUMP_LOOP_COUNT)
		{
			FTEMU_printf("JUMP loop\n");
			if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("JUMP loop");
			break;
		}
		++loopCount;
#endif

		uint32_t v = displayList[c]; // (up to 2048 ops)
EvaluateDisplayListValue:
		switch (v >> 24)
		{
		case FT800EMU_DL_DISPLAY:
			goto DisplayListDisplay;
		case FT800EMU_DL_BITMAP_SOURCE:
			bitmapInfo[gs.BitmapHandle].Source = v & FT800EMU_ADDR_MASK;
			break;
		case FT800EMU_DL_BITMAP_HANDLE:
			gs.BitmapHandle = v & 0x1F;
			break;
		case FT800EMU_DL_BITMAP_LAYOUT:
			{
				BitmapInfo &bi = bitmapInfo[gs.BitmapHandle];
				const int format = (v >> 19) & 0x1F;
				bi.LayoutFormat = format;
#ifdef FT810EMU_MODE
				int stride = (bi.LayoutStride & 0xC00) | ((v >> 9) & 0x3FF);
				if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 4096; } // correct behaviour is probably 'infinite'?
				bi.LayoutStride = stride;
				bi.LayoutHeight = (bi.LayoutHeight & 0x600) | (v & 0x1FF);
				if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 2048; } // correct behaviour is probably 'infinite'?
#else
				int stride = (v >> 9) & 0x3FF;
				if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 1024; } // correct behaviour is probably 'infinite'?
				bi.LayoutStride = stride;
				bi.LayoutHeight = v & 0x1FF;
				if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 512; } // correct behaviour is probably 'infinite'?
#endif
				bi.LayoutWidth = getLayoutWidth(format, stride);
			}
			break;
		case FT800EMU_DL_BITMAP_SIZE:
			bitmapInfo[gs.BitmapHandle].SizeFilter = (v >> 20) & 0x1;
			bitmapInfo[gs.BitmapHandle].SizeWrapX = (v >> 19) & 0x1;
			bitmapInfo[gs.BitmapHandle].SizeWrapY = (v >> 18) & 0x1;
#ifdef FT810EMU_MODE
			bitmapInfo[gs.BitmapHandle].SizeWidth = (bitmapInfo[gs.BitmapHandle].SizeWidth & 0x600) | ((v >> 9) & 0x1FF);
			if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 2048; // verify
			bitmapInfo[gs.BitmapHandle].SizeHeight = (bitmapInfo[gs.BitmapHandle].SizeHeight & 0x600) | (v & 0x1FF);
			if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 2048; // verify
#else
			bitmapInfo[gs.BitmapHandle].SizeWidth = (v >> 9) & 0x1FF;
			if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 512; // verify
			bitmapInfo[gs.BitmapHandle].SizeHeight = v & 0x1FF;
			if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 512; // verify
#endif
			break;
		case FT800EMU_DL_CALL:
			callstack.push((int)c);
			c = (v & 0xFFFF) - 1;
			break;
		case FT800EMU_DL_JUMP:
			c = (v & 0xFFFF) - 1;
			break;
		case FT800EMU_DL_SAVE_CONTEXT:
			gsstack.push(gs);
			break;
		case FT800EMU_DL_RESTORE_CONTEXT:
            if (gsstack.empty()) {
                gs = GraphicsState(
#ifdef FT810EMU_MODE
					false
#endif
					);
            } else {
                gs = gsstack.top();
                gsstack.pop();
            }
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
#ifdef FT810EMU_MODE
		case FT800EMU_DL_BITMAP_LAYOUT_H:
			{
				BitmapInfo &bi = bitmapInfo[gs.BitmapHandle];
				int stride = (bi.LayoutStride & 0x3FF) | (((v >> 2) & 0x3) << 10);
				if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 4096; } // correct behaviour is probably 'infinite'?
				bi.LayoutStride = stride;
				bi.LayoutHeight = (bi.LayoutHeight & 0x1FF) | ((v & 0x3) << 9);
				if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 2048; } // correct behaviour is probably 'infinite'?
				bi.LayoutWidth = getLayoutWidth(bi.LayoutFormat, stride);
			}
			break;
		case FT800EMU_DL_BITMAP_SIZE_H:
			bitmapInfo[gs.BitmapHandle].SizeWidth = (bitmapInfo[gs.BitmapHandle].SizeWidth & 0x1FF) | (((v >> 2) & 0x3) << 9);
			if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 2048; // verify
			bitmapInfo[gs.BitmapHandle].SizeHeight = (bitmapInfo[gs.BitmapHandle].SizeHeight & 0x1FF) | ((v & 0x3) << 9);
			if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 2048; // verify
			break;
#endif
		}
	}
DisplayListDisplay:
	;
}

template <bool debugTrace>
void processPart(argb8888 *const screenArgb8888, const bool upsideDown, const bool mirrored FT810EMU_SWAPXY_PARAM, const uint32_t hsize, const uint32_t vsize, const uint32_t yIdx, const uint32_t yInc, BitmapInfo *const bitmapInfo)
{
	uint8_t *const ram = Memory.getRam();
	const uint32_t *displayList = Memory.getDisplayList();
	uint8_t bt[FT800EMU_SCREEN_WIDTH_MAX]; // tag buffer (per thread value)
	uint8_t bs[FT800EMU_SCREEN_WIDTH_MAX]; // stencil buffer (per-thread values!)

	intptr_t lines_processed = 0;
	bool debugLimiterEffective = false;
	int debugLimiterIndex = 0;
	for (uint32_t y = yIdx; y < vsize; y += yInc)
	{
		VertexState vs = VertexState();
		int primitive = 0;
		GraphicsState gs = GraphicsState(
#ifdef FT810EMU_MODE
			swapXY
#endif
			);
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
				bt[i] = 255;
			}
		}

		// run display list
		int debugCounter = 0;
		for (size_t c = 0; c < FT800EMU_DISPLAY_LIST_SIZE; ++c)
		{
			if (s_DebugLimiter)
			{
				if (debugCounter >= s_DebugLimiter)
				{
					if (y == 0)
					{
						debugLimiterEffective = true;
					}
					break;
				}
				if (y == 0) debugLimiterIndex = (int)c;
#if !FT800EMU_LIMIT_JUMP_LOOP
				++debugCounter;
#endif
			}

#if FT800EMU_LIMIT_JUMP_LOOP
			if (debugCounter > FT800EMU_LIMIT_JUMP_LOOP_COUNT)
			{
				FTEMU_printf("JUMP loop\n");
				if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("JUMP loop");
				break;
			}
			++debugCounter;
#endif

			if (debugTrace)
			{
				s_DebugTraceLine = (uint32_t)c;
			}

			gs.DebugDisplayListIndex = (int)c;

			uint32_t v = displayList[c]; // (up to 2048 ops)

			/*if (y == 0)
			{
				FTEMU_printf("DL: %i: %x\n", c, v);
			}*/

EvaluateDisplayListValue:
			switch (v >> 30)
			{
			case 0:
				switch (v >> 24)
				{
				case FT800EMU_DL_DISPLAY:
					goto DisplayListDisplay;
				case FT800EMU_DL_BITMAP_SOURCE:
					bitmapInfo[gs.BitmapHandle].Source = v & FT800EMU_ADDR_MASK;
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
						BitmapInfo &bi = bitmapInfo[gs.BitmapHandle];
						const int format = (v >> 19) & 0x1F;
						bi.LayoutFormat = format;
#ifdef FT810EMU_MODE
						int stride = (bi.LayoutStride & 0xC00) | ((v >> 9) & 0x3FF);
						if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 4096; } // correct behaviour is probably 'infinite'?
						bi.LayoutStride = stride;
						bi.LayoutHeight = (bi.LayoutHeight & 0x600) | (v & 0x1FF);
						if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 2048; } // correct behaviour is probably 'infinite'?
#else
						int stride = (v >> 9) & 0x3FF;
						if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 1024; } // correct behaviour is probably 'infinite'?
						bi.LayoutStride = stride;
						bi.LayoutHeight = v & 0x1FF;
						if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 512; } // correct behaviour is probably 'infinite'?
#endif
						bi.LayoutWidth = getLayoutWidth(format, stride);
					}
					break;
				case FT800EMU_DL_BITMAP_SIZE:
					bitmapInfo[gs.BitmapHandle].SizeFilter = (v >> 20) & 0x1;
					bitmapInfo[gs.BitmapHandle].SizeWrapX = (v >> 19) & 0x1;
					bitmapInfo[gs.BitmapHandle].SizeWrapY = (v >> 18) & 0x1;
#ifdef FT810EMU_MODE
					bitmapInfo[gs.BitmapHandle].SizeWidth = (bitmapInfo[gs.BitmapHandle].SizeWidth & 0x600) | ((v >> 9) & 0x1FF);
					if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 2048; // verify
					bitmapInfo[gs.BitmapHandle].SizeHeight = (bitmapInfo[gs.BitmapHandle].SizeHeight & 0x600) | (v & 0x1FF);
					if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 2048; // verify
#else
					bitmapInfo[gs.BitmapHandle].SizeWidth = (v >> 9) & 0x1FF;
					if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 512; // verify
					bitmapInfo[gs.BitmapHandle].SizeHeight = v & 0x1FF;
					if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 512; // verify
#endif
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
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformB = SIGNED_N(v, 17);
					else
#endif
					gs.BitmapTransformA = SIGNED_N(v, 17);
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_B:
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformA = SIGNED_N(v, 17);
					else
#endif
					gs.BitmapTransformB = SIGNED_N(v, 17);
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_C: // 15.8 signed
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformF = SIGNED_N(v, 24);
					else
#endif
					gs.BitmapTransformC = SIGNED_N(v, 24);
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_D:
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformE = SIGNED_N(v, 17);
					else
#endif
					gs.BitmapTransformD = SIGNED_N(v, 17);
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_E:
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformD = SIGNED_N(v, 17);
					else
#endif
					gs.BitmapTransformE = SIGNED_N(v, 17);
					break;
				case FT800EMU_DL_BITMAP_TRANSFORM_F:
#ifdef FT810EMU_MODE
					if (swapXY) gs.BitmapTransformC = SIGNED_N(v, 24);
					else
#endif
					gs.BitmapTransformF = SIGNED_N(v, 24);
					break;
				case FT800EMU_DL_SCISSOR_XY:
#ifdef FT810EMU_MODE
					if (swapXY)
					{
						gs.ScissorX.U = v & 0x7FF;
						gs.ScissorY.U = (v >> 11) & 0x7FF;
					}
					else
					{
						gs.ScissorY.U = v & 0x7FF;
						gs.ScissorX.U = (v >> 11) & 0x7FF;
					}
					goto UpdateScissorX2Y2;
#else
					gs.ScissorY.U = v & 0x1FF;
					gs.ScissorX.U = (v >> 9) & 0x1FF;
					goto UpdateScissorX2Y2;
#endif
					break;
				case FT800EMU_DL_SCISSOR_SIZE:
#ifdef FT810EMU_MODE
					if (swapXY)
					{
						gs.ScissorWidth = v & 0xFFF;
						gs.ScissorHeight = (v >> 12) & 0xFFF;
					}
					else
					{
						gs.ScissorHeight = v & 0xFFF;
						gs.ScissorWidth = (v >> 12) & 0xFFF;
					}
				UpdateScissorX2Y2:
					gs.ScissorX2.U = gs.ScissorX.I + gs.ScissorWidth;
					gs.ScissorY2.U = gs.ScissorY.I + gs.ScissorHeight;
					if (swapXY) gs.ScissorY2.U = min(hsize, gs.ScissorY2.U);
					else gs.ScissorX2.U = min(hsize, gs.ScissorX2.U);
#else
					gs.ScissorHeight = v & 0x3FF;
					gs.ScissorWidth = (v >> 10) & 0x3FF;
				UpdateScissorX2Y2:
					gs.ScissorX2.U = min(hsize, gs.ScissorX.I + gs.ScissorWidth);
					gs.ScissorY2.U = gs.ScissorY.I + gs.ScissorHeight;
#endif
					break;
				case FT800EMU_DL_CALL:
					if (callstack.size() >= 1024)
					{
						FTEMU_printf("Invalid CALL() in display list\n");
						if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid CALL() in display list");
						goto DisplayListDisplay;
					}
					callstack.push((int)c);
					c = (v & 0xFFFF) - 1;
					break;
				case FT800EMU_DL_JUMP:
					c = (v & 0xFFFF) - 1;
					break;
				case FT800EMU_DL_BEGIN:
					primitive = v & 0x0F;
					vs.Set = false;
					break;
				case FT800EMU_DL_COLOR_MASK:
					gs.ColorMaskARGB = (((v >> 1) & 0x1) * 0xFF)
						| (((v >> 2) & 0x1) * 0xFF00)
						| (((v >> 3) & 0x1) * 0xFF0000)
						| ((v & 0x1) * 0xFF000000);
					break;
				case FT800EMU_DL_END:
					primitive = 0;
					break;
				case FT800EMU_DL_SAVE_CONTEXT:
					gsstack.push(gs);
					break;
				case FT800EMU_DL_RESTORE_CONTEXT:
                    if (gsstack.empty()) {
                        gs = GraphicsState(
#ifdef FT810EMU_MODE
							swapXY
#endif							
							);
                        gs.ScissorWidth = hsize;
                        gs.ScissorHeight = vsize;
                        gs.ScissorX2.U = hsize;
                        gs.ScissorY2.U = vsize;
                    } else {
                        gs = gsstack.top();
                        gsstack.pop();
                    }
					break;
				case FT800EMU_DL_RETURN:
					if (callstack.empty())
					{
						FTEMU_printf("Invalid RETURN() in display list\n");
						if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid RETURN() in display list");
					}
					else
					{
						c = callstack.top();
						callstack.pop();
					}
					break;
				case FT800EMU_DL_MACRO:
					v = Memory.rawReadU32(ram, REG_MACRO_0 + (4 * (v & 0x01)));
					// What happens when the macro macros itself? :)
					goto EvaluateDisplayListValue;
					break;
				case FT800EMU_DL_CLEAR:
					if (y >= gs.ScissorY.U && y < gs.ScissorY2.U)
					{
						if (debugTrace)
						{
							if (v && gs.ScissorX.U <= s_DebugTraceX && s_DebugTraceX < gs.ScissorX2.U)
							{
								if (*s_DebugTraceStackSize < s_DebugTraceStackMax)
								{
									s_DebugTraceStack[*s_DebugTraceStackSize] = s_DebugTraceLine;
									++(*s_DebugTraceStackSize);
								}
							}
						}
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
#ifdef FT810EMU_MODE
				case FT800EMU_DL_VERTEX_FORMAT:
					gs.VertexFormatShift = 4 - (v & 0x7);
					break;
				case FT800EMU_DL_BITMAP_LAYOUT_H:
					{
						BitmapInfo &bi = bitmapInfo[gs.BitmapHandle];
						int stride = (bi.LayoutStride & 0x3FF) | (((v >> 2) & 0x3) << 10);
						if (stride == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout stride invalid\n", gs.DebugDisplayListIndex);*/ stride = 4096; } // correct behaviour is probably 'infinite'?
						bi.LayoutStride = stride;
						bi.LayoutHeight = (bi.LayoutHeight & 0x1FF) | ((v & 0x3) << 9);
						if (bi.LayoutHeight == 0) { /*if (y == 0) FTEMU_printf("%i: Bitmap layout height invalid\n", gs.DebugDisplayListIndex);*/ bi.LayoutHeight = 2048; } // correct behaviour is probably 'infinite'?
						bi.LayoutWidth = getLayoutWidth(bi.LayoutFormat, stride);
					}
					break;
				case FT800EMU_DL_BITMAP_SIZE_H:
					bitmapInfo[gs.BitmapHandle].SizeWidth = (bitmapInfo[gs.BitmapHandle].SizeWidth & 0x1FF) | (((v >> 2) & 0x3) << 9);
					if (bitmapInfo[gs.BitmapHandle].SizeWidth == 0) bitmapInfo[gs.BitmapHandle].SizeWidth = 2048; // verify
					bitmapInfo[gs.BitmapHandle].SizeHeight = (bitmapInfo[gs.BitmapHandle].SizeHeight & 0x1FF) | ((v & 0x3) << 9);
					if (bitmapInfo[gs.BitmapHandle].SizeHeight== 0) bitmapInfo[gs.BitmapHandle].SizeHeight = 2048; // verify
					break;
				case FT800EMU_DL_PALETTE_SOURCE:
					gs.PaletteSource = v & FT800EMU_ADDR_MASK;
					break;
				case FT800EMU_DL_VERTEX_TRANSLATE_X:
					gs.VertexTranslateX = SIGNED_N(v & 0x1FFFF, 17);
					break;
				case FT800EMU_DL_VERTEX_TRANSLATE_Y:
					gs.VertexTranslateY = SIGNED_N(v & 0x1FFFF, 17);
					break;
				case FT800EMU_DL_NOP:
					// no-op
					break;
#endif
				default:
					FTEMU_printf("%i: Invalid display list entry %i\n", (int)c, (int)(v >> 24));
					if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Invalid display list entry");
				}
				break;
			case FT800EMU_DL_VERTEX2II:
				{
					int px = ((v >> 21) & 0x1FF) * 16;
					int py = ((v >> 12) & 0x1FF) * 16;
#ifdef FT810EMU_MODE
					px += gs.VertexTranslateX;
					py += gs.VertexTranslateY;
					if (swapXY) std::swap(px, py);
#endif
					switch (primitive)
					{
					case BITMAPS:
						displayBitmap<debugTrace>(gs, bc, bs, bt, y, hsize, px, py,
							((v >> 7) & 0x1F),
							v & 0x7F,
#ifdef FT810EMU_MODE
							swapXY,
#endif
							bitmapInfo);
						break;
					case POINTS:
						displayPoint<debugTrace>(gs, bc, bs, bt, y, hsize, px, py);
						break;
					case LINES:
						displayLines<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case LINE_STRIP:
						displayLineStrip<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_R:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripB<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
							displayEdgeStripR<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_L:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripA<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
							displayEdgeStripL<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_A:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripL<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
							displayEdgeStripA<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_B:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripR<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
							displayEdgeStripB<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case RECTS:
						displayRects<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					}
				}
				break;
			case FT800EMU_DL_VERTEX2F:
				{
					int px = (v >> 15) & 0x3FFF;
					if ((v >> 15) & 0x4000) px = px - 0x4000;
					int py = (v) & 0x3FFF;
					if ((v) & 0x4000) py = py - 0x4000;
#ifdef FT810EMU_MODE
					px <<= gs.VertexFormatShift;
					py <<= gs.VertexFormatShift;
					px += gs.VertexTranslateX;
					py += gs.VertexTranslateY;
					if (swapXY) std::swap(px, py);
#endif
					switch (primitive)
					{
					case BITMAPS:
						displayBitmap<debugTrace>(gs, bc, bs, bt, y, hsize, px, py,
							gs.BitmapHandle,
							gs.Cell,
#ifdef FT810EMU_MODE
							swapXY,
#endif
							bitmapInfo);
						break;
					case POINTS:
						displayPoint<debugTrace>(gs, bc, bs, bt, y, hsize, px, py);
						break;
					case LINES:
						displayLines<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case LINE_STRIP:
						displayLineStrip<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_R:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripB<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
						displayEdgeStripR<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_L:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripA<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
						displayEdgeStripL<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_A:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripL<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
						displayEdgeStripA<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case EDGE_STRIP_B:
#ifdef FT810EMU_MODE
						if (swapXY) displayEdgeStripR<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						else
#endif
						displayEdgeStripB<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					case RECTS:
						displayRects<debugTrace>(gs, bc, bs, bt, y, hsize, vs, px, py);
						break;
					}
				}
				break;
			}
		}
DisplayListDisplay:
		// Check tag query
		if (Memory.rawReadU32(ram, REG_TAG_Y) == y)
		// if (Memory.rawReadU32(ram, REG_TAG_Y) == ((Memory.rawReadU32(ram, REG_ROTATE) & 0x01) ? vsize - y - 1 : y))
		{
			uint32_t tag_x = Memory.rawReadU32(ram, REG_TAG_X);
			if (tag_x < hsize)
			{
				// Write tag out
				// Memory.rawWriteU32(ram, REG_TAG, bt[mirrored ? hsize - tag_x - 1 : tag_x]);
				Memory.rawWriteU32(ram, REG_TAG, bt[tag_x]);
			}
		}
		// Check touch
		if ((TouchClass::multiTouch())
			? (Memory.rawReadU32(ram, REG_CTOUCH_TOUCH0_XY) != 0x80008000)
			: (Memory.rawReadU32(ram, REG_TOUCH_RZ) <= Memory.rawReadU32(ram, REG_TOUCH_RZTHRESH))) // Touching harder than the threshold
		{
			uint32_t tag_xy = Memory.rawReadU32(ram, REG_TOUCH_TAG_XY);
			uint32_t tag_y = tag_xy & 0xFFFF;
			if (tag_y == y)
			// if (tag_y == ((Memory.rawReadU32(ram, REG_ROTATE) & 0x01) ? vsize - y - 1 : y))
			{
				uint32_t tag_x = tag_xy >> 16;
				if (tag_x < hsize)
				{
					// Write tag out
					// Memory.rawWriteU32(ram, REG_TOUCH_TAG, bt[mirrored ? hsize - tag_x - 1 : tag_x]);
					Memory.rawWriteU32(ram, REG_TOUCH_TAG, bt[tag_x]);
				}
			}
		}
		// TODO: MULTITOUCH 1,2,3,4
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
		// backlight emulation
		else if (s_RegPwmDutyEmulation)
		{
			uint32_t pwmduty = Memory.rawReadU32(ram, REG_PWM_DUTY);
			if (pwmduty < 128)
			{
				for (uint32_t x = 0; x < hsize; ++x)
				{
					bc[x] = mulalpha128_argb(bc[x], pwmduty);
				}
			}
		}
		// mirror display
		if (mirrored)
		{
			for (uint32_t xl = 0; xl < (hsize >> 1); ++xl)
			{
				uint32_t xr = hsize - xl - 1;
				std::swap(bc[xl], bc[xr]);
			}
		}
		++lines_processed;
	}

	if (yIdx == 0)
	{
		s_DebugLimiterEffective = false;
		s_DebugLimiterIndex = debugLimiterIndex;
		s_DebugLimiterEffective = debugLimiterEffective;
	}

	if (!lines_processed)
	{
		processBlankDL(bitmapInfo);
	}
}

} /* anonymous namespace */

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
int launchGraphicsProcessorThread(void *startInfo)
{
	unsigned long taskId = 0;
	void *taskHandle;
	taskHandle = FT8XXEMU::System.setThreadGamesCategory(&taskId);
	FT8XXEMU::System.disableAutomaticPriorityBoost();
	FT8XXEMU::System.makeRealtimePriorityThread();

	ThreadInfo *li = static_cast<ThreadInfo *>(startInfo);
	for (; ; )
	{
		SDL_SemWait(li->StartSem);

		if (!li->Running)
		{
			break;
		}

		processPart<false>(
			li->ScreenArgb8888, 
			li->UpsideDown, 
			li->Mirrored, 
#ifdef FT810EMU_MODE
			li->SwapXY,
#endif
			li->HSize, 
			li->VSize, 
			li->YIdx, 
			li->YInc, 
			li->Bitmap);

		// FTEMU_printf("%i: sem post (%i) ->\n", Memory.rawReadU32(Memory.getRam(), REG_FRAMES), SDL_SemValue(li->EndSem));
		SDL_SemPost(li->EndSem);
		// FTEMU_printf("%i: sem post (%i) <-\n", Memory.rawReadU32(Memory.getRam(), REG_FRAMES), SDL_SemValue(li->EndSem));
	}

	FT8XXEMU::System.revertThreadCategory(taskHandle);

	return 0;
}
#else
#	ifdef WIN32
DWORD WINAPI launchGraphicsProcessorThread(void *startInfo)
{
	FT8XXEMU::ThreadState threadState;
	threadState.foreground();
	threadState.noboost();
	threadState.realtime();
	threadState.setName("FT8XXEMU Graphics Slave");
	bool realtime = true;

	ThreadInfo *li = static_cast<ThreadInfo *>(startInfo);
	for (; ; )
	{
		WaitForSingleObject(li->StartEvent, INFINITE);

		if (!li->Running)
		{
			break;
		}

		// Adjust thread priority
		if (s_ThreadPriorityRealtime != realtime)
		{
			realtime = s_ThreadPriorityRealtime;
			if (realtime) threadState.realtime();
			else threadState.prioritize();
		}

		processPart<false>(
			li->ScreenArgb8888, 
			li->UpsideDown, 
			li->Mirrored, 
#ifdef FT810EMU_MODE
			li->SwapXY,
#endif
			li->HSize, 
			li->VSize, 
			li->YIdx, 
			li->YInc, 
			li->Bitmap);

		SetEvent(li->EndEvent);
	}

	threadState.reset();

	return 0;
}
#	endif
#endif

void GraphicsProcessorClass::setThreadPriority(bool realtime)
{
	s_ThreadPriorityRealtime = realtime;
}

void GraphicsProcessorClass::process(
	argb8888 *screenArgb8888, 
	bool upsideDown, 
	bool mirrored,
#ifdef FT810EMU_MODE
	bool swapXY,
#endif
	uint32_t hsize, 
	uint32_t vsize, 
	uint32_t yIdx, 
	uint32_t yInc)

{
	uint8_t *const ram = Memory.getRam();

	// Store the touch tag xy used for lookup
	Memory.rawWriteU32(ram, REG_TOUCH_TAG_XY, Memory.rawReadU32(ram, REG_TOUCH_SCREEN_XY));

	for (int i = 1; i < s_ThreadCount; ++i)
	{
		// Launch threads
		// processPart(screenArgb8888, upsideDown, mirrored, hsize, vsize, (i * yInc) + yIdx, s_ThreadCount * yInc);
		ThreadInfo *li = &s_ThreadInfos[i - 1];
		li->ScreenArgb8888 = screenArgb8888;
		li->UpsideDown = upsideDown;
		li->Mirrored = mirrored;
#ifdef FT810EMU_MODE
		li->SwapXY = swapXY;
#endif
		li->HSize = hsize;
		li->VSize = vsize;
		li->YIdx = (i * yInc) + yIdx;
		li->YInc = s_ThreadCount * yInc;
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
		// li->Thread = SDL_CreateThreadFT(launchThread, static_cast<void *>(li));
		SDL_SemPost(li->StartSem);
#else
#	ifdef WIN32
		SetEvent(li->StartEvent);
#	else
		processPart<false>(screenArgb8888, upsideDown, mirrored FT810EMU_SWAPXY, hsize, vsize, (i * yInc) + yIdx, s_ThreadCount * yInc, s_BitmapInfoMain);
#	endif
#endif
	}

	// Run part on this thread
	processPart<false>(screenArgb8888, upsideDown, mirrored FT810EMU_SWAPXY, hsize, vsize, yIdx, s_ThreadCount * yInc, s_BitmapInfoMain);

	for (int i = 1; i < s_ThreadCount; ++i)
	{
		// Wait for threads
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
		ThreadInfo *li = &s_ThreadInfos[i - 1];
		// SDL_WaitThread(li->Thread, NULL);
		// SDL_mutexP(li->StartMutex);
		// SDL_mutexV(li->StartMutex);
		// FTEMU_printf("%i: sem wait %i (%i)->\n", Memory.rawReadU32(ram, REG_FRAMES), i, SDL_SemValue(li->EndSem));
		SDL_SemWait(li->EndSem);
		// FTEMU_printf("%i: sem wait %i (%i)<-\n", Memory.rawReadU32(ram, REG_FRAMES), i, SDL_SemValue(li->EndSem));
#else
#	ifdef WIN32
		ThreadInfo *li = &s_ThreadInfos[i - 1];
		WaitForSingleObject(li->EndEvent, INFINITE);
		ResetEvent(li->EndEvent);
#	endif
#endif
	}
}

void GraphicsProcessorClass::processBlank()
{
	processBlankDL(s_BitmapInfoMain);

	// Copy bitmap infos to thread local
	for (size_t i = 0; i < s_ThreadInfos.size(); ++i)
	{
		memcpy(&s_ThreadInfos[i].Bitmap, &s_BitmapInfoMain, sizeof(s_BitmapInfoMain));
	}
}

void GraphicsProcessorClass::processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize)
{
	argb8888 buffer[FT800EMU_SCREEN_WIDTH_MAX];
	argb8888 *dummyBuffer = buffer - (y * hsize);
	BitmapInfo bitmapInfo[32];
	memcpy(&bitmapInfo, &s_BitmapInfoMain, sizeof(s_BitmapInfoMain));
	s_DebugTraceX = x;
	s_DebugTraceStack = result;
	s_DebugTraceStackMax = *size;
	*size = 0;
	s_DebugTraceStackSize = size;
	processPart<true>(dummyBuffer, false, false FT810EMU_SWAPXY_FALSE, hsize, y + 1, y, FT800EMU_SCREEN_HEIGHT_MAX, bitmapInfo); // TODO: REG_ROTATE
	s_DebugTraceX = ~0;
	s_DebugTraceStackMax = 0;
	s_DebugTraceStackSize = &s_DebugTraceStackMax;
	s_DebugTraceStack = NULL;
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

void GraphicsProcessorClass::setDebugLimiter(int debugLimiter)
{
	s_DebugLimiter = debugLimiter;
}

int GraphicsProcessorClass::getDebugLimiter()
{
	return s_DebugLimiter;
}

bool GraphicsProcessorClass::getDebugLimiterEffective()
{
	return s_DebugLimiterEffective;
}

int GraphicsProcessorClass::getDebugLimiterIndex()
{
	return s_DebugLimiterIndex;
}

/*
// Sets operation trace on specified point
void GraphicsProcessorClass::setDebugTrace(uint32_t x, uint32_t y)
{
	s_DebugTraceX = x;
	s_DebugTraceY = y;
}

// Disables or enables tracing
void GraphicsProcessorClass::setDebugTrace(bool enabled)
{
	s_DebugTraceEnabled = enabled;
}

// Returns the debug tracing state
void GraphicsProcessorClass::getDebugTrace(bool &enabled, uint32_t &x, uint32_t &y)
{
	enabled = s_DebugTraceEnabled;
	x = s_DebugTraceX;
	y = s_DebugTraceY;
}

// Returns a *copy* of the debug trace
void GraphicsProcessorClass::getDebugTrace(std::vector<int> &result)
{
	for (int i = 0; i < s_DebugTraceStackData.size(); ++i)
	{
		result.push_back(s_DebugTraceStackData[i]);
	}
}
*/

} /* namespace FT800EMU */

/* end of file */
