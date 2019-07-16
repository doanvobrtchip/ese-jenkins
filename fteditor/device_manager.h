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

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace FTEDITOR
{

class MainWindow;
class DeviceDisplaySettingsDialog;

class ContentManager;

#if FT800_DEVICE_MANAGER

/**
 * DeviceManager
 * \brief DeviceManager
 * \author Paul Jiao Shouwu
 * \author Jan Boon (Kaetemi)
 */
class DeviceManager : public QWidget
{
	Q_OBJECT

public:
	DeviceManager(MainWindow *parent);
	virtual ~DeviceManager();

	typedef uint32_t DeviceId; // Change type to whatever needed

	struct DeviceInfo
	{
		DeviceId Id;
		size_t DeviceIdx;
		int Host;
		int DeviceIntf;
		std::string SerialNumber;
		// std::string DisplayName;
		QTreeWidgetItem *View;
		void *EveHalContext;
	};

	typedef DeviceInfo *EveModuleDeviceInforPtr;

private:
	MainWindow *m_MainWindow;
	QTreeWidget *m_DeviceList;
	std::map<DeviceId, DeviceInfo *> m_DeviceInfo;
	QPushButton *m_ConnectButton;
	QPushButton *m_DisconnectButton;
	QPushButton *m_UploadRamDlButton;
	QPushButton *m_UploadCoprocessorContentButton;
	QPushButton *m_UploadFlashButton;
	DeviceDisplaySettingsDialog *m_displaySettingsDialog;

private slots:
	void deviceDisplaySettings();
	void refreshDevices();
	void connectDevice();
	//bool connectDeviceBT8xx(Gpu_Hal_Context_t *phost, DeviceInfo * devInfo);
	//bool connectDeviceFT8xx(Gpu_Hal_Context_t *phost, DeviceInfo * devInfo);
	void disconnectDevice();
	//void syncDevice();
	//void loadContent2Device(ContentManager *contentManager, Gpu_Hal_Context_t *phost);
	void uploadRamDl();
	void uploadCoprocessorContent();
	void uploadFlash();
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
	void updateSelection();

private:
	DeviceManager(const DeviceManager &) = delete;
	DeviceManager &operator=(const DeviceManager &) = delete;
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
