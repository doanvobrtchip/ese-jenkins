#ifndef DEVICE_ADD_NEW_DIALOG_H_
#define DEVICE_ADD_NEW_DIALOG_H_

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
#include "device_manager.h"
#include "ui_add_new_device.h"

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

class DeviceManager;

class DeviceAddNewDialog : public QDialog
{
	Q_OBJECT

	static const QStringList PROPERTIES;

public:
	explicit DeviceAddNewDialog(QWidget *parent = 0);
	void execute();
	void editDevice(QString jsonPath);
	void examineDevice(QString jsonPath);

	static QString buildJsonFilePath(QString name);

signals:
	void deviceAdded(QString deviceName, QString jsonPath);
	void deviceEdited(QString deviceName, QString jsonPath);

private slots:
	void addDevice();
	void onEveTypeChange(QString eveType);

private:
	
	void prepareData();
	void loadData(QString jsonPath);
	void showData(QString jsonPath);
	bool isExistDeviceName(QString nameToCheck);

private:
	Ui::DeviceAddNewDialog *ui;

	bool isEdited;
	QString editPath;
};

#endif
}
#endif // COPTIONDIALOG_H
