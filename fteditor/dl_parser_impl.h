/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
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
#if defined(FTEDITOR_PARSER_VC1)
#include <vc.h>
#elif defined(FTEDITOR_PARSER_VC2)
#include <vc2.h>
#endif

// Project includes
#include "constant_mapping.h"

namespace FTEDITOR {

static std::map<std::string, int> s_IdMap;
static std::map<std::string, int> s_ParamMap;

static std::map<std::string, int> s_CmdIdMap;
static std::map<std::string, int> s_CmdParamMap;

#if defined(FTEDITOR_PARSER_VC1)
static std::map<std::string, int> s_CmdIdMapFT801;
#endif

#if defined(FTEDITOR_PARSER_VC1)
#define DL_ID_NB 39
#define CMD_ID_NB 54
#elif defined(FTEDITOR_PARSER_VC2)
#define DL_ID_NB 46
#define CMD_ID_NB 68
#endif
static int s_ParamCount[DL_ID_NB];
static int s_CmdParamCount[CMD_ID_NB];
static bool s_CmdParamString[CMD_ID_NB];

static std::string s_CmdIdList[CMD_ID_NB];

// Sign-extend the n-bit value v
#define SIGNED_N(v, n) \
    (((int32_t)((v) << (32-(n)))) >> (32-(n)))

#if defined(FTEDITOR_PARSER_VC1)
void DlParser::initVC1()
#elif defined(FTEDITOR_PARSER_VC2)
void DlParser::initVC2()
#endif
{
	if (!s_IdMap.size())
	{
		s_IdMap["DISPLAY"] = FTEDITOR_DL_DISPLAY;
		s_ParamCount[FTEDITOR_DL_DISPLAY] = 0;
		s_IdMap["BITMAP_SOURCE"] = FTEDITOR_DL_BITMAP_SOURCE;
		s_ParamCount[FTEDITOR_DL_BITMAP_SOURCE] = 1;
		s_IdMap["CLEAR_COLOR_RGB"] = FTEDITOR_DL_CLEAR_COLOR_RGB;
		s_ParamCount[FTEDITOR_DL_CLEAR_COLOR_RGB] = 3;
		s_IdMap["TAG"] = FTEDITOR_DL_TAG;
		s_ParamCount[FTEDITOR_DL_TAG] = 1;
		s_IdMap["COLOR_RGB"] = FTEDITOR_DL_COLOR_RGB;
		s_ParamCount[FTEDITOR_DL_COLOR_RGB] = 3;
		s_IdMap["BITMAP_HANDLE"] = FTEDITOR_DL_BITMAP_HANDLE;
		s_ParamCount[FTEDITOR_DL_BITMAP_HANDLE] = 1;
		s_IdMap["CELL"] = FTEDITOR_DL_CELL;
		s_ParamCount[FTEDITOR_DL_CELL] = 1;
		s_IdMap["BITMAP_LAYOUT"] = FTEDITOR_DL_BITMAP_LAYOUT;
		s_ParamCount[FTEDITOR_DL_BITMAP_LAYOUT] = 3;
		s_IdMap["BITMAP_SIZE"] = FTEDITOR_DL_BITMAP_SIZE;
		s_ParamCount[FTEDITOR_DL_BITMAP_SIZE] = 5;
		s_IdMap["ALPHA_FUNC"] = FTEDITOR_DL_ALPHA_FUNC;
		s_ParamCount[FTEDITOR_DL_ALPHA_FUNC] = 2;
		s_IdMap["STENCIL_FUNC"] = FTEDITOR_DL_STENCIL_FUNC;
		s_ParamCount[FTEDITOR_DL_STENCIL_FUNC] = 3;
		s_IdMap["BLEND_FUNC"] = FTEDITOR_DL_BLEND_FUNC;
		s_ParamCount[FTEDITOR_DL_BLEND_FUNC] = 2;
		s_IdMap["STENCIL_OP"] = FTEDITOR_DL_STENCIL_OP;
		s_ParamCount[FTEDITOR_DL_STENCIL_OP] = 2;
		s_IdMap["POINT_SIZE"] = FTEDITOR_DL_POINT_SIZE;
		s_ParamCount[FTEDITOR_DL_POINT_SIZE] = 1;
		s_IdMap["LINE_WIDTH"] = FTEDITOR_DL_LINE_WIDTH;
		s_ParamCount[FTEDITOR_DL_LINE_WIDTH] = 1;
		s_IdMap["CLEAR_COLOR_A"] = FTEDITOR_DL_CLEAR_COLOR_A;
		s_ParamCount[FTEDITOR_DL_CLEAR_COLOR_A] = 1;
		s_IdMap["COLOR_A"] = FTEDITOR_DL_COLOR_A;
		s_ParamCount[FTEDITOR_DL_COLOR_A] = 1;
		s_IdMap["CLEAR_STENCIL"] = FTEDITOR_DL_CLEAR_STENCIL;
		s_ParamCount[FTEDITOR_DL_CLEAR_STENCIL] = 1;
		s_IdMap["CLEAR_TAG"] = FTEDITOR_DL_CLEAR_TAG;
		s_ParamCount[FTEDITOR_DL_CLEAR_TAG] = 1;
		s_IdMap["STENCIL_MASK"] = FTEDITOR_DL_STENCIL_MASK;
		s_ParamCount[FTEDITOR_DL_STENCIL_MASK] = 1;
		s_IdMap["TAG_MASK"] = FTEDITOR_DL_TAG_MASK;
		s_ParamCount[FTEDITOR_DL_TAG_MASK] = 1;
		s_IdMap["BITMAP_TRANSFORM_A"] = FTEDITOR_DL_BITMAP_TRANSFORM_A;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_A] = 1;
		s_IdMap["BITMAP_TRANSFORM_B"] = FTEDITOR_DL_BITMAP_TRANSFORM_B;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_B] = 1;
		s_IdMap["BITMAP_TRANSFORM_C"] = FTEDITOR_DL_BITMAP_TRANSFORM_C;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_C] = 1;
		s_IdMap["BITMAP_TRANSFORM_D"] = FTEDITOR_DL_BITMAP_TRANSFORM_D;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_D] = 1;
		s_IdMap["BITMAP_TRANSFORM_E"] = FTEDITOR_DL_BITMAP_TRANSFORM_E;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_E] = 1;
		s_IdMap["BITMAP_TRANSFORM_F"] = FTEDITOR_DL_BITMAP_TRANSFORM_F;
		s_ParamCount[FTEDITOR_DL_BITMAP_TRANSFORM_F] = 1;
		s_IdMap["SCISSOR_XY"] = FTEDITOR_DL_SCISSOR_XY;
		s_ParamCount[FTEDITOR_DL_SCISSOR_XY] = 2;
		s_IdMap["SCISSOR_SIZE"] = FTEDITOR_DL_SCISSOR_SIZE;
		s_ParamCount[FTEDITOR_DL_SCISSOR_SIZE] = 2;
		s_IdMap["CALL"] = FTEDITOR_DL_CALL;
		s_ParamCount[FTEDITOR_DL_CALL] = 1;
		s_IdMap["JUMP"] = FTEDITOR_DL_JUMP;
		s_ParamCount[FTEDITOR_DL_JUMP] = 1;
		s_IdMap["BEGIN"] = FTEDITOR_DL_BEGIN;
		s_ParamCount[FTEDITOR_DL_BEGIN] = 1;
		s_IdMap["COLOR_MASK"] = FTEDITOR_DL_COLOR_MASK;
		s_ParamCount[FTEDITOR_DL_COLOR_MASK] = 4;
		s_IdMap["END"] = FTEDITOR_DL_END;
		s_ParamCount[FTEDITOR_DL_END] = 0;
		s_IdMap["SAVE_CONTEXT"] = FTEDITOR_DL_SAVE_CONTEXT;
		s_ParamCount[FTEDITOR_DL_SAVE_CONTEXT] = 0;
		s_IdMap["RESTORE_CONTEXT"] = FTEDITOR_DL_RESTORE_CONTEXT;
		s_ParamCount[FTEDITOR_DL_RESTORE_CONTEXT] = 0;
		s_IdMap["RETURN"] = FTEDITOR_DL_RETURN;
		s_ParamCount[FTEDITOR_DL_RETURN] = 0;
		s_IdMap["MACRO"] = FTEDITOR_DL_MACRO;
		s_ParamCount[FTEDITOR_DL_MACRO] = 1;
		s_IdMap["CLEAR"] = FTEDITOR_DL_CLEAR;
		s_ParamCount[FTEDITOR_DL_CLEAR] = 3;
