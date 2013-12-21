/**
 * dl_highlighter.cpp
 * $Id$
 * \file dl_highlighter.cpp
 * \brief dl_highlighter.cpp
 * \date 2013-11-06 06:55GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "dl_highlighter.h"

// STL includes

// Qt includes

// Emulator includes

// Project includes
#include "dl_parser.h"

using namespace std;

namespace FT800EMUQT {

DlHighlighter::DlHighlighter(QTextDocument *parent, bool coprocessor) : QSyntaxHighlighter(parent), m_ModeCoprocessor(coprocessor)
{
	errorFormat.setForeground(Qt::red);
	badIdFormat.setForeground(Qt::magenta);
	idFormat.setForeground(Qt::darkBlue);
	paramFormat.setForeground(Qt::darkCyan);
	numberFormat.setForeground(Qt::darkGreen);
	stringFormat.setForeground(QColor(255, 128, 0)); // orange
}

DlHighlighter::~DlHighlighter()
{

}

void DlHighlighter::highlightBlock(const QString &text)
{
	DlParsed parsed;
	DlParser::parse(parsed, text, m_ModeCoprocessor);
	// setFormat
	if (parsed.IdLength)
	{
		setFormat(parsed.IdIndex, parsed.IdLength, parsed.ValidId ? idFormat : badIdFormat);
	}

	for (int p = 0; p < DLPARSED_MAX_PARAMETER; ++p)
	{
		if (parsed.ParameterLength[p])
		{
			if (p == parsed.StringParameterAt)
			{
				setFormat(parsed.ParameterIndex[p], parsed.ParameterLength[p],
					(parsed.ValidParameter[p] ? stringFormat : errorFormat));
			}
			else
			{
				setFormat(parsed.ParameterIndex[p], parsed.ParameterLength[p],
					parsed.NumericParameter[p]
						? (parsed.ValidParameter[p] ? numberFormat : errorFormat)
						: (parsed.ValidParameter[p] ? paramFormat : badIdFormat));
			}
		}
	}

	if (parsed.BadCharacterIndex >= 0)
	{
		setFormat(parsed.BadCharacterIndex, 1, errorFormat);
	}
}

} /* namespace FT800EMUQT */

/* end of file */
