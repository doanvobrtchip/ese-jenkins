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

DlHighlighter::DlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
	errorFormat.setForeground(Qt::red);
	badIdFormat.setForeground(Qt::magenta);
	idFormat.setForeground(Qt::blue);
	paramFormat.setForeground(Qt::green);
}

DlHighlighter::~DlHighlighter()
{

}

void DlHighlighter::highlightBlock(const QString &text)
{
	DlParsed parsed;
	DlParser::parse(parsed, text);
	// setFormat
	if (parsed.IdLength)
	{
		setFormat(parsed.IdIndex, parsed.IdLength, parsed.ValidId ? idFormat : badIdFormat);
	}
	
	
	if (parsed.BadCharacterIndex >= 0)
	{
		setFormat(parsed.BadCharacterIndex, 1, errorFormat);
	}
}

} /* namespace FT800EMUQT */

/* end of file */
