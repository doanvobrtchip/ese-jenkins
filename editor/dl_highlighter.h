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

#ifndef FT800EMUQT_DL_HIGHLIGHTER_H
#define FT800EMUQT_DL_HIGHLIGHTER_H

// STL includes

// Qt includes
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
class QTextDocument;

// Emulator includes

// Project includes


namespace FT800EMUQT {

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
	DlHighlighter(QTextDocument *parent = 0);
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
	
}; /* class DlHighlighter */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_HIGHLIGHTER_H */

/* end of file */
