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

// http://qt-project.org/doc/qt-4.8/widgets-codeeditor-codeeditor-h.html

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

#include <QObject>
#include <QPlainTextEdit>
#include <vector>

#include "undo_stack_disabler.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QUndoStack;
class QCompleter;

class LineNumberArea;
class UndoEditor;

namespace FTEDITOR {
class InteractiveViewport;
}

typedef FTEDITOR::UndoStackDisabler<QPlainTextEdit> CodeEditorParent;

class CodeEditor : public CodeEditorParent {
  Q_OBJECT

 public:
  CodeEditor(QWidget *parent = 0);
  ~CodeEditor();

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();
  void setMaxLinesNotice(int lines) { m_MaxLinesNotice = lines; }

  // void setUndoStack(QUndoStack *undo_stack);
  void undo() {
    m_UndoNeedsClosure = false;
    QPlainTextEdit::undo();
  }

  void beginUndoCombine(const QString &message);
  void setUndoCombine(int combineId, const QString &message);
  void endUndoCombine();

  void setCompleter(QCompleter *c);
  QCompleter *completer() const;

  void setStepHighlight(int index) {
    if (m_StepHighlight != index) {
      m_StepHighlight = index;
      highlightCurrentLine();
    }
  }
  void setTraceHighlights(const std::vector<int> &indices);

  void setInteractiveDelete(bool status) {
    m_InteractiveDelete = status; /*printf("Interactive delete %s\n",
                                     m_InteractiveDelete ? "ON" : "OFF");*/
  }

  void setKeyHandler(FTEDITOR::InteractiveViewport *keyHandler) {
    m_KeyHandler = keyHandler;
    if (keyHandler) {
      m_LastKeyHandler = keyHandler;
    }
  }

 protected:
  virtual void resizeEvent(QResizeEvent *event);
  virtual void keyPressEvent(QKeyEvent *e);
  // virtual void contextMenuEvent(QContextMenuEvent *event);
  virtual void focusInEvent(QFocusEvent *event);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);
  void documentUndoCommandAdded();
  void undoIndexChanged(int idx);
  void insertCompletion(const QString &completion);

 private:
  QString textUnderCursor() const;

 private:
  QWidget *lineNumberArea;
  int m_MaxLinesNotice;
  // QUndoStack *m_UndoStack;
  bool m_UndoIndexDummy;
  bool m_UndoNeedsClosure;
  bool m_UndoIsClosing;
  QCompleter *m_Completer;
  int m_StepHighlight;
  int m_LastStepHighlight;
  bool m_StepMovingCursor;
  std::vector<int> m_TraceHighlights;
  std::vector<int> m_TraceStack;
  int m_CombineId;
  int m_LastCombineId;
  QString m_UndoRedoMessage;
  bool m_InteractiveDelete;
  FTEDITOR::InteractiveViewport *m_KeyHandler;
  FTEDITOR::InteractiveViewport *m_LastKeyHandler;
  QClipboard *cb;
  QString latestText;
};

class LineNumberArea : public QWidget {
 public:
  LineNumberArea(CodeEditor *editor) : QWidget(editor) { codeEditor = editor; }

  QSize sizeHint() const { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

 protected:
  void paintEvent(QPaintEvent *event) {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

 private:
  CodeEditor *codeEditor;
};

#endif
