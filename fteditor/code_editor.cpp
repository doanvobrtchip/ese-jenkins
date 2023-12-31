/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

// http://qt-project.org/doc/qt-4.8/widgets-codeeditor-codeeditor-cpp.html
// http://qt-project.org/doc/qt-4.8/tools-customcompleter.html

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include <QtGui>
#include <QUndoCommand>
#include <QUndoStack>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QClipboard>
#include <QApplication>

#include "src/utils/LoggerUtil.h"
#include "code_editor.h"
#include "interactive_viewport.h"

CodeEditor::CodeEditor(QWidget *parent)
    : CodeEditorParent(parent)
    , m_MaxLinesNotice(0)
    ,
    // m_UndoStack(NULL),
    m_UndoIndexDummy(false)
    , m_UndoNeedsClosure(false)
    , m_UndoIsClosing(false)
    , m_Completer(NULL)
    , m_StepHighlight(-1)
    , m_LastStepHighlight(-1)
    , m_CombineId(-1)
    , m_LastCombineId(917681768)
    , m_KeyHandler(NULL)
    , m_LastKeyHandler(NULL)
    , m_BlockNumberTag("")
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
	connect(this, &CodeEditor::cursorChanged, this, &CodeEditor::highlightCurrentLine);
	connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(documentUndoCommandAdded()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();

	// REVIEW 2023-01-19: There are multiple instances of code editor.
	// This seems to be running the same handler on the application/system level clipboard.
	// Not sure if this is the right way. Does this affect every copy paste even outside the appplication?
	// To test.

	// This affects every copy paste even outside the appplication.
	// We have a condition to make sure that we modify the clipboard in the right case.

	// QPlainTextEdit is using paragraph break instead of line break if we use
	// copy()
	m_Clipboard = QGuiApplication::clipboard();
	connect(m_Clipboard, &QClipboard::dataChanged, this, [&]() {
		if (m_LatestText.isEmpty()) return;
		QPlainTextEdit cbPTE, latestPTE;
		cbPTE.setPlainText(m_Clipboard->text());
		latestPTE.setPlainText(m_LatestText);
		QString latestPT = latestPTE.toPlainText();
		QString cbPT = cbPTE.toPlainText();
		if (latestPT == cbPT && m_Clipboard->text() != latestPT)
		{
			debugLog(QString("QPlainTextEdit | Update clipboard"));
			m_Clipboard->setText(latestPT);
		}
	});
	connect(this, &QPlainTextEdit::selectionChanged, this, [&]() {
		auto curTxt = this->textCursor().selectedText();
		if (!curTxt.isEmpty())
		{
			m_LatestText = curTxt;
		}
	});
}

void CodeEditor::focusInEvent(QFocusEvent *event)
{
	QPlainTextEdit::focusInEvent(event);
	emit focusIn();
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier)
	{
		int angle = event->angleDelta().y();
		if (angle > 0 && font().pointSize() < 180)
		{
			zoomIn(2);
		}
		else if (angle <= 0 && font().pointSize() > 5)
		{
			zoomOut(2);
		}
		return;
	}
	CodeEditorParent::wheelEvent(event);
}

void CodeEditor::mousePressEvent(QMouseEvent *e)
{
	int latestCursorPos = textCursor().position();
	CodeEditorParent::mousePressEvent(e);
	if (textCursor().position() != latestCursorPos) {
		emit cursorChanged();
	}
}

/*void CodeEditor::setUndoStack(QUndoStack *undo_stack)
{
    // setUndoRedoEnabled(undo_stack == NULL);
    // document()->setUndoRedoEnabled(true);
    m_UndoStack = undo_stack;
    connect(undo_stack, SIGNAL(indexChanged(int)), this, SLOT(undoIndexChanged(int)));
}*/

class UndoEditor : public QUndoCommand
{
public:
	UndoEditor(CodeEditor *editor, int id)
	    : QUndoCommand()
	    , m_Editor(editor)
	    , m_DoneDummy(false)
	    , m_CombineId(id)
	    , m_UndoCount(1)
	{
	}
	virtual ~UndoEditor() { }
	virtual int id() const
	{ /*printf("*** ret %i ***\n", m_CombineId);*/
		return m_CombineId;
	}
	virtual void undo()
	{ /*printf("*** undo %i ***\n", m_UndoCount);*/
		for (int i = 0; i < m_UndoCount; ++i) m_Editor->undo();
	}
	virtual void redo()
	{
		if (m_DoneDummy)
		{ /*printf("*** redo %i ***\n", m_UndoCount);*/
			for (int i = 0; i < m_UndoCount; ++i) m_Editor->redo();
		}
		else { m_DoneDummy = true; }
	}
	virtual bool mergeWith(const QUndoCommand *command)
	{
		++m_UndoCount;
		return true;
	}

private:
	friend class CodeEditor;
	CodeEditor *m_Editor;
	bool m_DoneDummy;
	int m_CombineId;
	int m_UndoCount;
};

