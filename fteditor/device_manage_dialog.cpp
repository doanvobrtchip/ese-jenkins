#include "device_manage_dialog.h"
#include "device_manager.h"
#include "device_add_new_dialog.h"
#include "constant_mapping.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include < QDirIterator>

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

const QString DeviceManageDialog::DEVICE_SYNC_PATH = "/device_sync/";
const QString DeviceManageDialog::BUILD_IN_DEVICE_SYNC_PATH = "/device_sync/build-in/";

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
	connect(ui->btnCloneDevice, SIGNAL(clicked()), this, SLOT(cloneDevice()));
	connect(ui->btnBuildInCloneDevice, SIGNAL(clicked()), this, SLOT(cloneBuildInDevice()));
	connect(ui->btnExamineButton, SIGNAL(clicked()), this, SLOT(examineDevice()));

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
}

void DeviceManageDialog::execute()
{
	loadAllDevice();
	show();
}

void DeviceManageDialog::loadDevice(QListWidget * lw, QString jsonPath)
{
	CustomDeviceInfo cdi;
	getCustomDeviceInfo(jsonPath, cdi);
	QString deviceName = cdi.DeviceName;

	if (!deviceName.isEmpty())
	{
		QListWidgetItem *item = new QListWidgetItem(deviceName, lw);
		item->setData(Qt::UserRole, jsonPath);
		lw->addItem(item);
		lw->setCurrentItem(item);
		lw->setFocus();
	}
}

