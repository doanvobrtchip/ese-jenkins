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

#ifndef FTEDITOR_DL_PARSER_H
#define FTEDITOR_DL_PARSER_H

// STL includes
#include <vector>
#include <map>
#include <string>

// Qt includes
#include <QObject>
#include <QString>

// Emulator includes
#include "ft8xxemu_inttypes.h"

// Project includes
#include "constant_mapping.h"

namespace FTEDITOR {

#define DL_ENUM_BLEND_NB 6
extern const char *g_DlEnumBlend[DL_ENUM_BLEND_NB];

#define DL_ENUM_COMPARE_NB 8
extern const char *g_DlEnumCompare[DL_ENUM_COMPARE_NB];

#define DL_ENUM_STENCIL_NB 6
extern const char *g_DlEnumStencil[DL_ENUM_STENCIL_NB];

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

struct ParameterArray
{
	int Values[DLPARSED_MAX_PARAMETER];
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

	static void getIdentifiers(int deviceIntf, QStringList &list, bool coprocessor = false);
	static void getParams(int deviceIntf, QStringList &list, bool coprocessor = false);

	static void parse(int deviceIntf, DlParsed &parsed, const QString &line, bool coprocessor = false, bool dynamic = false);
	static uint32_t compile(int deviceIntf, const DlParsed &parsed); // compile DL & cmd (cmd returns just identifier)
	static void compile(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed); // compile CMD parameters
	static void toString(int deviceIntf, std::string &dst, uint32_t v); // DL only
	static QString toString(int deviceIntf, uint32_t v); // DL only
	static void toString(int deviceIntf, std::string &dst, const DlParsed &parsed); // DL and CMD
	static QString toString(int deviceIntf, const DlParsed &parsed); // DL and CMD

	static void escapeString(std::string &dst, const std::string &src);
	static void unescapeString(std::string &dst, const std::string &src);

private:
	static void initVC1();
	static void initVC2();
	static uint32_t compileVC1(int deviceIntf, const DlParsed &parsed); // compile DL & cmd (cmd returns just identifier)
	static uint32_t compileVC2(int deviceIntf, const DlParsed &parsed); // compile DL & cmd (cmd returns just identifier)
	static void compileVC1(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed); // compile CMD parameters
	static void compileVC2(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed); // compile CMD parameters
	static void toStringVC1(int deviceIntf, std::string &dst, uint32_t v); // DL only
	static void toStringVC2(int deviceIntf, std::string &dst, uint32_t v); // DL only
	static void toStringVC1(int deviceIntf, std::string &dst, const DlParsed &parsed); // DL and CMD
	static void toStringVC2(int deviceIntf, std::string &dst, const DlParsed &parsed); // DL and CMD
	static ParameterArray *defaultParamVC1();
	static ParameterArray *defaultParamVC2();
	static ParameterArray *defaultCmdParamVC1();
	static ParameterArray *defaultCmdParamVC2();

	static const std::map<std::string, int> *m_IdMap[FTEDITOR_DEVICE_NB];
	static const std::map<std::string, int> *m_ParamMap[FTEDITOR_DEVICE_NB];

	static const std::map<std::string, int> *m_CmdIdMap[FTEDITOR_DEVICE_NB];
	static const std::map<std::string, int> *m_CmdParamMap[FTEDITOR_DEVICE_NB];

	static const int *m_ParamCount[FTEDITOR_DEVICE_NB];
	static const int *m_CmdParamCount[FTEDITOR_DEVICE_NB];
	static const bool *m_CmdParamString[FTEDITOR_DEVICE_NB];

	static const std::string *m_CmdIdList[FTEDITOR_DEVICE_NB];

}; /* class DlParser */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DL_PARSER_H */

/* end of file */
