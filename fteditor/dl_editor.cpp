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
#include <QApplication>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QCompleter>
#include <QStringListModel>
#include <QAbstractItemView>

// Emulator includes
#include <ft8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "interactive_viewport.h"
#include "code_editor.h"
#include "dl_highlighter.h"
#include "properties_editor.h"
#include "toolbox.h"
#include "interactive_properties.h"
#include "constant_common.h"

using namespace std;

namespace FTEDITOR {

extern int g_StepCmdLimit;

DlEditor::DlEditor(MainWindow *parent, bool coprocessor) : QWidget(parent), m_MainWindow(parent), m_Reloading(false), m_CompleterIdentifiersActive(true),
m_PropertiesEditor(NULL), m_PropLine(-1), m_PropIdLeft(-1), m_PropIdRight(-1), m_ModeMacro(false), m_ModeCoprocessor(coprocessor), m_EditingInteractive(false),
m_CompleterModel(NULL)
{
	m_DisplayListShared[0] = DISPLAY();

	m_CodeEditor = new CodeEditor();
	m_CodeEditor->setMaxLinesNotice(FTEDITOR_DL_SIZE);
	// m_CodeEditor->setReadOnly(true);
	// m_CodeEditor->setFocusPolicy(Qt::NoFocus);
	// m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_CodeEditor);
	// layout->addWidget(m_CommandInput);
	setLayout(layout);

	m_DlHighlighter = new DlHighlighter(m_CodeEditor->document(), coprocessor);

	// connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));

	connect(m_CodeEditor->document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentContentsChange(int, int, int)));
	connect(m_CodeEditor->document(), SIGNAL(blockCountChanged(int)), this, SLOT(documentBlockCountChanged(int)));
	connect(m_CodeEditor, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorPositionChanged()));

	bindCurrentDevice();

	m_CompleterModel = new QStringListModel(m_CodeEditor);
	m_CompleterModel->setStringList(m_CompleterIdentifiers);

	m_Completer = new QCompleter(m_CompleterModel, m_CodeEditor);
	m_CodeEditor->setCompleter(m_Completer);

	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListParsed[i].ValidId = false;
		m_DisplayListShared[i] = DISPLAY();
	}
}

DlEditor::~DlEditor()
{

}

void DlEditor::bindCurrentDevice()
{
	m_CompleterIdentifiers.clear();
	m_CompleterParams.clear();

	DlParser::getIdentifiers(FTEDITOR_CURRENT_DEVICE, m_CompleterIdentifiers, m_ModeCoprocessor);
	DlParser::getParams(FTEDITOR_CURRENT_DEVICE, m_CompleterParams, m_ModeCoprocessor);

	if (m_CompleterModel)
	{
		if (m_CompleterIdentifiersActive)
			m_CompleterModel->setStringList(m_CompleterIdentifiers);
		else
			m_CompleterModel->setStringList(m_CompleterParams);
		m_Completer->popup()->hide();
	}

	lockDisplayList();
	for (int i = 0; i < m_CodeEditor->document()->blockCount(); ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	m_DisplayListModified = true;
	unlockDisplayList();

	m_DlHighlighter->rehighlight();
	m_InvalidState = true;
}

void DlEditor::setUndoStack(QUndoStack *undo_stack)
{
	m_CodeEditor->setUndoStack(undo_stack);
}

void DlEditor::clearUndoStack()
{
	m_CodeEditor->document()->clearUndoRedoStacks();
}

void DlEditor::setModeMacro()
{
	m_ModeMacro = true;
	m_CodeEditor->setMaxLinesNotice(FT800EMU_MACRO_SIZE);
	/*QFontMetrics m(m_CodeEditor->font()) ;
	int height = m.lineSpacing() ;
	m_CodeEditor->setFixedHeight(3 * height) ;*/
}

void DlEditor::clear()
{
	m_CodeEditor->clear();
	lockDisplayList();
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	m_CodeEditor->textCursor().setPosition(0);
	unlockDisplayList();
	m_InvalidState = true;
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
	m_CodeEditor->setInteractiveDelete(false);

	bool firstLine = true;
	m_Reloading = true;
	if (!fromEmulator)
	{
		m_DisplayListModified = true;
	}
	int dcount = 0;
	for (int i = 0; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
	{
		if (!m_ModeMacro && m_DisplayListShared[i] == DISPLAY() && dcount > 0)
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
			QString line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, m_DisplayListShared[i]);
			m_DisplayListParsed[i] = DlParsed();
			// verify parsing ->
			DlParser::parse(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i], line, m_ModeCoprocessor);
			uint32_t compiled = DlParser::compile(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i]);
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
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	if (QApplication::instance()->closingDown()) return;

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

	editingLine(block);
}

void DlEditor::documentContentsChange(int position, int charsRemoved, int charsAdded)
{
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	//printf("contents change %i %i\n", charsRemoved, charsAdded);

	if (QApplication::instance()->closingDown()) return;

	if (m_Reloading)
		return;

	int charsEdited = max(charsRemoved, charsAdded);
	QTextBlock firstBlock = m_CodeEditor->document()->findBlock(position);
	int blockNb = firstBlock.blockNumber();
	int firstPosition = firstBlock.position();
	charsEdited += (position - firstPosition);

	//int count = 0;

	lockDisplayList();
	while (charsEdited > 0)
	{
		if (blockNb < m_CodeEditor->document()->blockCount())
		{
			QTextBlock block = m_CodeEditor->document()->findBlockByNumber(blockNb);
			parseLine(block);
			charsEdited -= block.length();
			++blockNb;
			//++count;
		}
		else
		{
			break;
		}
	}
	m_DisplayListModified = true;
	unlockDisplayList();
}

