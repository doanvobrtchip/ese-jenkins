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
#include <string>

// Qt includes
#include <QString>

// Emulator includes

// Project includes


namespace FT800EMUQT {
	
struct DlParsed
{
	std::string IdText;
	int IdLeft;
	int IdRight;
	int Parameter[8];
	
	bool ValidId;
	bool ValidParameterCount;
	bool ValidParameter[8];
	
	int IdIndex;
	int IdLength;
	int ParameterIndex[8];
	int ParameterLength[8];
	
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
	
}; /* class DlParser */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_PARSER_H */

/* end of file */
