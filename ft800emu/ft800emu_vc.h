/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
BT815 Emulator Library
Copyright (C) 2016-2017  Bridgetek Pte Lte
*/

#ifndef FT800EMU_VC_H
#define FT800EMU_VC_H

#ifdef BT815EMU_MODE
#	include "vc3.h"
#elif defined(FT810EMU_MODE)
#	include "vc2.h"
#else
#	include "vc.h"
#endif

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
#ifdef FT810EMU_MODE
#	define FT810EMU_DL_VERTEX_FORMAT 39
#	define FT810EMU_DL_BITMAP_LAYOUT_H 40
#	define FT810EMU_DL_BITMAP_SIZE_H 41
#	define FT810EMU_DL_PALETTE_SOURCE 42
#	define FT810EMU_DL_VERTEX_TRANSLATE_X 43
#	define FT810EMU_DL_VERTEX_TRANSLATE_Y 44
#	define FT810EMU_DL_NOP 45
#endif
#ifdef BT815EMU_MODE
#	define BT815EMU_DL_BITMAP_EXT_FORMAT 46
#	define BT815EMU_DL_BITMAP_SWIZZLE 47
#	define BT815EMU_DL_INT_FRR 48
#endif

#endif /* #ifndef FT800EMU_VC_H */

/* end of file */
