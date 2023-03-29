/*!
 * @file RamCMD.cpp
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamCMD.h"

// Emulator includes
#include <bt8xxemu_diag.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "constant_mapping.h"
#include "customize/QHexView.h"
#include "inspector.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamCMD::RamCMD(Inspector *parent) : RamBase(parent) {
  setObjectName("RamCMD");
  setWindowTitle("RAM_CMD");
  m_btnOpenDlg->setStatusTip(tr("Show the RAM_CMD Window"));
  m_lbTitle->setText(tr("RAM_CMD"));

  m_hexView = new QHexView;
  m_hexView->setUseContentArea(false);
  m_hexView->setSelectedAddress(0, 0);

  m_lbUint = new QLabel(tr("Uint:"));
  m_lbUint->setStatusTip(tr("Decimal value (4 bytes, Little Endian)"));
  m_lbUint->setMinimumWidth(25); 

  // Set default width for QLineEdit
  m_leAddress = new QLineEdit;
  m_leAddress->setFocus();
  QString placeholderText = tr("Enter decimal or hexadecimal");
  auto textSize = m_leAddress->fontMetrics().size(0, placeholderText);
  m_leAddress->setPlaceholderText(placeholderText);
  m_leAddress->setMaximumWidth(textSize.width() + 10);
  m_leAddress->setMinimumWidth(30);
  m_focusWhenOpen = m_leAddress;

  m_lbAddress = new QLabel(tr("Address:"));
  m_lbAddress->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_btnSearch = new QPushButton(tr("Jump"));
  m_btnSearch->setDefault(true);
  m_btnSearch->setMaximumWidth(80);
  m_btnSearch->setMinimumWidth(50);

  m_lytSearch = new QHBoxLayout;
  m_lytSearch->setAlignment(Qt::AlignRight);
  m_lytSearch->addWidget(m_lbUint, 0);
  m_lytSearch->addWidget(m_lbAddress);
  m_lytSearch->addWidget(m_leAddress, 2);
  m_lytSearch->addWidget(m_btnSearch, 1);
  
  auto layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignRight);
  auto groupBox = new QGroupBox(this);
  auto vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(3, 3, 3, 3);
  vBoxLayout->addLayout(m_lytTitle);
  vBoxLayout->addWidget(m_hexView);
  vBoxLayout->addLayout(m_lytSearch);
  groupBox->setLayout(vBoxLayout);
  layout->addWidget(groupBox);
  m_widget->setLayout(layout);
  resize(615, 327);

  connect(m_btnSearch, &QPushButton::clicked, this, &RamCMD::goToAddress);
  connect(m_leAddress, &QLineEdit::returnPressed, this, &RamCMD::goToAddress);
  connect(m_hexView, &QHexView::uintChanged, this, &RamCMD::setLabelUint);
  connect(m_insp, &Inspector::updateView, this, &RamCMD::updateView);
}

void RamCMD::goToAddress() {
  bool ok;
  QString text = m_leAddress->text();
  int address = text.toUInt(&ok, 10);
  if (!ok) {
    address = text.toUInt(&ok, 16);
  }
  if (ok) {
    if (m_hexView->showFromAddress(address)) {
      m_hexView->setSelectedAddress(address, 1);
    } else if (m_hexView->showFromOffset(address)) {
      m_hexView->setSelectedOffset(address, 1);
    } else
      return;
    m_hexView->updateUint();
    m_hexView->setFocus();
  }
}

void RamCMD::setLabelUint(QString valueStr, uint value) {
  m_lbUint->setText(
      valueStr.isEmpty()
          ? tr("Uint:")
          : QString(tr("Uint: %1 (%2)")).arg(value).arg(valueStr));
}

void RamCMD::updateView() {
  int startAddr = g_Addr[FTEDITOR_CURRENT_DEVICE][FTEDITOR_RAM_CMD];
  auto ram = BT8XXEMU_getRam(g_Emulator);
  uint8_t *ramCMD = ram + startAddr;
  QByteArray byteArr;
  auto ramCMDChar = static_cast<char *>(static_cast<void *>(ramCMD));
  byteArr.append(ramCMDChar, 4096);  // 4KB
  auto data = m_hexView->data();
  if (data && byteArr == data->getData(0, data->size())) {
    return;
  }
  m_hexView->setStartAddress(startAddr);
  m_hexView->setData(new QHexView::DataStorageArray(byteArr));
  m_hexView->viewport()->update();
}
}  // namespace FTEDITOR
