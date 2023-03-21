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

RamReg::RamReg(Inspector *parent)
    : RamBase(parent)
{
	setObjectName("RamReg");
	setWindowTitle("RAM_REG");
	m_OpenDlgBtn->setStatusTip(tr("Show/Hide the RAM_REG Window"));
	m_TitleLabel->setText(tr("RAM_REG"));

	m_Registers = new QTreeWidget(parent);
	m_Registers->setColumnCount(4);
	m_Registers->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_Registers->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Registers->setContextMenuPolicy(Qt::CustomContextMenu);
	m_Registers->installEventFilter(this);
	m_Registers->viewport()->installEventFilter(this);

	m_CopyAct = new QAction(parent);
	m_CopyAct->setText(tr("Copy"));

	m_ContextMenu = new QMenu(parent);
	m_ContextMenu->addAction(m_CopyAct);

	QStringList regHeaders;
	regHeaders.push_back(tr("Address"));
	regHeaders.push_back(tr("Id"));
	regHeaders.push_back(tr("Raw"));
	regHeaders.push_back(tr("Text"));
	m_Registers->setHeaderLabels(regHeaders);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	QGroupBox *groupBox = new QGroupBox(this);
	QVBoxLayout *vBoxLayout = new QVBoxLayout();
	vBoxLayout->setContentsMargins(5, 5, 5, 5);
	vBoxLayout->addLayout(m_TitleLayout);
	vBoxLayout->addWidget(m_Registers);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);
	m_Widget->setLayout(layout);

	connect(m_CopyAct, &QAction::triggered, this, &RamReg::onCopy);
	connect(m_Registers, &QTreeWidget::customContextMenuRequested, this,
	    &RamReg::onPrepareContextMenu);
	connect(m_Inspector, &Inspector::updateView, this, &RamReg::updateView);
	connect(m_Inspector, &Inspector::initDisplayReg, this,
	    &RamReg::initDisplayReg);
	connect(m_Inspector, &Inspector::releaseDisplayReg, this,
	    &RamReg::releaseDisplayReg);
}

bool RamReg::wantRegister(int regEnum)
{
	switch (regEnum)
	{
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

void RamReg::updateView(int dlCMDCount)
{
	if (m_RegisterItems.size() == FTEDITOR_REG_NB)
	{
		uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
		for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum)
		{
			if (m_RegisterItems[regEnum])
			{
				uint32_t regValue;
				switch (regEnum)
				{
				case FTEDITOR_REG_CMD_DL: {
					regValue = (dlCMDCount << 2);
					break;
				}
				case FTEDITOR_REG_CMDB_SPACE: {
					/*uint32_t wp = rawReadU32(REG_CMD_WRITE);
					uint32_t rp = rawReadU32(REG_CMD_READ);
					return 4092 - ((wp - rp) & 0xFFF);*/
					uint32_t wpaddr = reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE);
					uint32_t rpaddr = reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ);
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
				if (m_RegisterCopy[regEnum] != regValue)
				{
					m_RegisterCopy[regEnum] = regValue;
					m_RegisterItems[regEnum]->setText(2, ConvertUtil::asRaw(regValue));
					switch (regEnum)
					{
					case FTEDITOR_REG_MACRO_0:
					case FTEDITOR_REG_MACRO_1:
						m_RegisterItems[regEnum]->setText(3,
						    ConvertUtil::asText(regValue));
						break;
					case FTEDITOR_REG_TOUCH_RAW_XY:
					case FTEDITOR_REG_TOUCH_SCREEN_XY:
					case FTEDITOR_REG_TOUCH_TAG_XY:
						m_RegisterItems[regEnum]->setText(
						    3, ConvertUtil::asInt(regValue >> 16) + ", " + ConvertUtil::asInt(regValue & 0xFFFF));
						break;
					case FTEDITOR_REG_TRACKER:
						m_RegisterItems[regEnum]->setText(
						    3, ConvertUtil::asInt(regValue >> 16) + ", " + ConvertUtil::asInt(regValue & 0xFF));
						break;
					case FTEDITOR_REG_TOUCH_TRANSFORM_A:
					case FTEDITOR_REG_TOUCH_TRANSFORM_B:
					case FTEDITOR_REG_TOUCH_TRANSFORM_C:
					case FTEDITOR_REG_TOUCH_TRANSFORM_D:
					case FTEDITOR_REG_TOUCH_TRANSFORM_E:
					case FTEDITOR_REG_TOUCH_TRANSFORM_F:
						m_RegisterItems[regEnum]->setText(
						    3, ConvertUtil::asSignedInt16F(regValue));
						break;
					default:
						m_RegisterItems[regEnum]->setText(3,
						    ConvertUtil::asInt(regValue));
						break;
					}
				}
			}
		}
	}
}

void RamReg::initDisplayReg()
{
	// 102400
	// const uint8_t *ram = BT8XXEMU_getRam();
	m_RegisterCopy.reserve(FTEDITOR_REG_NB);
	m_RegisterItems.reserve(FTEDITOR_REG_NB);
	for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum)
	{
		uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, regEnum);
		if (wantRegister(regEnum))
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(m_Registers);
			item->setText(0, ConvertUtil::asRaw(addr));
			item->setText(1, regToString(FTEDITOR_CURRENT_DEVICE, regEnum));
			uint32_t regValue = 0; // reinterpret_cast<const uint32_t &>(ram[addr]);
			item->setText(2, ConvertUtil::asRaw(regValue));
			item->setText(3, ConvertUtil::asInt(regValue));
			m_RegisterCopy.push_back(regValue);
			m_RegisterItems.push_back(item);
		}
		else
		{
			m_RegisterCopy.push_back(0);
			m_RegisterItems.push_back(NULL);
		}
	}
	m_RegisterCopy.resize(FTEDITOR_REG_NB);
	m_RegisterItems.resize(FTEDITOR_REG_NB);

	for (int i = 0; i < 3; ++i) m_Registers->resizeColumnToContents(i);
}

void RamReg::releaseDisplayReg()
{
	for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum)
	{
		if (m_RegisterItems[regEnum])
		{
			delete m_RegisterItems[regEnum];
		}
	}
	m_RegisterCopy.clear();
	m_RegisterItems.clear();
}

void RamReg::onPrepareContextMenu(const QPoint &pos)
{
	QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(sender());

	m_ContextMenu->exec(treeWidget->mapToGlobal(pos));
}

void RamReg::onCopy() { CommonUtil::copy(m_Registers); }

void RamReg::openDialog(bool checked)
{
	RamBase::openDialog();
	resize(500, 357);
	m_Registers->setFocus(Qt::MouseFocusReason);
}
void RamReg::dockBack(bool checked)
{
	RamBase::dockBack(checked);
	m_Registers->setFocus(Qt::MouseFocusReason);
}

bool RamReg::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_Registers && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->matches(QKeySequence::Copy))
		{
			onCopy();
			return true;
		}
	}
	if ((watched == m_Registers || watched == m_Registers->viewport()) && event->type() == QEvent::Wheel)
	{
		auto e = static_cast<QWheelEvent *>(event);
		if (e->modifiers() & Qt::ControlModifier)
		{
			int angle = e->angleDelta().y();
			auto ptSize = m_Registers->font().pointSize();
			if (angle > 0 && ptSize < 180)
			{
				m_Registers->setFont(QFont("Segoe UI", ptSize + 2));
			}
			else if (angle <= 0 && ptSize > 5)
			{
				m_Registers->setFont(QFont("Segoe UI", ptSize - 2));
			}
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}

} // namespace FTEDITOR
