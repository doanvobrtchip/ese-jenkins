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
#include <sstream>
#include <map>

// Qt includes

// Emulator includes
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

void DlParser::init()
{
	if (!s_IdMap.size())
	{
		s_IdMap["DISPLAY"] = FT800EMU_DL_DISPLAY;
		s_IdMap["BITMAP_SOURCE"] = FT800EMU_DL_BITMAP_SOURCE;
		s_IdMap["CLEAR_COLOR_RGB"] = FT800EMU_DL_CLEAR_COLOR_RGB;
		s_IdMap["TAG"] = FT800EMU_DL_TAG;
		s_IdMap["COLOR_RGB"] = FT800EMU_DL_COLOR_RGB;
		s_IdMap["BITMAP_HANDLE"] = FT800EMU_DL_BITMAP_HANDLE;
		s_IdMap["CELL"] = FT800EMU_DL_CELL;
		s_IdMap["BITMAP_LAYOUT"] = FT800EMU_DL_BITMAP_LAYOUT;
		s_IdMap["BITMAP_SIZE"] = FT800EMU_DL_BITMAP_SIZE;
		s_IdMap["ALPHA_FUNC"] = FT800EMU_DL_ALPHA_FUNC;
		s_IdMap["STENCIL_FUNC"] = FT800EMU_DL_STENCIL_FUNC;
		s_IdMap["BLEND_FUNC"] = FT800EMU_DL_BLEND_FUNC;
		s_IdMap["STENCIL_OP"] = FT800EMU_DL_STENCIL_OP;
		s_IdMap["POINT_SIZE"] = FT800EMU_DL_POINT_SIZE;
		s_IdMap["LINE_WIDTH"] = FT800EMU_DL_LINE_WIDTH;
		s_IdMap["CLEAR_COLOR_A"] = FT800EMU_DL_CLEAR_COLOR_A;
		s_IdMap["COLOR_A"] = FT800EMU_DL_COLOR_A;
		s_IdMap["CLEAR_STENCIL"] = FT800EMU_DL_CLEAR_STENCIL;
		s_IdMap["CLEAR_TAG"] = FT800EMU_DL_CLEAR_TAG;
		s_IdMap["STENCIL_MASK"] = FT800EMU_DL_STENCIL_MASK;
		s_IdMap["TAG_MASK"] = FT800EMU_DL_TAG_MASK;
		s_IdMap["BITMAP_TRANSFORM_A"] = FT800EMU_DL_BITMAP_TRANSFORM_A;
		s_IdMap["BITMAP_TRANSFORM_B"] = FT800EMU_DL_BITMAP_TRANSFORM_B;
		s_IdMap["BITMAP_TRANSFORM_C"] = FT800EMU_DL_BITMAP_TRANSFORM_C;
		s_IdMap["BITMAP_TRANSFORM_D"] = FT800EMU_DL_BITMAP_TRANSFORM_D;
		s_IdMap["BITMAP_TRANSFORM_E"] = FT800EMU_DL_BITMAP_TRANSFORM_E;
		s_IdMap["BITMAP_TRANSFORM_F"] = FT800EMU_DL_BITMAP_TRANSFORM_F;
		s_IdMap["SCISSOR_XY"] = FT800EMU_DL_SCISSOR_XY;
		s_IdMap["SCISSOR_SIZE"] = FT800EMU_DL_SCISSOR_SIZE;
		s_IdMap["CALL"] = FT800EMU_DL_CALL;
		s_IdMap["JUMP"] = FT800EMU_DL_JUMP;
		s_IdMap["BEGIN"] = FT800EMU_DL_BEGIN;
		s_IdMap["COLOR_MASK"] = FT800EMU_DL_COLOR_MASK;
		s_IdMap["END"] = FT800EMU_DL_END;
		s_IdMap["SAVE_CONTEXT"] = FT800EMU_DL_SAVE_CONTEXT;
		s_IdMap["RESTORE_CONTEXT"] = FT800EMU_DL_RESTORE_CONTEXT;
		s_IdMap["RETURN"] = FT800EMU_DL_RETURN;
		s_IdMap["MACRO"] = FT800EMU_DL_MACRO;
		s_IdMap["CLEAR"] = FT800EMU_DL_CLEAR;
	}
}

void DlParser::parse(DlParsed &parsed, const QString &line)
{
	init();
	
	QByteArray chars = line.toLatin1();
	const char *src = chars.constData();
	const int len = chars.size();
	
	parsed.BadCharacterIndex = -1;
	
	parsed.ValidId = false;
	parsed.IdIndex = 0;
	parsed.IdLength = 0;
	std::stringstream idss;
	bool failId = false;
	
	int i = 0;
	
	for (; ; ++i)
	{
		if (i < len)
		{
			char c = src[i];
			if (c >= 'a' && c <= 'z') 
			{
				c = c - 'a' + 'A'; /* uppercase */
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
			else if (c == '(')
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
	}
	else if (parsed.IdText == "VERTEX2II")
	{
		parsed.IdLeft = FT800EMU_DL_VERTEX2II;
		parsed.ValidId = true;
	}
	else
	{
		std::map<std::string, int>::iterator it = s_IdMap.find(parsed.IdText);
		if (it != s_IdMap.end())
		{
			parsed.IdRight = it->second;
			parsed.ValidId = true;
		}
	}
	
	if (failId)
	{
		return;
	}
	
	// todo loop over parameters text-
}

} /* namespace FT800EMUQT */

/* end of file */
