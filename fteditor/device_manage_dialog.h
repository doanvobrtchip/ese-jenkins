#ifndef DEVICE_MANAGE_DIALOG_H_
#define DEVICE_MANAGE_DIALOG_H_

#include <QDialog>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <qradiobutton.h>
#include <qgroupbox.h>
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

public:
	explicit DeviceManageDialog(DeviceManager *parent = 0);
	void execute();

	static QJsonObject getDeviceJson(QString jsonPath);
	static void getCustomDeviceInfo(QString jsonPath, CustomDeviceInfo & cdi);

private:
	void loadDevice(QString jsonPath);
	void loadAllDevice();
	
private slots:
	void addDevice();
	void editDevice();
	void removeDevice();
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
