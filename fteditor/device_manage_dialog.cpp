#include "device_manage_dialog.h"
#include "device_manager.h"
#include "device_add_new_dialog.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include < QDirIterator>

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

const QString DeviceManageDialog::DEVICE_SYNC_PATH = "/device_sync/";

DeviceManageDialog::DeviceManageDialog(DeviceManager *parent)
    : QDialog(parent)
    , pParent(parent)
    , ui(new Ui::DeviceManageDialog)
    , m_DeviceAddnewDialog(NULL)
{
	ui->setupUi(this);

	connect(ui->btnAddNewDevice, SIGNAL(clicked()), this, SLOT(addDevice()));
	connect(ui->btnEditDevice, SIGNAL(clicked()), this, SLOT(editDevice()));
	connect(ui->btnRemoveDevice, SIGNAL(clicked()), this, SLOT(removeDevice()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
}

void DeviceManageDialog::execute()
{
	loadAllDevice();
	show();
}

void DeviceManageDialog::loadDevice(QString jsonPath)
{
	QString deviceName = getDeviceName(jsonPath);

	if (!deviceName.isEmpty())
	{
		QListWidgetItem *item = new QListWidgetItem(deviceName, ui->deviceListWidget);
		item->setData(Qt::UserRole, jsonPath);
		ui->deviceListWidget->addItem(item);
	}
}

void DeviceManageDialog::loadAllDevice()
{
	ui->deviceListWidget->clear();

	QDirIterator it(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		loadDevice(it.next());
	}

	if (ui->deviceListWidget->count() > 0)
	{
		ui->deviceListWidget->item(0)->setSelected(true);
	}
	ui->deviceListWidget->setFocus();
}

void DeviceManageDialog::addDevice()
{
	DeviceAddNewDialog * anDialog = new DeviceAddNewDialog(this);
	connect(anDialog, &DeviceAddNewDialog::deviceAdded, this, &DeviceManageDialog::onDeviceAdded);
	anDialog->execute();
}

void DeviceManageDialog::editDevice()
{
	QListWidgetItem *item = ui->deviceListWidget->selectedItems().count() > 0 ? ui->deviceListWidget->selectedItems().at(0) : NULL;
	if (item)
	{
		ui->deviceListWidget->setCurrentItem(item);
		QString jsonPath = item->data(Qt::UserRole).toString();
		DeviceAddNewDialog *anDialog = new DeviceAddNewDialog(this);
		connect(anDialog, &DeviceAddNewDialog::deviceEdited, this, &DeviceManageDialog::onDeviceEdited);
		anDialog->editDevice(jsonPath);
	}
}

void DeviceManageDialog::removeDevice()
{
	QListWidgetItem *item = ui->deviceListWidget->selectedItems().count() > 0 ? ui->deviceListWidget->selectedItems().at(0) : NULL;
	if (item)
	{
		QString jsonPath = item->data(Qt::UserRole).toString();
		QFile(jsonPath).remove();

		int row = ui->deviceListWidget->row(item);
		ui->deviceListWidget->takeItem(row);
		delete item;
	}
}

void DeviceManageDialog::onDeviceAdded(QString deviceName, QString jsonPath)
{
	QListWidgetItem *item = new QListWidgetItem(deviceName, ui->deviceListWidget);
	item->setData(Qt::UserRole, jsonPath);
	ui->deviceListWidget->addItem(item);
}

void DeviceManageDialog::onDeviceEdited(QString deviceName, QString jsonPath)
{
	QListWidgetItem *item = ui->deviceListWidget->currentItem();
	if (item)
	{
		item->setText(deviceName);
		item->setData(Qt::UserRole, jsonPath);
	}
}

QJsonObject DeviceManageDialog::getDeviceJson(QString jsonPath)
{
	QFile f(jsonPath);
	int count = 0;
	while (!f.open(QIODevice::Text | QIODevice::ReadOnly) && count < 5)
	{
		QThread::msleep(100);
		count++;
	}

	if (!f.isOpen())
		return QJsonObject();

	QJsonDocument jd = QJsonDocument::fromJson(f.readAll());
	QJsonObject jo = jd.object();

	f.close();

	return jo;
}

QString DeviceManageDialog::getDeviceName(QString jsonPath)
{
	QJsonObject jo = getDeviceJson(jsonPath);

	if (jo.contains("Device Name") && jo["Device Name"].isString())
	{
		return jo["Device Name"].toString();
	}
		
	return QString("");
}

QString DeviceManageDialog::getDeviceChip(QString jsonPath)
{
	QJsonObject jo = getDeviceJson(jsonPath);

	if (jo.contains("EVE") && jo["EVE"].isString())
	{
		return jo["EVE"].toString();
	}

	return QString("");
}

QString DeviceManageDialog::getDeviceScreenSize(QString jsonPath)
{
	QJsonObject jo = getDeviceJson(jsonPath);

	if (jo.contains("Screen Width") && jo["Screen Width"].isString() &&
		jo.contains("Screen Height") && jo["Screen Height"].isString())
	{
		return jo["Screen Width"].toString() + "x" + jo["Screen Height"].toString();
	}

	return QString("");
}

#endif
}
