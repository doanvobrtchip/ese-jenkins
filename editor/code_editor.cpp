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

 #include <QtGui>
 #include <QUndoCommand>
 #include <QUndoStack>

 #include "code_editor.h"


 CodeEditor::CodeEditor(QWidget *parent) 
	: QPlainTextEdit(parent), 
		m_MaxLinesNotice(0),
		m_UndoStack(NULL),
		m_UndoIndexDummy(false),
		m_UndoNeedsClosure(false),
		m_UndoIsClosing(false)
 {
     lineNumberArea = new LineNumberArea(this);

     connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
     connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
     connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	 connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(documentUndoCommandAdded()));
	 
     updateLineNumberAreaWidth(0);
     highlightCurrentLine();
 }

void CodeEditor::setUndoStack(QUndoStack *undo_stack)
{
	// setUndoRedoEnabled(undo_stack == NULL);
	// document()->setUndoRedoEnabled(true);
	m_UndoStack = undo_stack;
	connect(undo_stack, SIGNAL(indexChanged(int)), this, SLOT(undoIndexChanged(int)));
}

class UndoEditor : public QUndoCommand
{
public:
	UndoEditor(CodeEditor *editor) : QUndoCommand(), m_Editor(editor), m_DoneDummy(false) { }
	virtual ~UndoEditor() { }
	virtual void undo() { /*printf("*** undo ***\n");*/ m_Editor->undo(); }
	virtual void redo() { if (m_DoneDummy) { /*printf("*** redo ***\n");*/ m_Editor->redo(); } else { m_DoneDummy = true; } }
	
private:
	CodeEditor *m_Editor;
	bool m_DoneDummy;
	
};

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
	UndoEditor *uc = new UndoEditor(this);
	uc->setText(tr("UndoEditCode"));
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

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
	if (m_UndoStack && (e->modifiers() == Qt::ControlModifier)
		&& (e->key() == Qt::Key_Z))
	{
		// trap
		m_UndoStack->undo();
	}
	else if (m_UndoStack && (e->modifiers() == Qt::ControlModifier)
		&& (e->key() == Qt::Key_Y))
	{
		// trap
		m_UndoStack->redo();
	}
	else
	{
		QPlainTextEdit::keyPressEvent(e);
	}
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
	if (m_UndoStack)
	{
		// trap
	}
}

 int CodeEditor::lineNumberAreaWidth()
 {
     /*int digits = 1;
     int max = qMax(1, blockCount());
     while (max >= 10) {
         max /= 10;
         ++digits;
     }*/
     int digits = 4;

     int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

     return space + 4;
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



 void CodeEditor::highlightCurrentLine()
 {
     QList<QTextEdit::ExtraSelection> extraSelections;

     if (!isReadOnly()) {
         QTextEdit::ExtraSelection selection;

         QColor lineColor = QColor(Qt::lightGray).lighter(120);

         selection.format.setBackground(lineColor);
         selection.format.setProperty(QTextFormat::FullWidthSelection, true);
         selection.cursor = textCursor();
         selection.cursor.clearSelection();
         extraSelections.append(selection);
     }

     setExtraSelections(extraSelections);
 }



 void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
 {
     QPainter painter(lineNumberArea);
     // painter.fillRect(event->rect(), QColor(Qt::lightGray));


     QTextBlock block = firstVisibleBlock();
     int blockNumber = block.blockNumber();
     int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
     int bottom = top + (int) blockBoundingRect(block).height();

     while (block.isValid() && top <= event->rect().bottom()) {
         if (block.isVisible() && bottom >= event->rect().top()) {
             QString number = QString::number(blockNumber/* + 1*/); 
             painter.setPen((m_MaxLinesNotice && blockNumber >= m_MaxLinesNotice) ? Qt::red : Qt::black);
             painter.drawText(0, top, lineNumberArea->width() - 4, fontMetrics().height(),
                              Qt::AlignRight, number);
         }

         block = block.next();
         top = bottom;
         bottom = top + (int) blockBoundingRect(block).height();
         ++blockNumber;
     }
 }
