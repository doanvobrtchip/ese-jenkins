/*!
 * @file ScriptEditor.h
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <Qsci/qsciscintilla.h>

class QsciLexer;

class ScriptEditor : public QsciScintilla {
  Q_OBJECT

 public:
  ScriptEditor(QsciLexer *lexer, QWidget *parent);
  ~ScriptEditor() = default;

 signals:
  void creatingContextMenu(QMenu *menu);

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e) override;
};

#endif  // SCRIPTEDITOR_H
