/**
 * dl_parser.cpp
 * $Id$
 * \file dl_parser.cpp
 * \brief dl_parser.cpp
 * \date 2013-11-06 08:55GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "dl_parser.h"

// STL includes
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

// Qt includes
#include <QStringList>

// Emulator includes
#include "ft8xxemu_inttypes.h"
#include "ft800emu_memory.h"
#include "ft800emu_vc.h"

// Project includes

namespace FT800EMUQT {

static std::map<std::string, int> s_IdMap;
static std::map<std::string, int> s_ParamMap;

static std::map<std::string, int> s_CmdIdMap;
static std::map<std::string, int> s_CmdParamMap;

// TODO_FT810 DL_ID_NB 46
#ifdef FT810EMU_MODE
#define DL_ID_NB 39
#define CMD_ID_NB 68
#else
#define DL_ID_NB 39
#define CMD_ID_NB 54
#endif
static int s_ParamCount[DL_ID_NB];
static int s_CmdParamCount[CMD_ID_NB];
static bool s_CmdParamString[CMD_ID_NB];

static std::string s_CmdIdList[CMD_ID_NB];

// Sign-extend the n-bit value v
#define SIGNED_N(v, n) \
    (((int32_t)((v) << (32-(n)))) >> (32-(n)))

const char *g_DlEnumBlend[DL_ENUM_BLEND_NB] = {
	"ZERO",
	"ONE",
	"SRC_ALPHA",
	"DST_ALPHA",
	"ONE_MINUS_SRC_ALPHA",
	"ONE_MINUS_DST_ALPHA",
};

const char *g_DlEnumCompare[DL_ENUM_COMPARE_NB] = {
	"NEVER",
	"LESS",
	"LEQUAL",
	"GREATER",
	"GEQUAL",
	"EQUAL",
	"NOTEQUAL",
	"ALWAYS",
};

const char *g_DlEnumStencil[DL_ENUM_STENCIL_NB] = {
	"ZERO",
	"KEEP",
	"REPLACE",
	"INCR",
	"DECR",
	"INVERT",
};

const char *g_DlEnumBitmapFormat[DL_ENUM_BITMAP_FORMAT_NB] = {
	"ARGB1555", // 0
	"L1", // 1
	"L4", // 2
	"L8", // 3
	"RGB332", // 4
	"ARGB2", // 5
	"ARGB4", // 6
	"RGB565", // 7
#ifdef FT810EMU_MODE
	"8", // 8 deprecated
#else
	"PALETTED", // 8
#endif
	"TEXT8X8", // 9
	"TEXTVGA", // 10
	"BARGRAPH", // 11
#ifdef FT810EMU_MODE
	"12", // 12 does not exist
	"13", // 13 does not exist
	"PALETTED565", // 14
	"PALETTED4444", // 15
	"PALETTED8", // 16
	"L2", // 17
#endif
};

const char *g_DlEnumBitmapFilter[DL_ENUM_BITMAP_FILTER_NB] = {
	"NEAREST",
	"BILINEAR",
};

const char *g_DlEnumBitmapWrap[DL_ENUM_BITMAP_WRAP_NB] = {
	"BORDER",
	"REPEAT",
};

const char *g_DlEnumPrimitive[DL_ENUM_PRIMITIVE_NB] = {
	"0",
	"BITMAPS",
	"POINTS",
	"LINES",
	"LINE_STRIP",
	"EDGE_STRIP_R",
	"EDGE_STRIP_L",
	"EDGE_STRIP_A",
	"EDGE_STRIP_B",
	"RECTS",
};

void DlParser::init()
{
	if (!s_IdMap.size())
	{
		s_IdMap["DISPLAY"] = FT800EMU_DL_DISPLAY;
		s_ParamCount[FT800EMU_DL_DISPLAY] = 0;
		s_IdMap["BITMAP_SOURCE"] = FT800EMU_DL_BITMAP_SOURCE;
		s_ParamCount[FT800EMU_DL_BITMAP_SOURCE] = 1;
		s_IdMap["CLEAR_COLOR_RGB"] = FT800EMU_DL_CLEAR_COLOR_RGB;
		s_ParamCount[FT800EMU_DL_CLEAR_COLOR_RGB] = 3;
		s_IdMap["TAG"] = FT800EMU_DL_TAG;
		s_ParamCount[FT800EMU_DL_TAG] = 1;
		s_IdMap["COLOR_RGB"] = FT800EMU_DL_COLOR_RGB;
		s_ParamCount[FT800EMU_DL_COLOR_RGB] = 3;
		s_IdMap["BITMAP_HANDLE"] = FT800EMU_DL_BITMAP_HANDLE;
		s_ParamCount[FT800EMU_DL_BITMAP_HANDLE] = 1;
		s_IdMap["CELL"] = FT800EMU_DL_CELL;
		s_ParamCount[FT800EMU_DL_CELL] = 1;
		s_IdMap["BITMAP_LAYOUT"] = FT800EMU_DL_BITMAP_LAYOUT;
		s_ParamCount[FT800EMU_DL_BITMAP_LAYOUT] = 3;
		s_IdMap["BITMAP_SIZE"] = FT800EMU_DL_BITMAP_SIZE;
		s_ParamCount[FT800EMU_DL_BITMAP_SIZE] = 5;
		s_IdMap["ALPHA_FUNC"] = FT800EMU_DL_ALPHA_FUNC;
		s_ParamCount[FT800EMU_DL_ALPHA_FUNC] = 2;
		s_IdMap["STENCIL_FUNC"] = FT800EMU_DL_STENCIL_FUNC;
		s_ParamCount[FT800EMU_DL_STENCIL_FUNC] = 3;
		s_IdMap["BLEND_FUNC"] = FT800EMU_DL_BLEND_FUNC;
		s_ParamCount[FT800EMU_DL_BLEND_FUNC] = 2;
		s_IdMap["STENCIL_OP"] = FT800EMU_DL_STENCIL_OP;
		s_ParamCount[FT800EMU_DL_STENCIL_OP] = 2;
		s_IdMap["POINT_SIZE"] = FT800EMU_DL_POINT_SIZE;
		s_ParamCount[FT800EMU_DL_POINT_SIZE] = 1;
		s_IdMap["LINE_WIDTH"] = FT800EMU_DL_LINE_WIDTH;
		s_ParamCount[FT800EMU_DL_LINE_WIDTH] = 1;
		s_IdMap["CLEAR_COLOR_A"] = FT800EMU_DL_CLEAR_COLOR_A;
		s_ParamCount[FT800EMU_DL_CLEAR_COLOR_A] = 1;
		s_IdMap["COLOR_A"] = FT800EMU_DL_COLOR_A;
		s_ParamCount[FT800EMU_DL_COLOR_A] = 1;
		s_IdMap["CLEAR_STENCIL"] = FT800EMU_DL_CLEAR_STENCIL;
		s_ParamCount[FT800EMU_DL_CLEAR_STENCIL] = 1;
		s_IdMap["CLEAR_TAG"] = FT800EMU_DL_CLEAR_TAG;
		s_ParamCount[FT800EMU_DL_CLEAR_TAG] = 1;
		s_IdMap["STENCIL_MASK"] = FT800EMU_DL_STENCIL_MASK;
		s_ParamCount[FT800EMU_DL_STENCIL_MASK] = 1;
		s_IdMap["TAG_MASK"] = FT800EMU_DL_TAG_MASK;
		s_ParamCount[FT800EMU_DL_TAG_MASK] = 1;
		s_IdMap["BITMAP_TRANSFORM_A"] = FT800EMU_DL_BITMAP_TRANSFORM_A;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_A] = 1;
		s_IdMap["BITMAP_TRANSFORM_B"] = FT800EMU_DL_BITMAP_TRANSFORM_B;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_B] = 1;
		s_IdMap["BITMAP_TRANSFORM_C"] = FT800EMU_DL_BITMAP_TRANSFORM_C;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_C] = 1;
		s_IdMap["BITMAP_TRANSFORM_D"] = FT800EMU_DL_BITMAP_TRANSFORM_D;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_D] = 1;
		s_IdMap["BITMAP_TRANSFORM_E"] = FT800EMU_DL_BITMAP_TRANSFORM_E;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_E] = 1;
		s_IdMap["BITMAP_TRANSFORM_F"] = FT800EMU_DL_BITMAP_TRANSFORM_F;
		s_ParamCount[FT800EMU_DL_BITMAP_TRANSFORM_F] = 1;
		s_IdMap["SCISSOR_XY"] = FT800EMU_DL_SCISSOR_XY;
		s_ParamCount[FT800EMU_DL_SCISSOR_XY] = 2;
		s_IdMap["SCISSOR_SIZE"] = FT800EMU_DL_SCISSOR_SIZE;
		s_ParamCount[FT800EMU_DL_SCISSOR_SIZE] = 2;
		s_IdMap["CALL"] = FT800EMU_DL_CALL;
		s_ParamCount[FT800EMU_DL_CALL] = 1;
		s_IdMap["JUMP"] = FT800EMU_DL_JUMP;
		s_ParamCount[FT800EMU_DL_JUMP] = 1;
		s_IdMap["BEGIN"] = FT800EMU_DL_BEGIN;
		s_ParamCount[FT800EMU_DL_BEGIN] = 1;
		s_IdMap["COLOR_MASK"] = FT800EMU_DL_COLOR_MASK;
		s_ParamCount[FT800EMU_DL_COLOR_MASK] = 4;
		s_IdMap["END"] = FT800EMU_DL_END;
		s_ParamCount[FT800EMU_DL_END] = 0;
		s_IdMap["SAVE_CONTEXT"] = FT800EMU_DL_SAVE_CONTEXT;
		s_ParamCount[FT800EMU_DL_SAVE_CONTEXT] = 0;
		s_IdMap["RESTORE_CONTEXT"] = FT800EMU_DL_RESTORE_CONTEXT;
		s_ParamCount[FT800EMU_DL_RESTORE_CONTEXT] = 0;
		s_IdMap["RETURN"] = FT800EMU_DL_RETURN;
		s_ParamCount[FT800EMU_DL_RETURN] = 0;
		s_IdMap["MACRO"] = FT800EMU_DL_MACRO;
		s_ParamCount[FT800EMU_DL_MACRO] = 1;
		s_IdMap["CLEAR"] = FT800EMU_DL_CLEAR;
		s_ParamCount[FT800EMU_DL_CLEAR] = 3;
#ifdef FT810EMU_MODE
		s_IdMap["VERTEX_FORMAT"] = FT800EMU_DL_VERTEX_FORMAT;
		s_ParamCount[FT800EMU_DL_VERTEX_FORMAT] = 1;
		s_IdMap["BITMAP_LAYOUT_H"] = FT800EMU_DL_BITMAP_LAYOUT_H;
		s_ParamCount[FT800EMU_DL_BITMAP_LAYOUT_H] = 2;
		s_IdMap["BITMAP_SIZE_H"] = FT800EMU_DL_BITMAP_SIZE_H;
		s_ParamCount[FT800EMU_DL_BITMAP_SIZE_H] = 2;
		s_IdMap["PALETTE_SOURCE"] = FT800EMU_DL_PALETTE_SOURCE;
		s_ParamCount[FT800EMU_DL_PALETTE_SOURCE] = 1;
		s_IdMap["VERTEX_TRANSLATE_X"] = FT800EMU_DL_VERTEX_TRANSLATE_X;
		s_ParamCount[FT800EMU_DL_VERTEX_TRANSLATE_X] = 1;
		s_IdMap["VERTEX_TRANSLATE_Y"] = FT800EMU_DL_VERTEX_TRANSLATE_Y;
		s_ParamCount[FT800EMU_DL_VERTEX_TRANSLATE_Y] = 1;
		s_IdMap["NOP"] = FT800EMU_DL_NOP;
		s_ParamCount[FT800EMU_DL_NOP] = 0;
#endif
	}
	if (!s_ParamMap.size())
	{
		s_ParamMap["ALWAYS"] = ALWAYS;
		s_ParamMap["ARGB1555"] = ARGB1555;
		s_ParamMap["ARGB2"] = ARGB2;
		s_ParamMap["ARGB4"] = ARGB4;
		s_ParamMap["BARGRAPH"] = BARGRAPH;
		s_ParamMap["BILINEAR"] = BILINEAR;
		s_ParamMap["BITMAPS"] = BITMAPS;
		s_ParamMap["BORDER"] = BORDER;
		s_ParamMap["DECR"] = DECR;
		s_ParamMap["DST_ALPHA"] = DST_ALPHA;
		s_ParamMap["EDGE_STRIP_A"] = EDGE_STRIP_A;
		s_ParamMap["EDGE_STRIP_B"] = EDGE_STRIP_B;
		s_ParamMap["EDGE_STRIP_L"] = EDGE_STRIP_L;
		s_ParamMap["EDGE_STRIP_R"] = EDGE_STRIP_R;
		s_ParamMap["EQUAL"] = EQUAL;
		s_ParamMap["GEQUAL"] = GEQUAL;
		s_ParamMap["GREATER"] = GREATER;
		s_ParamMap["INCR"] = INCR;
		s_ParamMap["INVERT"] = INVERT;
		s_ParamMap["KEEP"] = KEEP;
		s_ParamMap["L1"] = L1;
#if FT810EMU_MODE
		s_ParamMap["L2"] = L2;
#endif
		s_ParamMap["L4"] = L4;
		s_ParamMap["L8"] = L8;
		s_ParamMap["LEQUAL"] = LEQUAL;
		s_ParamMap["LESS"] = LESS;
		s_ParamMap["LINEAR_SAMPLES"] = LINEAR_SAMPLES;
		s_ParamMap["LINES"] = LINES;
		s_ParamMap["LINE_STRIP"] = LINE_STRIP;
		s_ParamMap["NEAREST"] = NEAREST;
		s_ParamMap["NEVER"] = NEVER;
		s_ParamMap["NOTEQUAL"] = NOTEQUAL;
		s_ParamMap["ONE"] = ONE;
		s_ParamMap["ONE_MINUS_DST_ALPHA"] = ONE_MINUS_DST_ALPHA;
		s_ParamMap["ONE_MINUS_SRC_ALPHA"] = ONE_MINUS_SRC_ALPHA;
#if FT810EMU_MODE
		s_ParamMap["PALETTED8"] = PALETTED8;
		s_ParamMap["PALETTED4444"] = PALETTED4444;
		s_ParamMap["PALETTED565"] = PALETTED565;
#else
		s_ParamMap["PALETTED"] = PALETTED;
#endif
		s_ParamMap["POINTS"] = POINTS;
		s_ParamMap["RECTS"] = RECTS;
		s_ParamMap["REPEAT"] = REPEAT;
		s_ParamMap["REPLACE"] = REPLACE;
		s_ParamMap["RGB332"] = RGB332;
		s_ParamMap["RGB565"] = RGB565;
		s_ParamMap["SRC_ALPHA"] = SRC_ALPHA;
		s_ParamMap["TEXT8X8"] = TEXT8X8;
		s_ParamMap["TEXTVGA"] = TEXTVGA;
		s_ParamMap["ZERO"] = ZERO;
	}
	if (!s_CmdIdMap.size())
	{
		s_CmdIdMap["CMD_DLSTART"] = CMD_DLSTART & 0xFF;
		s_CmdParamCount[CMD_DLSTART & 0xFF] = 0;
		s_CmdParamString[CMD_DLSTART & 0xFF] = false;
		s_CmdIdMap["CMD_SWAP"] = CMD_SWAP & 0xFF;
		s_CmdParamCount[CMD_SWAP & 0xFF] = 0;
		s_CmdParamString[CMD_SWAP & 0xFF] = false;
		s_CmdIdMap["CMD_INTERRUPT"] = CMD_INTERRUPT & 0xFF;
		s_CmdParamCount[CMD_INTERRUPT & 0xFF] = 1;
		s_CmdParamString[CMD_INTERRUPT & 0xFF] = false;
		// s_CmdIdMap["CMD_CRC"] = CMD_CRC & 0xFF;
		// s_CmdParamCount[CMD_CRC & 0xFF] = 3;
		// s_CmdParamString[CMD_CRC & 0xFF] = false; // undocumented
		// s_CmdIdMap["CMD_HAMMERAUX"] = CMD_HAMMERAUX & 0xFF;
		// s_CmdParamCount[CMD_HAMMERAUX & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_HAMMERAUX & 0xFF] = false;
		// s_CmdIdMap["CMD_MARCH"] = CMD_MARCH & 0xFF;
		// s_CmdParamCount[CMD_MARCH & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_MARCH & 0xFF] = false;
		// s_CmdIdMap["CMD_IDCT"] = CMD_IDCT & 0xFF;
		// s_CmdParamCount[CMD_IDCT & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_IDCT & 0xFF] = false;
		// s_CmdIdMap["CMD_EXECUTE"] = CMD_EXECUTE & 0xFF;
		// s_CmdParamCount[CMD_EXECUTE & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_EXECUTE & 0xFF] = false;
		// s_CmdIdMap["CMD_GETPOINT"] = CMD_GETPOINT & 0xFF;
		// s_CmdParamCount[CMD_GETPOINT & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_GETPOINT & 0xFF] = false;
		s_CmdIdMap["CMD_BGCOLOR"] = CMD_BGCOLOR & 0xFF;
		s_CmdParamCount[CMD_BGCOLOR & 0xFF] = 3; // argb
		s_CmdParamString[CMD_BGCOLOR & 0xFF] = false;
		s_CmdIdMap["CMD_FGCOLOR"] = CMD_FGCOLOR & 0xFF;
		s_CmdParamCount[CMD_FGCOLOR & 0xFF] = 3; // argb
		s_CmdParamString[CMD_FGCOLOR & 0xFF] = false;
		s_CmdIdMap["CMD_GRADIENT"] = CMD_GRADIENT & 0xFF;
		s_CmdParamCount[CMD_GRADIENT & 0xFF] = 2 + 3 + 2 + 3; // argb
		s_CmdParamString[CMD_GRADIENT & 0xFF] = false;
		s_CmdIdMap["CMD_TEXT"] = CMD_TEXT & 0xFF;
		s_CmdParamCount[CMD_TEXT & 0xFF] = 5;
		s_CmdParamString[CMD_TEXT & 0xFF] = true;
		s_CmdIdMap["CMD_BUTTON"] = CMD_BUTTON & 0xFF;
		s_CmdParamCount[CMD_BUTTON & 0xFF] = 7;
		s_CmdParamString[CMD_BUTTON & 0xFF] = true;
		s_CmdIdMap["CMD_KEYS"] = CMD_KEYS & 0xFF;
		s_CmdParamCount[CMD_KEYS & 0xFF] = 7;
		s_CmdParamString[CMD_KEYS & 0xFF] = true;
		s_CmdIdMap["CMD_PROGRESS"] = CMD_PROGRESS & 0xFF;
		s_CmdParamCount[CMD_PROGRESS & 0xFF] = 7;
		s_CmdParamString[CMD_PROGRESS & 0xFF] = false;
		s_CmdIdMap["CMD_SLIDER"] = CMD_SLIDER & 0xFF;
		s_CmdParamCount[CMD_SLIDER & 0xFF] = 7;
		s_CmdParamString[CMD_SLIDER & 0xFF] = false;
		s_CmdIdMap["CMD_SCROLLBAR"] = CMD_SCROLLBAR & 0xFF;
		s_CmdParamCount[CMD_SCROLLBAR & 0xFF] = 8;
		s_CmdParamString[CMD_SCROLLBAR & 0xFF] = false;
		s_CmdIdMap["CMD_TOGGLE"] = CMD_TOGGLE & 0xFF;
		s_CmdParamCount[CMD_TOGGLE & 0xFF] = 7;
		s_CmdParamString[CMD_TOGGLE & 0xFF] = true;
		s_CmdIdMap["CMD_GAUGE"] = CMD_GAUGE & 0xFF;
		s_CmdParamCount[CMD_GAUGE & 0xFF] = 8;
		s_CmdParamString[CMD_GAUGE & 0xFF] = false;
		s_CmdIdMap["CMD_CLOCK"] = CMD_CLOCK & 0xFF;
		s_CmdParamCount[CMD_CLOCK & 0xFF] = 8;
		s_CmdParamString[CMD_CLOCK & 0xFF] = false;
		s_CmdIdMap["CMD_CALIBRATE"] = CMD_CALIBRATE & 0xFF;
		s_CmdParamCount[CMD_CALIBRATE & 0xFF] = 1;
		s_CmdParamString[CMD_CALIBRATE & 0xFF] = false;
		s_CmdIdMap["CMD_SPINNER"] = CMD_SPINNER & 0xFF;
		s_CmdParamCount[CMD_SPINNER & 0xFF] = 4;
		s_CmdParamString[CMD_SPINNER & 0xFF] = false;
		s_CmdIdMap["CMD_STOP"] = CMD_STOP & 0xFF;
		s_CmdParamCount[CMD_STOP & 0xFF] = 0;
		s_CmdParamString[CMD_STOP & 0xFF] = false;
		// s_CmdIdMap["CMD_MEMCRC"] = CMD_MEMCRC & 0xFF; // don't support reading values
		// s_CmdParamCount[CMD_MEMCRC & 0xFF] = 3;
		// s_CmdParamString[CMD_MEMCRC & 0xFF] = false;
		// s_CmdIdMap["CMD_REGREAD"] = CMD_REGREAD & 0xFF; // don't support reading values
		// s_CmdParamCount[CMD_REGREAD & 0xFF] = 2;
		// s_CmdParamString[CMD_REGREAD & 0xFF] = false;
		// s_CmdIdMap["CMD_MEMWRITE"] = CMD_MEMWRITE & 0xFF; // don't support streaming data into cmd
		// s_CmdParamCount[CMD_MEMWRITE & 0xFF] = 2;
		// s_CmdParamString[CMD_MEMWRITE & 0xFF] = false;
		s_CmdIdMap["CMD_MEMSET"] = CMD_MEMSET & 0xFF;
		s_CmdParamCount[CMD_MEMSET & 0xFF] = 3;
		s_CmdParamString[CMD_MEMSET & 0xFF] = false;
		s_CmdIdMap["CMD_MEMZERO"] = CMD_MEMZERO & 0xFF;
		s_CmdParamCount[CMD_MEMZERO & 0xFF] = 2;
		s_CmdParamString[CMD_MEMZERO & 0xFF] = false;
		s_CmdIdMap["CMD_MEMCPY"] = CMD_MEMCPY & 0xFF;
		s_CmdParamCount[CMD_MEMCPY & 0xFF] = 3;
		s_CmdParamString[CMD_MEMCPY & 0xFF] = false;
		s_CmdIdMap["CMD_APPEND"] = CMD_APPEND & 0xFF;
		s_CmdParamCount[CMD_APPEND & 0xFF] = 2;
		s_CmdParamString[CMD_APPEND & 0xFF] = false;
		s_CmdIdMap["CMD_SNAPSHOT"] = CMD_SNAPSHOT & 0xFF;
		s_CmdParamCount[CMD_SNAPSHOT & 0xFF] = 1;
		s_CmdParamString[CMD_SNAPSHOT & 0xFF] = false;
		// s_CmdIdMap["CMD_TOUCH_TRANSFORM"] = CMD_TOUCH_TRANSFORM & 0xFF;
		// s_CmdParamCount[CMD_TOUCH_TRANSFORM & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_TOUCH_TRANSFORM & 0xFF] = false;
		// s_CmdIdMap["CMD_BITMAP_TRANSFORM"] = CMD_BITMAP_TRANSFORM & 0xFF;
		// s_CmdParamCount[CMD_BITMAP_TRANSFORM & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_BITMAP_TRANSFORM & 0xFF] = false;
		// s_CmdIdMap["CMD_INFLATE"] = CMD_INFLATE & 0xFF; // don't support streaming data into cmd
		// s_CmdParamCount[CMD_INFLATE & 0xFF] = 1;
		// s_CmdParamString[CMD_INFLATE & 0xFF] = false;
		// s_CmdIdMap["CMD_GETPTR"] = CMD_GETPTR & 0xFF;
		// s_CmdParamCount[CMD_GETPTR & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_GETPTR & 0xFF] = false;
		// s_CmdIdMap["CMD_LOADIMAGE"] = CMD_LOADIMAGE & 0xFF;  // don't support streaming data into cmd
		// s_CmdParamCount[CMD_LOADIMAGE & 0xFF] = 2;
		// s_CmdParamString[CMD_LOADIMAGE & 0xFF] = false;
		// s_CmdIdMap["CMD_GETPROPS"] = CMD_GETPROPS & 0xFF;
		// s_CmdParamCount[CMD_GETPROPS & 0xFF] = 0; // undocumented
		// s_CmdParamString[CMD_GETPROPS & 0xFF] = false;
		s_CmdIdMap["CMD_LOADIDENTITY"] = CMD_LOADIDENTITY & 0xFF;
		s_CmdParamCount[CMD_LOADIDENTITY & 0xFF] = 0;
		s_CmdParamString[CMD_LOADIDENTITY & 0xFF] = false;
		s_CmdIdMap["CMD_TRANSLATE"] = CMD_TRANSLATE & 0xFF;
		s_CmdParamCount[CMD_TRANSLATE & 0xFF] = 2;
		s_CmdParamString[CMD_TRANSLATE & 0xFF] = false;
		s_CmdIdMap["CMD_SCALE"] = CMD_SCALE & 0xFF;
		s_CmdParamCount[CMD_SCALE & 0xFF] = 2;
		s_CmdParamString[CMD_SCALE & 0xFF] = false;
		s_CmdIdMap["CMD_ROTATE"] = CMD_ROTATE & 0xFF;
		s_CmdParamCount[CMD_ROTATE & 0xFF] = 1;
		s_CmdParamString[CMD_ROTATE & 0xFF] = false;
		s_CmdIdMap["CMD_SETMATRIX"] = CMD_SETMATRIX & 0xFF;
		s_CmdParamCount[CMD_SETMATRIX & 0xFF] = 0;
		s_CmdParamString[CMD_SETMATRIX & 0xFF] = false;
		s_CmdIdMap["CMD_SETFONT"] = CMD_SETFONT & 0xFF;
		s_CmdParamCount[CMD_SETFONT & 0xFF] = 2;
		s_CmdParamString[CMD_SETFONT & 0xFF] = false;
		s_CmdIdMap["CMD_TRACK"] = CMD_TRACK & 0xFF;
		s_CmdParamCount[CMD_TRACK & 0xFF] = 5;
		s_CmdParamString[CMD_TRACK & 0xFF] = false;
		s_CmdIdMap["CMD_DIAL"] = CMD_DIAL & 0xFF;
		s_CmdParamCount[CMD_DIAL & 0xFF] = 5;
		s_CmdParamString[CMD_DIAL & 0xFF] = false;
		s_CmdIdMap["CMD_NUMBER"] = CMD_NUMBER & 0xFF;
		s_CmdParamCount[CMD_NUMBER & 0xFF] = 5;
		s_CmdParamString[CMD_NUMBER & 0xFF] = false;
		s_CmdIdMap["CMD_SCREENSAVER"] = CMD_SCREENSAVER & 0xFF;
		s_CmdParamCount[CMD_SCREENSAVER & 0xFF] = 0;
		s_CmdParamString[CMD_SCREENSAVER & 0xFF] = false;
		s_CmdIdMap["CMD_SKETCH"] = CMD_SKETCH & 0xFF;
		s_CmdParamCount[CMD_SKETCH & 0xFF] = 6;
		s_CmdParamString[CMD_SKETCH & 0xFF] = false;
#ifndef FT810EMU_MODE // Deprecated in FT810
		s_CmdIdMap["CMD_CSKETCH"] = CMD_CSKETCH & 0xFF;
		s_CmdParamCount[CMD_CSKETCH & 0xFF] = 7;
		s_CmdParamString[CMD_CSKETCH & 0xFF] = false;
#endif
		s_CmdIdMap["CMD_LOGO"] = CMD_LOGO & 0xFF;
		s_CmdParamCount[CMD_LOGO & 0xFF] = 0;
		s_CmdParamString[CMD_LOGO & 0xFF] = false;
		s_CmdIdMap["CMD_COLDSTART"] = CMD_COLDSTART & 0xFF;
		s_CmdParamCount[CMD_COLDSTART & 0xFF] = 0;
		s_CmdParamString[CMD_COLDSTART & 0xFF] = false;
		// s_CmdIdMap["CMD_GETMATRIX"] = CMD_GETMATRIX & 0xFF; // don't support reading values
		// s_CmdParamCount[CMD_GETMATRIX & 0xFF] = 6;
		// s_CmdParamString[CMD_GETMATRIX & 0xFF] = false;
		s_CmdIdMap["CMD_GRADCOLOR"] = CMD_GRADCOLOR & 0xFF;
		s_CmdParamCount[CMD_GRADCOLOR & 0xFF] = 3; // rgb
		s_CmdParamString[CMD_GRADCOLOR & 0xFF] = false;
#ifdef FT810EMU_MODE
		s_CmdIdMap["CMD_SETROTATE"] = CMD_SETROTATE & 0xFF;
		s_CmdParamCount[CMD_SETROTATE & 0xFF] = 1; // 0-7
		s_CmdParamString[CMD_SETROTATE & 0xFF] = false;
		s_CmdIdMap["CMD_SNAPSHOT2"] = CMD_SNAPSHOT2 & 0xFF;
		s_CmdParamCount[CMD_SNAPSHOT2 & 0xFF] = 6;
		s_CmdParamString[CMD_SNAPSHOT2 & 0xFF] = false;
		s_CmdIdMap["CMD_SETBASE"] = CMD_SETBASE & 0xFF;
		s_CmdParamCount[CMD_SETBASE & 0xFF] = 1;
		s_CmdParamString[CMD_SETBASE & 0xFF] = false;
		s_CmdIdMap["CMD_MEDIAFIFO"] = CMD_MEDIAFIFO & 0xFF;
		s_CmdParamCount[CMD_MEDIAFIFO & 0xFF] = 2;
		s_CmdParamString[CMD_MEDIAFIFO & 0xFF] = false;
		s_CmdIdMap["CMD_PLAYVIDEO"] = CMD_PLAYVIDEO & 0xFF;
		s_CmdParamCount[CMD_PLAYVIDEO & 0xFF] = 1;
		s_CmdParamString[CMD_PLAYVIDEO & 0xFF] = false;
		s_CmdIdMap["CMD_SETFONT2"] = CMD_SETFONT2 & 0xFF;
		s_CmdParamCount[CMD_SETFONT2 & 0xFF] = 3;
		s_CmdParamString[CMD_SETFONT2 & 0xFF] = false;
		s_CmdIdMap["CMD_SETSCRATCH"] = CMD_SETSCRATCH & 0xFF;
		s_CmdParamCount[CMD_SETSCRATCH & 0xFF] = 1;
		s_CmdParamString[CMD_SETSCRATCH & 0xFF] = false;
		/*s_CmdIdMap["CMD_INT_RAMSHARED"] = CMD_INT_RAMSHARED & 0xFF;
		s_CmdParamCount[CMD_INT_RAMSHARED & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_INT_RAMSHARED & 0xFF] = false;*/
		/*s_CmdIdMap["CMD_INT_SWLOADIMAGE"] = CMD_INT_SWLOADIMAGE & 0xFF;
		s_CmdParamCount[CMD_INT_SWLOADIMAGE & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_INT_SWLOADIMAGE & 0xFF] = false;*/
		s_CmdIdMap["CMD_ROMFONT"] = CMD_ROMFONT & 0xFF;
		s_CmdParamCount[CMD_ROMFONT & 0xFF] = 2;
		s_CmdParamString[CMD_ROMFONT & 0xFF] = false;
		s_CmdIdMap["CMD_VIDEOSTART"] = CMD_VIDEOSTART & 0xFF;
		s_CmdParamCount[CMD_VIDEOSTART & 0xFF] = 0;
		s_CmdParamString[CMD_VIDEOSTART & 0xFF] = false;
		s_CmdIdMap["CMD_VIDEOFRAME"] = CMD_VIDEOFRAME & 0xFF;
		s_CmdParamCount[CMD_VIDEOFRAME & 0xFF] = 2;
		s_CmdParamString[CMD_VIDEOFRAME & 0xFF] = false;
		/*s_CmdIdMap["CMD_SYNC"] = CMD_SYNC & 0xFF;
		s_CmdParamCount[CMD_SYNC & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_SYNC & 0xFF] = false;*/
		s_CmdIdMap["CMD_SETBITMAP"] = CMD_SETBITMAP & 0xFF;
		s_CmdParamCount[CMD_SETBITMAP & 0xFF] = 4;
		s_CmdParamString[CMD_SETBITMAP & 0xFF] = false;
