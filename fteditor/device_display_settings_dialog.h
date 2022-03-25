#ifndef DEVICE_DISPLAY_SETTINGS_DIALOG_H_
#define DEVICE_DISPLAY_SETTINGS_DIALOG_H_

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
#include "device_manager.h"

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

class DeviceManager;

class DeviceDisplaySettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DeviceDisplaySettingsDialog(DeviceManager *parent = 0);
	void execute();
	void setInitialScreenSize(QString screenSize);

private:
	QGroupBox *createRadioButtonsGroup();
	void updateSyncDeviceSelection();
	void addCustomDevice(QLayout *layout);

private slots:
	void saveInputValues();

private:
	DeviceManager *pParent;

	qint32 maxScreenWidth;
	qint32 maxScreenHeight;
	qint16 inputSpinboxMin;
	qint16 inputSpinboxMax;

	QGridLayout *gridLayout;
	QDialogButtonBox *buttonBox;
	QDialogButtonBox *defaultSettingsButtonBox;

	QList<QRadioButton *> m_CustomRadioButtonList;
};

#endif
}
#endif // COPTIONDIALOG_H
