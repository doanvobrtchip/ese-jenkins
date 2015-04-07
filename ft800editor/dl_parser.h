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
#include <vector>

// Qt includes
#include <QObject>
#include <QString>

// Emulator includes
#include "ft8xxemu_inttypes.h"
#include "ft800emu_vc.h"

// Project includes

#define RAM_G_MAX (256 * 1024)

namespace FT800EMUQT {

#define DL_ENUM_BLEND_NB 6
extern const char *g_DlEnumBlend[DL_ENUM_BLEND_NB];

#define DL_ENUM_COMPARE_NB 8
extern const char *g_DlEnumCompare[DL_ENUM_COMPARE_NB];

#define DL_ENUM_STENCIL_NB 6
extern const char *g_DlEnumStencil[DL_ENUM_STENCIL_NB];

#ifdef FT810EMU_MODE
#define DL_ENUM_BITMAP_FORMAT_NB 17
#else
#define DL_ENUM_BITMAP_FORMAT_NB 12
#endif
#define DL_ENUM_BITMAP_FORMAT_NB_BASIC (DL_ENUM_BITMAP_FORMAT_NB - 3)
extern const char *g_DlEnumBitmapFormat[DL_ENUM_BITMAP_FORMAT_NB];

#define DL_ENUM_BITMAP_FILTER_NB 2
extern const char *g_DlEnumBitmapFilter[DL_ENUM_BITMAP_FILTER_NB];

#define DL_ENUM_BITMAP_WRAP_NB 2
extern const char *g_DlEnumBitmapWrap[DL_ENUM_BITMAP_WRAP_NB];

#define DL_ENUM_PRIMITIVE_NB 10
extern const char *g_DlEnumPrimitive[DL_ENUM_PRIMITIVE_NB];

#define DLPARSED_MAX_PARAMETER 12
struct DlParsed
{
	std::string IdText;
	int IdLeft;
	int IdRight;
	union { uint32_t U; int I; } Parameter[DLPARSED_MAX_PARAMETER];

	bool ValidId;
	bool ValidParameter[DLPARSED_MAX_PARAMETER];
	bool NumericParameter[DLPARSED_MAX_PARAMETER];

	int IdIndex;
	int IdLength;
	int ParameterIndex[DLPARSED_MAX_PARAMETER];
	int ParameterLength[DLPARSED_MAX_PARAMETER];

	int ExpectedParameterCount;
	int BadCharacterIndex;
	bool ExpectedStringParameter;

	bool ValidStringParameter; // single string parameter at end
	std::string StringParameter;
	int StringParameterAt; // temporary pq
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

	static void getIdentifiers(QStringList &list, bool coprocessor = false);
	static void getParams(QStringList &list, bool coprocessor = false);

	static void parse(DlParsed &parsed, const QString &line, bool coprocessor = false);
	static uint32_t compile(const DlParsed &parsed); // compile DL & cmd (cmd returns just identifier)
	static void compile(std::vector<uint32_t> &compiled, const DlParsed &parsed); // compile CMD parameters
	static void toString(std::string &dst, uint32_t v); // DL only
	static QString toString(uint32_t v); // DL only
	static void toString(std::string &dst, const DlParsed &parsed); // DL and CMD
	static QString toString(const DlParsed &parsed); // DL and CMD

	static void escapeString(std::string &dst, const std::string &src);
	static void unescapeString(std::string &dst, const std::string &src);

}; /* class DlParser */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_PARSER_H */

/* end of file */
