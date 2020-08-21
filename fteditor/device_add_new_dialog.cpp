#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "device_add_new_dialog.h"
#include "device_manager.h"
#include "device_manage_dialog.h"
#include "constant_mapping.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

const QStringList DeviceAddNewDialog::PROPERTIES = {"Device Name", "Description", "Vendor", "Version", "Connection Type", "EVE Type", "Flash Model",
							"Flash Size (MB)", "System Clock (MHz)", "External Clock", "Screen Width", "Screen Height", "REG_HCYCLE", "REG_HOFFSET",
							"REG_HSYNC0", "REG_HSYNC1", "REG_VCYCLE", "REG_VOFFSET", "REG_VSYNC0", "REG_VSYNC1", "REG_SWIZZLE", "REG_PCLK_POL",
							"REG_HSIZE", "REG_VSIZE", "REG_CSPREAD", "REG_DITHER", "REG_PCLK", "REG_OUTBITS",
							"REG_PCLK_2X", "REG_PCLK_FREQ", "REG_AH_HCYCLE_MAX", "REG_ADAPTIVE_FRAMERATE" };

DeviceAddNewDialog::DeviceAddNewDialog(QWidget * parent)
    : QDialog(parent)
	, ui(new Ui::DeviceAddNewDialog)
    , isEdited(false)
{
	ui->setupUi(this);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(addDevice()));

	ui->DeviceTableWidget->horizontalHeader()->setMinimumSectionSize(170);
}

void DeviceAddNewDialog::execute()
{
	prepareData();
	show();
}

void DeviceAddNewDialog::editDevice(QString jsonPath)
{
	isEdited = true;
	this->setWindowTitle("Edit Device");
	editPath = jsonPath;
	loadData(jsonPath);
	show();
}

void DeviceAddNewDialog::examineDevice(QString jsonPath)
{
	QPushButton * pb = NULL;
	pb = ui->buttonBox->button(QDialogButtonBox::Ok);
	if (pb)		pb->setVisible(false);
	
	pb = ui->buttonBox->button(QDialogButtonBox::Cancel);
	if (pb)		pb->setText("Close");
	
	this->setWindowTitle("Examine Device");
	showData(jsonPath);
	show();
}

void DeviceAddNewDialog::addDevice()
{
	QJsonObject jo;
	QTableWidgetItem *item;
	QString property;
	QString value;

	for (int i = 0; i < ui->DeviceTableWidget->rowCount(); i++)
	{
		property = (item = ui->DeviceTableWidget->item(i, 0)) != NULL ? item->text() : QString("");
		value = (item = ui->DeviceTableWidget->item(i, 1)) != NULL ? item->text() : QString("");
	
		if (value.isEmpty())
		{
			QComboBox * cb = dynamic_cast<QComboBox *>(ui->DeviceTableWidget->cellWidget(i, 1));
			QSpinBox * sb = dynamic_cast<QSpinBox *>(ui->DeviceTableWidget->cellWidget(i, 1));
			if (cb)
			{
				jo[property] = cb->currentText();
			}
			else if (sb)
			{
				jo[property] = sb->value();
			}
			else if (item != NULL && !item->flags().testFlag(Qt::ItemIsEnabled))
			{
				jo[property] = value;
			}
			else
			{
				QMessageBox::warning(this, "Found empty cell", "Please fill value to all cells!");
				return;
			}
		}
		else
		{
			jo[property] = value;
		}
	}

	jo["Built-in"] = false;

	if (jo.contains("Device Name") && jo["Device Name"].isString())
	{
		QString deviceName = jo["Device Name"].toString();

		if (isEdited) {
			if (deviceName != QFileInfo(editPath).baseName() && isExistDeviceName(deviceName))
			{
				QMessageBox::warning(this, "Exist Device Name", "Device name is existed!");
				return;
			}
		}
		else if (isExistDeviceName(deviceName)) {
			QMessageBox::warning(this, "Exist Device Name", "Device name is existed!");
			return;
		}
			
		QString fp;
		if (isEdited)
		{
			QFile(editPath).remove();
		}
		fp = buildJsonFilePath(deviceName);

		QFile f(fp);

		if (!f.open(QIODevice::Text | QIODevice::WriteOnly))
			return;

		QJsonDocument jd(jo);
		f.write(jd.toJson());
		f.flush();
		f.close();

		isEdited ? emit deviceEdited(deviceName, fp) : emit deviceAdded(deviceName, fp);
		close();
	}
}

