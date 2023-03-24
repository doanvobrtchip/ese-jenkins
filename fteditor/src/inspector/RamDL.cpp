/*!
 * @file RamDL.cpp
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamDL.h"

#include <QEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtEndian>
// STL includes
#include <stdio.h>

#include <QTreeWidgetItem>
#include <iomanip>
#include <sstream>
// Emulator includes
#include <bt8xxemu_diag.h>

#include "inspector.h"
#include "utils/CommonUtil.h"
#include "utils/ConvertUtil.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamDL::RamDL(Inspector *parent)
    : RamBase(parent), m_dlCMDCount(0), m_firstDisplayCMD(-1) {
  setObjectName("RamDL");
  setWindowTitle("RAM_DL");
  m_btnOpenDlg->setStatusTip(tr("Show the RAM_DL Window"));
  m_lbTitle->setText(tr("RAM_DL"));

  m_hUsage = new bool[FTED_NUM_HANDLES];
  m_displayList = new QTreeWidget(parent);
  m_displayList->setColumnCount(3);
  m_displayList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_displayList->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_displayList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_displayList->installEventFilter(this);
  m_displayList->viewport()->installEventFilter(this);
  m_focusWhenOpen = m_displayList;
  
  m_actCopy = new QAction(parent);
  m_actCopy->setText(tr("Copy"));
  m_menuContext = new QMenu(parent);
  m_menuContext->addAction(m_actCopy);

  QStringList dlHeaders;
  dlHeaders.push_back(tr(""));
  dlHeaders.push_back(tr("Raw"));
  dlHeaders.push_back(tr("Text"));
  m_displayList->setHeaderLabels(dlHeaders);

  QString raw = ConvertUtil::asRaw(0);
  QString text = ConvertUtil::asText(0);
  for (int i = 0; i < FTEDITOR_DL_SIZE; ++i) {
    std::stringstream idx;
    idx << i;
    m_displayListCopy[i] = 0;
    m_displayListUpdate[i] = false;
    m_displayListItems[i] = new QTreeWidgetItem(m_displayList);
    m_displayListItems[i]->setText(0, idx.str().c_str());
    m_displayListItems[i]->setText(1, raw);
    m_displayListItems[i]->setText(2, text);
  }

  m_displayListItems[0]->setText(0, "9999");
  m_displayListItems[0]->setText(1, "0x000000000");
  for (int i = 0; i < 2; ++i) {
    m_displayList->resizeColumnToContents(i);
  }
  m_displayListItems[0]->setText(0, "0");
  m_displayListItems[0]->setText(1, raw);

  auto layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  auto groupBox = new QGroupBox(this);
  auto vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(5, 5, 5, 5);
  vBoxLayout->addLayout(m_lytTitle);
  vBoxLayout->addWidget(m_displayList);
  groupBox->setLayout(vBoxLayout);
  layout->addWidget(groupBox);
  m_widget->setLayout(layout);
  resize(500, 304);
  
  updateData();
  updateView();
  connect(m_actCopy, &QAction::triggered, this, &RamDL::onCopy);
  connect(m_displayList, &QTreeWidget::customContextMenuRequested, this,
          &RamDL::onPrepareContextMenu);
  connect(m_insp, &Inspector::updateData, this, &RamDL::updateData);
  connect(m_insp, &Inspector::updateView, this, &RamDL::updateView);
}

void RamDL::updateData() {
  if (!g_Emulator) return;

  m_dlCMDCount = 0;
  m_firstDisplayCMD = -1;
  for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
    m_hUsage[handle] = false;

  const uint32_t *dl = BT8XXEMU_getDisplayList(g_Emulator);
  for (int i = 0; i < FTEDITOR_DL_SIZE; ++i) {
    if (m_displayListCopy[i] != dl[i]) {
      m_displayListCopy[i] = dl[i];
      m_displayListUpdate[i] = true;
    }
    if ((m_displayListCopy[i] >> 24) == FTEDITOR_DL_BITMAP_HANDLE) {
      uint32_t handle = (m_displayListCopy[i] & 0x1F);
      // printf("BITMAP_HANDLE: %i\n", handle);
      if (handle < FTED_NUM_HANDLES) {
        m_hUsage[handle] = true;
      }
    }

    if (m_displayListCopy[i] > 0) {
      m_dlCMDCount++;
    } else if (m_firstDisplayCMD == -1) {
      m_firstDisplayCMD = i;
    }
  }
}

void RamDL::updateView() {
  for (int i = 0; i < FTEDITOR_DL_SIZE; ++i) {
    if (m_displayListUpdate[i]) {
      m_displayListUpdate[i] = false;
      m_displayListItems[i]->setText(1,
                                     ConvertUtil::asRaw(m_displayListCopy[i]));
      m_displayListItems[i]->setText(2,
                                     ConvertUtil::asText(m_displayListCopy[i]));
    }
  }
}

QByteArray RamDL::getDLBinary(bool isBigEndian) {
  int columnCount = m_displayList->headerItem()->columnCount();
  int currRow = 0, currColumn = 0;
  uint32_t rawValue = 0;
  QTreeWidgetItem *item = 0;
  bool ok;
  bool isDisplayCmd = false;
  int displayCmdPos = 0;
  QString currText;
  QByteArray result;

  typedef union DataConvert {
    uint8_t bytes[4];
    uint32_t data;
  } DataConvert;
  DataConvert dataConvert;

  for (currRow = 0; currRow < m_displayList->topLevelItemCount(); currRow++) {
    item = m_displayList->topLevelItem(currRow);
    for (currColumn = 0; currColumn < columnCount; ++currColumn) {
      currText = item->text(currColumn);
      if (!currText.startsWith("0x")) continue;

      rawValue = currText.toUInt(&ok, 16);
      if (currText == "0x00000000") {
        if (isDisplayCmd && displayCmdPos == (currRow - 1)) return result;
        isDisplayCmd = true;
        displayCmdPos = currRow;
      } else if (isBigEndian && ok) {
        rawValue = qToBigEndian(rawValue);
      }

      dataConvert.data = rawValue;
      for (int i = 3; i >= 0; --i) {
        result.append(dataConvert.bytes[i]);
      }
    }
  }
  return result;
}

QString RamDL::getDLContent(bool isBigEndian) {
  QString text("");
  int len = m_displayList->headerItem()->columnCount();
  int i = 0, j = 0;
  uint32_t lit = 0, big = 0;
  QTreeWidgetItem *item = 0;
  bool ok;
  bool isDisplayCmd = false;
  int pos = 0;
  QString iText("");

  for (i = 0; i < m_displayList->topLevelItemCount(); i++) {
    item = m_displayList->topLevelItem(i);
    for (j = 1; j < len; ++j) {
      iText = item->text(j);

      if (iText == "0x00000000") {
        if (isDisplayCmd && pos == (i - 1)) {
          return text;
        }

        isDisplayCmd = true;
        pos = i;
      }

      if (iText.startsWith("0x")) {
        if (isBigEndian) {
          lit = iText.toUInt(&ok, 16);
          big = 0;
          if (ok) {
            big = qToBigEndian(lit);
          }
          text += QString("0x%1").arg(big, 8, 16, QChar('0')) + "\t// ";
        } else {
          text += iText + "\t// ";
        }
      } else {
        text += iText + '\t';
      }
    }
    text.replace(text.length() - 1, 1, '\n');
  }

  return text;
}

QTreeWidget *RamDL::DisplayList() const { return m_displayList; }

bool *RamDL::handleUsage() const { return m_hUsage; }

void RamDL::onPrepareContextMenu(const QPoint &pos) {
  QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(sender());

  m_menuContext->exec(treeWidget->mapToGlobal(pos));
}

int RamDL::firstDisplayCMD() const { return m_firstDisplayCMD; }

bool RamDL::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_displayList && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->matches(QKeySequence::Copy)) {
      onCopy();
      return true;
    }
  }

  if ((watched == m_displayList || watched == m_displayList->viewport()) &&
      event->type() == QEvent::Wheel) {
    auto e = static_cast<QWheelEvent *>(event);
    if (e->modifiers() & Qt::ControlModifier) {
      int angle = e->angleDelta().y();
      auto ptSize = m_displayList->font().pointSize();
      if (angle > 0 && ptSize < 180) {
        m_displayList->setFont(QFont("Segoe UI", ptSize + 2));
      } else if (angle <= 0 && ptSize > 5) {
        m_displayList->setFont(QFont("Segoe UI", ptSize - 2));
      }
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

int RamDL::dlCMDCount() const { return m_dlCMDCount; }

void RamDL::onCopy() { CommonUtil::copy(m_displayList); }

}  // namespace FTEDITOR
