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

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

const QStringList DeviceAddNewDialog::PROPERTIES = {"Device Name", "Vender", "Version", "EVE", "Connection", "Flash Model", "Flash Size (MB)", "Screen Width", "Screen Height"};

DeviceAddNewDialog::DeviceAddNewDialog(QWidget * parent)
    : QDialog(parent)
	, ui(new Ui::DeviceAddNewDialog)
    , isEdited(false)
{
	ui->setupUi(this);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(addDevice()));
}

void DeviceAddNewDialog::execute()
{
	prepareData();
	show();
}

void DeviceAddNewDialog::editDevice(QString jsonPath)
{
	isEdited = true;
	editPath = jsonPath;
	loadData(jsonPath);
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
		
		if (value == NULL)
		{
			QComboBox * cb = static_cast<QComboBox *>(ui->DeviceTableWidget->cellWidget(i, 1));
			if (cb)
			{
				value = cb->currentText();
			}
			else
			{
				QMessageBox::warning(this, "Found empty cell", "Please fill value to all cells!");
				return;
			}
		}

		jo[property] = value;
	}

	if (jo.contains("Device Name") && jo["Device Name"].isString())
	{
		QString fp = buildJsonFilePath(jo["Device Name"].toString());
		QFile f(fp);

		f.remove();
		if (!f.open(QIODevice::Text | QIODevice::WriteOnly))
			return;

		QJsonDocument jd(jo);
		f.write(jd.toJson());
		f.flush();
		f.close();

		isEdited ? emit deviceEdited(jo["Device Name"].toString(), fp) : emit deviceAdded(jo["Device Name"].toString(), fp);
		close();
	}
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

	int count = 0;
	do
	{
		res = path + QString("%1_%2.json").arg(name).arg(count);
		count++;
	} while (QFile::exists(res));

	return res;
}

void DeviceAddNewDialog::prepareData()
{
	QComboBox *cb = NULL;
	QString cg = "* { background-color: #f5f5f5; }";
	QString cw = "* { background-color: #ffffff; }";

	for (int i = 0; i < PROPERTIES.size(); i++)
	{
		ui->DeviceTableWidget->insertRow(i);
		ui->DeviceTableWidget->setItem(i, 0, new QTableWidgetItem(PROPERTIES[i]));

		if (PROPERTIES[i] == "Device Name")
		{
			ui->DeviceTableWidget->setItem(i, 1, new QTableWidgetItem("New Device"));
		}
		else if (PROPERTIES[i] == "EVE")
		{
			cb = new QComboBox(this);
			cb->setStyleSheet( i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "BT81X" << "FT81X" << "FT80X");
			cb->setCurrentIndex(0);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Flash Size (MB)")
		{
			QComboBox *cb = new QComboBox(this);
			cb->setStyleSheet(i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "2" << "4" << "8" << "16" << "32" << "64" << "128" << "512");
			cb->setCurrentIndex(3);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Connection")
		{
			cb = new QComboBox(this);
			cb->setStyleSheet(i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "FT4222" << "MPSSE");
			cb->setCurrentIndex(0);
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
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
	f.remove();

	QJsonObject jo = jd.object();

	if (jo.isEmpty())
		return;
	
	QComboBox *cb = NULL;
	QString cg = "* { background-color: #f5f5f5; }";
	QString cw = "* { background-color: #ffffff; }";
	   
	for (int i = 0; i < PROPERTIES.size(); i++)
	{
		ui->DeviceTableWidget->insertRow(i);
		ui->DeviceTableWidget->setItem(i, 0, new QTableWidgetItem(PROPERTIES[i]));

		if (!jo.contains(PROPERTIES[i]) || !jo[PROPERTIES[i]].isString())
			continue;

		if (PROPERTIES[i] == "EVE")
		{
			cb = new QComboBox(this);
			cb->setStyleSheet(i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "BT81X"
			                           << "FT81X"
			                           << "FT80X");
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Flash Size (MB)")
		{
			QComboBox *cb = new QComboBox(this);
			cb->setStyleSheet(i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "2"
			                           << "4"
			                           << "8"
			                           << "16"
			                           << "32"
			                           << "64"
			                           << "128"
			                           << "512");
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else if (PROPERTIES[i] == "Connection")
		{
			cb = new QComboBox(this);
			cb->setStyleSheet(i % 2 == 0 ? cw : cg);
			cb->addItems(QStringList() << "FT4222"
			                           << "MPSSE");
			cb->setCurrentText(jo[PROPERTIES[i]].toString());
			ui->DeviceTableWidget->setCellWidget(i, 1, cb);
		}
		else
		{
			ui->DeviceTableWidget->setItem(i, 1, new QTableWidgetItem(jo[PROPERTIES[i]].toString()));
		}
	}
}

#endif
}
