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

#pragma warning(disable : 26812)  // Unscoped enum
#pragma warning(disable : 26495)  // Uninitialized member
#pragma warning(disable : 26444)  // Unnamed objects

#include "Inspector.h"

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
#include "inspector/RamCMD.h"
#include "inspector/RamDL.h"
#include "inspector/RamG.h"
#include "inspector/RamReg.h"
#include "main_window.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

Inspector::Inspector(MainWindow *parent)
    : QWidget(parent), m_MainWindow(parent) {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(3, 0, 0, 2);
  m_Splitter = new QSplitter(this);

  m_RamDL = new RamDL(this);
  m_RamDL->dockBack();

  m_RamReg = new RamReg(this);
  m_RamReg->dockBack();

  m_RamG = new RamG(this);
  m_RamG->dockBack();

  m_RamCMD = new RamCMD(this);
  m_RamCMD->dockBack();

  layout->addWidget(m_Splitter);

  // bindCurrentDevice();
  m_countHandleBitmap = 0;
  ComponentBase::finishedSetup(this, m_MainWindow);
}

void Inspector::bindCurrentDevice() { emit initDisplayReg(); }

void Inspector::unbindCurrentDevice() { emit releaseDisplayReg(); }

void Inspector::frameEmu() {
  emit updateData();
  auto handleUsage = m_RamDL->handleUsage();

  for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
    m_HandleUsage[handle] = handleUsage[handle];

  setCountHandleUsage(countHandleUsage());
}

void Inspector::frameQt() {
  int dl_cmd_count = m_RamDL->dlCMDCount();
  int first_display_cmd = m_RamDL->firstDisplayCMD();

  if (m_MainWindow->cmdEditor()->isInvalid()) {
    dl_cmd_count = 0;
  } else if (!m_MainWindow->dlEditor()->isInvalid()) {
    // both editor are valid, get the first command DISPLAY()
    dl_cmd_count = first_display_cmd;
  }

  emit updateView(dl_cmd_count);
}

int Inspector::countHandleUsage() {
  int result = 0;
  for (int i = 0; i < FTED_NUM_HANDLES; ++i)
    if (m_HandleUsage[i]) ++result;
  return result;
}

void Inspector::setCountHandleUsage(int value) {
  if (m_countHandleBitmap != value) {
    m_countHandleBitmap = value;
    emit countHandleBitmapChanged(m_countHandleBitmap);
  }
}

QByteArray Inspector::getDLBinary(bool isBigEndian) {
  return m_RamDL->getDLBinary(isBigEndian);
}

QString Inspector::getDLContent(bool isBigEndian) {
  return m_RamDL->getDLContent(isBigEndian);
}

MainWindow *Inspector::mainWindow() { return m_MainWindow; }

void Inspector::addSplitter(QWidget *widget) {
  m_Splitter->addWidget(widget);
}

RamG *Inspector::ramG() const { return m_RamG; }

RamReg *Inspector::ramReg() const { return m_RamReg; }

RamCMD *Inspector::ramCMD() const { return m_RamCMD; }

RamDL *Inspector::ramDL() const { return m_RamDL; }

} /* namespace FTEDITOR */

/* end of file */
