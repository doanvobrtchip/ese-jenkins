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
#include <sstream>
#include <map>

// Qt includes

// Emulator includes
#include "ft800emu_inttypes.h"
#include "vc.h"

// Project includes

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

namespace FT800EMUQT {

static std::map<std::string, int> s_IdMap;
static std::map<std::string, int> s_ParamMap;
static int s_ParamCount[39];

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
		s_ParamMap["PALETTED"] = PALETTED;
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
}

void DlParser::parse(DlParsed &parsed, const QString &line)
{
	init();
	
	QByteArray chars = line.toLatin1();
	const char *src = chars.constData();
	const int len = chars.size();
	
	parsed.BadCharacterIndex = -1;
	
	for (int p = 0; p < 8; ++p)
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
	}
	
	if (failId)
	{
		return;
	}
	
	// for each possible parameter
	bool failParam = false;
	int finalIndex = -1;
	for (int p = 0; p < 8; ++p)
	{
		parsed.ParameterIndex[p] = i;
		std::stringstream pss;
		for (; ; ++i)
		{
			if (i < len)
			{
				char c = src[i];
				if (c >= 'a' && c <= 'z') 
				{
					c = c - 'a' + 'A'; // uppercase
				}
				if (parsed.ParameterLength[p]== 0 && (c == ' ' || c == '\t'))
				{
					++parsed.ParameterIndex[p]; /* pre-trim */
				}
				else if ((c >= '0' && c <= '9') && parsed.ParameterIndex[p] + parsed.ParameterLength[p] == i)
				{
					pss << c;
					++parsed.ParameterLength[p];
				}
				else if (((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')) && parsed.ParameterIndex[p] + parsed.ParameterLength[p] == i)
				{
					parsed.NumericParameter[p] = false;
					pss << c;
					++parsed.ParameterLength[p];
				}
				else if (c == ' ' || c == '\t')
				{
					/* post-trim */
				}
				else if (parsed.ParameterLength[p] > 0 && c == ',')
				{
					/* valid, more, continue */
					++i;
					break;
				}
				else if (parsed.ParameterLength[p] > 0 && c == ')')
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
		
		// validate named parameter
		if (p < parsed.ExpectedParameterCount || !parsed.ValidId)
		{
			std::string ps = pss.str();
			if (parsed.NumericParameter[p] && ps.length() > 0)
			{
				parsed.Parameter[p] = atoi(ps.c_str());
				parsed.ValidParameter[p] = true;
			}
			else
			{
				std::map<std::string, int>::iterator it = s_ParamMap.find(ps);
				if (it != s_ParamMap.end())
				{
					parsed.Parameter[p] = it->second;
					parsed.ValidParameter[p] = true;
				}
			}
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
			return;
		}
	}
}

} /* namespace FT800EMUQT */

/* end of file */
