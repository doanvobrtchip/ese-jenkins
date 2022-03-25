/**
 * device_manager.h
 * $Id$
 * \file device_manager.h
 * \brief device_manager.h
 * \date 2014-01-27 18:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_DEVICE_INFO_CUSTOM_H
#define FTEDITOR_DEVICE_INFO_CUSTOM_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

 // Emulator includes
#ifdef _USE_MATH_DEFINES
#undef _USE_MATH_DEFINES
#endif
#include <EVE_Hal.h>

namespace FTEDITOR
{

#ifdef FTEDITOR_DEVICE_MANAGER

struct CustomDeviceInfo
{
	CustomDeviceInfo();

	QString DeviceName;
	QString Description;
	int EVE_Type;
	QString ConnectionType;
	QString FlashModel;
	int FlashSize;
	int SystemClock;
	QString ScreenSize; // width x height
	bool isBuiltin;
	bool ExternalClock;

	EVE_ConfigParameters configParams;
};

#endif /* FTEDITOR_DEVICE_MANAGER */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DEVICE_INFO_CUSTOM_H */

/* end of file */
