/*!
 * @file UintSpinBox.h
 * @date 11/16/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef UINTSPINBOX_H
#define UINTSPINBOX_H

#include <QAbstractSpinBox>

class UintSpinBox : public QAbstractSpinBox {
  Q_OBJECT

 public:
  UintSpinBox(uint32_t initValue);
  QValidator::State validate(QString &input, int &pos) const override;
  void stepBy(int steps) override;
  void setCurrentConverter(int newCurrentConverter);
  QAbstractSpinBox::StepEnabled stepEnabled() const override;
  void setText(uint32_t value);
  QString getText();
  QLineEdit *getLineEdit();

 signals:
  void changeFinished(const QString &);
  void hasAcceptableValue(const uint32_t &);
};

#endif  // UINTSPINBOX_H
