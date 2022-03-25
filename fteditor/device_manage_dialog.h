#ifndef DEVICE_MANAGE_DIALOG_H_
#define DEVICE_MANAGE_DIALOG_H_

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

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

// Qt includes
#include <QDialog>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>

// Project includes
#include "ui_device_manage.h"
#include "device_manager.h"

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

class DeviceManager;
class DeviceAddNewDialog;

class DeviceManageDialog : public QDialog
{
	Q_OBJECT

public:
	static const QString DEVICE_SYNC_PATH;
	static const QString BUILD_IN_DEVICE_SYNC_PATH;

public:
	explicit DeviceManageDialog(DeviceManager *parent = 0);
	void execute();

	static QJsonObject getDeviceJson(QString jsonPath);
	static void getCustomDeviceInfo(QString jsonPath, CustomDeviceInfo & cdi);

private:
	void loadDevice(QListWidget *lw, QString jsonPath);
	void loadAllDevice();
	
	void doClone(QListWidget *lw);

private slots:
	void addDevice();
	void editDevice();
	void removeDevice();
	void cloneDevice();
	void cloneBuildInDevice();
	void examineDevice();

	void onDeviceAdded(QString deviceName, QString jsonPath);
	void onDeviceEdited(QString deviceName, QString jsonPath);

private:
	DeviceManager *pParent;
	Ui::DeviceManageDialog *ui;
	
	DeviceAddNewDialog *m_DeviceAddnewDialog;
};

#endif
}
#endif // DEVICE_MANAGE_DIALOG_H_
