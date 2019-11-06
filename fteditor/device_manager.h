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
class QLabel;
class QProgressBar;

namespace FTEDITOR
{

class MainWindow;
class DeviceDisplaySettingsDialog;
class ContentManager;
class DeviceManageDialog;

#if FT800_DEVICE_MANAGER

struct CustomDeviceInfo
{
	CustomDeviceInfo();

	QString DeviceName;
	QString Description;
	int EVE_Type;
	QString ConnectionType;
	QString FlashModel;
	int FlashSize;
	QString ScreenSize; // width x height
	bool isBuiltin;

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
	int CUS_REG_OUTBITS;
	bool ExternalOsc;

};

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
		// std::string SerialNumber;
		// std::string DisplayName;
		QTreeWidgetItem *View;
		void *EveHalContext;
	};

	typedef DeviceInfo *EveModuleDeviceInforPtr;
	void setDeviceAndScreenSize(QString displaySize, QString syncDevice, QString jsonPath = QString(), bool custom = false);
	const QString &getSelectedDeviceName() { return m_SelectedDeviceName; }

private:
	MainWindow *m_MainWindow;
	QTreeWidget *m_DeviceList;
	std::map<DeviceId, DeviceInfo *> m_DeviceInfo;
	QPushButton *m_ConnectButton;
	QPushButton *m_DisconnectButton;
	QPushButton *m_UploadRamDlButton;
	QPushButton *m_UploadCoprocessorContentButton;
	QPushButton *m_UploadFlashContentButton;
	QPushButton *m_UploadFlashBlobButton;

	QPushButton *m_RefreshButton;
	QPushButton *m_DeviceDisplayButton;
	QPushButton *m_DeviceManageButton;

	DeviceDisplaySettingsDialog *m_DisplaySettingsDialog;
	DeviceManageDialog *m_DeviceManageDialog;

	bool m_IsCustomDevice;
	CustomDeviceInfo m_CDI;
	QString m_DeviceJsonPath;

	QString m_SelectedDisplaySize;
	QString m_SelectedDeviceName;

	bool m_Busy;
	bool m_Abort;

	QProgressBar *m_StreamProgress;
	uint32_t m_StreamTransfered;

private slots:
	void deviceManage();
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
	void uploadFlashContent();
	void uploadFlashBlob();
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void abortRequest();

private:
	static bool cbCmdWait(void *phost);
	void initProgressDialog(QDialog *progressDialog, QLabel *progressLabel, QProgressBar *progressBar, QProgressBar *progressSubBar);
	void updateSelection();
	bool waitFlush(DeviceInfo *devInfo);

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
