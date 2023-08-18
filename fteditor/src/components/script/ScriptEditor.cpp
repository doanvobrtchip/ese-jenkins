/*!
 * @file ScriptEditor.cpp
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ScriptEditor.h"

#include <Qsci/qscilexer.h>

#include <QContextMenuEvent>
#include <QMenu>

ScriptEditor::ScriptEditor(QsciLexer *lexer, QWidget *parent)
    : QsciScintilla(parent) {
  QFont font(lexer->defaultFont());
  QFontMetrics fontmetrics(font);

  // Current line visible with special background color
  setCaretLineVisible(true);
  setCaretForegroundColor(palette().text().color());
  setCaretLineBackgroundColor(palette().color(QPalette::Midlight));

  setMarginsFont(font);
  setMarginLineNumbers(0, true);
  setMarginsBackgroundColor(palette().window().color());
  setMarginsForegroundColor(palette().text().color().darker());
  setMarginWidth(0, fontmetrics.horizontalAdvance(QStringLiteral("00000")));

  setWhitespaceSize(1);
  setWhitespaceForegroundColor(QColor(250, 125, 0));
  setWhitespaceVisibility(WhitespaceVisibility::WsVisible);
  setIndentationGuidesForegroundColor(QColor(189, 189, 189, 64));

  setTabWidth(4);
  setAutoIndent(true);
  setIndentationWidth(4);
  setIndentationGuides(true);
  setIndentationsUseTabs(false);

  setLexer(lexer);
}

void ScriptEditor::contextMenuEvent(QContextMenuEvent *ev) {
  QMenu *menu = createStandardContextMenu();

  if (menu) {
    menu->addSeparator();

    emit creatingContextMenu(menu);

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(ev->globalPos());
  }
}
