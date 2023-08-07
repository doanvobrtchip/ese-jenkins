/*!
 * @file RamReg.cpp
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamReg.h"

// Emulator includes
#include <bt8xxemu_diag.h>

#include <QEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "constant_mapping.h"
#include "inspector.h"
#include "utils/CommonUtil.h"
#include "utils/ConvertUtil.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamReg::RamReg(Inspector *parent) : RamBase(parent) {
  setObjectName("RamReg");
  setWindowTitle("RAM_REG");
  m_btnOpenDlg->setStatusTip(tr("Show the RAM_REG Window"));
  m_lbTitle->setText(tr("RAM_REG"));

  m_registers = new QTreeWidget(parent);
  m_registers->setColumnCount(4);
  m_registers->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_registers->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_registers->setContextMenuPolicy(Qt::CustomContextMenu);
  m_registers->setIndentation(2);
  m_registers->installEventFilter(this);
  m_registers->viewport()->installEventFilter(this);
  m_focusWhenOpen = m_registers;

  m_actCopy = new QAction(parent);
  m_actCopy->setText(tr("Copy"));

  m_menuContext = new QMenu(parent);
  m_menuContext->addAction(m_actCopy);

  QStringList regHeaders;
  regHeaders.push_back(tr("Address"));
  regHeaders.push_back(tr("Id"));
  regHeaders.push_back(tr("Raw"));
  regHeaders.push_back(tr("Text"));
  m_registers->setHeaderLabels(regHeaders);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  QGroupBox *groupBox = new QGroupBox(this);
  QVBoxLayout *vBoxLayout = new QVBoxLayout();
  vBoxLayout->setContentsMargins(3, 3, 3, 3);
  vBoxLayout->addLayout(m_lytTitle);
  vBoxLayout->addWidget(m_registers);
  groupBox->setLayout(vBoxLayout);
  layout->addWidget(groupBox);
  m_widget->setLayout(layout);
  resize(500, 357);

  connect(m_actCopy, &QAction::triggered, this, &RamReg::onCopy);
  connect(m_registers, &QTreeWidget::customContextMenuRequested, this,
          &RamReg::onPrepareContextMenu);
  connect(m_insp, &Inspector::updateView, this, &RamReg::updateView);
  connect(m_insp, &Inspector::initDisplayReg, this, &RamReg::initDisplayReg);
  connect(m_insp, &Inspector::releaseDisplayReg, this,
          &RamReg::releaseDisplayReg);
}

bool RamReg::wantRegister(int regEnum) {
  switch (regEnum) {
      // Whitelist useful registers
    case FTEDITOR_REG_ID:
    case FTEDITOR_REG_FRAMES:
    case FTEDITOR_REG_CLOCK:
    case FTEDITOR_REG_HSIZE:
    case FTEDITOR_REG_VSIZE:
    case FTEDITOR_REG_ROTATE:
    case FTEDITOR_REG_CMD_READ:
    case FTEDITOR_REG_CMD_WRITE:
    case FTEDITOR_REG_CMD_DL:
    case FTEDITOR_REG_TOUCH_RZ:
    case FTEDITOR_REG_TOUCH_RAW_XY:
    case FTEDITOR_REG_TOUCH_SCREEN_XY:
    case FTEDITOR_REG_TOUCH_TAG_XY:
    case FTEDITOR_REG_TOUCH_TAG:
    case FTEDITOR_REG_TOUCH_TRANSFORM_A:
    case FTEDITOR_REG_TOUCH_TRANSFORM_B:
    case FTEDITOR_REG_TOUCH_TRANSFORM_C:
    case FTEDITOR_REG_TOUCH_TRANSFORM_D:
    case FTEDITOR_REG_TOUCH_TRANSFORM_E:
    case FTEDITOR_REG_TOUCH_TRANSFORM_F:
    case FTEDITOR_REG_TRACKER:
    case FTEDITOR_REG_MACRO_0:
    case FTEDITOR_REG_MACRO_1:
      return true;
    case FTEDITOR_REG_CMDB_SPACE:
    case FTEDITOR_REG_MEDIAFIFO_READ:
    case FTEDITOR_REG_MEDIAFIFO_WRITE:
      return FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810;
    case FTEDITOR_REG_FLASH_STATUS:
    case FTEDITOR_REG_MEDIAFIFO_BASE:
    case FTEDITOR_REG_MEDIAFIFO_SIZE:
    case FTEDITOR_REG_FLASH_SIZE:
    case FTEDITOR_REG_ANIM_ACTIVE:
    case FTEDITOR_REG_PLAY_CONTROL:
      return FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815;
    default:
      return false;
  }
}

void RamReg::updateView(int dlCMDCount) {
  if (m_registerItems.size() == FTEDITOR_REG_NB) {
    uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
    for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum) {
      if (m_registerItems[regEnum]) {
        uint32_t regValue;
        switch (regEnum) {
          case FTEDITOR_REG_CMD_DL: {
            regValue = (dlCMDCount << 2);
            break;
          }
          case FTEDITOR_REG_CMDB_SPACE: {
            /*uint32_t wp = rawReadU32(REG_CMD_WRITE);
            uint32_t rp = rawReadU32(REG_CMD_READ);
            return 4092 - ((wp - rp) & 0xFFF);*/
            uint32_t wpaddr =
                reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE);
            uint32_t rpaddr =
                reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ);
            uint32_t wp = reinterpret_cast<uint32_t &>(ram[wpaddr]);
            uint32_t rp = reinterpret_cast<uint32_t &>(ram[rpaddr]);
            regValue = 4092 - ((wp - rp) & 0xFFF);
            break;
          }
          default: {
            uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, regEnum);
            regValue = reinterpret_cast<uint32_t &>(ram[addr]);
            break;
          }
        }
        if (m_registerCopy[regEnum] != regValue) {
          m_registerCopy[regEnum] = regValue;
          m_registerItems[regEnum]->setText(2, ConvertUtil::asRaw(regValue));
          switch (regEnum) {
            case FTEDITOR_REG_MACRO_0:
            case FTEDITOR_REG_MACRO_1:
              m_registerItems[regEnum]->setText(3,
                                                ConvertUtil::asText(regValue));
              break;
            case FTEDITOR_REG_TOUCH_RAW_XY:
            case FTEDITOR_REG_TOUCH_SCREEN_XY:
            case FTEDITOR_REG_TOUCH_TAG_XY:
              m_registerItems[regEnum]->setText(
                  3, ConvertUtil::asInt(regValue >> 16) + ", " +
                         ConvertUtil::asInt(regValue & 0xFFFF));
              break;
            case FTEDITOR_REG_TRACKER:
              m_registerItems[regEnum]->setText(
                  3, ConvertUtil::asInt(regValue >> 16) + ", " +
                         ConvertUtil::asInt(regValue & 0xFF));
              break;
            case FTEDITOR_REG_TOUCH_TRANSFORM_A:
            case FTEDITOR_REG_TOUCH_TRANSFORM_B:
            case FTEDITOR_REG_TOUCH_TRANSFORM_C:
            case FTEDITOR_REG_TOUCH_TRANSFORM_D:
            case FTEDITOR_REG_TOUCH_TRANSFORM_E:
            case FTEDITOR_REG_TOUCH_TRANSFORM_F:
              m_registerItems[regEnum]->setText(
                  3, ConvertUtil::asSignedInt16F(regValue));
              break;
            case FTEDITOR_REG_PLAY_CONTROL:
              emit regPlayControlChanged(regValue & 0xFF);
            default:
              m_registerItems[regEnum]->setText(3,
                                                ConvertUtil::asInt(regValue));
              break;
          }
        }
      }
    }
  }
}

