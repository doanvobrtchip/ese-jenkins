/**
 * dl_parser.h
 * $Id$
 * \file dl_parser.h
 * \brief dl_parser.h
 * \date 2013-11-06 08:55GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_DL_PARSER_H
#define FT800EMUQT_DL_PARSER_H

// STL includes

// Qt includes
#include <QString>

// Emulator includes
#define NOMINMAX
#include "ft800emu_inttypes.h"

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
	
struct DlParsed
{
	std::string IdText;
	int IdLeft;
	int IdRight;
	uint32_t Parameter[8];
	
	bool ValidId;
	bool ValidParameter[8];
	bool NumericParameter[8];
	
	int IdIndex;
	int IdLength;
	int ParameterIndex[8];
	int ParameterLength[8];
	
	int ExpectedParameterCount;
	int BadCharacterIndex;
};

/**
 * DlParser
 * \brief DlParser
 * \date 2013-11-06 08:55GMT
 * \author Jan Boon (Kaetemi)
 */
class DlParser
{	
public:
	static void init();
	
	static void getIdentifiers(QStringList &list);
	static void getParams(QStringList &list);
	
	static void parse(DlParsed &parsed, const QString &line);	
	static uint32_t compile(const DlParsed &parsed);
	static void toString(std::string &dst, uint32_t v);
	static QString toString(uint32_t v);
	
}; /* class DlParser */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_PARSER_H */

/* end of file */
