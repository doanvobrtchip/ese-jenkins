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

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes

// Project includes

namespace FT800EMUQT {

class MainWindow;

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
	struct DeviceInfo
	{
		// ...
	};

	MainWindow *m_MainWindow;
	// QVBoxLayout *m_DeviceList;

private:
	DeviceManager(const DeviceManager &);
	DeviceManager &operator=(const DeviceManager &);

}; /* class DeviceManager */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DEVICE_MANAGER_H */

/* end of file */