void DeviceManageDialog::loadAllDevice()
{
	// load custome device
	ui->deviceListWidget->clear();
	ui->BuildInDeviceListWidget->clear();

	QDirIterator it(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QString path = it.next();

		if (path.contains(DeviceManageDialog::BUILD_IN_DEVICE_SYNC_PATH))
			loadDevice(ui->BuildInDeviceListWidget, path);
		else 
			loadDevice(ui->deviceListWidget, path);
	}

	if (ui->deviceListWidget->count() > 0)
	{
		ui->deviceListWidget->item(0)->setSelected(true);
		ui->deviceListWidget->setFocus();
	}
	else
	{
		ui->BuildInDeviceListWidget->setFocus();
	}
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

void DeviceManageDialog::cloneDevice()
{
	doClone(ui->deviceListWidget);
}

void DeviceManageDialog::cloneBuildInDevice()
{
	doClone(ui->BuildInDeviceListWidget);
}

void DeviceManageDialog::doClone(QListWidget *lw)
{
	QListWidgetItem *item = lw->selectedItems().count() > 0 ? lw->selectedItems().at(0) : NULL;
	if (item)
	{
		lw->setCurrentItem(item);
		QString jsonPath = item->data(Qt::UserRole).toString();

		QFileInfo fi(jsonPath);

		QJsonObject jo = getDeviceJson(jsonPath);
		if (!jo.contains("Device Name"))
			return;

		QString newName = DeviceAddNewDialog::buildJsonFilePath(fi.baseName());
		jo["Device Name"] = QFileInfo(newName).baseName();

		QJsonDocument jd;
		jd.setObject(jo);
		
		QFile f(newName);
		if (f.open(QIODevice::WriteOnly))
		{
			f.write(jd.toJson());
			f.close();
		}
		
		loadDevice(ui->deviceListWidget, newName);
		lw->setFocus();
	}
}

void DeviceManageDialog::examineDevice()
{
	QListWidgetItem *item = ui->BuildInDeviceListWidget->selectedItems().count() > 0 ? ui->BuildInDeviceListWidget->selectedItems().at(0) : NULL;
	if (item)
	{
		ui->BuildInDeviceListWidget->setCurrentItem(item);
		QString jsonPath = item->data(Qt::UserRole).toString();
		DeviceAddNewDialog *anDialog = new DeviceAddNewDialog(this);
		anDialog->examineDevice(jsonPath);
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

void DeviceManageDialog::getCustomDeviceInfo(QString jsonPath, CustomDeviceInfo & cdi)
{
	QJsonObject jo = getDeviceJson(jsonPath);

	if (jo.contains("Device Name") && jo["Device Name"].isString())
	{
		cdi.DeviceName = jo["Device Name"].toString();
	}

	if (jo.contains("EVE Type") && jo["EVE Type"].isString())
	{
		if (jo["EVE Type"].toString() == "FT80X")
			cdi.EVE_Type = FTEDITOR_FT800;
		else if (jo["EVE Type"].toString() == "FT81X")
			cdi.EVE_Type = FTEDITOR_FT813;
		else
			cdi.EVE_Type = FTEDITOR_BT815;
	}

	if (jo.contains("Connection Type") && jo["Connection Type"].isString())
	{
		cdi.ConnectionType = jo["Connection Type"].toString();
	}

	if (jo.contains("Flash Model") && jo["Flash Model"].isString())
	{
		cdi.FlashModel = jo["Flash Model"].toString();
	}

	if (jo.contains("Flash Size (MB)") && jo["Flash Size (MB)"].isString())
	{
		cdi.FlashSize = jo["Flash Size (MB)"].toString().toInt();
	}

	if (jo.contains("Screen Width") && jo.contains("Screen Height"))
	{
		cdi.ScreenSize = QString("%1x%2").arg(jo["Screen Width"].toInt()).arg(jo["Screen Height"].toInt());
	}

	if (jo.contains("REG_HCYCLE"))
	{
		cdi.CUS_REG_HCYCLE = jo["REG_HCYCLE"].toInt();
	}

	if (jo.contains("REG_HOFFSET"))
	{
		cdi.CUS_REG_HOFFSET = jo["REG_HOFFSET"].toInt();
	}

	if (jo.contains("REG_HSYNC0"))
	{
		cdi.CUS_REG_HSYNC0 = jo["REG_HSYNC0"].toInt();
	}

	if (jo.contains("REG_HSYNC1"))
	{
		cdi.CUS_REG_HSYNC1 = jo["REG_HSYNC1"].toInt();
	}

	if (jo.contains("REG_VCYCLE"))
	{
		cdi.CUS_REG_VCYCLE = jo["REG_VCYCLE"].toInt();
	}

	if (jo.contains("REG_VOFFSET"))
	{
		cdi.CUS_REG_VOFFSET = jo["REG_VOFFSET"].toInt();
	}

	if (jo.contains("REG_VSYNC0"))
	{
		cdi.CUS_REG_VSYNC0 = jo["REG_VSYNC0"].toInt();
	}

	if (jo.contains("REG_VSYNC1"))
	{
		cdi.CUS_REG_VSYNC1 = jo["REG_VSYNC1"].toInt();
	}

	if (jo.contains("REG_SWIZZLE"))
	{
		cdi.CUS_REG_SWIZZLE = jo["REG_SWIZZLE"].toInt();
	}

	if (jo.contains("REG_PCLK_POL"))
	{
		cdi.CUS_REG_PCLK_POL = jo["REG_PCLK_POL"].toInt();
	}

	if (jo.contains("REG_HSIZE"))
	{
		cdi.CUS_REG_HSIZE = jo["REG_HSIZE"].toInt();
	}

	if (jo.contains("REG_VSIZE"))
	{
		cdi.CUS_REG_VSIZE = jo["REG_VSIZE"].toInt();
	}

	if (jo.contains("REG_CSPREAD"))
	{
		cdi.CUS_REG_CSPREAD = jo["REG_CSPREAD"].toInt();
	}

	if (jo.contains("REG_DITHER"))
	{
		cdi.CUS_REG_DITHER = jo["REG_DITHER"].toInt();
	}

	if (jo.contains("REG_PCLK"))
	{
		cdi.CUS_REG_PCLK = jo["REG_PCLK"].toInt();
	}
}

#endif
}
