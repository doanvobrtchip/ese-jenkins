/**
 * interactive_properties.cpp
 * $Id$
 * \file interactive_properties.cpp
 * \brief interactive_properties.cpp
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "interactive_properties.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>

// Emulator includes
#include <vc.h>

// Project includes
#include "main_window.h"
#include "dl_editor.h"

using namespace std;

namespace FT800EMUQT {

InteractiveProperties::InteractiveProperties(MainWindow *parent) : QWidget(parent), m_MainWindow(parent),
	m_LineEditor(NULL), m_LineNumber(0)
{
	QVBoxLayout *layout = new QVBoxLayout();
	// ...
}

InteractiveProperties::~InteractiveProperties()
{

}

void InteractiveProperties::setEditorLine(DlEditor *editor, int line)
{
	m_LineNumber = line;
	if (editor != m_LineEditor)
	{
		m_LineEditor = editor;
		if (editor)
		{
			// ...
		}
		else
		{
			// ...
		}
	}
}

void InteractiveProperties::unsetEditorLine()
{
	m_LineEditor = NULL;
	// ...
}

} /* namespace FT800EMUQT */

/* end of file */
