/**
 * toolbox.h
 * $Id$
 * \file toolbox.h
 * \brief toolbox.h
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_TOOLBOX_H
#define FT800EMUQT_TOOLBOX_H

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

class QListWidget;

namespace FT800EMUQT {

class MainWindow;
class DlEditor;

/**
 * Toolbox
 * \brief Toolbox
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */
class Toolbox : public QWidget
{
	Q_OBJECT

public:
	Toolbox(MainWindow *parent);
	virtual ~Toolbox();

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void unsetEditorLine();

	QListWidget *listWidget() { return m_Tools; }

	uint32_t getSelection();

private:
	MainWindow *m_MainWindow;
	QListWidget *m_Tools;

	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

private:
	Toolbox(const Toolbox &);
	Toolbox &operator=(const Toolbox &);

}; /* class Toolbox */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_TOOLBOX_H */

/* end of file */
