/**
 * DlEditor.h
 * $Id$
 * \file DlEditor.h
 * \brief DlEditor.h
 * \date 2013-11-05 09:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_DL_EDITOR_H
#define FT800EMUQT_DL_EDITOR_H

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes

// Project includes

class CodeEditor;

namespace FT800EMUQT {

/**
 * DlEditor
 * \brief DlEditor
 * \date 2010-02-05 20:27GMT
 * \author Jan Boon (Kaetemi)
 */
class DlEditor : public QWidget
{
	Q_OBJECT
	
public:
	DlEditor(QWidget *parent);
	virtual ~DlEditor();

// private slots:
//	void returnPressed();

private:
	CodeEditor *m_CodeEditor;

private:
	DlEditor(const DlEditor &);
	DlEditor &operator=(const DlEditor &);
	
}; /* class DlEditor */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_EDITOR_H */

/* end of file */
