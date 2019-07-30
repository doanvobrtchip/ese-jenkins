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

#ifndef FTEDITOR_DEVICE_MANAGER_H
#define FTEDITOR_DEVICE_MANAGER_H

#ifdef FTEDITOR_DEVICE_MANAGER
#define FT800_DEVICE_MANAGER 1
#else
#define FT800_DEVICE_MANAGER 0
#endif

// STL includes
#include <map>

// Qt includes
#include <QWidget>
#include <qmap.h>

// Emulator includes
#include <bt8xxemu_inttypes.h>

#if FT800_DEVICE_MANAGER
//mpsse lib includes -- Windows
#undef POINTS
#include <Windows.h>
#include "Platform.h"
#include "Gpu_Hal.h"
#endif

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace FTEDITOR {

class MainWindow;
class DeviceDisplaySettingsDialog;
class ContentManager;
class DeviceManageDialog;

#if FT800_DEVICE_MANAGER

typedef struct customDeviceInfo
{
	QString DeviceName;
	int EVE_Type;
	QString ConnectionType;
	QString FlashModel;
	int FlashSize;
	QString ScreenSize;       // width x height

	int CUS_REG_HCYCLE;
	int CUS_REG_HOFFSET;
	int CUS_REG_HSYNC0;
	int CUS_REG_HSYNC1;
	int CUS_REG_VCYCLE;
	int CUS_REG_VOFFSET;
	int CUS_REG_VSYNC0;
	int CUS_REG_VSYNC1;
	int CUS_REG_SWIZZLE;
	int CUS_REG_PCLK_POL;
	int CUS_REG_HSIZE;
	int CUS_REG_VSIZE;
	int CUS_REG_CSPREAD;
	int CUS_REG_DITHER;
	int CUS_REG_PCLK;

	customDeviceInfo & operator = (const customDeviceInfo & s)
	{
		CUS_REG_HCYCLE = s.CUS_REG_HCYCLE;
		return (*this);
	}

} CustomDeviceInfo;

/**
 * DeviceManager
 * \brief DeviceManager
 * \date 2014-01-27 18:59GMT
 * \author Jan Boon (Kaetemi)
 */
class DeviceManager : public QWidget
{
	Q_OBJECT

public:
	DeviceManager(MainWindow *parent);
	virtual ~DeviceManager();
	void setDeviceandScreenSize(QString displaySize, QString syncDevice, QString jsonPath = QString(), bool custom = false);

	QString getCurrentDisplaySize();
	void setCurrentDisplaySize(QString displaySize);
	QString getSyncDeviceName();
	void setSyncDeviceName(QString deviceName);

	typedef uint32_t DeviceId; // Change type to whatever needed

	struct DeviceInfo
	{
		DeviceId Id;
		QTreeWidgetItem *View;
		char description[65];
		bool Connected;
		void* handle;
		// Add necessary device specific data here
	};

	typedef DeviceInfo * EveModuleDeviceInforPtr;

private:


	QString currScreenSize;
	QString selectedSyncDevice;
	MainWindow *m_MainWindow;
	QTreeWidget *m_DeviceList;
	std::map<DeviceId, DeviceInfo *> m_DeviceInfo;
	QPushButton *m_ConnectButton;
	QPushButton *m_DisconnectButton;
	QPushButton *m_SendImageButton;
	DeviceDisplaySettingsDialog *m_displaySettingsDialog;
	DeviceManageDialog *m_DeviceManageDialog;

    int		syncDeviceEVEType;
	bool	m_isCustomDevice;
	CustomDeviceInfo m_CDI;

private slots:
	void deviceManage();
	void deviceDisplaySettings();
	void refreshDevices();
	void connectDevice();
    bool connectDeviceBT8xx(Gpu_Hal_Context_t *phost, DeviceInfo * devInfo);
    bool connectDeviceFT8xx(Gpu_Hal_Context_t *phost, DeviceInfo * devInfo);
	void disconnectDevice();
	void syncDevice();
    void loadContent2Device(ContentManager *contentManager, Gpu_Hal_Context_t *phost);
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
	void updateSelection();

private:
	DeviceManager(const DeviceManager &) {};
	DeviceManager &operator=(const DeviceManager &) {};	
}; /* class DeviceManager */

#else /* FT800_DEVICE_MANAGER */

class DeviceManager : public QWidget
{
	Q_OBJECT
};

#endif /* FT800_DEVICE_MANAGER */



} /* namespace FTEDITOR */

#if FT800_DEVICE_MANAGER

Q_DECLARE_METATYPE(FTEDITOR::DeviceManager::DeviceInfo *)

#endif /* FT800_DEVICE_MANAGER */

#endif /* #ifndef FTEDITOR_DEVICE_MANAGER_H */

/* end of file */
