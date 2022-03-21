
#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "device_display_settings_dialog.h"
#include "device_manager.h"

#include "constant_mapping.h"

#include "device_manage_dialog.h"
#include <QDirIterator>

namespace FTEDITOR
{

#if FT800_DEVICE_MANAGER

extern QString g_ApplicationDataDir;

DeviceDisplaySettingsDialog::DeviceDisplaySettingsDialog(DeviceManager *parent)
    : QDialog(parent)
    , pParent(parent)
    , inputSpinboxMin(-32767)
    , inputSpinboxMax(32767)
{
	gridLayout = new QGridLayout(this);
	gridLayout->setSizeConstraint(QLayout::SetFixedSize);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	gridLayout->addWidget(createRadioButtonsGroup(), 0, 0);
	gridLayout->addWidget(buttonBox, 1, 0);

	setLayout(gridLayout);

	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveInputValues()));
}

//this layout consists of a mutually exclusive radio buttons for the screen resolutions
QGroupBox *DeviceDisplaySettingsDialog::createRadioButtonsGroup()
{
	QGroupBox *groupBox = new QGroupBox(tr("Device type"));
	QVBoxLayout *VBox = new QVBoxLayout;
	
	addCustomDevice(VBox);
	groupBox->setLayout(VBox);

	return groupBox;
}

void DeviceDisplaySettingsDialog::addCustomDevice(QLayout *layout)
{
	m_CustomRadioButtonList.clear();

	QRadioButton *rb = NULL;

	// load device from folder device_sync
	auto processDirectory = [&](QDirIterator &it) {
		while (it.hasNext())
		{
			QString path = it.next();

			CustomDeviceInfo cdi;
			DeviceManageDialog::getCustomDeviceInfo(path, cdi);

			if (cdi.DeviceName.isEmpty())
				continue;

			rb = new QRadioButton(cdi.DeviceName, this);
			QFont ft = rb->font();
			ft.setBold(cdi.isBuiltin ? true : false);
			rb->setFont(ft);
			rb->setToolTip(cdi.isBuiltin ? ("Built-in Device\n" + cdi.Description) : cdi.Description);
			rb->setProperty("EVE_TYPE", cdi.EVE_Type);
			rb->setProperty("SCREEN_SIZE", cdi.ScreenSize);
			rb->setProperty("JSON_PATH", path);

			m_CustomRadioButtonList.append(rb);
			layout->addWidget(rb);
		}
	};

	QDirIterator it(QApplication::applicationDirPath() + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
	processDirectory(it);

	if (QApplication::applicationDirPath() != g_ApplicationDataDir)
	{
		QDirIterator it(g_ApplicationDataDir + DeviceManageDialog::DEVICE_SYNC_PATH, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
		processDirectory(it);
	}
}

void DeviceDisplaySettingsDialog::updateSyncDeviceSelection()
{
	int currentDevice;
	QString selectedDevice = pParent->getSelectedDeviceName();

	for each(QRadioButton * rb in m_CustomRadioButtonList)
	{
		rb->setVisible(false);
	}

	if (FTEDITOR_CURRENT_DEVICE <= FTEDITOR_FT800 || FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801)
	{
		currentDevice = FTEDITOR_FT800;
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT880)
	{
		currentDevice = FTEDITOR_FT810;
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT880 && FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT815)
	{
		currentDevice = FTEDITOR_BT880;
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815 && FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT817)
	{
		currentDevice = FTEDITOR_BT815;
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT817)
	{
		currentDevice = FTEDITOR_BT817;
	}

	for each(QRadioButton * rb in m_CustomRadioButtonList)
	{
		if (rb->property("EVE_TYPE").toInt() == currentDevice)
		{
			rb->setVisible(true);
		}
		if (selectedDevice == rb->text())
			rb->setChecked(true);
	}
}

void DeviceDisplaySettingsDialog::execute()
{
	updateSyncDeviceSelection();
	show();
}


void DeviceDisplaySettingsDialog::saveInputValues()
{
	for each(QRadioButton * rb in m_CustomRadioButtonList)
	{
		if (rb->isChecked())
		{
			pParent->setDeviceAndScreenSize(rb->property("SCREEN_SIZE").toString(), rb->text(), rb->property("JSON_PATH").toString(), true);
		}
	}

	this->accept();
}

#endif

}


