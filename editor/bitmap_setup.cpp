/**
 * bitmap_setup.cpp
 * $Id$
 * \file bitmap_setup.cpp
 * \brief bitmap_setup.cpp
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "bitmap_setup.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QHeaderView>
#include <QUndoCommand>
#include <QFileDialog>
#include <QFileInfo>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QJsonDocument>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateTime>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

BitmapSetup::BitmapSetup(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
{

}

BitmapSetup::~BitmapSetup()
{

}

} /* namespace FT800EMUQT */

/* end of file */