#endif
		for (std::map<std::string, int>::iterator it = s_CmdIdMap.begin(), end = s_CmdIdMap.end(); it != end; ++it)
		{
			s_CmdIdList[it->second] = it->first;
		}
	}
	if (!s_CmdParamMap.size())
	{
		s_CmdParamMap["OPT_CENTER"] = OPT_CENTER;
		s_CmdParamMap["OPT_CENTERX"] = OPT_CENTERX;
		s_CmdParamMap["OPT_CENTERY"] = OPT_CENTERY;
		s_CmdParamMap["OPT_FLAT"] = OPT_FLAT;
		s_CmdParamMap["OPT_MONO"] = OPT_MONO;
		s_CmdParamMap["OPT_NOBACK"] = OPT_NOBACK;
		s_CmdParamMap["OPT_NODL"] = OPT_NODL;
		s_CmdParamMap["OPT_NOHANDS"] = OPT_NOHANDS;
		s_CmdParamMap["OPT_NOHM"] = OPT_NOHM;
		s_CmdParamMap["OPT_NOPOINTER"] = OPT_NOPOINTER;
		s_CmdParamMap["OPT_NOSECS"] = OPT_NOSECS;
		s_CmdParamMap["OPT_NOTICKS"] = OPT_NOTICKS;
		s_CmdParamMap["OPT_RIGHTX"] = OPT_RIGHTX;
		s_CmdParamMap["OPT_SIGNED"] = OPT_SIGNED;
#ifdef FT810EMU_MODE
		s_CmdParamMap["OPT_NOTEAR"] = OPT_NOTEAR;
		s_CmdParamMap["OPT_FULLSCREEN"] = OPT_FULLSCREEN;
		s_CmdParamMap["OPT_MEDIAFIFO"] = OPT_MEDIAFIFO;
		s_CmdParamMap["OPT_SOUND"] = OPT_SOUND;
#endif
	}
}

