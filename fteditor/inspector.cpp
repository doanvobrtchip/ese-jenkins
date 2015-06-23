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

// Emulator includes
#include <ft8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

namespace FTEDITOR {

static QString asRaw(uint32_t value)
{
	std::stringstream raw;
	raw << "0x" << std::setfill('0') << std::setw(8) << std::hex << value;
	return raw.str().c_str();
}

static QString asInt(uint32_t value)
{
	std::stringstream res;
	res << value;
	return res.str().c_str();
}

static QString asText(uint32_t value)
{
	return DlParser::toString(FTEDITOR_CURRENT_DEVICE, value);
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

	bindCurrentDevice();
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
	case FTEDITOR_REG_HSIZE:
	case FTEDITOR_REG_VSIZE:
	case FTEDITOR_REG_ROTATE:
	case FTEDITOR_REG_CMD_READ:
	case FTEDITOR_REG_CMD_WRITE:
	case FTEDITOR_REG_CMD_DL:
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
	default:
		return false;
	}
}

void Inspector::initDisplayReg()
{
	// 102400
	//const uint8_t *ram = FT8XXEMU_getRam();
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

	const uint32_t *dl = FT8XXEMU_getDisplayList();
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
}

void Inspector::frameQt()
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_DisplayListUpdate[i])
		{
			m_DisplayListUpdate[i] = false;
			m_DisplayListItems[i]->setText(1, asRaw(m_DisplayListCopy[i]));
			m_DisplayListItems[i]->setText(2, asText(m_DisplayListCopy[i]));
		}
	}

	if (m_RegisterItems.size() == FTEDITOR_REG_NB)
	{
		uint8_t *ram = FT8XXEMU_getRam();
		for (int regEnum = 0; regEnum < FTEDITOR_REG_NB; ++regEnum)
		{
			if (m_RegisterItems[regEnum])
			{
				uint32_t regValue;
				switch (regEnum)
				{
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
					case FTEDITOR_REG_TOUCH_SCREEN_XY:
					case FTEDITOR_REG_TOUCH_TAG_XY:
						m_RegisterItems[regEnum]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFFFF));
						break;
					case FTEDITOR_REG_TRACKER:
						m_RegisterItems[regEnum]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFF));
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

} /* namespace FTEDITOR */

/* end of file */
