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

signals:
	void deviceAdded(QString deviceName, QString jsonPath);
	void deviceEdited(QString deviceName, QString jsonPath);

private slots:
	void addDevice();

private:
	QString buildJsonFilePath(QString name);
	void prepareData();
	void loadData(QString jsonPath);

private:
	Ui::DeviceAddNewDialog *ui;

	bool isEdited;
	QString editPath;
};

#endif
}
#endif // COPTIONDIALOG_H
