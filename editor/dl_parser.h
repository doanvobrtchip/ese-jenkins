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
	
	static void parse(DlParsed &parsed, const QString &line);	
	static uint32_t compile(const DlParsed &parsed);
	static void toString(std::string &dst, uint32_t v);
	static QString toString(uint32_t v);
	
}; /* class DlParser */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_PARSER_H */

/* end of file */