#if defined(FTEDITOR_PARSER_VC2)
		s_IdMap["VERTEX_FORMAT"] = FTEDITOR_DL_VERTEX_FORMAT;
		s_ParamCount[FTEDITOR_DL_VERTEX_FORMAT] = 1;
		s_IdMap["BITMAP_LAYOUT_H"] = FTEDITOR_DL_BITMAP_LAYOUT_H;
		s_ParamCount[FTEDITOR_DL_BITMAP_LAYOUT_H] = 2;
		s_IdMap["BITMAP_SIZE_H"] = FTEDITOR_DL_BITMAP_SIZE_H;
		s_ParamCount[FTEDITOR_DL_BITMAP_SIZE_H] = 2;
		s_IdMap["PALETTE_SOURCE"] = FTEDITOR_DL_PALETTE_SOURCE;
		s_ParamCount[FTEDITOR_DL_PALETTE_SOURCE] = 1;
		s_IdMap["VERTEX_TRANSLATE_X"] = FTEDITOR_DL_VERTEX_TRANSLATE_X;
		s_ParamCount[FTEDITOR_DL_VERTEX_TRANSLATE_X] = 1;
		s_IdMap["VERTEX_TRANSLATE_Y"] = FTEDITOR_DL_VERTEX_TRANSLATE_Y;
		s_ParamCount[FTEDITOR_DL_VERTEX_TRANSLATE_Y] = 1;
		s_IdMap["NOP"] = FTEDITOR_DL_NOP;
		s_ParamCount[FTEDITOR_DL_NOP] = 0;
