
#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "device_manage_dialog.h"

// Qt includes
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QDirIterator>

// Project includes
#include "device_manager.h"
#include "device_add_new_dialog.h"
#include "constant_mapping.h"
#include "device_info_custom.h"

namespace FTEDITOR {

#if FT800_DEVICE_MANAGER

extern QString g_ApplicationDataDir;

const QString DeviceManageDialog::DEVICE_SYNC_PATH = "/device_sync/";
const QString DeviceManageDialog::BUILD_IN_DEVICE_SYNC_PATH = "/device_sync/builtin/";

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

	connect(ui->deviceListWidget, &QListWidget::itemDoubleClicked, ui->btnEditDevice, &QPushButton::clicked);
	connect(ui->BuildInDeviceListWidget, &QListWidget::itemDoubleClicked, ui->btnExamineButton, &QPushButton::clicked);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
}

void DeviceManageDialog::execute()
{
	loadAllDevice();
	show();
}

void DeviceManageDialog::loadDevice(QListWidget *lw, QString jsonPath)
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

	auto processDirectory = [&](QDirIterator &it) {
		while (it.hasNext())
		{
			QString path = it.next();

			if (path.contains(DeviceManageDialog::BUILD_IN_DEVICE_SYNC_PATH))
				loadDevice(ui->BuildInDeviceListWidget, path);
			else
				loadDevice(ui->deviceListWidget, path);
		}
	};

	QDirIterator it(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
	processDirectory(it);

	if (QApplication::applicationDirPath() != g_ApplicationDataDir)
	{
		QDirIterator it(g_ApplicationDataDir + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
		processDirectory(it);
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
	DeviceAddNewDialog *anDialog = new DeviceAddNewDialog(this);
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
		jo["Built-in"] = false;

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

void DeviceManageDialog::getCustomDeviceInfo(QString jsonPath, CustomDeviceInfo &cdi)
{
	QJsonObject jo = getDeviceJson(jsonPath);

	if (jo.contains("Device Name") && jo["Device Name"].isString())
	{
		cdi.DeviceName = jo["Device Name"].toString();
	}

	if (jo.contains("Description") && jo["Description"].isString())
	{
		cdi.Description = jo["Description"].toString();
	}

	if (jo.contains("Built-in") && jo["Built-in"].isBool())
	{
		cdi.isBuiltin = jo["Built-in"].toBool();
	}
	else
	{
		cdi.isBuiltin = false;
	}

	if (jo.contains("EVE Type") && jo["EVE Type"].isString())
	{
		if (jo["EVE Type"].toString() == "FT80X")
			cdi.EVE_Type = FTEDITOR_FT800;
		else if (jo["EVE Type"].toString() == "FT81X")
			cdi.EVE_Type = FTEDITOR_FT810;
		else if (jo["EVE Type"].toString() == "BT88X")
			cdi.EVE_Type = FTEDITOR_BT880;
		else if (jo["EVE Type"].toString() == "BT815_816")
			cdi.EVE_Type = FTEDITOR_BT815;
		else
			cdi.EVE_Type = FTEDITOR_BT817;
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

	if (jo.contains("System Clock (MHz)") && jo["System Clock (MHz)"].isString())
	{
		cdi.SystemClock = jo["System Clock (MHz)"].toString().toInt();
	}

	if (jo.contains("Screen Width") && jo.contains("Screen Height"))
	{
		cdi.ScreenSize = QString("%1x%2").arg(jo["Screen Width"].toInt()).arg(jo["Screen Height"].toInt());
	}

	if (jo.contains("REG_HCYCLE"))
	{
		cdi.configParams.HCycle = jo["REG_HCYCLE"].toInt();
	}

	if (jo.contains("REG_HOFFSET"))
	{
		cdi.configParams.HOffset = jo["REG_HOFFSET"].toInt();
	}

	if (jo.contains("REG_HSYNC0"))
	{
		cdi.configParams.HSync0 = jo["REG_HSYNC0"].toInt();
	}

	if (jo.contains("REG_HSYNC1"))
	{
		cdi.configParams.HSync1 = jo["REG_HSYNC1"].toInt();
	}

	if (jo.contains("REG_VCYCLE"))
	{
		cdi.configParams.VCycle = jo["REG_VCYCLE"].toInt();
	}

	if (jo.contains("REG_VOFFSET"))
	{
		cdi.configParams.VOffset = jo["REG_VOFFSET"].toInt();
	}

	if (jo.contains("REG_VSYNC0"))
	{
		cdi.configParams.VSync0 = jo["REG_VSYNC0"].toInt();
	}

	if (jo.contains("REG_VSYNC1"))
	{
		cdi.configParams.VSync1 = jo["REG_VSYNC1"].toInt();
	}

	if (jo.contains("REG_SWIZZLE"))
	{
		cdi.configParams.Swizzle = jo["REG_SWIZZLE"].toInt();
	}

	if (jo.contains("REG_PCLK_POL"))
	{
		cdi.configParams.PCLKPol = jo["REG_PCLK_POL"].toInt();
	}

	if (jo.contains("REG_HSIZE"))
	{
		cdi.configParams.Width = jo["REG_HSIZE"].toInt();
	}

	if (jo.contains("REG_VSIZE"))
	{
		cdi.configParams.Height = jo["REG_VSIZE"].toInt();
	}

	if (jo.contains("REG_CSPREAD"))
	{
		cdi.configParams.CSpread = jo["REG_CSPREAD"].toInt();
	}

	if (jo.contains("REG_DITHER"))
	{
		cdi.configParams.Dither = jo["REG_DITHER"].toInt();
	}

	if (jo.contains("REG_PCLK"))
	{
		cdi.configParams.PCLK = jo["REG_PCLK"].toInt();
	}

	if (jo.contains("REG_OUTBITS") )
	{
		if (jo["REG_OUTBITS"].isString())
		{		
			if (jo["REG_OUTBITS"].toString() == DeviceAddNewDialog::REG_OUTBITS_6bits) {
				cdi.configParams.OutBitsR = (0x1B6 >> 6) & 0x7;
				cdi.configParams.OutBitsG = (0x1B6 >> 3) & 0x7;
				cdi.configParams.OutBitsB = 0x1B6 & 0x7;
			}
			else {			
				cdi.configParams.OutBitsR = 0;
				cdi.configParams.OutBitsG = 0;
				cdi.configParams.OutBitsB = 0;
			}
		}
		else
		{
			int t = jo["REG_OUTBITS"].toInt();
			cdi.configParams.OutBitsR = (t >> 6) & 0x7;
			cdi.configParams.OutBitsG = (t >> 3) & 0x7;
			cdi.configParams.OutBitsB = t & 0x7;
		}
	}

	if (jo.contains("External Clock"))
	{
		cdi.ExternalClock = jo["External Clock"].toString() == "true" ? true : false;
	}

	if (jo.contains("REG_PCLK_2X"))
	{
		cdi.configParams.PCLK2X = jo["REG_PCLK_2X"].toInt();
	}

	if (jo.contains("REG_PCLK_FREQ"))
	{
		cdi.configParams.PCLKFreq = jo["REG_PCLK_FREQ"].toInt();
	}

	if (jo.contains("REG_AH_HCYCLE_MAX"))
	{
		cdi.configParams.AhHCycleMax = jo["REG_AH_HCYCLE_MAX"].toInt();
	}

	if (jo.contains("REG_ADAPTIVE_FRAMERATE"))
	{
		cdi.configParams.AdaptiveFrameRate = jo["REG_ADAPTIVE_FRAMERATE"].toInt();
	}
}

#endif
}