void DlParser::getIdentifiers(QStringList &list, bool coprocessor)
{
	init();

	for (std::map<std::string, int>::iterator it = s_IdMap.begin(), end = s_IdMap.end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
	}

	list.push_back("VERTEX2II");
	list.push_back("VERTEX2F");

	if (coprocessor)
	{
		for (std::map<std::string, int>::iterator it = s_CmdIdMap.begin(), end = s_CmdIdMap.end(); it != end; ++it)
		{
			list.push_back(QString(it->first.c_str()));
		}
	}
}

void DlParser::getParams(QStringList &list, bool coprocessor)
{
	init();

	for (std::map<std::string, int>::iterator it = s_ParamMap.begin(), end = s_ParamMap.end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
	}

	if (coprocessor)
	{
		for (std::map<std::string, int>::iterator it = s_CmdParamMap.begin(), end = s_CmdParamMap.end(); it != end; ++it)
		{
			list.push_back(QString(it->first.c_str()));
		}
	}
}

void DlParser::parse(DlParsed &parsed, const QString &line, bool coprocessor)
{
	init();

	QByteArray chars = line.toLatin1();
	const char *src = chars.constData();
	const int len = chars.size();

	parsed.BadCharacterIndex = -1;

	for (int p = 0; p < DLPARSED_MAX_PARAMETER; ++p)
	{
		parsed.ValidParameter[p] = false;
		parsed.NumericParameter[p] = true;
		parsed.ParameterLength[p] = 0;
	}

	parsed.ValidId = false;
	parsed.IdIndex = 0;
	parsed.IdLength = 0;
	std::stringstream idss;
	bool failId = false;

	int i = 0;

	// find function name
	for (; ; ++i)
	{
		if (i < len)
		{
			char c = src[i];
			if (c >= 'a' && c <= 'z')
			{
				c = c - 'a' + 'A'; // uppercase
			}
			if (parsed.IdLength == 0 && (c == ' ' || c == '\t'))
			{
				++parsed.IdIndex; /* pre-trim */
			}
			else if (((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')) && parsed.IdIndex + parsed.IdLength == i)
			{
				idss << c;
				++parsed.IdLength;
			}
			else if (c == ' ' || c == '\t')
			{
				/* post-trim */
			}
			else if (parsed.IdLength > 0 && c == '(')
			{
				/* valid, continue */
				++i;
				break;
			}
			else
			{
				parsed.BadCharacterIndex = i;
				failId = true; /* bad character after or inside name */
				break;
			}

		}
		else
		{
			failId = true; /* fail incomplete entry */
			break;
		}
	}

	parsed.IdText = idss.str();

	parsed.IdLeft = 0;
	parsed.IdRight = 0;
	parsed.ExpectedStringParameter = false;
	parsed.StringParameterAt = DLPARSED_MAX_PARAMETER;

	if (parsed.IdText == "VERTEX2F")
	{
		parsed.IdLeft = FT800EMU_DL_VERTEX2F;
		parsed.ValidId = true;
		parsed.ExpectedParameterCount = 2;
	}
	else if (parsed.IdText == "VERTEX2II")
	{
		parsed.IdLeft = FT800EMU_DL_VERTEX2II;
		parsed.ValidId = true;
		parsed.ExpectedParameterCount = 4;
	}
	else
	{
		std::map<std::string, int>::iterator it = s_IdMap.find(parsed.IdText);
		if (it != s_IdMap.end())
		{
			parsed.IdRight = it->second;
			parsed.ValidId = true;
			parsed.ExpectedParameterCount = s_ParamCount[parsed.IdRight];
		}
		if (coprocessor)
		{
			it = s_CmdIdMap.find(parsed.IdText);
			if (it != s_CmdIdMap.end())
			{
				parsed.IdLeft = 0xFFFFFF00;
				parsed.IdRight = it->second;
				parsed.ValidId = true;
				parsed.ExpectedParameterCount = s_CmdParamCount[parsed.IdRight];
				parsed.ExpectedStringParameter = s_CmdParamString[parsed.IdRight];
			}
		}
	}

	int p = 0;
	if (!failId)
	{
		// for each possible parameter
		bool failParam = false;
		int finalIndex = -1;
		int pq;
		for (pq = 0; p < DLPARSED_MAX_PARAMETER && pq < DLPARSED_MAX_PARAMETER; ++p, ++pq)
		{
			bool combineParameter = false; // temporary method for using | operator // CMD_CLOCK(100, 100, 50, OPT_FLAT | OPT_NOTICKS, 0, 0, 0, 0), pq is a TEMPORARY trick that shifts the actual parameters from the metadata
		CombineParameter:
			bool hexadecimal = false;
			bool combinedParameter = combineParameter;
			combineParameter = false;
			parsed.ParameterIndex[pq] = i;
			std::stringstream pss;
		ContinueParameter:
			for (; ; ++i)
			{
				if (i < len)
				{
					char c = src[i];
					if (c >= 'a' && c <= 'z')
					{
						c = c - 'a' + 'A'; // uppercase
					}
					if (parsed.ParameterLength[pq] == 0 && (c == ' ' || c == '\t'))
					{
						++parsed.ParameterIndex[pq]; /* pre-trim */
					}
					else if (parsed.ParameterLength[pq] == 0 && (c == '"') && p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)
					{
						/* begin string, only works on last parameter */ // CMD_TEXT(50, 119, 31, 0, "hello world")
						pss << c;
						++i;
						goto ParseString;
					}
					else if (parsed.ParameterLength[pq] == 0 && (c == '\'') && (!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)))
					{
						pss << c;
						++i;
						goto ParseChar;
					}
					else if (parsed.ParameterLength[pq] == 0 && (c == '-'))
					{
						pss << c;
						++parsed.ParameterLength[pq];
					}
					else if (((c >= '0' && c <= '9') || (hexadecimal && (c >= 'A' && c <= 'F'))) && parsed.ParameterIndex[pq] + parsed.ParameterLength[pq] == i && (!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)))
					{
						pss << c;
						++parsed.ParameterLength[pq];
					}
					else if (parsed.ParameterLength[pq] == 1 && src[i - 1] == '0' && (c == 'X') && (!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)))
					{
						pss.clear();
						hexadecimal = true;
						++parsed.ParameterLength[pq];
						pss << std::hex;
					}
					else if (((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')) && parsed.ParameterIndex[pq] + parsed.ParameterLength[pq] == i  && (!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)))
					{
						parsed.NumericParameter[pq] = false;
						pss << c;
						++parsed.ParameterLength[pq];
					}
					else if (c == ' ' || c == '\t')
					{
						/* post-trim */
					}
					else if (parsed.ParameterLength[pq] > 0 && c == ',')
					{
						/* valid, more, continue */
						++i;
						break;
					}
					else if (parsed.ParameterLength[pq] > 0 && c == '|')
					{
						/* valid, more, continue */
						++i;
						combineParameter = true;
						break;
					}
					else if (((p == 0) || (parsed.ParameterLength[pq] > 0)) && c == ')')
					{
						/* valid, last, continue */
						finalIndex = i;
						++i;
						break;
					}
					else
					{
						parsed.BadCharacterIndex = i;
						failParam = true; /* bad character after or inside name */
						break;
					}
				}
				else
				{
					failParam = true; /* fail incomplete entry */
					break;
				}
			}

			goto ValidateNamed;
		ParseString:
			for (; ; ++i)
			{
				if (i < len)
				{
					char c = src[i];
					if (c == '\\')
					{
						// unescape
						pss << '\\';
						++i;
						if (i < len)
						{
							c = src[i];
							pss << c;
						}
						else
						{
							pss << '\\';
						}
					}
					else if (c == '"')
					{
						// end (post-trim)
						pss << c;
						++i;
						parsed.ParameterLength[pq] = i - parsed.ParameterIndex[pq];
						goto ContinueParameter;
					}
					else
					{
						pss << c;
					}
				}
				else
				{
					failParam = true; /* fail incomplete entry */
					break;
				}
			}

			goto ValidateNamed;
		ParseChar:
			for (; ; ++i)
			{
				if (i < len)
				{
					char c = src[i];
					pss << c;
					if (c == '\\')
					{
						// skip
						++i;
						pss << src[i];
					}
					else if (c == '\'')
					{
						// end (post-trim)
						++i;
						parsed.ParameterLength[pq] = i - parsed.ParameterIndex[pq];
						goto ContinueParameter;
					}
				}
				else
				{
					failParam = true; /* fail incomplete entry */
					break;
				}
			}

		ValidateNamed:

			// validate named parameter
			if (p < parsed.ExpectedParameterCount || !parsed.ValidId)
			{
				std::string ps = pss.str();
				if (p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)
				{
					// CMD_TEXT(50, 119, 31, 0, "hello world")
					if (ps.length())
					{
						std::string psubstr = ps.substr(1, ps.size() - 2);
						std::string psstr;
						unescapeString(psstr, psubstr);
						parsed.Parameter[p].U = 0;
						parsed.StringParameter = psstr;
						parsed.ValidParameter[pq] = true;
						parsed.StringParameterAt = pq;
					}
				}
				else if (ps.length() > 0 && ps[0] == '\'')
				{
					int vchar = 0;
					for (int ci = (int)ps.length() - 2; ci > 0; --ci)
					{
						vchar <<= 8;
						vchar |= ps[ci];
					}
					parsed.Parameter[p].I = (combinedParameter ? parsed.Parameter[p].I : 0) | vchar;
					parsed.ValidParameter[pq] = true;
				}
				else if (hexadecimal && parsed.NumericParameter[pq] && ps.length() > 0)
				{
					int vhex;
					pss >> vhex;
					parsed.Parameter[p].I = (combinedParameter ? parsed.Parameter[p].I : 0) | vhex;
					parsed.ValidParameter[pq] = true;
				}
				else if (parsed.NumericParameter[pq] && ps.length() > 0)
				{
					parsed.Parameter[p].I = (combinedParameter ? parsed.Parameter[p].I : 0) | atoi(ps.c_str());
					parsed.ValidParameter[pq] = true;
				}
				else
				{
					std::map<std::string, int>::iterator it = s_ParamMap.find(ps);
					if (it != s_ParamMap.end())
					{
						parsed.Parameter[p].U = (combinedParameter ? parsed.Parameter[p].U : 0) | it->second;
						parsed.ValidParameter[pq] = true;
					}
					else if (coprocessor)
					{
						it = s_CmdParamMap.find(ps);
						if (it != s_CmdParamMap.end())
						{
							parsed.Parameter[p].U = (combinedParameter ? parsed.Parameter[p].U : 0) | it->second;
							parsed.ValidParameter[pq] = true;
						}
						else
						{
							parsed.Parameter[p].U = 0;
						}
					}
					else
					{
						parsed.Parameter[p].U = 0;
					}
				}
			}
			else
			{
				parsed.Parameter[p].U = 0;
			}

			if (finalIndex >= 0)
			{
				if (parsed.BadCharacterIndex == -1)
				{
					if (p + 1 < parsed.ExpectedParameterCount)
					{
						// not enough params
						parsed.BadCharacterIndex = finalIndex;
					}
				}
				break;
			}

			if (failParam)
			{
				break;
			}

			if (combineParameter)
			{
				// parsed.ParameterLength[p] = 0;
				++pq;
				goto CombineParameter;
			}
		}
	}

	// Clear unfilled to be safe
	if (parsed.ValidId)
	{
		if (!failId) ++p;
		for (; p < parsed.ExpectedParameterCount && p < DLPARSED_MAX_PARAMETER; ++p)
		{
			parsed.Parameter[p].U = 0;
		}
	}
}

uint32_t DlParser::compile(const DlParsed &parsed)
{
	const uint32_t *p = static_cast<const uint32_t *>(static_cast<const void *>(parsed.Parameter));
	if (parsed.IdLeft == FT800EMU_DL_VERTEX2F)
	{
		return VERTEX2F(p[0], p[1]);
	}
	else if (parsed.IdLeft == FT800EMU_DL_VERTEX2II)
	{
		return VERTEX2II(p[0], p[1], p[2], p[3]);
	}
	else if (parsed.IdLeft == 0xFFFFFF00) // Coprocessor
	{
		return 0xFFFFFF00 | (parsed.IdRight);
	}
	else switch (parsed.IdRight)
	{
		case FT800EMU_DL_DISPLAY:
			return DISPLAY();
		case FT800EMU_DL_BITMAP_SOURCE:
			return BITMAP_SOURCE(p[0]);
		case FT800EMU_DL_CLEAR_COLOR_RGB:
			return CLEAR_COLOR_RGB(p[0], p[1], p[2]);
		case FT800EMU_DL_TAG:
			return TAG(p[0]);
		case FT800EMU_DL_COLOR_RGB:
			return COLOR_RGB(p[0], p[1], p[2]);
		case FT800EMU_DL_BITMAP_HANDLE:
			return BITMAP_HANDLE(p[0]);
		case FT800EMU_DL_CELL:
			return CELL(p[0]);
		case FT800EMU_DL_BITMAP_LAYOUT:
			return BITMAP_LAYOUT(p[0], p[1], p[2]);
		case FT800EMU_DL_BITMAP_SIZE:
			return BITMAP_SIZE(p[0], p[1], p[2], p[3], p[4]);
		case FT800EMU_DL_ALPHA_FUNC:
			return ALPHA_FUNC(p[0], p[1]);
		case FT800EMU_DL_STENCIL_FUNC:
			return STENCIL_FUNC(p[0], p[1], p[2]);
		case FT800EMU_DL_BLEND_FUNC:
			return BLEND_FUNC(p[0], p[1]);
		case FT800EMU_DL_STENCIL_OP:
			return STENCIL_OP(p[0], p[1]);
		case FT800EMU_DL_POINT_SIZE:
			return POINT_SIZE(p[0]);
		case FT800EMU_DL_LINE_WIDTH:
			return LINE_WIDTH(p[0]);
		case FT800EMU_DL_CLEAR_COLOR_A:
			return CLEAR_COLOR_A(p[0]);
		case FT800EMU_DL_COLOR_A:
			return COLOR_A(p[0]);
		case FT800EMU_DL_CLEAR_STENCIL:
			return CLEAR_STENCIL(p[0]);
		case FT800EMU_DL_CLEAR_TAG:
			return CLEAR_TAG(p[0]);
		case FT800EMU_DL_STENCIL_MASK:
			return STENCIL_MASK(p[0]);
		case FT800EMU_DL_TAG_MASK:
			return TAG_MASK(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_A:
			return BITMAP_TRANSFORM_A(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_B:
			return BITMAP_TRANSFORM_B(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_C:
			return BITMAP_TRANSFORM_C(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_D:
			return BITMAP_TRANSFORM_D(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_E:
			return BITMAP_TRANSFORM_E(p[0]);
		case FT800EMU_DL_BITMAP_TRANSFORM_F:
			return BITMAP_TRANSFORM_F(p[0]);
		case FT800EMU_DL_SCISSOR_XY:
			return SCISSOR_XY(p[0], p[1]);
		case FT800EMU_DL_SCISSOR_SIZE:
			return SCISSOR_SIZE(p[0], p[1]);
		case FT800EMU_DL_CALL:
			return CALL(p[0]);
		case FT800EMU_DL_JUMP:
			return JUMP(p[0]);
		case FT800EMU_DL_BEGIN:
			return BEGIN(p[0]);
		case FT800EMU_DL_COLOR_MASK:
			return COLOR_MASK(p[0], p[1], p[2], p[3]);
		case FT800EMU_DL_END:
			return END();
		case FT800EMU_DL_SAVE_CONTEXT:
			return SAVE_CONTEXT();
		case FT800EMU_DL_RESTORE_CONTEXT:
			return RESTORE_CONTEXT();
		case FT800EMU_DL_RETURN:
			return RETURN();
		case FT800EMU_DL_MACRO:
			return MACRO(p[0]);
		case FT800EMU_DL_CLEAR:
			return CLEAR(p[0], p[1], p[2]);
#ifdef FT810EMU_MODE
		case FT800EMU_DL_VERTEX_FORMAT:
			return VERTEX_FORMAT(p[0]);
		case FT800EMU_DL_BITMAP_LAYOUT_H:
			return BITMAP_LAYOUT_H(p[0], p[1]);
		case FT800EMU_DL_BITMAP_SIZE_H:
			return BITMAP_SIZE_H(p[0], p[1]);
		case FT800EMU_DL_PALETTE_SOURCE:
			return PALETTE_SOURCE(p[0]);
		case FT800EMU_DL_VERTEX_TRANSLATE_X:
			return VERTEX_TRANSLATE_X(p[0]);
		case FT800EMU_DL_VERTEX_TRANSLATE_Y:
			return VERTEX_TRANSLATE_Y(p[0]);
		case FT800EMU_DL_NOP:
			return NOP();
#endif
	}
	return DISPLAY();
}

void DlParser::compile(std::vector<uint32_t> &compiled, const DlParsed &parsed) // compile CMD parameters
{
	if (parsed.ValidId)
	{
		if (parsed.IdLeft == 0xFFFFFF00)
		{
			switch (0xFFFFFF00 | parsed.IdRight)
			{
				case CMD_BGCOLOR:
				case CMD_FGCOLOR:
				case CMD_GRADCOLOR:
				{
					uint32_t rgb =
						parsed.Parameter[0].U << 16
						| parsed.Parameter[1].U << 8
						| parsed.Parameter[2].U;
					compiled.push_back(rgb);
					break;
				}
				case CMD_GRADIENT:
				{
					uint32_t xy0 = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy0);
					uint32_t rgba0 = parsed.Parameter[2].U << 16
						| parsed.Parameter[3].U << 8
						| parsed.Parameter[4].U;
					compiled.push_back(rgba0);
					uint32_t xy1 = parsed.Parameter[6].U << 16
						| parsed.Parameter[5].U;
					compiled.push_back(xy1);
					uint32_t rgba1 = parsed.Parameter[7].U << 16
						| parsed.Parameter[8].U << 8
						| parsed.Parameter[9].U;
					compiled.push_back(rgba1);
					break;
				}
				case CMD_TEXT:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t fo = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(fo);
					for (int i = 0; i < parsed.StringParameter.size() + 1; i += 4)
					{
						// CMD_TEXT(50, 119, 31, 0, "hello world")
						uint32_t c0 = ((i) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i] : 0;
						uint32_t c1 = ((i + 1) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 1] : 0;
						uint32_t c2 = ((i + 2) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 2] : 0;
						uint32_t c3 = ((i + 3) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 3] : 0;
						uint32_t s = c0 | c1 << 8 | c2 << 16 | c3 << 24;
						compiled.push_back(s);
					}
					break;
				}
				case CMD_BUTTON:
				case CMD_KEYS:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					uint32_t fo = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(fo);
					for (int i = 0; i < parsed.StringParameter.size() + 1; i += 4)
					{
						uint32_t c0 = ((i) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i] : 0;
						uint32_t c1 = ((i + 1) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 1] : 0;
						uint32_t c2 = ((i + 2) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 2] : 0;
						uint32_t c3 = ((i + 3) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 3] : 0;
						uint32_t s = c0 | c1 << 8 | c2 << 16 | c3 << 24;
						compiled.push_back(s);
					}
					break;
				}
				case CMD_PROGRESS:
				case CMD_SLIDER:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					uint32_t ov = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(ov);
					uint32_t r = parsed.Parameter[6].U & 0xFFFF;
					compiled.push_back(r);
					break;
				}
				case CMD_SCROLLBAR:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					uint32_t ov = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(ov);
					uint32_t sr = parsed.Parameter[7].U << 16
						| parsed.Parameter[6].U & 0xFFFF;
					compiled.push_back(sr);
					break;
				}
				case CMD_TOGGLE:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wf = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wf);
					uint32_t os = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(os);
					for (int i = 0; i < parsed.StringParameter.size() + 1; i += 4)
					{
						uint32_t c0 = ((i) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i] : 0;
						uint32_t c1 = ((i + 1) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 1] : 0;
						uint32_t c2 = ((i + 2) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 2] : 0;
						uint32_t c3 = ((i + 3) < parsed.StringParameter.size()) ? (unsigned char)parsed.StringParameter[i + 3] : 0;
						uint32_t s = c0 | c1 << 8 | c2 << 16 | c3 << 24;
						compiled.push_back(s);
					}
					break;
				}
				case CMD_GAUGE:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t ro = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(ro);
					uint32_t mm = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(mm);
					uint32_t vr = parsed.Parameter[7].U << 16
						| parsed.Parameter[6].U & 0xFFFF;
					compiled.push_back(vr);
					break;
				}
				case CMD_CLOCK:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t ro = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(ro);
					uint32_t hm = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(hm);
					uint32_t sm = parsed.Parameter[7].U << 16
						| parsed.Parameter[6].U & 0xFFFF;
					compiled.push_back(sm);
					break;
				}
				case CMD_SPINNER:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t ss = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(ss);
					break;
				}
				case CMD_DLSTART:
				case CMD_SWAP:
				case CMD_STOP:
				case CMD_LOADIDENTITY:
				case CMD_SETMATRIX:
				case CMD_SCREENSAVER:
				case CMD_LOGO:
				case CMD_COLDSTART:
#ifdef FT810EMU_MODE
				case CMD_VIDEOSTART:
#endif
				{
					break;
				}
				case CMD_SNAPSHOT:
				case CMD_INFLATE:
				case CMD_ROTATE:
				case CMD_INTERRUPT:
				case CMD_CALIBRATE:
#ifdef FT810EMU_MODE
				case CMD_SETROTATE:
				case CMD_PLAYVIDEO:
				case CMD_SETSCRATCH:
#endif
				{
					compiled.push_back(parsed.Parameter[0].U);
					break;
				}
				case CMD_MEMWRITE:
				case CMD_APPEND:
				case CMD_LOADIMAGE:
				case CMD_TRANSLATE:
				case CMD_SCALE:
				case CMD_SETFONT:
#ifdef FT810EMU_MODE
				case CMD_MEDIAFIFO:
				case CMD_ROMFONT:
				case CMD_VIDEOFRAME:
#endif
				{
					compiled.push_back(parsed.Parameter[0].U);
					compiled.push_back(parsed.Parameter[1].U);
					break;
				}
				case CMD_MEMZERO:
				{
					uint32_t ptr = parsed.Parameter[0].U;
					uint32_t num = parsed.Parameter[1].U;
					if (ptr >= FT800EMU_RAM_SIZE)
						ptr = FT800EMU_RAM_SIZE - 1;
					if (ptr + num >= FT800EMU_RAM_SIZE)
						num = FT800EMU_RAM_SIZE - ptr - 1;
					compiled.push_back(ptr);
					compiled.push_back(num);
					break;
				}
				case CMD_MEMSET:
				{
					uint32_t ptr = parsed.Parameter[0].U;
					uint32_t num = parsed.Parameter[2].U;
					if (ptr >= FT800EMU_RAM_SIZE)
						ptr = FT800EMU_RAM_SIZE - 1;
					if (ptr + num >= FT800EMU_RAM_SIZE)
						num = FT800EMU_RAM_SIZE - ptr - 1;
					compiled.push_back(ptr);
					compiled.push_back(parsed.Parameter[1].U);
					compiled.push_back(num);
					break;
				}
				case CMD_MEMCPY:
				{
					uint32_t dst = parsed.Parameter[0].U;
					uint32_t src = parsed.Parameter[1].U;
					uint32_t num = parsed.Parameter[2].U;
					if (dst >= FT800EMU_RAM_SIZE)
						dst = FT800EMU_RAM_SIZE - 1;
					if (src >= FT800EMU_RAM_SIZE)
						src = FT800EMU_RAM_SIZE - 1;
					if (dst + num >= FT800EMU_RAM_SIZE)
						num = FT800EMU_RAM_SIZE - dst - 1;
					if (src + num >= FT800EMU_RAM_SIZE)
						num = FT800EMU_RAM_SIZE - src - 1;
					compiled.push_back(dst);
					compiled.push_back(src);
					compiled.push_back(num);
					break;
				}
				case CMD_TRACK:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					uint32_t t = parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(t);
					break;
				}
				case CMD_DIAL:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t ro = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(ro);
					uint32_t v = parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(v);
					break;
				}
				case CMD_NUMBER:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t fo = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(fo);
					compiled.push_back(parsed.Parameter[4].U);
					break;
				}
				case CMD_SKETCH:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					compiled.push_back(parsed.Parameter[4].U);
					uint32_t f = parsed.Parameter[5].U & 0xFFFF;
					compiled.push_back(f);
					break;
				}