#endif
	}
	if (!s_ParamMap.size())
	{
		s_ParamMap["ALWAYS"] = ALWAYS;
		s_ParamMap["ARGB1555"] = ARGB1555;
		s_ParamMap["ARGB2"] = ARGB2;
		s_ParamMap["ARGB4"] = ARGB4;
#if defined(FTEDITOR_PARSER_VC2)
		s_ParamMap["ARGB8_SNAPSHOT"] = ARGB8_SNAPSHOT;
#endif
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
#if defined(FTEDITOR_PARSER_VC2)
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
#if defined(FTEDITOR_PARSER_VC2)
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
		s_CmdIdMap["CMD_LOADIMAGE"] = CMD_LOADIMAGE & 0xFF; // STREAMING DATA
		s_CmdParamCount[CMD_LOADIMAGE & 0xFF] = 3;
		s_CmdParamString[CMD_LOADIMAGE & 0xFF] = true;
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
#if defined(FTEDITOR_PARSER_VC1) // Deprecated in FT810
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
#if defined(FTEDITOR_PARSER_VC2)
		/*s_CmdIdMap["CMD_SETROTATE"] = CMD_SETROTATE & 0xFF;
		s_CmdParamCount[CMD_SETROTATE & 0xFF] = 1; // 0-7
		s_CmdParamString[CMD_SETROTATE & 0xFF] = false;*/ // currently don't support because we don't remap coordinates in the editor yet
		s_CmdIdMap["CMD_SNAPSHOT2"] = CMD_SNAPSHOT2 & 0xFF;
		s_CmdParamCount[CMD_SNAPSHOT2 & 0xFF] = 6;
		s_CmdParamString[CMD_SNAPSHOT2 & 0xFF] = false;
		s_CmdIdMap["CMD_SETBASE"] = CMD_SETBASE & 0xFF;
		s_CmdParamCount[CMD_SETBASE & 0xFF] = 1;
		s_CmdParamString[CMD_SETBASE & 0xFF] = false;
		s_CmdIdMap["CMD_MEDIAFIFO"] = CMD_MEDIAFIFO & 0xFF;
		s_CmdParamCount[CMD_MEDIAFIFO & 0xFF] = 2;
		s_CmdParamString[CMD_MEDIAFIFO & 0xFF] = false;
		s_CmdIdMap["CMD_PLAYVIDEO"] = CMD_PLAYVIDEO & 0xFF; // STREAMING DATA
		s_CmdParamCount[CMD_PLAYVIDEO & 0xFF] = 2;
		s_CmdParamString[CMD_PLAYVIDEO & 0xFF] = true;
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

#if defined(FTEDITOR_PARSER_VC1)
		s_CmdIdMapFT801 = s_CmdIdMap;
		s_CmdIdMap.erase("CMD_CSKETCH");
#endif
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
#if defined(FTEDITOR_PARSER_VC2)
		s_CmdParamMap["OPT_NOTEAR"] = OPT_NOTEAR;
		s_CmdParamMap["OPT_FULLSCREEN"] = OPT_FULLSCREEN;
		s_CmdParamMap["OPT_MEDIAFIFO"] = OPT_MEDIAFIFO;
		s_CmdParamMap["OPT_SOUND"] = OPT_SOUND;
#endif
	}