void DeviceAddNewDialog::onEveTypeChange(QString eveType)
{
	bool isFlashUsed = eveType.startsWith("BT");

	QTableWidgetItem * item = NULL;
	QWidget * w = NULL;
	for (int i = 0; i < ui->DeviceTableWidget->rowCount(); i++)
	{
		item = ui->DeviceTableWidget->item(i, 0);
		if (!item->text().startsWith("Flash"))
			continue;
		
		isFlashUsed ? item->setFlags(item->flags() | Qt::ItemIsEnabled ) : item->setFlags(item->flags() & ~Qt::ItemIsEnabled);

		w = ui->DeviceTableWidget->cellWidget(i, 1);
		if (w)
		{
			w->setEnabled(isFlashUsed);
		}
		else 
		{
			item = ui->DeviceTableWidget->item(i, 1);
			isFlashUsed ? item->setFlags(item->flags() | Qt::ItemIsEnabled ) : item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		}
	}
}

bool DeviceAddNewDialog::isExistDeviceName(QString nameToCheck)
{
	return QFile::exists(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH + nameToCheck + ".json");
}

QString DeviceAddNewDialog::buildJsonFilePath(QString name)
{
	QString res("");
	QString path("");
	path.append(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH);

	QDir d(path);
	if (!d.exists())
	{
		d.mkpath(path);
	}
	
	res = QString("%1%2.json").arg(path).arg(name);
	if (!QFile::exists(res))
	{
		return res;
	}

	int count = 0;
	res = path + QString("%1 - Clone.json").arg(name);
	while (QFile::exists(res))
	{
		++count;
		res = path + QString("%1 - Clone (%2).json").arg(name).arg(count);
	}

	return res;
}

void DeviceAddNewDialog::prepareData()
{
	QTableWidgetItem * item = NULL;
	QComboBox *cb = NULL;
	QSpinBox *sb = NULL;
	QString sg = "QSpinBox { background-color: #e9e7e3; border: none; }";
	QString sw = "QSpinBox { background-color: #ffffff; border: none; }";

	for (int i = 0; i < PROPERTIES.size(); i++)
	{
		ui->DeviceTableWidget->insertRow(i);

		item = new QTableWidgetItem(PROPERTIES[i]);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));
		ui->DeviceTableWidget->setItem(i, 0, item);

		if (PROPERTIES[i] == "Device Name")
		{
			ui->DeviceTableWidget->setItem(i, 1, new QTableWidgetItem("New Device"));
		}
		else if (PROPERTIES[i] == "Description")
		{
			ui->DeviceTableWidget->setItem(i, 1, new QTableWidgetItem("Custom Device"));
		}
		else if (PROPERTIES[i] == "EVE Type")
		{
			cb = new QComboBox(this);
			cb->addItems(QStringList() << "BT817_818" << "BT815_816" << "FT81X" << "FT80X");
			cb->setCurrentIndex(0);
			connect(cb, &QComboBox::currentTextChanged, this, &DeviceAddNewDialog::onEveTypeChange);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Flash Size (MB)")
		{
			QComboBox *cb = new QComboBox(this);
			cb->addItems(QStringList() << "2" << "4" << "8" << "16" << "32" << "64" << "128" << "512");
			cb->setCurrentIndex(3);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "System Clock (MHz)")
		{
			QComboBox *cb = new QComboBox(this);
			cb->addItems(QStringList() << "24"
			                           << "36"
									   << "48"
			                           << "60"
			                           << "72"
			                           << "84");
			cb->setCurrentIndex(3);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Connection Type")
		{
			cb = new QComboBox(this);
			cb->addItems(QStringList() << "FT4222" << "MPSSE");
			cb->setCurrentIndex(0);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "External Clock")
		{
			cb = new QComboBox(this);
			cb->addItems(QStringList() << "true"
			                           << "false");
			cb->setCurrentIndex(1);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Screen Width" ||
				 PROPERTIES[i] == "Screen Height" || 
				 PROPERTIES[i].startsWith("REG_"))
		{
			sb = new QSpinBox(this);
			sb->setMinimum(0);
			sb->setMaximum(9999);
			sb->setButtonSymbols(QSpinBox::NoButtons);
			sb->setStyleSheet(i % 2 == 0 ? sw : sg);
			ui->DeviceTableWidget->setCellWidget(i, 1, sb);
		}
		else
		{
			ui->DeviceTableWidget->setItem(i, 1, new QTableWidgetItem(""));
		}
	}
}

