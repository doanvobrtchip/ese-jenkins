/**
 * dl_editor.cpp
 * $Id$
 * \file dl_editor.cpp
 * \brief dl_editor.cpp
 * \date 2013-11-05 09:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "dl_editor.h"

// STL includes

// Qt includes
#include <QVBoxLayout>

// Emulator includes

// Project includes
#include "code_editor.h"

using namespace std;

namespace FT800EMUQT {

DlEditor::DlEditor(QWidget *parent) : QWidget(parent)
{
	m_CodeEditor = new CodeEditor();
	m_CodeEditor->setMaxLinesNotice(1024);
	// m_CodeEditor->setReadOnly(true);
	// m_CodeEditor->setFocusPolicy(Qt::NoFocus);
	// m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_CodeEditor);
	// layout->addWidget(m_CommandInput);
	setLayout(layout);

	// connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
}

DlEditor::~DlEditor()
{

}

// m_DisplayerOutput->append(str.c_str());

/* void DlEditor::returnPressed()
{
	QString text = m_CommandInput->text();
	if (text.isEmpty())
		return;

	// std::string cmd = text.toAscii().data();
	// execute(cmd);

	m_CommandInput->clear();
} */

} /* namespace FT800EMUQT */

/* end of file */
