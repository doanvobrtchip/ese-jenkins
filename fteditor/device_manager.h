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

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

#ifdef FTEDITOR_DEVICE_MANAGER
#define FT800_DEVICE_MANAGER 1
#else
#define FT800_DEVICE_MANAGER 0
#endif

// STL includes
#ifdef M_PI
#ifdef _USE_MATH_DEFINES
#undef _USE_MATH_DEFINES
#endif
#else
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#endif
#include <cmath>
#include <map>

// Qt includes
#include <QWidget>
#include <QMap>

#include "ComponentBase.h"

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
struct CustomDeviceInfo;

#if FT800_DEVICE_MANAGER

/**
 * DeviceManager
 * \brief DeviceManager
 * \author Paul Jiao Shouwu
 * \author Jan Boon (Kaetemi)
 */
class DeviceManager : public QWidget, public ComponentBase
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
public slots:
	void setupConnections(QObject *obj) override{};
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
	std::unique_ptr<CustomDeviceInfo> m_CDI;
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

	int projectChipID();

private:
	DeviceManager(const DeviceManager &) = delete;
	DeviceManager &operator=(const DeviceManager &) = delete;
signals:
	void displaySizeChanged(int hSize, int vSize);
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