void DeviceAddNewDialog::loadData(QString jsonPath)
{
	QFile f(jsonPath);

	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QJsonDocument jd = QJsonDocument::fromJson(f.readAll());
	f.close();

	QJsonObject jo = jd.object();

	if (jo.isEmpty())
		return;
	
	prepareData();

	QComboBox * cb = NULL;
	QSpinBox * sb = NULL;
	QTableWidgetItem * item = NULL;

	for (int i = 0; i < PROPERTIES.size(); i++)
	{
		if (!jo.contains(PROPERTIES[i]))
			continue;

		if (PROPERTIES[i] == "EVE Type")
		{
			cb = (QComboBox * )ui->DeviceTableWidget->cellWidget(i, 1);
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
		}
		else if (PROPERTIES[i] == "Flash Size (MB)")
		{
			cb = (QComboBox * )ui->DeviceTableWidget->cellWidget(i, 1);
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
		}
		else if (PROPERTIES[i] == "System Clock (MHz)")
		{
			cb = (QComboBox *)ui->DeviceTableWidget->cellWidget(i, 1);
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
		}
		else if (PROPERTIES[i] == "Connection Type")
		{
			cb = (QComboBox * )ui->DeviceTableWidget->cellWidget(i, 1);
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
		}
		else if (PROPERTIES[i] == "External Clock")
		{
			cb = (QComboBox *)ui->DeviceTableWidget->cellWidget(i, 1);
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
		}
		else if (PROPERTIES[i] == "Screen Width" || PROPERTIES[i] == "Screen Height" || PROPERTIES[i].startsWith("REG_"))
		{
			sb = (QSpinBox *)ui->DeviceTableWidget->cellWidget(i, 1);
			sb->setValue(jo[PROPERTIES[i]].toInt());
		}
		else
		{
			item = ui->DeviceTableWidget->item(i, 1);
			if (item) item->setText(jo[PROPERTIES[i]].toString());
		}
	}
}

void DeviceAddNewDialog::showData(QString jsonPath)
{
	QFile f(jsonPath);

	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QJsonDocument jd = QJsonDocument::fromJson(f.readAll());
	f.close();

	QJsonObject jo = jd.object();

	if (jo.isEmpty())
		return;

	QTableWidgetItem * item = NULL;
	QString v;

	for (int i = 0; i < PROPERTIES.size(); i++)
	{
		ui->DeviceTableWidget->insertRow(i);

		item = new QTableWidgetItem(PROPERTIES[i]);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));
		ui->DeviceTableWidget->setItem(i, 0, item);

		if (!jo.contains(PROPERTIES[i]))
			continue;
		
		if (jo[PROPERTIES[i]].isString())
			v = jo[PROPERTIES[i]].toString();
		else
			v = QString::number(jo[PROPERTIES[i]].toInt());

		item = new QTableWidgetItem(v);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));
		ui->DeviceTableWidget->setItem(i, 1, item);
	}
}

#endif
}
