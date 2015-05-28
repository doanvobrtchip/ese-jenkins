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
#include <ft8xxemu_inttypes.h>

//#include <deviceDisplaySettingsDialog.h>
// Project includes

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace FTEDITOR {

class MainWindow;
class DeviceDisplaySettingsDialog;

#if FT800_DEVICE_MANAGER

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
	void setDeviceandScreenSize(QString displaySize, QString syncDevice);

	QString getCurrentDisplaySize();
	void setCurrentDisplaySize(QString displaySize);
	QString getSyncDeviceName();
	void setSyncDeviceName(QString deviceName);
	

private:
	typedef uint32_t DeviceId; // Change type to whatever needed
	struct DeviceInfo
	{
		DeviceId Id;
		QTreeWidgetItem *View;
		bool Connected;
		void* handle;
		// Add necessary device specific data here
	};

	QString currScreenSize = "480x272";
	QString selectedSyncDevice = "VM800B43A";
	MainWindow *m_MainWindow;
	QTreeWidget *m_DeviceList;
	std::map<DeviceId, DeviceInfo *> m_DeviceInfo;
	QPushButton *m_ConnectButton;
	QPushButton *m_DisconnectButton;
	QPushButton *m_SendImageButton;
	DeviceDisplaySettingsDialog *displaySettingsDialog = NULL;


private slots:
	void deviceDisplaySettings();
	void refreshDevices();
	void connectDevice();
	void disconnectDevice();
	void syncDevice();
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
	void updateSelection();

private:
	DeviceManager(const DeviceManager &);
	DeviceManager &operator=(const DeviceManager &);
	
	

}; /* class DeviceManager */

#else /* FT800_DEVICE_MANAGER */

class DeviceManager : public QWidget
{
	Q_OBJECT
};

#endif /* FT800_DEVICE_MANAGER */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DEVICE_MANAGER_H */

/* end of file */
