/**
 * interactive_properties.h
 * $Id$
 * \file interactive_properties.h
 * \brief interactive_properties.h
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INTERACTIVE_PROPERTIES_H
#define FT800EMUQT_INTERACTIVE_PROPERTIES_H

// STL includes
#include <vector>

// Qt includes
#include <QGroupBox>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

namespace FT800EMUQT {

class MainWindow;
class DlEditor;

/**
 * InteractiveProperties
 * \brief InteractiveProperties
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */
class InteractiveProperties : public QGroupBox
{
	Q_OBJECT

public:
	InteractiveProperties(MainWindow *parent);
	virtual ~InteractiveProperties();

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void modifiedEditorLine();

private:
	MainWindow *m_MainWindow;

	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

private:
	InteractiveProperties(const InteractiveProperties &);
	InteractiveProperties &operator=(const InteractiveProperties &);

}; /* class InteractiveProperties */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INTERACTIVE_PROPERTIES_H */

/* end of file */