void CodeEditor::beginUndoCombine(const QString &message)
{
	++m_LastCombineId;
	m_CombineId = m_LastCombineId;
	m_UndoRedoMessage = message;
}

void CodeEditor::setUndoCombine(int combineId, const QString &message)
{
	m_CombineId = combineId;
	m_UndoRedoMessage = message;
}

void CodeEditor::endUndoCombine()
{
	m_CombineId = -1;
	m_UndoRedoMessage = QString();
}

void CodeEditor::documentUndoCommandAdded()
{
	if (m_UndoIsClosing)
	{
		// skip
		m_UndoIsClosing = false;
		return;
	}

	/*printf("************ display list undo command added ************\n");*/
	m_UndoIndexDummy = true;
	m_UndoNeedsClosure = true;
	UndoEditor *uc = new UndoEditor(this, m_CombineId);
	uc->setText(m_UndoRedoMessage.isEmpty() ? tr("Edit code") : m_UndoRedoMessage);
	m_UndoStack->push(uc);
}

void CodeEditor::undoIndexChanged(int idx)
{
	if (m_UndoIndexDummy)
	{
		// skip
		m_UndoIndexDummy = false;
	}
	else
	{
		/*printf("*** undo index changed!!! ***\n");*/
		if (m_UndoNeedsClosure)
		{
			/*printf("*** close current undo!!! ***\n");*/
			m_UndoIsClosing = true;
			textCursor().insertText("\n");
			undo();
			m_UndoNeedsClosure = false;
		}
	}
}

namespace FTEDITOR {
void tempBeginIdel(CodeEditor *dlEditor);
void tempEndIdel(CodeEditor *dlEditor);
void editorPurgePalette8(CodeEditor *dlEditor, int &line);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
	if (m_Completer && m_Completer->popup()->isVisible())
	{
		// The following keys are forwarded by the completer to the widget
		switch (e->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return; // let the completer do default behavior
		default:
			break;
		}
	}

	if (m_KeyHandler)
	{
		m_KeyHandler->keyPressEvent(e);
		return;
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E);

	if (!isShortcut || !m_Completer)
	{
		if (e->key() == Qt::Key_Delete && m_InteractiveDelete)
		{
			debugLog("Interactive delete");
			FTEDITOR::tempBeginIdel(this);
			QTextCursor c = textCursor();
			m_Deleting = true;
			for (int i = m_SelectedLines.size() - 1; i >= 0; i--)
			{
				int line = m_SelectedLines.at(i);
				FTEDITOR::editorPurgePalette8(this, line);
				c.setPosition(document()->findBlockByNumber(line).position());
				c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
				c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
				c.insertText("");
				emit cursorChanged();
				// cleanup BEGIN/END ->
				{
					QString begin = document()->findBlockByNumber(line - 1).text();
					QString end = document()->findBlockByNumber(line).text();
					if (begin.toUpper().trimmed().startsWith("BEGIN") && end.toUpper().trimmed().startsWith("END"))
					{
						c.setPosition(document()->findBlockByNumber(line - 1).position());
						c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
						c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
						c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
						c.insertText("");
						emit cursorChanged();
					}
				}
			}

			FTEDITOR::tempEndIdel(this);
			// <- cleanup BEGIN/END
			setInteractiveDelete(true);
			m_Deleting = false;
			return;
		}
		else
		{
			int latestCursorPos = textCursor().position();
			CodeEditorParent::keyPressEvent(e);
			if (textCursor().position() != latestCursorPos) {
				emit cursorChanged();
			}
		}
	}

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!m_Completer || (ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 1 || eow.contains(e->text().right(1))))
	{
		m_Completer->popup()->hide();
		return;
	}

	if (completionPrefix != m_Completer->completionPrefix())
	{
		m_Completer->setCompletionPrefix(completionPrefix);
		m_Completer->popup()->setCurrentIndex(m_Completer->completionModel()->index(0, 0));
	}
	QRect cr = cursorRect();
	cr.setWidth(m_Completer->popup()->sizeHintForColumn(0)
	    + m_Completer->popup()->verticalScrollBar()->sizeHint().width());
	m_Completer->complete(cr); // popup it up!
}

/*void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_UndoStack)
    {
        // trap
    }
}*/