void DlEditor::documentBlockCountChanged(int newBlockCount)
{
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	//printf("blockcount change\n");

	if (QApplication::instance()->closingDown()) return;

	if (m_Reloading)
		return;

	lockDisplayList();
	for (int i = 0; i < newBlockCount && i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	for (int i = newBlockCount; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListParsed[i].ValidId = false;
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	unlockDisplayList();

	editorCursorPositionChanged();
}

void DlEditor::parseLine(QTextBlock block)
{
	QString line = block.text();
	int i = block.blockNumber();

	if (!m_InvalidState) m_InvalidState = DlState::requiresProcessing(m_DisplayListParsed[i]);

	m_DisplayListParsed[i] = DlParsed();
	DlParser::parse(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i], line, m_ModeCoprocessor);
	m_DisplayListShared[i] = DlParser::compile(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i]);

	// check for misformed lines and do a no-op (todo: mark them)
	if (m_DisplayListShared[i] == DISPLAY() && !m_DisplayListParsed[i].ValidId)
	{
		m_DisplayListShared[i] = JUMP(i + 1);
	}

	if (!m_InvalidState) m_InvalidState = DlState::requiresProcessing(m_DisplayListParsed[i]);
}

void DlEditor::replaceLine(int line, const DlParsed &parsed, int combineId, const QString &message)
{
	m_EditingInteractive = true;
	if (combineId >= 0) m_CodeEditor->setUndoCombine(combineId, message);
	QString linestr = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	//m_CodeEditor->setTextCursor(c);
	//c.select(QTextCursor::BlockUnderCursor);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.insertText(linestr);
	// editorCursorPositionChanged() needed? // VERIFY
	if (combineId >= 0) m_CodeEditor->endUndoCombine();
	m_EditingInteractive = false;
}

void DlEditor::removeLine(int line)
{
	m_EditingInteractive = true;
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
	c.insertText("");
	m_EditingInteractive = false;
}

int DlEditor::getLineCount()
{
	return m_CodeEditor->document()->blockCount();
}

void DlEditor::insertLine(int line, const DlParsed &parsed)
{
	m_EditingInteractive = true;
	QString linestr = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
	if (line == 0)
	{
		QTextCursor c = m_CodeEditor->textCursor();
		c.setPosition(0);
		c.insertText(linestr + "\n");
	}
	else
	{
		QTextCursor c = m_CodeEditor->textCursor();
		c.setPosition(m_CodeEditor->document()->findBlockByNumber(line - 1).position());
		c.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
		c.insertText("\n" + linestr);
	}
	// editorCursorPositionChanged() needed? // VERIFY
	m_EditingInteractive = false;
}

const DlParsed &DlEditor::getLine(int line) const
{
	return m_DisplayListParsed[line];
}

void DlEditor::selectLine(int line)
{
	m_EditingInteractive = true;
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	m_CodeEditor->setTextCursor(c);
	editingLine(m_CodeEditor->document()->findBlockByNumber(line));
	// editorCursorPositionChanged() instead of editingLine? // VERIFY
	m_EditingInteractive = false;
}

void DlEditor::editingLine(QTextBlock block)
{
	// update properties editor
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);
	if (m_PropertiesEditor->getEditWidgetSetter() != this
		|| block.blockNumber() != m_PropLine
		|| m_DisplayListParsed[block.blockNumber()].IdLeft != m_PropIdLeft
		|| m_DisplayListParsed[block.blockNumber()].IdRight != m_PropIdRight
		|| m_DisplayListParsed[block.blockNumber()].ValidId != m_PropIdValid
		|| !m_DisplayListParsed[block.blockNumber()].ValidId) // Necessary for the "Unknown command" info message
	{
		m_PropLine = block.blockNumber();
		if (m_PropLine < 0) // ?
		{
			m_PropertiesEditor->setInfo(QString());
			m_PropertiesEditor->setEditWidget(NULL, false, this);
			return;
		}
		m_PropIdLeft = m_DisplayListParsed[m_PropLine].IdLeft;
		m_PropIdRight = m_DisplayListParsed[m_PropLine].IdRight;
		m_PropIdValid = m_DisplayListParsed[m_PropLine].ValidId;
		m_MainWindow->toolbox()->setEditorLine(this, m_PropLine);
		m_MainWindow->viewport()->setEditorLine(this, m_PropLine);
		m_MainWindow->interactiveProperties()->setEditorLine(this, m_PropLine);
	}
	else
	{
		m_MainWindow->interactiveProperties()->modifiedEditorLine();
	}
}

void DlEditor::processState()
{
	if (!m_ModeMacro)
	{
		printf("DEBUG: Process state\n");
		DlState::process(FTEDITOR_CURRENT_DEVICE, m_State, m_PropLine, m_DisplayListParsed, m_ModeCoprocessor);
	}
	m_InvalidState = false;
}

void DlEditor::frame()
{
	// FIXME DL/CMD Mapping
	// update current step highlight
	int idx = FT8XXEMU_getDebugLimiterEffective() ? FT8XXEMU_getDebugLimiterIndex() : -1;
	if (idx > 0 && m_ModeCoprocessor)
	{
		if (idx < FTEDITOR_DL_SIZE)
		{
			idx = m_MainWindow->getDlCmd()[idx];
		}
		else
		{
			idx = -1;
		}
	}
	if (idx < 0 && m_ModeCoprocessor)
	{
		idx = (g_StepCmdLimit - 1);
	}
	m_CodeEditor->setStepHighlight(idx);
}

} /* namespace FTEDITOR */

/* end of file */
