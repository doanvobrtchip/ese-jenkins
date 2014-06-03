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
#include <vc.h>
#include <ft800emu_memory.h>

// Project includes
#include "main_window.h"
#include "dl_parser.h"

using namespace std;

namespace FT800EMUQT {

static const char *regNames[] = {
	"REG_ID", // 0
	"REG_FRAMES", // 1
	"REG_CLOCK", // 2
	"REG_FREQUENCY",// 3
	"REG_RENDERMODE",// 4
	"REG_SNAPY",// 5
	"REG_SNAPSHOT", // 6
	"REG_CPURESET", // 7
	"REG_TAP_CRC", // 8
	"REG_TAP_MASK", // 9
	"REG_HCYCLE", // 10
	"REG_HOFFSET", // 11
	"REG_HSIZE", // 12
	"REG_HSYNC0", // 13
	"REG_HSYNC1", // 14
	"REG_VCYCLE", // 15
	"REG_VOFFSET", // 16
	"REG_VSIZE", // 17
	"REG_VSYNC0",
	"REG_VSYNC1",
	"REG_DLSWAP", // 20
	"REG_ROTATE",
	"REG_OUTBITS",
	"REG_DITHER",
	"REG_SWIZZLE",
	"REG_CSPREAD",
	"REG_PCLK_POL",
	"REG_PCLK",
	"REG_TAG_X",
	"REG_TAG_Y",
	"REG_TAG", // 30
	"REG_VOL_PB", // 31
	"REG_VOL_SOUND", // 32
	"REG_SOUND", // 33
	"REG_PLAY", // 34
	"REG_GPIO_DIR", // 35
	"REG_GPIO", // 36
	"REG_EVE_INT", // 37
	"REG_INT_FLAGS",
	"REG_INT_EN",
	"REG_INT_MASK", // 40
	"REG_PLAYBACK_START",
	"REG_PLAYBACK_LENGTH",
	"REG_PLAYBACK_READPTR",
	"REG_PLAYBACK_FREQ",
	"REG_PLAYBACK_FORMAT",
	"REG_PLAYBACK_LOOP",
	"REG_PLAYBACK_PLAY",
	"REG_PWM_HZ",
	"REG_PWM_DUTY",
	"REG_MACRO_0", // 50
	"REG_MACRO_1",
	"REG_CYA0",
	"REG_CYA1",
	"REG_BUSYBITS",
	"RESERVED", // 55
	"REG_ROMSUB_SEL", // 56
	"REG_CMD_READ",
	"REG_CMD_WRITE",
	"REG_CMD_DL",
	"REG_TOUCH_MODE", // 60
	"REG_TOUCH_ADC_MODE",
	"REG_TOUCH_CHARGE",
	"REG_TOUCH_SETTLE",
	"REG_TOUCH_OVERSAMPLE",
	"REG_TOUCH_RZTHRESH",
	"REG_TOUCH_RAW_XY",
	"REG_TOUCH_RZ",
	"REG_TOUCH_SCREEN_XY",
	"REG_TOUCH_TAG_XY",
	"REG_TOUCH_TAG", // 70
	"REG_TOUCH_TRANSFORM_A",
	"REG_TOUCH_TRANSFORM_B",
	"REG_TOUCH_TRANSFORM_C",
	"REG_TOUCH_TRANSFORM_D",
	"REG_TOUCH_TRANSFORM_E",
	"REG_TOUCH_TRANSFORM_F",
	"RESERVED", // 77
	"RESERVED", // 78
	"REG_DATESTAMP", // 79
	"RESERVED", // 80
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED", // 90
	"RESERVED",
	"RESERVED", // 92
	"REG_TOUCH_DIRECT_XY", // 93
	"REG_TOUCH_DIRECT_Z1Z2", // 94
	"REG_TRACKER" // 95 - SPECIAL CASE
};

#define REG_TRACKER_IDX 95

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
	return DlParser::toString(value);
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
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		stringstream idx;
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