#if defined(FTEDITOR_PARSER_VC1)
	m_IdMap[FTEDITOR_FT800] = &s_IdMap;
	m_IdMap[FTEDITOR_FT801] = &s_IdMap;
	m_ParamMap[FTEDITOR_FT800] = &s_ParamMap;
	m_ParamMap[FTEDITOR_FT801] = &s_ParamMap;
	m_CmdIdMap[FTEDITOR_FT800] = &s_CmdIdMap;
	m_CmdIdMap[FTEDITOR_FT801] = &s_CmdIdMapFT801;
	m_CmdParamMap[FTEDITOR_FT800] = &s_CmdParamMap;
	m_CmdParamMap[FTEDITOR_FT801] = &s_CmdParamMap;
	m_ParamCount[FTEDITOR_FT800] = s_ParamCount;
	m_ParamCount[FTEDITOR_FT801] = s_ParamCount;
	m_CmdParamCount[FTEDITOR_FT800] = s_CmdParamCount;
	m_CmdParamCount[FTEDITOR_FT801] = s_CmdParamCount;
	m_CmdParamString[FTEDITOR_FT800] = s_CmdParamString;
	m_CmdParamString[FTEDITOR_FT801] = s_CmdParamString;
	m_CmdIdList[FTEDITOR_FT800] = s_CmdIdList;
	m_CmdIdList[FTEDITOR_FT801] = s_CmdIdList;
#elif defined(FTEDITOR_PARSER_VC2)
	m_IdMap[FTEDITOR_FT810] = &s_IdMap;
	m_IdMap[FTEDITOR_FT811] = &s_IdMap;
	m_ParamMap[FTEDITOR_FT810] = &s_ParamMap;
	m_ParamMap[FTEDITOR_FT811] = &s_ParamMap;
	m_CmdIdMap[FTEDITOR_FT810] = &s_CmdIdMap;
	m_CmdIdMap[FTEDITOR_FT811] = &s_CmdIdMap;
	m_CmdParamMap[FTEDITOR_FT810] = &s_CmdParamMap;
	m_CmdParamMap[FTEDITOR_FT811] = &s_CmdParamMap;
	m_ParamCount[FTEDITOR_FT810] = s_ParamCount;
	m_ParamCount[FTEDITOR_FT811] = s_ParamCount;
	m_CmdParamCount[FTEDITOR_FT810] = s_CmdParamCount;
	m_CmdParamCount[FTEDITOR_FT811] = s_CmdParamCount;
	m_CmdParamString[FTEDITOR_FT810] = s_CmdParamString;
	m_CmdParamString[FTEDITOR_FT811] = s_CmdParamString;
	m_CmdIdList[FTEDITOR_FT810] = s_CmdIdList;
	m_CmdIdList[FTEDITOR_FT811] = s_CmdIdList;
#endif
}

