/*!
 * @file UintSpinBox.cpp
 * @date 11/16/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "UintSpinBox.h"

#include <QKeyEvent>
#include <QLineEdit>

#include "define/ValueDefine.h"
#include "utils/ConvertUtil.h"
#include "utils/LoggerUtil.h"

UintSpinBox::UintSpinBox(uint32_t initValue) {
  setText(initValue);
  connect(this, &QAbstractSpinBox::editingFinished, this,
          [this]() { emit changeFinished(lineEdit()->text()); });

  connect(lineEdit(), &QLineEdit::textChanged, this, [this]() {
    if (!hasAcceptableInput()) return;
    bool ok;
    int value = lineEdit()->text().toUInt(&ok, DEC);
    if (!ok) value = lineEdit()->text().toUInt(&ok, HEX);
    emit hasAcceptableValue(value);
  });
}

QValidator::State UintSpinBox::validate(QString &input, int &pos) const {
  lineEdit()->setStyleSheet("color: black");
  if (input.isEmpty()) return QValidator::Intermediate;
  bool ok;
  input.toUInt(&ok, DEC);
  if (ok) return QValidator::Acceptable;
  input.toUInt(&ok, HEX);
  if (ok) return QValidator::Acceptable;
  this->lineEdit()->setStyleSheet("color: red");
  return QValidator::Intermediate;
}

void UintSpinBox::stepBy(int steps) {
  if (!hasAcceptableInput()) return;
  bool ok = false;
  uint result;
  result = lineEdit()->text().toUInt(&ok, DEC);
  if (ok) {
    result += steps;
    lineEdit()->setText(QString::number(result));
    return;
  }
  result = lineEdit()->text().toUInt(&ok, HEX);
  if (ok) {
    result += steps;
    if (lineEdit()->text().toLower().startsWith("0x"))
      lineEdit()->setText(FTEDITOR::ConvertUtil::uintToHex(result));
    else
      lineEdit()->setText(FTEDITOR::ConvertUtil::uintToHexNoPrefix(result));
  }
}

QAbstractSpinBox::StepEnabled UintSpinBox::stepEnabled() const {
  return StepUpEnabled | StepDownEnabled;
}

void UintSpinBox::setText(uint32_t value) {
  if (value == FTEDITOR::ConvertUtil::stringToDec(lineEdit()->text())) return;
  auto text = QString::number(value);
  lineEdit()->setText(text);
  emit changeFinished(text);
}

QString UintSpinBox::getText() { return lineEdit()->text(); }

QLineEdit *UintSpinBox::getLineEdit() { return lineEdit(); }
