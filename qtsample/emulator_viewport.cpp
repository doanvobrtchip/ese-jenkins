/**
 * emulator_viewport.cpp
 * $Id$
 * \file emulator_viewport.cpp
 * \brief emulator_viewport.cpp
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */
 
#include "emulator_viewport.h"

// STL includes

// Qt includes
#include <QAction>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>

// Emulator includes
#include <ft800emu_graphics_driver.h>

// Project includes
// #include "emulator_config.h"

namespace FTQT {

EmulatorViewport::EmulatorViewport(QWidget *parent) 
	: QWidget(parent)
	// m_EmulatorConfig(NULL)
{
	m_Pixmap = new QPixmap(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT);
	m_Label = new QLabel();
	m_Label->setPixmap(*m_Pixmap);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_Label);
	setLayout(layout);
}

EmulatorViewport::~EmulatorViewport()
{
	delete m_Label; m_Label = NULL;
	delete m_Pixmap; m_Pixmap = NULL;
}

} /* namespace FTQT */

/* end of file */