#if defined(FTEDITOR_PARSER_VC1)
uint32_t DlParser::compileVC1(int deviceIntf, const DlParsed &parsed)
#elif defined(FTEDITOR_PARSER_VC2)
uint32_t DlParser::compileVC2(int deviceIntf, const DlParsed &parsed)
#endif
{
	const uint32_t *p = static_cast<const uint32_t *>(static_cast<const void *>(parsed.Parameter));
	if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
	{
		return VERTEX2F(p[0], p[1]);
	}
	else if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
	{
		return VERTEX2II(p[0], p[1], p[2], p[3]);
	}
	else if (parsed.IdLeft == 0xFFFFFF00) // Coprocessor
	{
		return 0xFFFFFF00 | (parsed.IdRight);
	}
	else switch (parsed.IdRight)
	{
		case FTEDITOR_DL_DISPLAY:
			return DISPLAY();
		case FTEDITOR_DL_BITMAP_SOURCE:
			return BITMAP_SOURCE(p[0]);
		case FTEDITOR_DL_CLEAR_COLOR_RGB:
			return CLEAR_COLOR_RGB(p[0], p[1], p[2]);
		case FTEDITOR_DL_TAG:
			return TAG(p[0]);
		case FTEDITOR_DL_COLOR_RGB:
			return COLOR_RGB(p[0], p[1], p[2]);
		case FTEDITOR_DL_BITMAP_HANDLE:
			return BITMAP_HANDLE(p[0]);
		case FTEDITOR_DL_CELL:
			return CELL(p[0]);
		case FTEDITOR_DL_BITMAP_LAYOUT:
			return BITMAP_LAYOUT(p[0], p[1], p[2]);
		case FTEDITOR_DL_BITMAP_SIZE:
			return BITMAP_SIZE(p[0], p[1], p[2], p[3], p[4]);
		case FTEDITOR_DL_ALPHA_FUNC:
			return ALPHA_FUNC(p[0], p[1]);
		case FTEDITOR_DL_STENCIL_FUNC:
			return STENCIL_FUNC(p[0], p[1], p[2]);
		case FTEDITOR_DL_BLEND_FUNC:
			return BLEND_FUNC(p[0], p[1]);
		case FTEDITOR_DL_STENCIL_OP:
			return STENCIL_OP(p[0], p[1]);
		case FTEDITOR_DL_POINT_SIZE:
			return POINT_SIZE(p[0]);
		case FTEDITOR_DL_LINE_WIDTH:
			return LINE_WIDTH(p[0]);
		case FTEDITOR_DL_CLEAR_COLOR_A:
			return CLEAR_COLOR_A(p[0]);
		case FTEDITOR_DL_COLOR_A:
			return COLOR_A(p[0]);
		case FTEDITOR_DL_CLEAR_STENCIL:
			return CLEAR_STENCIL(p[0]);
		case FTEDITOR_DL_CLEAR_TAG:
			return CLEAR_TAG(p[0]);
		case FTEDITOR_DL_STENCIL_MASK:
			return STENCIL_MASK(p[0]);
		case FTEDITOR_DL_TAG_MASK:
			return TAG_MASK(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_A:
			return BITMAP_TRANSFORM_A(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_B:
			return BITMAP_TRANSFORM_B(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_C:
			return BITMAP_TRANSFORM_C(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_D:
			return BITMAP_TRANSFORM_D(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_E:
			return BITMAP_TRANSFORM_E(p[0]);
		case FTEDITOR_DL_BITMAP_TRANSFORM_F:
			return BITMAP_TRANSFORM_F(p[0]);
		case FTEDITOR_DL_SCISSOR_XY:
			return SCISSOR_XY(p[0], p[1]);
		case FTEDITOR_DL_SCISSOR_SIZE:
			return SCISSOR_SIZE(p[0], p[1]);
		case FTEDITOR_DL_CALL:
			return CALL(p[0]);
		case FTEDITOR_DL_JUMP:
			return JUMP(p[0]);
		case FTEDITOR_DL_BEGIN:
			return BEGIN(p[0]);
		case FTEDITOR_DL_COLOR_MASK:
			return COLOR_MASK(p[0], p[1], p[2], p[3]);
		case FTEDITOR_DL_END:
			return END();
		case FTEDITOR_DL_SAVE_CONTEXT:
			return SAVE_CONTEXT();
		case FTEDITOR_DL_RESTORE_CONTEXT:
			return RESTORE_CONTEXT();
		case FTEDITOR_DL_RETURN:
			return RETURN();
		case FTEDITOR_DL_MACRO:
			return MACRO(p[0]);
		case FTEDITOR_DL_CLEAR:
			return CLEAR(p[0], p[1], p[2]);
#if defined(FTEDITOR_PARSER_VC2)
		case FTEDITOR_DL_VERTEX_FORMAT:
			return VERTEX_FORMAT(p[0]);
		case FTEDITOR_DL_BITMAP_LAYOUT_H:
			return BITMAP_LAYOUT_H(p[0], p[1]);
		case FTEDITOR_DL_BITMAP_SIZE_H:
			return BITMAP_SIZE_H(p[0], p[1]);
		case FTEDITOR_DL_PALETTE_SOURCE:
			return PALETTE_SOURCE(p[0]);
		case FTEDITOR_DL_VERTEX_TRANSLATE_X:
			return VERTEX_TRANSLATE_X(p[0]);
		case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
			return VERTEX_TRANSLATE_Y(p[0]);
		case FTEDITOR_DL_NOP:
			return NOP();
#endif
	}
	return DISPLAY();
}

#if defined(FTEDITOR_PARSER_VC1)
void DlParser::compileVC1(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed) // compile CMD parameters
#elif defined(FTEDITOR_PARSER_VC2)
void DlParser::compileVC2(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed) // compile CMD parameters
#endif
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
					for (size_t i = 0; i < parsed.StringParameter.size() + 1; i += 4)
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
					for (size_t i = 0; i < parsed.StringParameter.size() + 1; i += 4)
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
					for (size_t i = 0; i < parsed.StringParameter.size() + 1; i += 4)
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
#if defined(FTEDITOR_PARSER_VC2)
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
#if defined(FTEDITOR_PARSER_VC2)
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
#if defined(FTEDITOR_PARSER_VC2)
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
					if (ptr >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						ptr = addressSpace(FTEDITOR_CURRENT_DEVICE) - 1;
					if (ptr + num >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						num = addressSpace(FTEDITOR_CURRENT_DEVICE) - ptr - 1;
					compiled.push_back(ptr);
					compiled.push_back(num);
					break;
				}
				case CMD_MEMSET:
				{
					uint32_t ptr = parsed.Parameter[0].U;
					uint32_t num = parsed.Parameter[2].U;
					if (ptr >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						ptr = addressSpace(FTEDITOR_CURRENT_DEVICE) - 1;
					if (ptr + num >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						num = addressSpace(FTEDITOR_CURRENT_DEVICE) - ptr - 1;
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
					if (dst >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						dst = addressSpace(FTEDITOR_CURRENT_DEVICE) - 1;
					if (src >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						src = addressSpace(FTEDITOR_CURRENT_DEVICE) - 1;
					if (dst + num >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						num = addressSpace(FTEDITOR_CURRENT_DEVICE) - dst - 1;
					if (src + num >= addressSpace(FTEDITOR_CURRENT_DEVICE))
						num = addressSpace(FTEDITOR_CURRENT_DEVICE) - src - 1;
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
#if defined(FTEDITOR_PARSER_VC1) // This is deprecated in FT810
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
#if defined(FTEDITOR_PARSER_VC2)
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
	const char *str = FTEDITOR::bitmapFormatToString(FTEDITOR_CURRENT_DEVICE, id);
	if (str[0])
	{
		dst << str;
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

#if defined(FTEDITOR_PARSER_VC1)
void DlParser::toStringVC1(int deviceIntf, std::string &dst, uint32_t v)
#elif defined(FTEDITOR_PARSER_VC2)
void DlParser::toStringVC2(int deviceIntf, std::string &dst, uint32_t v)
#endif
{
	std::stringstream res;

	switch (v >> 30)
	{
		case 0:
		{
			switch (v >> 24)
			{
				case FTEDITOR_DL_DISPLAY:
				{
					res << "DISPLAY()";
					break;
				}
				case FTEDITOR_DL_BITMAP_SOURCE:
				{
					int addr = v & addressMask(FTEDITOR_CURRENT_DEVICE);
					res << "BITMAP_SOURCE(";
					res << addr << ")";
					break;
				}
				case FTEDITOR_DL_CLEAR_COLOR_RGB:
				{
					int red = (v >> 16) & 0xFF;
					int green = (v >> 8) & 0xFF;
					int blue = v & 0xFF;
					res << "CLEAR_COLOR_RGB(";
					res << red << ", " << green << ", " << blue << ")";
					break;
				}
				case FTEDITOR_DL_TAG:
				{
					int s = v & 0xFF;
					res << "TAG(";
					res << s << ")";
					break;
				}
				case FTEDITOR_DL_COLOR_RGB:
				{
					int red = (v >> 16) & 0xFF;
					int green = (v >> 8) & 0xFF;
					int blue = v & 0xFF;
					res << "COLOR_RGB(";
					res << red << ", " << green << ", " << blue << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_HANDLE:
				{
					int handle = v & 0x1F;
					res << "BITMAP_HANDLE(";
					res << handle << ")";
					break;
				}
				case FTEDITOR_DL_CELL:
				{
					int cell = v & 0x7F;
					res << "CELL(";
					res << cell << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_LAYOUT:
				{
					int format = (v >> 19) & 0x1F;
					int linestride = (v >> 9) & 0x3FF;
					int height = v & 0x1FF;
					res << "BITMAP_LAYOUT(";
					bitmapFormatToString(res, format);
					res << ", " << linestride << ", " << height << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_SIZE:
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
				case FTEDITOR_DL_ALPHA_FUNC:
				{
					int func = (v >> 8) & 0x07;
					int ref = v & 0xFF;
					res << "ALPHA_FUNC(";
					compareToString(res, func);
					res << ", " << ref << ")";
					break;
				}
				case FTEDITOR_DL_STENCIL_FUNC:
				{
					int func = (v >> 16) & 0x7;
					int ref = (v >> 8) & 0xFF;
					int mask = v & 0xFF;
					res << "STENCIL_FUNC(";
					compareToString(res, func);
					res << ", " << ref <<  ", " << mask << ")";
					break;
				}
				case FTEDITOR_DL_BLEND_FUNC:
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
				case FTEDITOR_DL_STENCIL_OP:
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
				case FTEDITOR_DL_POINT_SIZE:
				{
					int size = v & 0x1FFF;
					res << "POINT_SIZE(";
					res << size << ")";
					break;
				}
				case FTEDITOR_DL_LINE_WIDTH:
				{
					int width = v & 0xFFF;
					res << "LINE_WIDTH(";
					res << width << ")";
					break;
				}
				case FTEDITOR_DL_CLEAR_COLOR_A:
				{
					int alpha = v & 0xFF;
					res << "CLEAR_COLOR_A(";
					res << alpha << ")";
					break;
				}
				case FTEDITOR_DL_COLOR_A:
				{
					int alpha = v & 0xFF;
					res << "COLOR_A(";
					res << alpha << ")";
					break;
				}
				case FTEDITOR_DL_CLEAR_STENCIL:
				{
					int s = v & 0xFF;
					res << "CLEAR_STENCIL(";
					res << s << ")";
					break;
				}
				case FTEDITOR_DL_CLEAR_TAG:
				{
					int s = v & 0xFF;
					res << "CLEAR_TAG(";
					res << s << ")";
					break;
				}
				case FTEDITOR_DL_STENCIL_MASK:
				{
					int mask = v & 0xFF;
					res << "STENCIL_MASK(";
					res << mask << ")";
					break;
				}
				case FTEDITOR_DL_TAG_MASK:
				{
					int mask = v & 0x01;
					res << "TAG_MASK(";
					res << mask << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_A:
				{
					int a = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_A(";
					res << a << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_B:
				{
					int b = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_B(";
					res << b << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_C:
				{
					int c = v & 0xFFFFFF;
					res << "BITMAP_TRANSFORM_C(";
					res << c << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_D:
				{
					int d = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_D(";
					res << d << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_E:
				{
					int e = v & 0x1FFFF;
					res << "BITMAP_TRANSFORM_E(";
					res << e << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_TRANSFORM_F:
				{
					int f = v & 0xFFFFFF;
					res << "BITMAP_TRANSFORM_F(";
					res << f << ")";
					break;
				}
				case FTEDITOR_DL_SCISSOR_XY:
				{
#if defined(FTEDITOR_PARSER_VC2)
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
				case FTEDITOR_DL_SCISSOR_SIZE:
				{
#if defined(FTEDITOR_PARSER_VC2)
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
				case FTEDITOR_DL_CALL:
				{
					int dest = v & 0xFFFF;
					res << "CALL(";
					res << dest << ")";
					break;
				}
				case FTEDITOR_DL_JUMP:
				{
					int dest = v & 0xFFFF;
					res << "JUMP(";
					res << dest << ")";
					break;
				}
				case FTEDITOR_DL_BEGIN:
				{
					int primitive = v & 0x0F;
					res << "BEGIN(";
					primToString(res, primitive);
					res << ")";
					break;
				}
				case FTEDITOR_DL_COLOR_MASK:
				{
					int r = (v >> 3) & 0x1;
					int g = (v >> 2) & 0x1;
					int b = (v >> 1) & 0x1;
					int a = v & 0x1;
					res << "COLOR_MASK(";
					res << r << ", " << g << ", " << b << ", " << a << ")";
					break;
				}
				case FTEDITOR_DL_END:
				{
					res << "END()";
					break;
				}
				case FTEDITOR_DL_SAVE_CONTEXT:
				{
					res << "SAVE_CONTEXT()";
					break;
				}
				case FTEDITOR_DL_RESTORE_CONTEXT:
				{
					res << "RESTORE_CONTEXT()";
					break;
				}
				case FTEDITOR_DL_RETURN:
				{
					res << "RETURN()";
					break;
				}
				case FTEDITOR_DL_MACRO:
				{
					int m = v & 0x01;
					res << "MACRO(";
					res << m << ")";
					break;
				}
				case FTEDITOR_DL_CLEAR:
				{
					int c = (v >> 2) & 0x1;
					int s = (v >> 1) & 0x1;
					int t = v & 0x1;
					res << "CLEAR(";
					res << c << ", " << s << ", " << t << ")";
					break;
				}
#if defined(FTEDITOR_PARSER_VC2)
				case FTEDITOR_DL_VERTEX_FORMAT:
				{
					int frac = v & 0x7;
					res << "VERTEX_FORMAT(";
					res << frac << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				{
					int linestride = (v >> 2) & 0x3;
					int height = v & 0x3;
					res << "BITMAP_LAYOUT_H(";
					res << linestride << ", " << height << ")";
					break;
				}
				case FTEDITOR_DL_BITMAP_SIZE_H:
				{
					int width = (v >> 2) & 0x3;
					int height = v & 0x3;
					res << "BITMAP_SIZE_H(";
					res << width << ", " << height << ")";
					break;
				}
				case FTEDITOR_DL_PALETTE_SOURCE:
				{
					int addr = v & addressMask(FTEDITOR_CURRENT_DEVICE);
					res << "PALETTE_SOURCE(";
					res << addr << ")";
					break;
				}
				case FTEDITOR_DL_VERTEX_TRANSLATE_X:
				{
					int x = SIGNED_N(v & 0x1FFFF, 17);
					res << "VERTEX_TRANSLATE_X(";
					res << x << ")";
					break;
				}
				case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
				{
					int y = SIGNED_N(v & 0x1FFFF, 17);
					res << "VERTEX_TRANSLATE_Y(";
					res << y << ")";
					break;
				}
				case FTEDITOR_DL_NOP:
				{
					res << "NOP()";
					break;
				}
#endif
			}
			break;
		}
		case FTEDITOR_DL_VERTEX2II:
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
		case FTEDITOR_DL_VERTEX2F:
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

static void optToString(std::stringstream &dst, uint32_t opt, uint32_t cmd)
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
#if defined(FTEDITOR_PARSER_VC2)
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
	}
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

#if defined(FTEDITOR_PARSER_VC1)
void DlParser::toStringVC1(int deviceIntf, std::string &dst, const DlParsed &parsed)
#elif defined(FTEDITOR_PARSER_VC2)
void DlParser::toStringVC2(int deviceIntf, std::string &dst, const DlParsed &parsed)
#endif
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
			int paramType = 0;
			// 1: constant
			// 2: bitmap format
			switch (parsed.IdRight | 0xFFFFFF00)
			{
			case CMD_TEXT:
			case CMD_GAUGE:
			case CMD_CLOCK:
			case CMD_DIAL:
			case CMD_NUMBER:
				if (p == 3) paramType = 1;
				break;
			case CMD_PROGRESS:
			case CMD_SLIDER:
			case CMD_SCROLLBAR:
			case CMD_TOGGLE:
				if (p == 4) paramType = 1;
				break;
			case CMD_BUTTON:
			case CMD_KEYS:
				if (p == 5) paramType = 1;
				break;
#if defined(FTEDITOR_PARSER_VC2)
			case CMD_SETBITMAP:
				if (p == 1) paramType = 2;
				break;
			case CMD_SNAPSHOT2:
				if (p == 0) paramType = 2;
				break;
#endif
			}
			switch (paramType)
			{
			case 1:
				optToString(res, parsed.Parameter[p].U, parsed.IdRight | 0xFFFFFF00);
				break;
			case 2:
				bitmapFormatToString(res, parsed.Parameter[p].U);
				break;
			default:
				res << parsed.Parameter[p].I;
				break;
			}
		}
	}

	res << ")";

	dst = res.str();
}

} /* namespace FTEDITOR */

/* end of file */
