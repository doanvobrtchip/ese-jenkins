/**
 * dl_highlighter.h
 * $Id$
 * \file dl_highlighter.h
 * \brief dl_highlighter.h
 * \date 2013-11-06 06:55GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_DL_HIGHLIGHTER_H
#define FTEDITOR_DL_HIGHLIGHTER_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes

// Qt includes
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
class QTextDocument;

// Emulator includes

// Project includes


namespace FTEDITOR {

/**
 * DlHighlighter
 * \brief DlHighlighter
 * \date 2013-11-06 06:55GMT
 * \author Jan Boon (Kaetemi)
 */
class DlHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	DlHighlighter(QTextDocument *parent = 0, bool coprocessor = false);
	virtual ~DlHighlighter();

protected:
	virtual void highlightBlock(const QString &text);

private:
	DlHighlighter(const DlHighlighter &);
	DlHighlighter &operator=(const DlHighlighter &);

	QTextCharFormat errorFormat;
	QTextCharFormat badIdFormat;
	QTextCharFormat idFormat;
	QTextCharFormat paramFormat;
	QTextCharFormat numberFormat;
	QTextCharFormat stringFormat;

	bool m_ModeCoprocessor;

}; /* class DlHighlighter */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DL_HIGHLIGHTER_H */

/* end of file */