#ifndef FT810EMU_MODE // This is deprecated in FT810
				case CMD_CSKETCH:
				{
					uint32_t xy = parsed.Parameter[1].U << 16
						| parsed.Parameter[0].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(wh);
					compiled.push_back(parsed.Parameter[4].U);
					uint32_t f = (parsed.Parameter[5].U & 0xFFFF)
						| (parsed.Parameter[6].U << 16);
					compiled.push_back(f);
					break;
				}
#endif
#ifdef FT810EMU_MODE
				case CMD_SETBASE:
				{
					uint32_t b = parsed.Parameter[0].U;
					if (b < 2) b = 10; // Otherwise freezes CP
					compiled.push_back(b);
					break;
				}
				case CMD_SETFONT2:
				{
					compiled.push_back(parsed.Parameter[0].U);
					compiled.push_back(parsed.Parameter[1].U);
					compiled.push_back(parsed.Parameter[2].U);
					break;
				}
				case CMD_SNAPSHOT2:
				{
					uint32_t fmt = parsed.Parameter[0].U;
					compiled.push_back(fmt);
					uint32_t ptr = parsed.Parameter[1].U;
					compiled.push_back(ptr);
					uint32_t xy = parsed.Parameter[3].U << 16
						| parsed.Parameter[2].U & 0xFFFF;
					compiled.push_back(xy);
					uint32_t wh = parsed.Parameter[5].U << 16
						| parsed.Parameter[4].U & 0xFFFF;
					compiled.push_back(wh);
					break;
				}
				case CMD_SETBITMAP:
				{
					compiled.push_back(parsed.Parameter[0].U);
					uint32_t fmtw = parsed.Parameter[1].U & 0xFFFF
						| parsed.Parameter[2].U << 16;
					compiled.push_back(fmtw);
					uint32_t h = parsed.Parameter[3].U;
					compiled.push_back(h);
				}
