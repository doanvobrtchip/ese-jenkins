/*!
 * @file UndoSpinBox.cpp
 * @date 8/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "UndoSpinBox.h"

#include <QSpinBox>

UndoSpinBox::UndoSpinBox(int id, QSpinBox *spinbox, int oldValue, bool *working)
    : QUndoCommand(),
      m_id(id),
      m_spinBox(spinbox),
      m_newValue(spinbox->value()),
      m_oldValue(oldValue),
      m_working(working) {}

void UndoSpinBox::undo() {
  *m_working = true;
  m_spinBox->setValue(m_oldValue);
  *m_working = false;
}

void UndoSpinBox::redo() {
  *m_working = true;
  m_spinBox->setValue(m_newValue);
  *m_working = false;
}

int UndoSpinBox::id() const { return m_id; }

bool UndoSpinBox::mergeWith(const QUndoCommand *command) {
  if (command->id() != id()) return false;
   m_newValue = static_cast<const UndoSpinBox *>(command)->m_newValue;
  return true;
}
