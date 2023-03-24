/*!
 * @file RamG.cpp
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamG.h"

// Emulator includes
#include <bt8xxemu_diag.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "content_manager.h"
#include "customize/QHexView.h"
#include "inspector.h"
#include "main_window.h"
#include "utils/LoggerUtil.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamG::RamG(Inspector *parent) : RamBase(parent) {
  setObjectName("RamG");
  setWindowTitle("RAM_G");
  m_btnOpenDlg->setStatusTip(tr("Show the RAM_G Window"));
  m_lbTitle->setText(tr("RAM_G"));

  m_hexView = new QHexView;
  m_lbUint = new QLabel(tr("Uint:"));
  m_lbUint->setStatusTip(tr("Decimal value (4 bytes, Little Endian)"));

  // Set default width for QLineEdit
  m_leAddress = new QLineEdit;
  m_leAddress->setFocus();
  QString placeholderText = tr("Enter decimal or hexadecimal");
  auto textSize = m_leAddress->fontMetrics().size(0, placeholderText);
  m_leAddress->setPlaceholderText(placeholderText);
  m_leAddress->setMaximumWidth(textSize.width() + 10);
  m_focusWhenOpen = m_leAddress;

  m_lbAddress = new QLabel(tr("Address:"));
  m_lbAddress->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_btnSearch = new QPushButton(tr("Jump"));
  m_btnSearch->setDefault(true);
  m_btnSearch->setMaximumWidth(90);

  m_lytSearch = new QHBoxLayout;
  m_lytSearch->setAlignment(Qt::AlignRight);
  m_lytSearch->addWidget(m_lbUint);
  m_lytSearch->addWidget(m_lbAddress);
  m_lytSearch->addWidget(m_leAddress);
  m_lytSearch->addWidget(m_btnSearch);

  auto layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignRight);
  auto groupBox = new QGroupBox(this);
  auto vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(5, 5, 5, 5);
  vBoxLayout->addLayout(m_lytTitle);
  vBoxLayout->addWidget(m_hexView);
  vBoxLayout->addLayout(m_lytSearch);
  groupBox->setLayout(vBoxLayout);
  layout->addWidget(groupBox);
  m_widget->setLayout(layout);
  resize(615, 327);

  connect(m_btnSearch, &QPushButton::clicked, this, &RamG::goToAddress);
  connect(m_leAddress, &QLineEdit::returnPressed, this, &RamG::goToAddress);
  connect(m_hexView, &QHexView::uintChanged, this, &RamG::setLabelUint);
  connect(m_insp, &Inspector::updateView, this, &RamG::updateView);
  connect(m_insp->mainWindow(), SIGNAL(readyToSetup(QObject *)), this,
          SLOT(setupConnections(QObject *)));
  setupConnections();
  emit m_insp->mainWindow()->readyToSetup(this);
}

void RamG::setupConnections(QObject *obj) {
  if (auto contentMgr = m_insp->mainWindow()->contentManager();
      contentMgr && (obj == contentMgr || obj == nullptr)) {
    auto currContentList = contentMgr->contentInfoList();
    for (auto &info : currContentList) {
      addContentItem(info);
    }

    auto contentList = contentMgr->contentList();
    connect(contentList, &QTreeWidget::currentItemChanged, this,
            &RamG::currItemContentChanged, Qt::UniqueConnection);
    connect(contentList, &QTreeWidget::itemPressed, this,
            &RamG::handleContentItemPressed, Qt::UniqueConnection);
    connect(contentList, &ContentTreeWidget::removeItem, this,
            &RamG::removeContentItem, Qt::UniqueConnection);
    connect(contentList, &ContentTreeWidget::addItem, this,
            &RamG::addContentItem, Qt::UniqueConnection);

    connect(contentMgr, &ContentManager::ramGlobalUsageChanged, this,
            &RamG::updateView, Qt::UniqueConnection);
    connect(m_hexView, &QHexView::currentInfoChanged, this,
            &RamG::handleCurrentInfoChanged, Qt::UniqueConnection);
  }

  if (auto inspDock = m_insp->mainWindow()->inspectorDock();
      inspDock &&
      (obj == m_insp->mainWindow()->inspectorDock() || obj == nullptr)) {
    connect(inspDock, &QDockWidget::visibilityChanged, this, &RamG::bindVisible,
            Qt::UniqueConnection);
  }
}

void RamG::currItemContentChanged(QTreeWidgetItem *current,
                                  QTreeWidgetItem *previous) {
  if (!current) {
    debugLog("No selected content");
    m_hexView->setSelectedAddress(0, 0);
    return;
  };
}

void RamG::handleContentItemPressed(QTreeWidgetItem *item) {
  if (!item) {
    m_hexView->setSelectedAddress(0, 0);
    return;
  };
  auto contentInfo =
      (ContentInfo *)(void *)item->data(0, Qt::UserRole).value<quintptr>();

  auto size =
      m_insp->mainWindow()->contentManager()->getContentSize(contentInfo);
  if (!contentInfo->MemoryLoaded || size <= 0 ||
      !contentInfo->BuildError.isEmpty()) {
    debugLog("Selected a non-memory content");
    m_hexView->setSelectedAddress(0, 0);
    return;
  }

  debugLog(QString("Selected a memory content | Addr: %1 | Size: %2 | Name: %3")
               .arg(contentInfo->MemoryAddress)
               .arg(size)
               .arg(contentInfo->DestName));
  if (!m_hexView->showFromAddress(contentInfo->MemoryAddress)) return;
  m_hexView->setSelectedAddress(contentInfo->MemoryAddress, size);
  if (m_visible) {
    m_hexView->setFocus(Qt::MouseFocusReason);
  }
}

void RamG::handleCurrentInfoChanged(ContentInfo *contentInfo) {
  emit updateCurrentInfo(contentInfo);
}

void RamG::removeContentItem(ContentInfo *contentInfo) {
  auto size =
      m_insp->mainWindow()->contentManager()->getContentSize(contentInfo);
  if (contentInfo->DataStorage == ContentInfo::Flash || size < 0) {
    debugLog("Remove a non-memory content");
    return;
  }
  debugLog("Remove a memory content");
  m_hexView->removeContentArea(contentInfo);
}

void RamG::bindVisible(bool visible) { m_visible = visible; }

void RamG::addContentItem(ContentInfo *contentInfo) {
  m_hexView->addContentArea(contentInfo);
}

void RamG::updateView() {
  auto ramG = BT8XXEMU_getRam(g_Emulator);
  QByteArray byteArr;
  auto ramGChar = static_cast<char *>(static_cast<void *>(ramG));
  int ramSize = g_Addr[FTEDITOR_CURRENT_DEVICE][FTEDITOR_RAM_G_END];
  byteArr.append(ramGChar, ramSize);
  auto data = m_hexView->data();
  if (data && byteArr == data->getData(0, data->size())) {
    return;
  }
  debugLog("RAM_G - Content has a change");
  m_hexView->setData(new QHexView::DataStorageArray(byteArr));
  m_hexView->viewport()->update();
  handleContentItemPressed(
      m_insp->mainWindow()->contentManager()->contentList()->currentItem());
}

void RamG::goToAddress() {
  bool ok;
  QString text = m_leAddress->text();
  int address = text.toUInt(&ok, 10);
  if (!ok) {
    address = text.toUInt(&ok, 16);
  }
  if (ok) {
    if (!m_hexView->showFromAddress(address)) return;
    m_hexView->setSelectedAddress(address, 1);
    m_hexView->updateUint();
    m_hexView->setFocus();
  }
}

void RamG::setLabelUint(QString valueStr, uint value) {
  QString text = QString(tr("Uint: %1 (%2)")).arg(value).arg(valueStr);
  m_lbUint->setText(text);
}

}  // namespace FTEDITOR
