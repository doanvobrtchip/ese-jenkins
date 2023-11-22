/*!
 * @file UndoLineEditeedit.cpp
 * @date 11/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "UndoLineEdit.h"

#include <QLineEdit>

UndoLineEdit::UndoLineEdit(int id, QLineEdit *lineEdit, QString oldValue,
                           bool *working)
    : QUndoCommand(),
      m_id(id),
      m_lineEdit(lineEdit),
      m_newValue(lineEdit->text()),
      m_oldValue(oldValue),
      m_working(working) {}

void UndoLineEdit::undo() {
  *m_working = true;
  m_lineEdit->setText(m_oldValue);
  *m_working = false;
}

void UndoLineEdit::redo() {
  *m_working = true;
  m_lineEdit->setText(m_newValue);
  *m_working = false;
}

int UndoLineEdit::id() const { return m_id; }

bool UndoLineEdit::mergeWith(const QUndoCommand *command) {
  if (command->id() != id()) return false;
  m_newValue = static_cast<const UndoLineEdit *>(command)->m_newValue;
  return true;
}
