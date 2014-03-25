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

#ifndef FT800EMUQT_DEVICE_MANAGER_H
#define FT800EMUQT_DEVICE_MANAGER_H

#define FT800_DEVICE_MANAGER 0
#define FT800_DEVICE_MANAGER_REALTIME 0

// STL includes
#include <map>

// Qt includes
#include <QWidget>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace FT800EMUQT {

class MainWindow;

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

private:
	typedef uint32_t DeviceId; // Change type to whatever needed
	struct DeviceInfo
	{
		DeviceId Id;
		QTreeWidgetItem *View;
		bool Connected;
		// ...
		// Add necessary device specific data here
	};

	MainWindow *m_MainWindow;
	QTreeWidget *m_DeviceList;
	std::map<DeviceId, DeviceInfo *> m_DeviceInfo;
	QPushButton *m_ConnectButton;
	QPushButton *m_DisconnectButton;
	QPushButton *m_SendImageButton;

private slots:
	void refreshDevices();
	void connectDevice();
	void disconnectDevice();
	void sendImage();
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

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DEVICE_MANAGER_H */

/* end of file */
