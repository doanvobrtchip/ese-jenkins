/*!
 * @file RamG.cpp
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamG.h"
// Emulator includes
#include <bt8xxemu_diag.h>

#include <QDockWidget>
#include <QGroupBox>
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

RamG::RamG(Inspector *parent) : QWidget(parent), m_Inspector(parent) {
  m_HexView = new QHexView;
  lineEdit = new QLineEdit;
  lbUint = new QLabel(tr("Uint:"));

  setupConnections();
  connect(m_Inspector->mainWindow(), SIGNAL(readyToSetup(QObject *)), this,
          SLOT(setupConnections(QObject *)));
  emit m_Inspector->mainWindow()->readyToSetup(this);
}

void RamG::setupConnections(QObject *obj) {
  auto m_MainWindow = m_Inspector->mainWindow();
  if (m_MainWindow->contentManager() &&
      (obj == m_MainWindow->contentManager() || obj == nullptr)) {
    auto contentList = m_MainWindow->contentManager()->contentList();
    connect(contentList, &QTreeWidget::currentItemChanged, this,
            &RamG::currItemContentChanged, Qt::UniqueConnection);
    connect(contentList, &QTreeWidget::itemPressed, this,
            &RamG::handleContentItemPressed, Qt::UniqueConnection);
    connect(contentList, &ContentTreeWidget::removeItem, this,
            &RamG::removeContentItem, Qt::UniqueConnection);
    connect(contentList, &ContentTreeWidget::addItem, this,
            &RamG::addContentItem, Qt::UniqueConnection);

    connect(m_MainWindow->contentManager(),
            &ContentManager::ramGlobalUsageChanged, this, &RamG::updateData,
            Qt::UniqueConnection);
    connect(m_HexView, &QHexView::currentInfoChanged, this,
            &RamG::handleCurrentInfoChanged, Qt::UniqueConnection);
  }

  if (m_MainWindow->inspectorDock() &&
      (obj == m_MainWindow->inspectorDock() || obj == nullptr)) {
    connect(m_MainWindow->inspectorDock(), &QDockWidget::visibilityChanged,
            this, &RamG::bindVisible, Qt::UniqueConnection);
  }
}

void RamG::updateData(int ramUsage) {
  auto ramG = BT8XXEMU_getRam(g_Emulator);
  int ramSize = g_Addr[FTEDITOR_CURRENT_DEVICE][FTEDITOR_RAM_G_END];
  QByteArray byteArr;
  auto temp = static_cast<char *>(static_cast<void *>(ramG));
  byteArr.append(temp, ramSize);
  auto data = m_HexView->data();
  if (data && byteArr == data->getData(0, data->size()) && ramUsage == -1) {
    return;
  }
  debugLog("RAM_G - Content has a change");
  m_HexView->setData(new QHexView::DataStorageArray(byteArr));
  m_HexView->viewport()->update();
  auto m_MainWindow = m_Inspector->mainWindow();
  handleContentItemPressed(
      m_MainWindow->contentManager()->contentList()->currentItem());
}

void RamG::currItemContentChanged(QTreeWidgetItem *current,
                                  QTreeWidgetItem *previous) {
  if (!current) {
    debugLog("No selected content");
    m_HexView->setSelected(0, 0);
    return;
  };
}

void RamG::handleContentItemPressed(QTreeWidgetItem *item) {
  if (!item) {
    m_HexView->setSelected(0, 0);
    return;
  };
  auto contentInfo =
      (ContentInfo *)(void *)item->data(0, Qt::UserRole).value<quintptr>();

  auto size =
      m_Inspector->mainWindow()->contentManager()->getContentSize(contentInfo);
  if (!contentInfo->MemoryLoaded || size <= 0 ||
      !contentInfo->BuildError.isEmpty()) {
    debugLog("Selected a non-memory content");
    m_HexView->setSelected(0, 0);
    return;
  }

  debugLog(QString("Selected a memory content | Addr: %1 | Size: %2 | Name: %3")
               .arg(contentInfo->MemoryAddress)
               .arg(size)
               .arg(contentInfo->DestName));
  m_HexView->showFromOffset(contentInfo->MemoryAddress);
  m_HexView->setSelected(contentInfo->MemoryAddress, size);
  if (m_Visible) {
    m_HexView->setFocus(Qt::MouseFocusReason);
  }
}

void RamG::bindVisible(bool visible) { m_Visible = visible; }

void RamG::removeContentItem(ContentInfo *contentInfo) {
  auto size =
      m_Inspector->mainWindow()->contentManager()->getContentSize(contentInfo);
  if (contentInfo->DataStorage == ContentInfo::Flash || size < 0) {
    debugLog("Remove a non-memory content");
    return;
  }
  debugLog("Remove a memory content");
  m_HexView->removeContentArea(contentInfo);
}

void RamG::addContentItem(ContentInfo *contentInfo) {
  m_HexView->addContentArea(contentInfo);
}

void RamG::handleCurrentInfoChanged(ContentInfo *contentInfo) {
  emit updateCurrentInfo(contentInfo);
}

void RamG::setupComponents(QGroupBox *ramGGroup) {
  ramGGroup->setTitle(tr("RAM_G"));
  auto ramgLayout = new QVBoxLayout();

  ramgLayout->addWidget(m_HexView);
  QString placeholderText = tr("Enter decimal or hexadecimal");

  auto lbAddress = new QLabel(tr("Address:"));
  lbAddress->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  // Set default width for QLineEdit
  auto textSize = lineEdit->fontMetrics().size(0, placeholderText);
  lineEdit->setFocus();
  lineEdit->setPlaceholderText(placeholderText);
  lineEdit->setMaximumWidth(textSize.width() + 10);

  auto searchButton = new QPushButton(tr("Jump"));
  searchButton->setDefault(true);
  searchButton->setMaximumWidth(90);

  auto searchLayout = new QHBoxLayout;
  searchLayout->setAlignment(Qt::AlignRight);
  searchLayout->addWidget(lbUint);
  searchLayout->addWidget(lbAddress);
  searchLayout->addWidget(lineEdit);
  searchLayout->addWidget(searchButton);

  ramgLayout->setAlignment(Qt::AlignRight);
  ramgLayout->addLayout(searchLayout);
  ramGGroup->setLayout(ramgLayout);

  auto handleGoToAddress = [this]() {
    bool ok;
    QString text = lineEdit->text();
    int address = text.toUInt(&ok, 10);
    if (!ok) {
      address = text.toUInt(&ok, 16);
    }
    if (ok) {
      m_HexView->showFromOffset(address);
      m_HexView->setSelected(address, 1);
      m_HexView->updateUint();
      m_HexView->setFocus();
    }
  };

  connect(searchButton, &QPushButton::clicked, this, handleGoToAddress);
  connect(lineEdit, &QLineEdit::returnPressed, this, handleGoToAddress);
  connect(m_HexView, &QHexView::uintChanged, this, [this](uint value) {
    QString text = QString(tr("Uint: %1")).arg(value);
    lbUint->setText(text);
  });
}

}  // namespace FTEDITOR
