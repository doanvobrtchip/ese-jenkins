/*!
 * @file UndoSpinBox.h
 * @date 8/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef UNDOSPINBOX_H
#define UNDOSPINBOX_H

#include <QUndoCommand>

class QSpinBox;

class UndoSpinBox : public QUndoCommand {
 public:
  UndoSpinBox(int id, QSpinBox *spinbox, int oldValue, bool *working);
  ~UndoSpinBox() = default;

  virtual void undo();
  virtual void redo();
  virtual int id() const;
  virtual bool mergeWith(const QUndoCommand *command);
  
  private:
  int m_id;
  QSpinBox *m_spinBox;
  int m_newValue;
  int m_oldValue;
  bool *m_working;
};

#endif  // UNDOSPINBOX_H