#endif
			}
		}
	}
}

static void primToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_PRIMITIVE_NB && (id >= 1))
	{
		dst << g_DlEnumPrimitive[id];
	}
	else
	{
		dst << id;
	}
}

static void bitmapFormatToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_BITMAP_FORMAT_NB)
	{
		dst << g_DlEnumBitmapFormat[id];
	}
	else
	{
		dst << id;
	}
}

static void bitmapWrapToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_BITMAP_WRAP_NB)
	{
		dst << g_DlEnumBitmapWrap[id];
	}
	else
	{
		dst << id;
	}
}

static void bitmapFilterToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_BITMAP_FILTER_NB)
	{
		dst << g_DlEnumBitmapFilter[id];
	}
	else
	{
		dst << id;
	}
}

static void blendToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_BLEND_NB)
	{
		dst << g_DlEnumBlend[id];
	}
	else
	{
		dst << id;
	}
}

static void compareToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_COMPARE_NB)
	{
		dst << g_DlEnumCompare[id];
	}
	else
	{
		dst << id;
	}
}

static void stencilToString(std::stringstream &dst, uint32_t id)
{
	if ((uint32_t)id < DL_ENUM_STENCIL_NB)
	{
		dst << g_DlEnumStencil[id];
	}
	else
	{
		dst << id;
	}
}

