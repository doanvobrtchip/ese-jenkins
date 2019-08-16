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
QGroupBox *DeviceDisplaySettingsDialog::createRadioButtonsGroup(){
	QGroupBox *groupBox = new QGroupBox(tr("Device type"));

	VM800B35A = new QRadioButton(tr("VM800B35A"));
	VM800B35A->setToolTip(tr("FT800 Basic module with 3.5\" display."));
	VM800B43A = new QRadioButton(tr("VM800B43A"));
	VM800B43A->setToolTip(tr("FT800 Basic module with 4.3\" display."));
	VM800B50A = new QRadioButton(tr("VM800B50A"));
	VM800B50A->setToolTip(tr("FT800 Basic module with 5.0\" display."));
	VM800C35A = new QRadioButton(tr("VM800C35A"));
	VM800C35A->setToolTip(tr("FT800 Credit card module with 3.5\" display."));
	VM800C43A = new QRadioButton(tr("VM800C43A"));
	VM800C43A->setToolTip(tr("FT800 Credit card module with 4.3\" display."));
	VM800C50A = new QRadioButton(tr("VM800C50A"));
	VM800C50A->setToolTip(tr("FT800 Credit card module with 5.0\" display."));
	VM800BU35A = new QRadioButton(tr("VM800BU35A"));
	VM800BU35A->setToolTip(tr("FT800 Basic USB module with 3.5\" display."));
	VM800BU43A = new QRadioButton(tr("VM800BU43A"));
	VM800BU43A->setToolTip(tr("FT800 Basic USB module with 4.3\" display."));
	VM800BU50A = new QRadioButton(tr("VM800BU50A"));
		
	VM800BU50A->setToolTip(tr("FT800 Basic USB module with 5.0\" display."));

	ME813AUWH50C = new QRadioButton(tr("ME813AU_WH50C(800x480)"));
	ME813AUWH50C->setToolTip(tr("FT813 module with 5.0\" display and FT4222 USB-SPI"));

	VM816C50A = new QRadioButton(tr("VM816C50A(800x480)"));
	VM816C50A->setToolTip(tr("VM816C module with 5.0\" display"));

    VM816CU50A = new QRadioButton(tr("VM816CU50A(800x480)"));
    VM816CU50A->setToolTip(tr("VM816CU module with 5.0\" display"));

	QVBoxLayout *VBox = new QVBoxLayout;
	VBox->addWidget(VM800B35A);
	VBox->addWidget(VM800B43A);
	VBox->addWidget(VM800B50A);
		
	VBox->addWidget(VM800C35A);
	VBox->addWidget(VM800C43A);
	VBox->addWidget(VM800C50A);
		

	VBox->addWidget(VM800BU35A);
	VBox->addWidget(VM800BU43A);
	VBox->addWidget(VM800BU50A);

	VBox->addWidget(ME813AUWH50C);
	VBox->addWidget(VM816C50A);
    VBox->addWidget(VM816CU50A);

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
			rb->setToolTip("Custom device");
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
	int currenDevice;
	QString selectedDevice = pParent->getSelectedDeviceName();

	; {
		VM800B35A->setVisible(false);
		VM800B43A->setVisible(false);
		VM800B50A->setVisible(false);
		VM800C35A->setVisible(false);
		VM800C50A->setVisible(false);
		VM800C43A->setVisible(false);
		VM800BU35A->setVisible(false);
		VM800BU43A->setVisible(false);
		VM800BU50A->setVisible(false);

		ME813AUWH50C->setVisible(false);

        VM816C50A->setVisible(false);
        VM816CU50A->setVisible(false);

		for each(QRadioButton * rb in m_CustomRadioButtonList)
		{
			rb->setVisible(false);
		}

	}

	if (FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT800 || FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801)
	{
		currenDevice = FTEDITOR_FT800;

		VM800B35A->setVisible(true);
		VM800B43A->setVisible(true);
		VM800B50A->setVisible(true);
		VM800C35A->setVisible(true);
		VM800C50A->setVisible(true);
		VM800C43A->setVisible(true);
		VM800BU35A->setVisible(true);
		VM800BU43A->setVisible(true);
		VM800BU50A->setVisible(true);

		if (selectedDevice == "VM800B35A")
			VM800B35A->setChecked(true);
		else if (selectedDevice == "VM800B43A")
			VM800B43A->setChecked(true);
		else if (selectedDevice == "VM800B50A")
			VM800B50A->setChecked(true);
		else if (selectedDevice == "VM800C35A")
			VM800C35A->setChecked(true);
		else if (selectedDevice == "VM800C43A")
			VM800C43A->setChecked(true);
		else if (selectedDevice == "VM800C50A")
			VM800C50A->setChecked(true);
		else if (selectedDevice == "VM800BU35A")
			VM800BU35A->setChecked(true);
		else if (selectedDevice == "VM800BU43A")
			VM800BU43A->setChecked(true);
		else if (selectedDevice == "VM800BU50A")
			VM800BU50A->setChecked(true);
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT815)
	{
		currenDevice = FTEDITOR_FT810;

		ME813AUWH50C->setVisible(true);

		if (selectedDevice == "ME813AU_WH50C(800x480)")
			ME813AUWH50C->setChecked(true);
	}
	else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
	{
		currenDevice = FTEDITOR_BT815;

		VM816C50A->setVisible(true);
        VM816CU50A->setVisible(true);

		if (selectedDevice == "VM816C50A(800x480)")
			VM816C50A->setChecked(true);
        else if (selectedDevice == "VM816CU50A(800x480)")
            VM816CU50A->setChecked(true);
	}

	for each(QRadioButton * rb in m_CustomRadioButtonList)
	{
		if (rb->property("EVE_TYPE").toInt() == currenDevice)
		{
			rb->setVisible(true);
		}
		if (selectedDevice == rb->text())
			rb->setChecked(true);
	}
}

void DeviceDisplaySettingsDialog::execute(){
	updateSyncDeviceSelection();
	show();
}


void DeviceDisplaySettingsDialog::saveInputValues(){
	if (VM800B35A->isChecked()) {
		pParent->setDeviceAndScreenSize("320x240", "VM800B35A");
	}
	else if (VM800C35A->isChecked()){
		pParent->setDeviceAndScreenSize("320x240", "VM800C35A");
	}
	else if (VM800BU35A->isChecked()){
		pParent->setDeviceAndScreenSize("320x240", "VM800BU35A");
	}
	else if (VM800B43A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800B43A");
	}
	else if (VM800C43A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800C43A");
	}
	else if (VM800BU43A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800BU43A");
	}
	else if (VM800B50A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800B50A");
	}
	else if (VM800C50A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800C50A");
	}
	else if (VM800BU50A->isChecked()){
		pParent->setDeviceAndScreenSize("480x272", "VM800BU50A");
	}else if (ME813AUWH50C->isChecked()){
		pParent->setDeviceAndScreenSize("800x480", "ME813AU_WH50C(800x480)");
	}
	else if (VM816C50A->isChecked()) {
		pParent->setDeviceAndScreenSize("800x480", "VM816C50A(800x480)");
	}
    else if (VM816CU50A->isChecked()) {
        pParent->setDeviceAndScreenSize("800x480", "VM816CU50A(800x480)");
    }
	else
	{
		for each(QRadioButton * rb in m_CustomRadioButtonList)
		{
			if (rb->isChecked())
			{
				pParent->setDeviceAndScreenSize(rb->property("SCREEN_SIZE").toString(), rb->text(), rb->property("JSON_PATH").toString(), true);
			}
		}
	}

	this->accept();
}

#endif

}