void RamReg::initDisplayReg() {
  // 102400
  // const uint8_t *ram = BT8XXEMU_getRam();
  m_registerCopy.reserve(FTEDITOR_REG_NB);
  m_registerItems.reserve(FTEDITOR_REG_NB);
  for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum) {
    if (wantRegister(regEnum)) {
	  uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, regEnum);
      QTreeWidgetItem *item = new QTreeWidgetItem(m_registers);
      item->setText(0, ConvertUtil::asRaw(addr));
      item->setText(1, regToString(FTEDITOR_CURRENT_DEVICE, regEnum));
      uint32_t regValue = 0;  // reinterpret_cast<const uint32_t &>(ram[addr]);
      item->setText(2, ConvertUtil::asRaw(regValue));
      item->setText(3, ConvertUtil::asInt(regValue));
      m_registerCopy.push_back(regValue);
      m_registerItems.push_back(item);
    } else {
      m_registerCopy.push_back(0);
      m_registerItems.push_back(NULL);
    }
  }
  m_registerCopy.resize(FTEDITOR_REG_NB);
  m_registerItems.resize(FTEDITOR_REG_NB);

  for (int i = 0; i < 3; ++i) m_registers->resizeColumnToContents(i);
}

void RamReg::releaseDisplayReg() {
  for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum) {
    if (m_registerItems[regEnum]) {
      delete m_registerItems[regEnum];
    }
  }
  m_registerCopy.clear();
  m_registerItems.clear();
}

void RamReg::onPrepareContextMenu(const QPoint &pos) {
  QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(sender());

  m_menuContext->exec(treeWidget->mapToGlobal(pos));
}

void RamReg::onCopy() { CommonUtil::copy(m_registers); }

bool RamReg::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_registers && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->matches(QKeySequence::Copy)) {
      onCopy();
      return true;
    }
  }
  if ((watched == m_registers || watched == m_registers->viewport()) &&
      event->type() == QEvent::Wheel) {
    auto e = static_cast<QWheelEvent *>(event);
    if (e->modifiers() & Qt::ControlModifier) {
      int angle = e->angleDelta().y();
      auto ptSize = m_registers->font().pointSize();
      if (angle > 0 && ptSize < 180) {
        m_registers->setFont(QFont("Segoe UI", ptSize + 2));
      } else if (angle <= 0 && ptSize > 5) {
        m_registers->setFont(QFont("Segoe UI", ptSize - 2));
      }
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

}  // namespace FTEDITOR
