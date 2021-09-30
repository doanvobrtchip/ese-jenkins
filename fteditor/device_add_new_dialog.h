#ifndef DEVICE_ADD_NEW_DIALOG_H_
#define DEVICE_ADD_NEW_DIALOG_H_

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

#include <QDialog>
#include <QLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <QHash>

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

	static const QMap<double, int> PCLK_FREQ_HASH;

	static const QString PCLK_Frequency;

public:
	static const QString REG_OUTBITS_6bits;
	static const QString REG_OUTBITS_8bits;

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
	void calculatePixelClockFreq();

private:
	
	void prepareData();
	void loadData(QString jsonPath);
	void showData(QString jsonPath);
	bool isExistDeviceName(QString nameToCheck);

private:
	Ui::DeviceAddNewDialog *ui;

	QComboBox * mSystemClock_CB;
	QComboBox * mRegPclkFreq_CB;
	QSpinBox  * mRegPclk_SB;
	QLabel    * mPclkFreq;

	bool isEdited;
	QString editPath;

	double reg_pclk;
	double reg_pclk_freq;
	double pixel_clock_freq;
	int    system_clock;
};

#endif
}
#endif // COPTIONDIALOG_H
