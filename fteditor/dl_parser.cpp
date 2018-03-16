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
#include <QRegularExpression>

// Emulator includes

// Project includes

namespace FTEDITOR {

const std::map<std::string, int> *DlParser::m_IdMap[FTEDITOR_DEVICE_NB];
const std::map<std::string, int> *DlParser::m_ParamMap[FTEDITOR_DEVICE_NB];

const std::map<std::string, int> *DlParser::m_CmdIdMap[FTEDITOR_DEVICE_NB];
const std::map<std::string, int> *DlParser::m_CmdParamMap[FTEDITOR_DEVICE_NB];

const int *DlParser::m_ParamCount[FTEDITOR_DEVICE_NB];
const int *DlParser::m_CmdParamCount[FTEDITOR_DEVICE_NB];
const bool *DlParser::m_CmdParamString[FTEDITOR_DEVICE_NB];

const std::string *DlParser::m_CmdIdList[FTEDITOR_DEVICE_NB];

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

const char *g_DlEnumSwizzle[DL_ENUM_SWIZZLE_NB] = {
	"ZERO", 
	"ONE", 
	"RED",
	"GREEN",
	"BLUE",
	"ALPHA",
};

const char *g_DlEnumAnimLoop[DL_ENUM_ANIM_LOOP_NB] = {
	"ANIM_ONCE",
	"ANIM_LOOP",
	"ANIM_HOLD",
};

void DlParser::init()
{
	initVC1();
	initVC2();
	initVC3();
}

void DlParser::getIdentifiers(int deviceIntf, QStringList &list, bool coprocessor)
{
	init();

	for (std::map<std::string, int>::const_iterator it = m_IdMap[deviceIntf]->begin(), end = m_IdMap[deviceIntf]->end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
	}

	list.push_back("VERTEX2II");
	list.push_back("VERTEX2F");

	if (coprocessor)
	{
		for (std::map<std::string, int>::const_iterator it = m_CmdIdMap[deviceIntf]->begin(), end = m_CmdIdMap[deviceIntf]->end(); it != end; ++it)
		{
			list.push_back(QString(it->first.c_str()));
		}
	}
}

void DlParser::getParams(int deviceIntf, QStringList &list, bool coprocessor)
{
	init();

	for (std::map<std::string, int>::const_iterator it = m_ParamMap[deviceIntf]->begin(), end = m_ParamMap[deviceIntf]->end(); it != end; ++it)
	{
		list.push_back(QString(it->first.c_str()));
	}

	if (coprocessor)
	{
		for (std::map<std::string, int>::const_iterator it = m_CmdParamMap[deviceIntf]->begin(), end = m_CmdParamMap[deviceIntf]->end(); it != end; ++it)
		{
			list.push_back(QString(it->first.c_str()));
		}
	}
}

void DlParser::parse(int deviceIntf, DlParsed &parsed, const QString &line, bool coprocessor, bool dynamic)
{
	init();

	QByteArray chars = line.toLatin1();
	const char *src = chars.constData();
	const int len = chars.size();

	parsed.BadCharacterIndex = -1;

	for (int p = 0; p < DLPARSED_MAX_SYMBOL; ++p)
	{
		parsed.ValidSymbol[p] = false;
		parsed.NumericSymbol[p] = true;
		parsed.SymbolLength[p] = 0;
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
	parsed.StringParameterAt = DLPARSED_MAX_SYMBOL;

	if (parsed.IdText == "VERTEX2F")
	{
		parsed.IdLeft = FTEDITOR_DL_VERTEX2F;
		parsed.ValidId = true;
		parsed.ExpectedParameterCount = 2;
	}
	else if (parsed.IdText == "VERTEX2II")
	{
		parsed.IdLeft = FTEDITOR_DL_VERTEX2II;
		parsed.ValidId = true;
		parsed.ExpectedParameterCount = 4;
	}
	else
	{
		std::map<std::string, int>::const_iterator it = m_IdMap[deviceIntf]->find(parsed.IdText);
		if (it != m_IdMap[deviceIntf]->end())
		{
			parsed.IdRight = it->second;
			parsed.ValidId = true;
			parsed.ExpectedParameterCount = m_ParamCount[deviceIntf][parsed.IdRight];
		}
		if (coprocessor)
		{
			it = m_CmdIdMap[deviceIntf]->find(parsed.IdText);
			if (it != m_CmdIdMap[deviceIntf]->end())
			{
				parsed.IdLeft = FTEDITOR_CO_COMMAND;
				parsed.IdRight = it->second;
				parsed.ValidId = true;
				parsed.ExpectedParameterCount = m_CmdParamCount[deviceIntf][parsed.IdRight];
				parsed.ExpectedStringParameter = m_CmdParamString[deviceIntf][parsed.IdRight];
			}
		}
	}

	ParameterOptions *defaultParam;
	if (parsed.ValidId)
	{
		switch (parsed.IdLeft)
		{
		case FTEDITOR_DL_INSTRUCTION:
			defaultParam = (deviceIntf >= FTEDITOR_BT815) ? defaultParamVC3()
				: ((deviceIntf >= FTEDITOR_FT810) ? defaultParamVC2() : defaultParamVC1());
			break;
		case FTEDITOR_CO_COMMAND:
			defaultParam = (deviceIntf >= FTEDITOR_BT815) ? defaultCmdParamVC3()
				: ((deviceIntf >= FTEDITOR_FT810) ? defaultCmdParamVC2() : defaultCmdParamVC1());
			break;
		default:
			defaultParam = NULL;
			break;
		}
	}
	else
	{
		defaultParam = NULL;
	}

	int p = 0;
	if (!failId)
	{
		// for each possible parameter
		bool failParam = false;
		int finalIndex = -1;
		int pq;
		for (pq = 0; p < DLPARSED_MAX_PARAMETER && pq < DLPARSED_MAX_SYMBOL; ++p, ++pq)
		{
			bool combineParameter = false; // temporary method for using | operator // CMD_CLOCK(100, 100, 50, OPT_FLAT | OPT_NOTICKS, 0, 0, 0, 0), pq is a TEMPORARY trick that shifts the actual parameters from the metadata
		CombineParameter:
			bool hexadecimal = false;
			bool combinedParameter = combineParameter;
			combineParameter = false;
			parsed.SymbolIndex[pq] = i;
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
					if (parsed.SymbolLength[pq] == 0 && (c == ' ' || c == '\t'))
					{
						++parsed.SymbolIndex[pq]; /* pre-trim */
					}
					else if (parsed.SymbolLength[pq] == 0 && (c == '"') && ((p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter) || dynamic))
					{
						/* begin string, only works on last parameter */ // CMD_TEXT(50, 119, 31, 0, "hello world")
						pss << QString(line[i]).toUtf8().constData(); // pss << c;
						++i;
						goto ParseString;
					}
					else if (parsed.SymbolLength[pq] == 0 && (c == '\'') && ((!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)) || dynamic))
					{
						pss << c;
						++i;
						goto ParseChar;
					}
					else if (parsed.SymbolLength[pq] == 0 && (c == '-'))
					{
						pss << c;
						++parsed.SymbolLength[pq];
					}
					else if (((c >= '0' && c <= '9') || (hexadecimal && (c >= 'A' && c <= 'F'))) && parsed.SymbolIndex[pq] + parsed.SymbolLength[pq] == i && ((!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)) || dynamic))
					{
						pss << c;
						++parsed.SymbolLength[pq];
					}
					else if (parsed.SymbolLength[pq] == 1 && src[i - 1] == '0' && (c == 'X') && ((!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)) || dynamic))
					{
						pss.clear();
						hexadecimal = true;
						++parsed.SymbolLength[pq];
						pss << std::hex;
					}
					else if (((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')) && parsed.SymbolIndex[pq] + parsed.SymbolLength[pq] == i  && ((!(p == (parsed.ExpectedParameterCount - 1) && parsed.ExpectedStringParameter)) || dynamic))
					{
						parsed.NumericSymbol[pq] = false;
						pss << c;
						++parsed.SymbolLength[pq];
					}
					else if (c == ' ' || c == '\t')
					{
						/* post-trim */
					}
					else if (parsed.SymbolLength[pq] > 0 && c == ',')
					{
						/* valid, more, continue */
						++i;
						break;
					}
					else if (parsed.SymbolLength[pq] > 0 && c == '|')
					{
						/* valid, more, continue */
						++i;
						combineParameter = true;
						break;
					}
					else if (((p == 0) || (parsed.SymbolLength[pq] > 0)) && c == ')')
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
							// c = src[i];
							pss << QString(line[i]).toUtf8().constData(); // pss << c;
						}
						else
						{
							pss << '\\';
						}
					}
					else if (c == '"')
					{
						// end (post-trim)
						pss << QString(line[i]).toUtf8().constData(); // pss << c;
						++i;
						parsed.SymbolLength[pq] = i - parsed.SymbolIndex[pq];
						goto ContinueParameter;
					}
					else
					{
						pss << QString(line[i]).toUtf8().constData(); // pss << c;
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
						parsed.SymbolLength[pq] = i - parsed.SymbolIndex[pq];
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
			if ((p < parsed.ExpectedParameterCount || dynamic) || !parsed.ValidId)
			{
				bool validateInt = false;
				std::string ps = pss.str();
				if ((p == (parsed.ExpectedParameterCount - 1) || dynamic) && parsed.ExpectedStringParameter)
				{
					// CMD_TEXT(50, 119, 31, 0, "hello world")
					if (ps.length())
					{
						std::string psubstr = ps.substr(1, ps.size() - 2);
						std::string psstr;
						unescapeString(psstr, psubstr);
						parsed.Parameter[p].U = 0;
						parsed.StringParameter = psstr;
						parsed.ValidSymbol[pq] = true;
						parsed.StringParameterAt = pq;
					}
				}
				else if (ps.length() > 0 && ps[0] == '\'')
				{
					std::string psstr;
					unescapeString(psstr, ps);
					int vchar = 0;
					for (int ci = (int)psstr.length() - 2; ci > 0; --ci)
					{
						vchar <<= 8;
						vchar |= psstr[ci];
					}
					parsed.Parameter[p].I = (combinedParameter ? parsed.Parameter[p].I : 0) | vchar;
					parsed.ValidSymbol[pq] = true;
					validateInt = true;
				}
				else if (hexadecimal && parsed.NumericSymbol[pq] && ps.length() > 0)
				{
					unsigned int vhex;
					pss >> vhex;
					parsed.Parameter[p].U = (combinedParameter ? parsed.Parameter[p].U : 0) | vhex;
					parsed.ValidSymbol[pq] = true;
					validateInt = true;
				}
				else if (parsed.NumericSymbol[pq] && ps.length() > 0)
				{
					parsed.Parameter[p].I = (combinedParameter ? parsed.Parameter[p].I : 0) | atoi(ps.c_str());
					parsed.ValidSymbol[pq] = true;
					validateInt = true;
				}
				else
				{
					// Non-uppercase constant workaround ->
					// (COMPRESSED_RGBA_ASTC_4x4_KHR)
					if (ps.length() > 23)
					{
						if (ps[22] == 'X') ps[22] = 'x';
						else if (ps[23] == 'X') ps[23] = 'x';
					}
					// <-
					std::map<std::string, int>::const_iterator it = m_ParamMap[deviceIntf]->find(ps);
					if (it != m_ParamMap[deviceIntf]->end())
					{
						parsed.Parameter[p].U = (combinedParameter ? parsed.Parameter[p].U : 0) | it->second;
						parsed.ValidSymbol[pq] = true;
					}
					else if (coprocessor)
					{
						it = m_CmdParamMap[deviceIntf]->find(ps);
						if (it != m_CmdParamMap[deviceIntf]->end())
						{
							parsed.Parameter[p].U = (combinedParameter ? parsed.Parameter[p].U : 0) | it->second;
							parsed.ValidSymbol[pq] = true;
						}
						else
						{
							parsed.Parameter[p].I = defaultParam ? defaultParam[parsed.IdRight].Default[p] : 0;
						}
					}
					else
					{
						parsed.Parameter[p].I = defaultParam ? defaultParam[parsed.IdRight].Default[p] : 0;
					}
				}
				if (validateInt && parsed.ValidSymbol[pq] && defaultParam)
				{
					parsed.ValidSymbol[pq] =
						((parsed.Parameter[p].I & defaultParam[parsed.IdRight].Mask[p]) == parsed.Parameter[p].I)
						&& (parsed.Parameter[p].I >= defaultParam[parsed.IdRight].Min[p]
						&& parsed.Parameter[p].I <= defaultParam[parsed.IdRight].Max[p]);
				}
			}
			else
			{
				parsed.Parameter[p].I = defaultParam ? defaultParam[parsed.IdRight].Default[p] : 0;
			}

			if (finalIndex >= 0)
			{
				if (parsed.BadCharacterIndex == -1)
				{
					if ((p + 1 < parsed.ExpectedParameterCount) && !dynamic)
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
				// parsed.SymbolLength[p] = 0;
				++pq;
				goto CombineParameter;
			}
		}
	}
	
	// Clear unfilled to be safe
	if (parsed.ValidId)
	{
		if (!failId) ++p;
		if (dynamic)
		{
			parsed.ExpectedParameterCount = p;
		}
		else
		{
			if (defaultParam)
			{
				for (; p < parsed.ExpectedParameterCount && p < DLPARSED_MAX_PARAMETER; ++p)
					parsed.Parameter[p].I = defaultParam[parsed.IdRight].Default[p];
			}
			else
			{
				for (; p < parsed.ExpectedParameterCount && p < DLPARSED_MAX_PARAMETER; ++p)
					parsed.Parameter[p].U = 0;
			}
		}
	}
}

uint32_t DlParser::compile(int deviceIntf, const DlParsed &parsed)
{
	if (deviceIntf >= FTEDITOR_BT815)
		return compileVC3(deviceIntf, parsed);
	else if (deviceIntf >= FTEDITOR_FT810)
		return compileVC2(deviceIntf, parsed);
	else
		return compileVC1(deviceIntf, parsed);
}

void DlParser::compile(int deviceIntf, std::vector<uint32_t> &compiled, const DlParsed &parsed) // compile CMD parameters
{
	if (deviceIntf >= FTEDITOR_BT815)
		compileVC3(deviceIntf, compiled, parsed);
	else if (deviceIntf >= FTEDITOR_FT810)
		compileVC2(deviceIntf, compiled, parsed);
	else
		compileVC1(deviceIntf, compiled, parsed);
}

void DlParser::toString(int deviceIntf, std::string &dst, uint32_t v)
{
	if (deviceIntf >= FTEDITOR_BT815)
		toStringVC3(deviceIntf, dst, v);
	else if (deviceIntf >= FTEDITOR_FT810)
		toStringVC2(deviceIntf, dst, v);
	else
		toStringVC1(deviceIntf, dst, v);
}

QString DlParser::toString(int deviceIntf, uint32_t v)
{
	std::string str;
	toString(deviceIntf, str, v);
	return QString(str.c_str());
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

void DlParser::toString(int deviceIntf, std::string &dst, const DlParsed &parsed)
{
	if (parsed.IdLeft == FTEDITOR_CO_COMMAND) // Coprocessor
	{
		if (deviceIntf >= FTEDITOR_BT815)
			toStringVC3(deviceIntf, dst, parsed);
		else if (deviceIntf >= FTEDITOR_FT810)
			toStringVC2(deviceIntf, dst, parsed);
		else
			toStringVC1(deviceIntf, dst, parsed);
	}
	else
	{
		uint32_t compiled = DlParser::compile(deviceIntf, parsed);
		DlParser::toString(deviceIntf, dst, compiled);
	}
}

QString DlParser::toString(int deviceIntf, const DlParsed &parsed)
{
	std::string str;
	toString(deviceIntf, str, parsed);
	return QString(str.c_str());
}

static int isCharUtf8(const char *s, size_t len)
{
	if (len > 0)
	{
		char c0 = s[0];
		if ((c0 & '\x80') == '\x00')
		{
			return 1;
		}
		if (len > 1)
		{
			char c1 = s[1];
			if ((c1 & '\xC0') == '\x80')
			{
				if ((c0 & '\xE0') == '\xC0')
				{
					return 2;
				}
				if (len > 2)
				{
					char c2 = s[2];
					if ((c2 & '\xC0') == '\x80')
					{
						if ((c0 & '\xF0') == '\xE0')
						{
							return 3;
						}
						if (len > 3)
						{
							char c3 = s[3];
							if (((c0 & '\xF8') == '\xF0')
								&& ((c3 & '\xC0') == '\x80'))
							{
								return 4;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

void DlParser::escapeString(std::string &dst, const std::string &src)
{
	std::stringstream res;
	for (size_t c = 0; c < src.size(); ++c)
	{
		unsigned char ch = src[c];
		if (ch == '\t') res << "\\t";
		else if (ch == '\\') res << "\\\\";
		else if (ch == '\n') res << "\\n";
		else if (ch == '\r') res << "\\r";
		else if (ch >= 32 && ch <= 126) res << ch;
		else if (int nb = isCharUtf8(&src[c], src.size() - c))
		{
			res << ch;
			for (int i = 1; i < nb; ++i)
			{
				++c;
				res << src[c];
			}
		}
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
	for (size_t i = 0; i < src.size(); ++i)
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
				case 'u':
				{
					if (i + 4 < src.size())
					{
						QString tmp = QString::fromStdString(src.substr(i + 1, 4));
						QRegularExpression hexMatcher("^[0-9A-F]{4}$", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = hexMatcher.match(tmp);
						if (match.hasMatch())
						{
							bool ok;
							unsigned int v = match.captured(0).toUInt(&ok, 16);
							if (ok)
							{
								res << QString(v).toStdString();
								i += 4;
								break;
							}
						}
					}

					res << "\\u";
					break;
				}
				case 'U':
				{
					if (i + 8 < src.size())
					{
						QString tmp = QString::fromStdString(src.substr(i + 1, 8));
						QRegularExpression hexMatcher("^[0-9A-F]{8}$", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = hexMatcher.match(tmp);
						if (match.hasMatch())
						{
							bool ok;
							unsigned int v = match.captured(0).toUInt(&ok, 16);
							if (ok)
							{
								res << QString(v).toStdString();
								i += 8;
								break;
							}
						}
					}

					res << "\\U";
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

} /* namespace FTEDITOR */

/* end of file */
