/**
 * inspector.cpp
 * $Id$
 * \file inspector.cpp
 * \brief inspector.cpp
 * \date 2014-01-29 16:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "inspector.h"

// STL includes
#include <sstream>
#include <iomanip>
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QGroupBox>
#include <QEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QtEndian>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

namespace FTEDITOR {

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

static QString asRaw(uint32_t value)
{
	std::stringstream raw;
	raw << "0x" << std::setfill('0') << std::setw(8) << std::hex << value;
	return raw.str().c_str();
}

static QString asSignedInt16F(uint32_t value)
{
	std::stringstream res;
	res << (int32_t)value;
	double fl = (double)(int32_t)value / 65536.0;
	res << " (" << fl << ")";
	return res.str().c_str();
}

static QString asSignedInt(uint32_t value)
{
	std::stringstream res;
	res << (int32_t)value;
	return res.str().c_str();
}

static QString asInt(uint32_t value)
{
	std::stringstream res;
	res << value;
	return res.str().c_str();
}

static QString asText(uint32_t value)
{
	QString line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, value);

	// verify parsing ->
	DlParsed parsed;
	DlParser::parse(FTEDITOR_CURRENT_DEVICE, parsed, line, false);
	uint32_t compiled = DlParser::compile(FTEDITOR_CURRENT_DEVICE, parsed);
	if (compiled != value && line.toLocal8Bit() != "")
	{
#if _DEBUG
		QByteArray chars = line.toLocal8Bit();
		printf("Parser bug '%s' -> expect %u, compiled %u\n", chars.constData(), value, compiled);
#endif
	}
	// <- verify parsing

	return line;
}

Inspector::Inspector(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
{
	QHBoxLayout *layout = new QHBoxLayout();

	QSplitter *splitter = new QSplitter(this);

	QGroupBox *dlGroup = new QGroupBox(this);
	dlGroup->setTitle(tr("RAM_DL"));
	QVBoxLayout *dlLayout = new QVBoxLayout();
	m_DisplayList = new QTreeWidget(this);
	m_DisplayList->setColumnCount(3);
	m_DisplayList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_DisplayList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_DisplayList->setContextMenuPolicy(Qt::CustomContextMenu);
	m_DisplayList->installEventFilter(this);
    connect(m_DisplayList, &QTreeWidget::customContextMenuRequested, this, &Inspector::onPrepareContextMenu);


	QStringList dlHeaders;
	dlHeaders.push_back(tr(""));
	dlHeaders.push_back(tr("Raw"));
	dlHeaders.push_back(tr("Text"));
	m_DisplayList->setHeaderLabels(dlHeaders);
	dlLayout->addWidget(m_DisplayList);
	dlGroup->setLayout(dlLayout);
	splitter->addWidget(dlGroup);

	QGroupBox *regGroup = new QGroupBox(this);
	regGroup->setTitle(tr("RAM_REG"));
	QVBoxLayout *regLayout = new QVBoxLayout();
	m_Registers = new QTreeWidget(this);
	m_Registers->setColumnCount(4);
	m_Registers->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_Registers->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_Registers->setContextMenuPolicy(Qt::CustomContextMenu);
	m_Registers->installEventFilter(this);
    connect(m_Registers, &QTreeWidget::customContextMenuRequested, this, &Inspector::onPrepareContextMenu);

	QStringList regHeaders;
	regHeaders.push_back(tr("Address"));
	regHeaders.push_back(tr("Id"));
	regHeaders.push_back(tr("Raw"));
	regHeaders.push_back(tr("Text"));
	m_Registers->setHeaderLabels(regHeaders);
	regLayout->addWidget(m_Registers);
	regGroup->setLayout(regLayout);
	splitter->addWidget(regGroup);

	layout->addWidget(splitter);
	setLayout(layout);

	QString raw = asRaw(0);
	QString text = asText(0);
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		std::stringstream idx;
		idx << i;
		QTreeWidgetItem *item = new QTreeWidgetItem(m_DisplayList);
		item->setText(0, idx.str().c_str());
		item->setText(1, raw);
		item->setText(2, text);
		m_DisplayListItems[i] = item;
	}

	m_DisplayListItems[0]->setText(0, "9999");
    for (int i = 0; i < 2; ++i)
        m_DisplayList->resizeColumnToContents(i);
	m_DisplayListItems[0]->setText(0, "0");

	// bindCurrentDevice();
    m_CopyAct = new QAction(tr("&New"), this);
    m_CopyAct->setText(tr("Copy"));
    connect(m_CopyAct, &QAction::triggered, this, &Inspector::onCopy);

    m_ContextMenu = new QMenu(this);
    m_ContextMenu->addAction(m_CopyAct);
	m_countHandleBitmap = 0;
}

Inspector::~Inspector()
{

}

void Inspector::bindCurrentDevice()
{
	initDisplayReg();

	for (int i = 0; i < 3; ++i)
		m_Registers->resizeColumnToContents(i);
}

void Inspector::unbindCurrentDevice()
{
	releaseDisplayReg();
}

bool wantRegister(int regEnum)
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

void Inspector::initDisplayReg()
{
	// 102400
	//const uint8_t *ram = BT8XXEMU_getRam();
	m_RegisterCopy.reserve(FTEDITOR_REG_NB);
	m_RegisterItems.reserve(FTEDITOR_REG_NB);
	for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum)
	{
		uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, regEnum);
		if (wantRegister(regEnum))
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(m_Registers);
			item->setText(0, asRaw(addr));
			item->setText(1, regToString(FTEDITOR_CURRENT_DEVICE, regEnum));
			uint32_t regValue = 0; // reinterpret_cast<const uint32_t &>(ram[addr]);
			item->setText(2, asRaw(regValue));
			item->setText(3, asInt(regValue));
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
}

void Inspector::releaseDisplayReg()
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

void Inspector::frameEmu()
{
	bool handleUsage[FTED_NUM_HANDLES];
	for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
		handleUsage[handle] = false;

	const uint32_t *dl = BT8XXEMU_getDisplayList(g_Emulator);
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_DisplayListCopy[i] != dl[i])
		{
			m_DisplayListCopy[i] = dl[i];
			m_DisplayListUpdate[i] = true;
		}
		if ((m_DisplayListCopy[i] >> 24) == FTEDITOR_DL_BITMAP_HANDLE)
		{
			uint32_t handle = (m_DisplayListCopy[i] & 0x1F);
			// printf("BITMAP_HANDLE: %i\n", handle);
			if (handle < FTED_NUM_HANDLES)
			{
				handleUsage[handle] = true;
			}
		}
	}

	for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
		m_HandleUsage[handle] = handleUsage[handle];

	setCountHandleUsage(countHandleUsage());
}

void Inspector::frameQt()
{
	int dl_cmd_count = 0;
	int first_display_cmd = -1;
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_DisplayListUpdate[i])
		{
			m_DisplayListUpdate[i] = false;
			m_DisplayListItems[i]->setText(1, asRaw(m_DisplayListCopy[i]));
			m_DisplayListItems[i]->setText(2, asText(m_DisplayListCopy[i]));
		}

		if (m_DisplayListCopy[i] > 0)
		{
			dl_cmd_count++;
		}
		else if (first_display_cmd == -1)
		{
			first_display_cmd = i;
		}
	}

	if (m_MainWindow->cmdEditor()->isInvalid())
	{
		dl_cmd_count = 0;	
	}
	else if (!m_MainWindow->dlEditor()->isInvalid())
	{
		// both editor are valid, get the first command DISPLAY()
		dl_cmd_count = first_display_cmd;
	} 

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
					case FTEDITOR_REG_CMD_DL:
					{
					    regValue = (dl_cmd_count << 2);
					    break;
					}
					case FTEDITOR_REG_CMDB_SPACE:
					{
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
					default:
					{
						uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, regEnum);
						regValue = reinterpret_cast<uint32_t &>(ram[addr]);
						break;
					}
				}
				if (m_RegisterCopy[regEnum] != regValue)
				{
					m_RegisterCopy[regEnum] = regValue;
					m_RegisterItems[regEnum]->setText(2, asRaw(regValue));
					switch (regEnum)
					{
					case FTEDITOR_REG_MACRO_0:
					case FTEDITOR_REG_MACRO_1:
						m_RegisterItems[regEnum]->setText(3, asText(regValue));
						break;
					case FTEDITOR_REG_TOUCH_RAW_XY:
					case FTEDITOR_REG_TOUCH_SCREEN_XY:
					case FTEDITOR_REG_TOUCH_TAG_XY:
						m_RegisterItems[regEnum]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFFFF));
						break;
					case FTEDITOR_REG_TRACKER:
						m_RegisterItems[regEnum]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFF));
						break;
					case FTEDITOR_REG_TOUCH_TRANSFORM_A:
					case FTEDITOR_REG_TOUCH_TRANSFORM_B:
					case FTEDITOR_REG_TOUCH_TRANSFORM_C:
					case FTEDITOR_REG_TOUCH_TRANSFORM_D:
					case FTEDITOR_REG_TOUCH_TRANSFORM_E:
					case FTEDITOR_REG_TOUCH_TRANSFORM_F:
						m_RegisterItems[regEnum]->setText(3, asSignedInt16F(regValue));
						break;
					default:
						m_RegisterItems[regEnum]->setText(3, asInt(regValue));
						break;
					}
				}
			}
		}
	}
}

int Inspector::countHandleUsage()
{
	int result = 0;
	for (int i = 0; i < FTED_NUM_HANDLES; ++i)
		if (m_HandleUsage[i])
			++result;
	return result;
}

void Inspector::setCountHandleUsage(int value)
{
	if (m_countHandleBitmap != value)
	{
		m_countHandleBitmap = value;
		emit countHandleBitmapChanged(m_countHandleBitmap);
	}
}

bool Inspector::eventFilter(QObject * watched, QEvent * event)
{
	if ((watched == m_DisplayList || watched == m_Registers) && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers().testFlag(Qt::ControlModifier))
		{
			copy(static_cast<const QTreeWidget *>(watched));
			return true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

QString Inspector::getDisplayListContent(bool isBigEndian)
{
	QString text("");
	int len = m_DisplayList->headerItem()->columnCount();
	int i = 0, j = 0;
	uint32_t lit = 0, big = 0;
	QTreeWidgetItem *item = 0;
	bool ok;
	bool isDisplayCmd = false;
	int pos = 0;
	QString iText("");

	for(i = 0; i < m_DisplayList->topLevelItemCount(); i++)
	{
		item = m_DisplayList->topLevelItem(i);
		for (j = 1; j < len; ++j)
		{
			iText = item->text(j);

			if (iText == "0x00000000")
			{
				if (isDisplayCmd && pos == (i - 1))
				{
					return text;
				}
				
				isDisplayCmd = true;
				pos = i;
			}

			if (iText.startsWith("0x"))
			{
				if (isBigEndian)
				{
					lit = iText.toUInt(&ok, 16);
					big = 0;
					if (ok)
					{
						big = qToBigEndian(lit);
					}
					text += QString("0x%1").arg(big, 8, 16, QChar('0')) + "\t// ";
				}
				else
				{
					text += iText + "\t// ";
				}
			}
			else
			{
				text += iText + '\t';
			}
		}
		text.replace(text.length() - 1, 1, '\n');
	}

	return text;
}

void Inspector::copy(const QTreeWidget * widget)
{
	QList<QTreeWidgetItem *> selectedItems = widget->selectedItems();
	QClipboard * clip = QApplication::clipboard();
	QString copy_text("");

	// get header
	int len = widget->headerItem()->columnCount();
	int i = 0;

	for (i; i < len; ++i)
	{
		copy_text += widget->headerItem()->text(i) + '\t';
	}
	copy_text.replace(copy_text.length() - 1, 1, '\n');

	foreach(QTreeWidgetItem* item, selectedItems)
	{
		for (i = 0; i < len; ++i)
		{
			copy_text += item->text(i) + '\t';
		}
		copy_text.replace(copy_text.length() - 1, 1, '\n');
	}

	clip->setText(copy_text);
}

void Inspector::onPrepareContextMenu(const QPoint &pos)
{
    QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(sender());

    m_ContextMenu->exec(treeWidget->mapToGlobal(pos));
}

void Inspector::onCopy()
{
    copy(m_DisplayList->hasFocus() ? m_DisplayList : m_Registers);
}

} /* namespace FTEDITOR */

/* end of file */