void DlParser::toString(std::string &dst, uint32_t v)
{
	std::stringstream res;

	switch (v >> 30)
	{
		case 0:
		{
			switch (v >> 24)
			{
				case FT800EMU_DL_DISPLAY:
				{
					res << "DISPLAY()";
					break;
				}
				case FT800EMU_DL_BITMAP_SOURCE:
				{
					int addr = v & FT800EMU_ADDR_MASK;
					res << "BITMAP_SOURCE(";
					res << addr << ")";
					break;
				}
				case FT800EMU_DL_CLEAR_COLOR_RGB:
				{
					int red = (v >> 16) & 0xFF;
					int green = (v >> 8) & 0xFF;
					int blue = v & 0xFF;
					res << "CLEAR_COLOR_RGB(";
					res << red << ", " << green << ", " << blue << ")";
					break;
				}
				case FT800EMU_DL_TAG:
				{
					int s = v & 0xFF;
					res << "TAG(";
					res << s << ")";
					break;
				}
				case FT800EMU_DL_COLOR_RGB:
				{
					int red = (v >> 16) & 0xFF;
					int green = (v >> 8) & 0xFF;
					int blue = v & 0xFF;
					res << "COLOR_RGB(";
					res << red << ", " << green << ", " << blue << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_HANDLE:
				{
					int handle = v & 0x1F;
					res << "BITMAP_HANDLE(";
					res << handle << ")";
					break;
				}
				case FT800EMU_DL_CELL:
				{
					int cell = v & 0x7F;
					res << "CELL(";
					res << cell << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_LAYOUT:
				{
					int format = (v >> 19) & 0x1F;
					int linestride = (v >> 9) & 0x3FF;
					int height = v & 0x1FF;
					res << "BITMAP_LAYOUT(";
					bitmapFormatToString(res, format);
					res << ", " << linestride << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_SIZE:
				{
					int filter = (v >> 20) & 0x1;
					int wrapx = (v >> 19) & 0x1;
					int wrapy = (v >> 18) & 0x1;
					int width = (v >> 9) & 0x1FF;
					int height = v & 0x1FF;
					res << "BITMAP_SIZE(";
					bitmapFilterToString(res, filter);
					res << ", ";
					bitmapWrapToString(res, wrapx);
					res << ", ";
					bitmapWrapToString(res, wrapy);
					res << ", ";
					res << width << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_ALPHA_FUNC:
				{
					int func = (v >> 8) & 0x07;
					int ref = v & 0xFF;
					res << "ALPHA_FUNC(";
					compareToString(res, func);
					res << ", " << ref << ")";
					break;
				}
				case FT800EMU_DL_STENCIL_FUNC:
				{
					int func = (v >> 16) & 0x7;
					int ref = (v >> 8) & 0xFF;
					int mask = v & 0xFF;
					res << "STENCIL_FUNC(";
					compareToString(res, func);
					res << ", " << ref <<  ", " << mask << ")";
					break;
				}
				case FT800EMU_DL_BLEND_FUNC:
				{
					int src = (v >> 3) & 0x7;
					int dst = v & 0x7;
					res << "BLEND_FUNC(";
					blendToString(res, src);
					res << ", ";
					blendToString(res, dst);
					res << ")";
					break;
				}
				case FT800EMU_DL_STENCIL_OP:
				{
					int sfail = (v >> 3) & 0x7;
					int spass = v & 0x7;
					res << "STENCIL_OP(";
					stencilToString(res, sfail);
					res << ", ";
					stencilToString(res, spass);
					res << ")";
					break;
				}
				case FT800EMU_DL_POINT_SIZE:
				{
					int size = v & 0x1FFF;
					res << "POINT_SIZE(";
					res << size << ")";
					break;
				}
				case FT800EMU_DL_LINE_WIDTH:
				{
					int width = v & 0xFFF;
					res << "LINE_WIDTH(";
					res << width << ")";
					break;
				}
				case FT800EMU_DL_CLEAR_COLOR_A:
				{
					int alpha = v & 0xFF;
					res << "CLEAR_COLOR_A(";
					res << alpha << ")";
					break;
				}
				case FT800EMU_DL_COLOR_A:
				{
					int alpha = v & 0xFF;
					res << "COLOR_A(";
					res << alpha << ")";
					break;
				}
				case FT800EMU_DL_CLEAR_STENCIL:
				{
					int s = v & 0xFF;
					res << "CLEAR_STENCIL(";
					res << s << ")";
					break;
				}
				case FT800EMU_DL_CLEAR_TAG:
				{
					int s = v & 0xFF;
					res << "CLEAR_TAG(";
					res << s << ")";
					break;
				}
				case FT800EMU_DL_STENCIL_MASK:
				{
					int mask = v & 0xFF;
					res << "STENCIL_MASK(";
					res << mask << ")";
					break;
				}
				case FT800EMU_DL_TAG_MASK:
				{
					int mask = v & 0x01;
					res << "TAG_MASK(";
					res << mask << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_A:
				{
					int a = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_A(";
					res << a << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_B:
				{
					int b = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_B(";
					res << b << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_C:
				{
					int c = v & 0xFFFFFF;
					res << "BITMAP_TRANSFORM_C(";
					res << c << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_D:
				{
					int d = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_D(";
					res << d << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_E:
				{
					int e = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_E(";
					res << e << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_F:
				{
					int f = v & 0xFFFFFF;
					res << "BITMAP_TRANSFORM_F(";
					res << f << ")";
					break;
				}
				case FT800EMU_DL_SCISSOR_XY:
				{
#ifdef FT810EMU_MODE
					int x = (v >> 11) & 0x7FF;
					int y = v & 0x7FF;
#else
					int x = (v >> 9) & 0x1FF;
					int y = v & 0x1FF;
#endif
					res << "SCISSOR_XY(";
					res << x << ", " << y << ")";
					break;
				}
				case FT800EMU_DL_SCISSOR_SIZE:
				{
#ifdef FT810EMU_MODE
					int width = (v >> 12) & 0xFFF;
					int height = v & 0xFFF;
#else
					int width = (v >> 10) & 0x3FF;
					int height = v & 0x3FF;
#endif
					res << "SCISSOR_SIZE(";
					res << width << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_CALL:
				{
					int dest = v & 0xFFFF;
					res << "CALL(";
					res << dest << ")";
					break;
				}
				case FT800EMU_DL_JUMP:
				{
					int dest = v & 0xFFFF;
					res << "JUMP(";
					res << dest << ")";
					break;
				}
				case FT800EMU_DL_BEGIN:
				{
					int primitive = v & 0x0F;
					res << "BEGIN(";
					primToString(res, primitive);
					res << ")";
					break;
				}
				case FT800EMU_DL_COLOR_MASK:
				{
					int r = (v >> 3) & 0x1;
					int g = (v >> 2) & 0x1;
					int b = (v >> 1) & 0x1;
					int a = v & 0x1;
					res << "COLOR_MASK(";
					res << r << ", " << g << ", " << b << ", " << a << ")";
					break;
				}
				case FT800EMU_DL_END:
				{
					res << "END()";
					break;
				}
				case FT800EMU_DL_SAVE_CONTEXT:
				{
					res << "SAVE_CONTEXT()";
					break;
				}
				case FT800EMU_DL_RESTORE_CONTEXT:
				{
					res << "RESTORE_CONTEXT()";
					break;
				}
				case FT800EMU_DL_RETURN:
				{
					res << "RETURN()";
					break;
				}
				case FT800EMU_DL_MACRO:
				{
					int m = v & 0x01;
					res << "MACRO(";
					res << m << ")";
					break;
				}
				case FT800EMU_DL_CLEAR:
				{
					int c = (v >> 2) & 0x1;
					int s = (v >> 1) & 0x1;
					int t = v & 0x1;
					res << "CLEAR(";
					res << c << ", " << s << ", " << t << ")";
					break;
				}
#ifdef FT810EMU_MODE
				case FT800EMU_DL_VERTEX_FORMAT:
				{
					int frac = v & 0x7;
					res << "VERTEX_FORMAT(";
					res << frac << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_LAYOUT_H:
				{
					int linestride = (v >> 2) & 0x3;
					int height = v & 0x3;
					res << "BITMAP_LAYOUT_H(";
					res << linestride << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_BITMAP_SIZE_H:
				{
					int width = (v >> 2) & 0x3;
					int height = v & 0x3;
					res << "BITMAP_SIZE_H(";
					res << width << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_PALETTE_SOURCE:
				{
					int addr = v & FT800EMU_ADDR_MASK;
					res << "PALETTE_SOURCE(";
					res << addr << ")";
					break;
				}
				case FT800EMU_DL_VERTEX_TRANSLATE_X:
				{
					int x = SIGNED_N(v & 0x1FFFF, 17);
					res << "VERTEX_TRANSLATE_X(";
					res << x << ")";
					break;
				}
				case FT800EMU_DL_VERTEX_TRANSLATE_Y:
				{
					int y = SIGNED_N(v & 0x1FFFF, 17);
					res << "VERTEX_TRANSLATE_Y(";
					res << y << ")";
					break;
				}
				case FT800EMU_DL_NOP:
				{
					res << "NOP()";
					break;
				}
#endif
			}
			break;
		}
		case FT800EMU_DL_VERTEX2II:
		{
			int px = ((v >> 21) & 0x1FF);
			int py = ((v >> 12) & 0x1FF);
			int handle = ((v >> 7) & 0x1F);
			int cell = v & 0x7F;
			res << "VERTEX2II(";
			res << px << ", " << py << ", ";
			res << handle << ", ";
			if (cell >= 32 && cell <= 126) res << "'" << (char)cell << "'";
			else res << cell;
			res << ")";
			break;
		}
		case FT800EMU_DL_VERTEX2F:
		{
			// int px = (v >> 15) & 0x3FFF;
			// if ((v >> 15) & 0x4000) px = px - 0x4000;
			// int py = (v) & 0x3FFF;
			// if ((v) & 0x4000) py = py - 0x4000;
			int px = (v >> 15) & 0x7FFF;
			int py = (v) & 0x7FFF;
			res << "VERTEX2F(";
			res << px << ", " << py << ")";
			break;
		}
	}

	dst = res.str();
}

QString DlParser::toString(uint32_t v)
{
	std::string str;
	toString(str, v);
	return QString(str.c_str());
}

void optToString(std::stringstream &dst, uint32_t opt, uint32_t cmd)
{
	if (opt == 0)
	{
		dst << "0";
		return;
	}
	bool combine = false;
	if (cmd != CMD_KEYS)
	{
		if (opt & OPT_MONO)
		{
			dst << "OPT_MONO";
			combine = true;
		}
		if (opt & OPT_NODL)
		{
			if (combine) dst << " | ";
			dst << "OPT_NODL";
			combine = true;
		}
	}
#ifdef FT810EMU_MODE
	if (opt & OPT_NOTEAR)
	{
		if (combine) dst << " | ";
		dst << "OPT_NOTEAR";
		combine = true;
	}
	if (opt & OPT_FULLSCREEN)
	{
		if (combine) dst << " | ";
		dst << "OPT_FULLSCREEN";
		combine = true;
	}
	if (opt & OPT_MEDIAFIFO)
	{
		if (combine) dst << " | ";
		dst << "OPT_MEDIAFIFO";
		combine = true;
	}
	if (opt & OPT_SOUND)
	{
		if (combine) dst << " | ";
		dst << "OPT_SOUND";
		combine = true;
	}
#endif
	if (cmd == CMD_NUMBER)
	{
		if (opt & OPT_SIGNED)
		{
			if (combine) dst << " | ";
			dst << "OPT_SIGNED";
			combine = true;
		}
	}
	else
	{
		if (opt & OPT_FLAT)
		{
			if (combine) dst << " | ";
			dst << "OPT_FLAT";
			combine = true;
		}
	}
	if ((opt & OPT_CENTER) == OPT_CENTER)
	{
		if (combine) dst << " | ";
		dst << "OPT_CENTER";
		combine = true;
	}
	else if (opt & OPT_CENTERX)
	{
		if (combine) dst << " | ";
		dst << "OPT_CENTERX";
		combine = true;
	}
	else if (opt & OPT_CENTERY)
	{
		if (combine) dst << " | ";
		dst << "OPT_CENTERY";
		combine = true;
	}
	if (opt & OPT_RIGHTX)
	{
		if (combine) dst << " | ";
		dst << "OPT_RIGHTX";
		combine = true;
	}
	if (opt & OPT_NOBACK)
	{
		if (combine) dst << " | ";
		dst << "OPT_NOBACK";
		combine = true;
	}
	if (opt & OPT_NOTICKS)
	{
		if (combine) dst << " | ";
		dst << "OPT_NOTICKS";
		combine = true;
	}
	if (cmd == CMD_GAUGE)
	{
		if (opt & OPT_NOPOINTER)
		{
			if (combine) dst << " | ";
			dst << "OPT_NOPOINTER";
			combine = true;
		}
		if (opt & OPT_NOSECS)
		{
			if (combine) dst << " | ";
			dst << "OPT_NOSECS";
			combine = true;
		}
	}
	else
	{
		if ((opt & OPT_NOHANDS) == OPT_NOHANDS)
		{
			if (combine) dst << " | ";
			dst << "OPT_NOHANDS";
			combine = true;
		}
		else if (opt & OPT_NOHM)
		{
			if (combine) dst << " | ";
			dst << "OPT_NOHM";
			combine = true;
		}
		else if (opt & OPT_NOSECS)
		{
			if (combine) dst << " | ";
			dst << "OPT_NOSECS";
			combine = true;
		}
	}
	if (cmd == CMD_KEYS)
	{
		if (opt & 0xFF)
		{
			if (combine) dst << " | ";
			if ((opt & 0xFF) >= 32 && (opt & 0xFF) <= 126)
			{
				dst << "'" << (char)(opt & 0xFF) << "'";
			}
			else
			{
				std::stringstream tmp;
				tmp << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (unsigned int)(opt & 0xFF);
				dst << tmp.str();
			}
		}
	}
}

/*
#define OPT_MONO             1UL
#define OPT_NODL             2UL
#define OPT_SIGNED           256UL <- special case
#define OPT_FLAT             256UL
#define OPT_CENTERX          512UL
#define OPT_CENTERY          1024UL
#define OPT_CENTER           1536UL ----
#define OPT_RIGHTX           2048UL
#define OPT_NOBACK           4096UL
#define OPT_NOTICKS          8192UL
#define OPT_NOHM             16384UL
#define OPT_NOPOINTER        16384UL <- special case
#define OPT_NOSECS           32768UL
#define OPT_NOHANDS          49152UL ---- */

void DlParser::toString(std::string &dst, const DlParsed &parsed)
{
	if (parsed.IdLeft == 0xFFFFFF00) // Coprocessor
	{
		std::stringstream res;

		res << s_CmdIdList[parsed.IdRight];
		res << "(";

		for (int p = 0; p < parsed.ExpectedParameterCount; ++p)
		{
			if (p != 0) res << ", ";
			if (p == parsed.ExpectedParameterCount - 1 && parsed.ExpectedStringParameter)
			{
				// String parameter
				res << "\"";
				std::string escstr;
				escapeString(escstr, parsed.StringParameter);
				res << escstr;
				res << "\"";
			}
			else
			{
				// Numeric parameter
				bool constantOpt = false;
				switch (parsed.IdRight | 0xFFFFFF00)
				{
				case CMD_TEXT:
				case CMD_GAUGE:
				case CMD_CLOCK:
				case CMD_DIAL:
				case CMD_NUMBER:
					if (p == 3) constantOpt = true;
					break;
				case CMD_PROGRESS:
				case CMD_SLIDER:
				case CMD_SCROLLBAR:
				case CMD_TOGGLE:
					if (p == 4) constantOpt = true;
					break;
				case CMD_BUTTON:
				case CMD_KEYS:
					if (p == 5) constantOpt = true;
					break;
				}
				if (constantOpt)
				{
					optToString(res, parsed.Parameter[p].U, parsed.IdRight | 0xFFFFFF00);
				}
				else
				{
					res << parsed.Parameter[p].I;
				}
			}
		}

		res << ")";

		dst = res.str();
	}
	else
	{
		uint32_t compiled = DlParser::compile(parsed);
		DlParser::toString(dst, compiled);
	}
}

QString DlParser::toString(const DlParsed &parsed)
{
	std::string str;
	toString(str, parsed);
	return QString(str.c_str());
}

void DlParser::escapeString(std::string &dst, const std::string &src)
{
	std::stringstream res;
	for (int c = 0; c < src.size(); ++c)
	{
		unsigned char ch = src[c];
		if (ch == '\t') res << "\\t";
		else if (ch == '\\') res << "\\\\";
		else if (ch == '\n') res << "\\n";
		else if (ch == '\r') res << "\\r";
		else if (ch >= 32 && ch <= 126) res << ch;
		else
		{
			std::stringstream tmp;
			tmp << "\\x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (unsigned int)ch;
			res << tmp.str();
		}
	}
	dst = res.str();
}

void DlParser::unescapeString(std::string &dst, const std::string &src)
{
	std::stringstream res;
	for (int i = 0; i < src.size(); ++i)
	{
		char c = src[i];
		if (c == '\\')
		{
			// unescape
			++i;
			if (i < src.size())
			{
				c = src[i];
				switch (c)
				{
				case 't':
					res << '\t';
					break;
				case 'n':
					res << '\n';
					break;
				case 'r':
					res << '\r';
					break;
				case 'x':
				{
					if (i + 2 < src.size()
						&& ('A' <= src[i + 1] && src[i + 1] <= 'F' || 'a' <= src[i + 1] && src[i + 1] <= 'f' || '0' <= src[i + 1] && src[i + 1] <= '9')
						&& ('A' <= src[i + 2] && src[i + 2] <= 'F' || 'a' <= src[i + 2] && src[i + 2] <= 'f' || '0' <= src[i + 2] && src[i + 2] <= '9'))
					{
						std::stringstream tmp;
						tmp << std::hex << src[i + 1] << src[i + 2];
						unsigned int ch;
						tmp >> ch;
						c = (char)(unsigned char)(ch & 0xFF);
						// printf("char %i\n", (int)c);
						res << c;
						i += 2;
					}
					else
					{
						res << "\\x";
					}
					break;
				}
				default:
					res << c;
					break;
				}
			}
			else
			{
				res << '\\';
			}
		}
		else
		{
			res << c;
		}
	}
	dst = res.str();
}

} /* namespace FT800EMUQT */

/* end of file */
