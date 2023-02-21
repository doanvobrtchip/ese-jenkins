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
#include <stdio.h>

#include <iomanip>
#include <sstream>

// Qt includes
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QMenu>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QtEndian>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "constant_common.h"
#include "constant_mapping.h"
#include "inspector/ram_dl/RamDLDockWidget.h"
#include "inspector/ram_dl/RamDLInspector.h"
#include "inspector/ram_g/RamGDockWidget.h"
#include "inspector/ram_g/RamGInspector.h"
#include "inspector/ram_reg/RamRegDockWidget.h"
#include "inspector/ram_reg/RamRegInspector.h"
#include "main_window.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

Inspector::Inspector(MainWindow *parent)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_RamDL(NULL)
    , m_RamDLDockWidget(NULL)
    , m_RamReg(NULL)
    , m_RamRegDockWidget(NULL)
    , m_RamG(NULL)
    , m_RamGDockWidget(NULL)
{
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(3, 0, 0, 2);
	QSplitter *splitter = new QSplitter(this);

	// Set up RAM_DL Inspector
	m_RamDL = new RamDLInspector(this);
	emit m_MainWindow->readyToSetup(m_RamDL);

	auto ramDLWidget = new QWidget(this);
	ramDLWidget->setLayout(m_RamDL->setupComponents());
	splitter->addWidget(ramDLWidget);

	// Set up RAM_REG Inspector
	m_RamReg = new RamRegInspector(this);
	emit m_MainWindow->readyToSetup(m_RamReg);

	auto ramRegWidget = new QWidget(this);
	ramRegWidget->setLayout(m_RamReg->setupComponents());
	splitter->addWidget(ramRegWidget);

	// Set up RAM_G inspector
	m_RamG = new RamGInspector(this);
	emit m_MainWindow->readyToSetup(m_RamG);

	auto ramGWidget = new QWidget(this);
	ramGWidget->setLayout(m_RamG->setupComponents());
	splitter->addWidget(ramGWidget);

	layout->addWidget(splitter);
	setLayout(layout);

	// bindCurrentDevice();
	m_countHandleBitmap = 0;
}

Inspector::~Inspector() { }

void Inspector::bindCurrentDevice() { emit initDisplayReg(); }

void Inspector::unbindCurrentDevice() { emit releaseDisplayReg(); }

void Inspector::frameEmu()
{
	emit updateData();
	auto handleUsage = m_RamDL->handleUsage();

	for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
		m_HandleUsage[handle] = handleUsage[handle];

	setCountHandleUsage(countHandleUsage());
}

void Inspector::frameQt()
{
	int dl_cmd_count = m_RamDL->dlCMDCount();
	int first_display_cmd = m_RamDL->firstDisplayCMD();

	if (m_MainWindow->cmdEditor()->isInvalid())
	{
		dl_cmd_count = 0;
	}
	else if (!m_MainWindow->dlEditor()->isInvalid())
	{
		// both editor are valid, get the first command DISPLAY()
		dl_cmd_count = first_display_cmd;
	}

	emit updateView(dl_cmd_count);
}

int Inspector::countHandleUsage()
{
	int result = 0;
	for (int i = 0; i < FTED_NUM_HANDLES; ++i)
		if (m_HandleUsage[i]) ++result;
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

QByteArray Inspector::getDLBinary(bool isBigEndian)
{
	return m_RamDL->getDLBinary(isBigEndian);
}

QString Inspector::getDLContent(bool isBigEndian)
{
	return m_RamDL->getDLContent(isBigEndian);
}

RamGInspector *Inspector::ramGInspector() const { return m_RamG; }

RamGDockWidget *Inspector::ramGDockWidget() const { return m_RamGDockWidget; }

void Inspector::initRamGDockWidget()
{
	m_RamGDockWidget = new RamGDockWidget(this);
	emit m_MainWindow->readyToSetup(m_RamGDockWidget);
	connect(m_RamGDockWidget, &QObject::destroyed, this,
	    [this](QObject *obj) { m_RamGDockWidget = NULL; });
}

RamRegDockWidget *Inspector::ramRegDockWidget() const { return m_RamRegDockWidget; }

void Inspector::initRamRegDockWidget()
{
	m_RamRegDockWidget = new RamRegDockWidget(this);
	emit m_MainWindow->readyToSetup(m_RamRegDockWidget);
	connect(m_RamRegDockWidget, &QObject::destroyed, this,
	    [this](QObject *obj) { m_RamRegDockWidget = NULL; });
}

RamDLDockWidget *Inspector::ramDLDockWidget() const { return m_RamDLDockWidget; }

void Inspector::initRamDLDockWidget()
{
	m_RamDLDockWidget = new RamDLDockWidget(this);
	emit m_MainWindow->readyToSetup(m_RamDLDockWidget);
	connect(m_RamDLDockWidget, &QObject::destroyed, this,
	    [this](QObject *obj) { m_RamDLDockWidget = NULL; });
}

} /* namespace FTEDITOR */

/* end of file */
