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
	"REG_ID",
	"REG_FRAMES",
	"REG_CLOCK",
	"REG_FREQUENCY",
	"REG_RENDERMODE",
	"REG_SNAPY",
	"REG_SNAPSHOT",
	"REG_CPURESET",
	"REG_TAP_CRC",
	"REG_TAP_MASK",
	"REG_HCYCLE",
	"REG_HOFFSET",
	"REG_HSIZE",
	"REG_HSYNC0",
	"REG_HSYNC1",
	"REG_VCYCLE",
	"REG_VOFFSET",
	"REG_VSIZE",
	"REG_VSYNC0",
	"REG_VSYNC1",
	"REG_DLSWAP",
	"REG_ROTATE",
	"REG_OUTBITS",
	"REG_DITHER",
	"REG_SWIZZLE",
	"REG_CSPREAD",
	"REG_PCLK_POL",
	"REG_PCLK",
	"REG_TAG_X",
	"REG_TAG_Y",
	"REG_TAG",
	"REG_VOL_PB",
	"REG_VOL_SOUND",
	"REG_SOUND",
	"REG_PLAY",
	"REG_GPIO_DIR",
	"REG_GPIO",
	"REG_EVE_INT",
	"REG_INT_FLAGS",
	"REG_INT_EN",
	"REG_INT_MASK",
	"REG_PLAYBACK_START",
	"REG_PLAYBACK_LENGTH",
	"REG_PLAYBACK_READPTR",
	"REG_PLAYBACK_FREQ",
	"REG_PLAYBACK_FORMAT",
	"REG_PLAYBACK_LOOP",
	"REG_PLAYBACK_PLAY",
	"REG_PWM_HZ",
	"REG_PWM_DUTY",
	"REG_MACRO_0",
	"REG_MACRO_1",
	"REG_CYA0",
	"REG_CYA1",
	"REG_BUSYBITS",
	"RESERVED",
	"REG_ROMSUB_SEL",
	"REG_CMD_READ",
	"REG_CMD_WRITE",
	"REG_CMD_DL",
	"REG_TOUCH_MODE",
	"REG_TOUCH_ADC_MODE",
	"REG_TOUCH_CHARGE",
	"REG_TOUCH_SETTLE",
	"REG_TOUCH_OVERSAMPLE",
	"REG_TOUCH_RZTHRESH",
	"REG_TOUCH_RAW_XY",
	"REG_TOUCH_RZ",
	"REG_TOUCH_SCREEN_XY",
	"REG_TOUCH_TAG_XY",
	"REG_TOUCH_TAG",
	"REG_TOUCH_TRANSFORM_A",
	"REG_TOUCH_TRANSFORM_B",
	"REG_TOUCH_TRANSFORM_C",
	"REG_TOUCH_TRANSFORM_D",
	"REG_TOUCH_TRANSFORM_E",
	"REG_TOUCH_TRANSFORM_F",
	"RESERVED",
	"RESERVED",
	"REG_DATESTAMP",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"REG_TOUCH_DIRECT_XY",
	"REG_TOUCH_DIRECT_Z1Z2",
};

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
	QHBoxLayout *layout = new QHBoxLayout(this);

	QSplitter *splitter = new QSplitter(this);

	QGroupBox *dlGroup = new QGroupBox(this);
	dlGroup->setTitle(tr("Display List"));
	QVBoxLayout *dlLayout = new QVBoxLayout(this);
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
	regGroup->setTitle(tr("Registers"));
	QVBoxLayout *regLayout = new QVBoxLayout(this);
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

	// 102400
	uint8_t *ram = FT800EMU::Memory.getRam();
	for (int i = RAM_REG; i < RAM_REG + (sizeof(regNames) / sizeof(char *) * 4); i += 4)
	{
		int id = (i - RAM_REG) / 4;
		QTreeWidgetItem *item = new QTreeWidgetItem(m_Registers);
		item->setText(0, asRaw(i));
		item->setText(1, regNames[id]);
		uint32_t regValue = FT800EMU::Memory.rawReadU32(ram, i);
		item->setText(2, asRaw(regValue));
		item->setText(3, asInt(regValue));
		m_RegisterCopy.push_back(regValue);
		m_RegisterItems.push_back(item);
	}

    for (int i = 0; i < 3; ++i)
        m_Registers->resizeColumnToContents(i);
}

Inspector::~Inspector()
{

}

void Inspector::frameEmu()
{
	const uint32_t *dl = FT800EMU::Memory.getDisplayList();
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		if (m_DisplayListCopy[i] != dl[i])
		{
			m_DisplayListCopy[i] = dl[i];
			m_DisplayListUpdate[i] = true;
		}
	}
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
		uint32_t regValue = FT800EMU::Memory.rawReadU32(ram, i);
		if (m_RegisterCopy[id] != regValue)
		{
			m_RegisterCopy[id] = regValue;
			m_RegisterItems[id]->setText(2, asRaw(regValue));
			m_RegisterItems[id]->setText(3, asInt(regValue));
		}
	}
}

} /* namespace FT800EMUQT */

/* end of file */