	initDisplayReg();

    for (int i = 0; i < 3; ++i)
        m_Registers->resizeColumnToContents(i);
}

Inspector::~Inspector()
{

}

bool wantId(int id, bool advanced)
{
	if (advanced)
	{
		switch (id)
		{
			// Blacklist undocumented registers
		case 5:
		case 37:
		case 55:
		case 56:
		case 77:
		case 78:
		case 80:
		case 81:
		case 82:
		case 83:
		case 84:
		case 85:
		case 86:
		case 87:
		case 88:
		case 89:
		case 90:
		case 91:
		case 92:
			return false;
		default:
			return true;
		}
	}
	else
	{
		switch (id)
		{
			// Whitelist useful registers
		case 0:
		case 1:
		case 12:
		case 17:
		case 21:
		case 68:
		case 69:
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
		case 75:
		case 76:
		case 57:
		case 58:
		case 59:
		case 95:
			return true;
		default:
			return false;
		}
	}
}

void Inspector::initDisplayReg()
{
	// 102400
	uint8_t *ram = FT800EMU::Memory.getRam();
	for (int i = RAM_REG; i < RAM_REG + (sizeof(regNames) / sizeof(char *) * 4); i += 4)
	{
		int id = (i - RAM_REG) / 4;
		if (wantId(id, false))
		{
			int ii = id == 95 ? REG_TRACKER : i;
			QTreeWidgetItem *item = new QTreeWidgetItem(m_Registers);
			item->setText(0, asRaw(ii));
			item->setText(1, regNames[id]);
			uint32_t regValue = FT800EMU::Memory.rawReadU32(ram, ii);
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
}

void Inspector::releaseDisplayReg()
{
	for (int i = RAM_REG; i < RAM_REG + (sizeof(regNames) / sizeof(char *) * 4); i += 4)
	{
		int id = (i - RAM_REG) / 4;
		if (m_RegisterItems[id])
		{
			delete m_RegisterItems[id];
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

	const uint32_t *dl = FT800EMU::Memory.getDisplayList();
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		if (m_DisplayListCopy[i] != dl[i])
		{
			m_DisplayListCopy[i] = dl[i];
			m_DisplayListUpdate[i] = true;
		}
		if ((m_DisplayListCopy[i] >> 24) == FT800EMU_DL_BITMAP_HANDLE)
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
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		if (m_DisplayListUpdate[i])
		{
			m_DisplayListUpdate[i] = false;
			m_DisplayListItems[i]->setText(1, asRaw(m_DisplayListCopy[i]));
			m_DisplayListItems[i]->setText(2, asText(m_DisplayListCopy[i]));
		}
	}

	uint8_t *ram = FT800EMU::Memory.getRam();
	for (int i = RAM_REG; i < RAM_REG + (sizeof(regNames) / sizeof(char *) * 4); i += 4)
	{
		int id = (i - RAM_REG) / 4;
		if (m_RegisterItems[id])
		{
			int ii = id == 95 ? REG_TRACKER : i;
			uint32_t regValue = FT800EMU::Memory.rawReadU32(ram, ii);
			if (m_RegisterCopy[id] != regValue)
			{
				m_RegisterCopy[id] = regValue;
				m_RegisterItems[id]->setText(2, asRaw(regValue));
				switch (ii)
				{
				case REG_MACRO_0:
				case REG_MACRO_1:
					m_RegisterItems[id]->setText(3, asText(regValue));
					break;
				case REG_TOUCH_SCREEN_XY:
				case REG_TOUCH_TAG_XY:
					m_RegisterItems[id]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFFFF));
					break;
				case REG_TRACKER:
					m_RegisterItems[id]->setText(3, asInt(regValue >> 16) + ", " + asInt(regValue & 0xFF));
					break;
				default:
					m_RegisterItems[id]->setText(3, asInt(regValue));
					break;
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

} /* namespace FT800EMUQT */

/* end of file */