int CodeEditor::lineNumberAreaWidth()
{
	/*int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
	max /= 10;
	++digits;
	}*/
	int digits = 4;
	int charFontWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
	if (m_BlockNumberTag.isEmpty())
		return 20 + digits * charFontWidth;
	return 6 + (m_BlockNumberTag.length() + digits) * charFontWidth;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::setTraceHighlights(const std::vector<int> &indices)
{
	bool different = false;
	different = true;
	if (indices.size() != m_TraceStack.size())
	{
		different = true;
	}
	else
	{
		for (size_t i = 0; i < indices.size(); ++i)
		{
			if (indices[i] != m_TraceStack[i])
			{
				different = true;
				break;
			}
		}
	}
	if (different)
	{
		m_TraceStack.clear();
		for (size_t i = 0; i < indices.size(); ++i)
		{
			m_TraceStack.push_back(indices[i]);
		}
		m_TraceHighlights.clear();
		if (indices.size() > 0)
		{
			m_TraceHighlights.push_back(indices[indices.size() - 1]); // Give last one extra highlight
			for (size_t i = 0; i < indices.size() - 1; ++i)
			{
				bool dupe = false;
				for (size_t j = 0; j < m_TraceHighlights.size(); ++j)
				{
					if (indices[i] == m_TraceHighlights[j])
					{
						dupe = true;
						break;
					}
				}
				if (!dupe)
				{
					m_TraceHighlights.push_back(indices[i]);
				}
			}
		}
		highlightCurrentLine();
	}
}

void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly())
	{
		if (m_StepHighlight >= 0 && m_StepHighlight < document()->blockCount())
		{
			if (m_LastStepHighlight != m_StepHighlight)
			{
				changeCursorByLine(m_StepHighlight);
				m_LastStepHighlight = m_StepHighlight;
				emit cursorChanged(false);
			}
		}
		// cursor position
		{
			QTextEdit::ExtraSelection selection;
			QColor lineColor = QColor(Qt::lightGray).lighter(120);
			selection.format.setBackground(lineColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			extraSelections.append(selection);
		}
		// step
		if (m_StepHighlight >= 0 && m_StepHighlight < document()->blockCount())
		{
			QTextEdit::ExtraSelection selection;
			QColor lineColor = QColor(Qt::yellow);
			selection.format.setBackground(lineColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.setPosition(document()->findBlockByNumber(m_StepHighlight).position());
			extraSelections.append(selection);
		}
		// trace
		if (m_TraceHighlights.size() > 0 && m_TraceHighlights[0] >= 0)
		{
			QTextEdit::ExtraSelection selection;
			QColor lineColor = QColor(Qt::green).lighter(120);
			selection.format.setBackground(lineColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.setPosition(document()->findBlockByNumber(m_TraceHighlights[0]).position());
			extraSelections.append(selection);
		}
		for (size_t i = 1; i < m_TraceHighlights.size(); ++i)
		{
			if (m_TraceHighlights[i] != m_StepHighlight && m_TraceHighlights[i] >= 0)
			{
				QTextEdit::ExtraSelection selection;
				QColor lineColor = QColor(Qt::green).lighter(160);
				selection.format.setBackground(lineColor);
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor = textCursor();
				selection.cursor.setPosition(document()->findBlockByNumber(m_TraceHighlights[i]).position());
				extraSelections.append(selection);
			}
		}
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::setCompleter(QCompleter *completer)
{
	if (m_Completer)
		QObject::disconnect(m_Completer, 0, this, 0);

	m_Completer = completer;

	if (!m_Completer)
		return;

	m_Completer->setWidget(this);
	m_Completer->setCompletionMode(QCompleter::PopupCompletion);
	m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(m_Completer, SIGNAL(activated(QString)),
	    this, SLOT(insertCompletion(QString)));
}

QCompleter *CodeEditor::completer() const
{
	return m_Completer;
}

void CodeEditor::insertCompletion(const QString &completion)
{
	if (m_Completer->widget() != this)
		return;
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_Completer->completionPrefix().length();
	// tc.movePosition(QTextCursor::Left);
	// tc.movePosition(QTextCursor::EndOfWord);
	tc.select(QTextCursor::WordUnderCursor);
	tc.setPosition(tc.selectionEnd());
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
	emit cursorChanged();
}

QString CodeEditor::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void CodeEditor::setBlockNumberTag(const QString &newBlockNumberTag)
{
	m_BlockNumberTag = newBlockNumberTag;
	updateLineNumberAreaWidth(0);
}

const QList<int> &CodeEditor::SelectedLines() const
{
	return m_SelectedLines;
}

void CodeEditor::setSelectedLines(const QList<int> &newSelectedLines)
{
	if (m_Deleting) return;
	m_SelectedLines.clear();
	m_SelectedLines = newSelectedLines;
}

void CodeEditor::changeCursorByLine(int line)
{
	QTextCursor c = textCursor();
	c.setPosition(document()->findBlockByNumber(line).position());
	setTextCursor(c);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	// painter.fillRect(event->rect(), QColor(Qt::lightGray));

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			QString number = m_BlockNumberTag + QString::number(blockNumber);
			lineNumberArea->setFont(QFont("Segoe UI", font().pointSize()));
			painter.setPen((m_MaxLinesNotice && blockNumber >= m_MaxLinesNotice)
			        ? Qt::red
			        : Qt::black);
			painter.drawText(0, top, lineNumberArea->width(),
			    fontMetrics().height(), Qt::AlignCenter, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		++blockNumber;
	}
}
