/**
 * device_manager.cpp
 * $Id$
 * \file device_manager.cpp
 * \brief device_manager.cpp
 * \date 2014-01-27 18:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "device_manager.h"

// STL includes

// Qt includes
#include <QVBoxLayout>

// Emulator includes
#include <vc.h>

// Project includes
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

#if FT800_DEVICE_MANAGER

DeviceManager::DeviceManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);
}

DeviceManager::~DeviceManager()
{

}

#endif /* FT800_DEVICE_MANAGER */

} /* namespace FT800EMUQT */

/* end of file */
