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
#include <vc.h>

// Project includes
#include "code_editor.h"
#include "dl_highlighter.h"
#include "dl_parser.h"

using namespace std;

namespace FT800EMUQT {

DlEditor::DlEditor(QWidget *parent) : QWidget(parent)
{
	m_CodeEditor = new CodeEditor();
	m_CodeEditor->setMaxLinesNotice(FT800EMU_DL_SIZE);
	// m_CodeEditor->setReadOnly(true);
	// m_CodeEditor->setFocusPolicy(Qt::NoFocus);
	// m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_CodeEditor);
	// layout->addWidget(m_CommandInput);
	setLayout(layout);
	
	m_DlHighlighter = new DlHighlighter(m_CodeEditor->document());

	// connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
}

DlEditor::~DlEditor()
{

}

void DlEditor::setUndoStack(QUndoStack *undo_stack)
{
	m_CodeEditor->setUndoStack(undo_stack);
}

void DlEditor::clearUndoStack()
{
	m_CodeEditor->document()->clearUndoRedoStacks();
}

void DlEditor::clear()
{
	m_CodeEditor->clear();
	lockDisplayList();
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	m_CodeEditor->textCursor().setPosition(0);
	unlockDisplayList();
}

void DlEditor::lockDisplayList()
{
	m_Mutex.lock();
}

void DlEditor::unlockDisplayList()
{
	m_Mutex.unlock();
}

// reloads the entire display list from m_DisplayListShared
void DlEditor::reloadDisplayList()
{
	lockDisplayList();
	m_DisplayListModified = true;
	int dcount = 0;
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		if (m_DisplayListShared[i] == DISPLAY() && dcount > 0)
		{
			++dcount;
		}
		else
		{
			if (dcount > 0)
			{
				for (int dc = 1; dc < dcount; ++dc)
				{
					m_CodeEditor->textCursor().insertText("\n");
				}
				dcount = 0;
			}
			if (m_DisplayListShared[i] == DISPLAY())
			{
				++dcount;
			}
			QString line = DlParser::toString(m_DisplayListShared[i]);
			DlParsed parsed;
			// verify parsing ->
			DlParser::parse(parsed, line);
			uint32_t compiled = DlParser::compile(parsed);
			if (compiled != m_DisplayListShared[i])
			{
				QByteArray chars = line.toLatin1();
				const char *src = chars.constData();
				printf("Parser bug at dl %i: '%s' -> expect %u, compiled %u\n", i, src, m_DisplayListShared[i], compiled);
			}
			// <- verify parsing
			m_CodeEditor->textCursor().insertText(line);
			m_CodeEditor->textCursor().insertText("\n");
		}
	}
	unlockDisplayList();
}

} /* namespace FT800EMUQT */

/* end of file */
