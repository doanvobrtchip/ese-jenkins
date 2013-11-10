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

// Qt includes
#include <QStringList>

// Emulator includes
#include "ft800emu_inttypes.h"
#include "vc.h"

// Project includes

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

void DlParser::getIdentifiers(QStringList &list)
{
	init();
	
	for (std::map<std::string, int>::iterator it = s_IdMap.begin(), end = s_IdMap.end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
	}
	
	list.push_back("VERTEX2II");
	list.push_back("VERTEX2F");
}

void DlParser::getParams(QStringList &list)
{
	init();
	
	for (std::map<std::string, int>::iterator it = s_ParamMap.begin(), end = s_ParamMap.end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
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
				else if ((p == 0 || parsed.ParameterLength[p]) > 0 && c == ')')
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

uint32_t DlParser::compile(const DlParsed &parsed)
{
	const uint32_t *p = parsed.Parameter;
	if (parsed.IdLeft == FT800EMU_DL_VERTEX2F)
	{
		return VERTEX2F(p[0], p[1]);
	}
	else if (parsed.IdLeft == FT800EMU_DL_VERTEX2II)
	{
		return VERTEX2II(p[0], p[1], p[2], p[3]);
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
	}
	return DISPLAY();
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
					int addr = v & 0xFFFFF;
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
					res << format << ", " << linestride << ", " << height << ")";
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
					res << filter << ", ";
					res << wrapx << ", " << wrapy << ", ";
					res << width << ", " << height << ")";
					break;
				}
				case FT800EMU_DL_ALPHA_FUNC:
				{
					int func = (v >> 8) & 0x07;
					int ref = v & 0xFF;
					res << "ALPHA_FUNC(";
					res << func << ", " << ref << ")";
					break;
				}
				case FT800EMU_DL_STENCIL_FUNC:
				{
					int func = (v >> 16) & 0x7;
					int ref = (v >> 8) & 0xFF;
					int mask = v & 0xFF;
					res << "STENCIL_FUNC(";
					res << func << ", " << ref <<  ", " << mask << ")";
					break;
				}
				case FT800EMU_DL_BLEND_FUNC:
				{
					int src = (v >> 3) & 0x7;
					int dst = v & 0x7;
					res << "BLEND_FUNC(";
					res << src << ", " << dst << ")";
					break;
				}
				case FT800EMU_DL_STENCIL_OP:
				{
					int sfail = (v >> 3) & 0x7;
					int spass = v & 0x7;
					res << "STENCIL_OP(";
					res << sfail << ", " << spass << ")";
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
					int x = (v >> 9) & 0x1FF;
					int y = v & 0x1FF;
					res << "SCISSOR_XY(";
					res << x << ", " << y << ")";
					break;
				}
				case FT800EMU_DL_SCISSOR_SIZE:
				{
					int width = (v >> 10) & 0x3FF;
					int height = v & 0x3FF;
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
					res << primitive << ")";
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
			res << handle << ", " << cell << ")";
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

} /* namespace FT800EMUQT */

/* end of file */
