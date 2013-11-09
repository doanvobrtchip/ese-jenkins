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
#include <QTextBlock>
#include <QCompleter>
#include <QStringListModel>
#include <QAbstractItemView>

// Emulator includes
#include <vc.h>

// Project includes
#include "code_editor.h"
#include "dl_highlighter.h"

using namespace std;

namespace FT800EMUQT {

DlEditor::DlEditor(QWidget *parent) : QWidget(parent), m_Reloading(false), m_CompleterIdentifiersActive(true)
{
	m_DisplayListShared[0] = DISPLAY();
	
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
	
	connect(m_CodeEditor->document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentContentsChange(int, int, int)));
	connect(m_CodeEditor->document(), SIGNAL(blockCountChanged(int)), this, SLOT(documentBlockCountChanged(int)));
	connect(m_CodeEditor, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorPositionChanged()));
	
	DlParser::getIdentifiers(m_CompleterIdentifiers);
	DlParser::getParams(m_CompleterParams);
	
	m_CompleterModel = new QStringListModel(m_CodeEditor);
	m_CompleterModel->setStringList(m_CompleterIdentifiers);
	
	m_Completer = new QCompleter(m_CompleterModel, m_CodeEditor);
	m_CodeEditor->setCompleter(m_Completer);
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
void DlEditor::reloadDisplayList(bool fromEmulator)
{
	bool firstLine = true;
	m_Reloading = true;
	if (!fromEmulator)
	{
		m_DisplayListModified = true;
	}
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
					if (firstLine) firstLine = false;
					else m_CodeEditor->textCursor().insertText("\n");
				}
				dcount = 0;
			}
			if (m_DisplayListShared[i] == DISPLAY())
			{
				++dcount;
			}
			QString line = DlParser::toString(m_DisplayListShared[i]);
			m_DisplayListParsed[i] = DlParsed();
			// verify parsing ->
			DlParser::parse(m_DisplayListParsed[i], line);
			uint32_t compiled = DlParser::compile(m_DisplayListParsed[i]);
			if (compiled != m_DisplayListShared[i])
			{
				QByteArray chars = line.toLatin1();
				const char *src = chars.constData();
				printf("Parser bug at dl %i: '%s' -> expect %u, compiled %u\n", i, src, m_DisplayListShared[i], compiled);
			}
			// <- verify parsing
			if (firstLine) firstLine = false;
			else m_CodeEditor->textCursor().insertText("\n");
			m_CodeEditor->textCursor().insertText(line);
		}
	}
	m_Reloading = false;
}

void DlEditor::editorCursorPositionChanged()
{
	QTextBlock block = m_CodeEditor->document()->findBlock(m_CodeEditor->textCursor().position());
	
	// switch between auto completers
	if (m_DisplayListParsed[block.blockNumber()].ValidId && m_CompleterIdentifiersActive)
	{
		m_CompleterIdentifiersActive = false;
		m_CompleterModel->setStringList(m_CompleterParams);
		m_Completer->popup()->hide();
	}
	else if (!m_DisplayListParsed[block.blockNumber()].ValidId && !m_CompleterIdentifiersActive)
	{
		m_CompleterIdentifiersActive = true;
		m_CompleterModel->setStringList(m_CompleterIdentifiers);
		m_Completer->popup()->hide();
	}
}

void DlEditor::documentContentsChange(int position, int charsRemoved, int charsAdded)
{
	if (m_Reloading)
		return;
	
	QTextBlock block = m_CodeEditor->document()->findBlock(position);
	
	lockDisplayList();
	parseLine(block);
	m_DisplayListModified = true;
	unlockDisplayList();
}

void DlEditor::documentBlockCountChanged(int newBlockCount)
{
	if (m_Reloading)
		return;
	
	lockDisplayList();
	for (int i = 0; i < newBlockCount && i < FT800EMU_DL_SIZE; ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	for (int i = newBlockCount; i < FT800EMU_DL_SIZE; ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	unlockDisplayList();
}

void DlEditor::parseLine(QTextBlock block)
{
	QString line = block.text();
	int i = block.blockNumber();
	m_DisplayListParsed[i] = DlParsed();
	DlParser::parse(m_DisplayListParsed[i], line);
	m_DisplayListShared[i] = DlParser::compile(m_DisplayListParsed[i]);
	
	// check for misformed lines and do a no-op (todo: mark them)
	if (m_DisplayListShared[i] == DISPLAY() && !m_DisplayListParsed[i].ValidId)
	{
		m_DisplayListShared[i] = JUMP(i + 1);
	}
}

} /* namespace FT800EMUQT */

/* end of file */
