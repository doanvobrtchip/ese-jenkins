#include "device_display_settings_dialog.h"
#include "device_manager.h"

#include "constant_mapping.h"


namespace FTEDITOR {

#if FT800_DEVICE_MANAGER

	DeviceDisplaySettingsDialog::DeviceDisplaySettingsDialog(DeviceManager *parent) :
		QDialog(parent), pParent(parent), inputSpinboxMin(-32767), inputSpinboxMax(32767)
	{
		gridLayout = new QGridLayout(this);
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

		gridLayout->addWidget(createRadioButtonsGroup(), 0, 0);
		//gridLayout->addWidget(createVM800CRadioButtonsGroup(), 0, 1);
		//gridLayout->addWidget(createVM801BRadioButtonsGroup(), 0, 2);
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

		groupBox->setLayout(VBox);

		return groupBox;
	}
	

	void DeviceDisplaySettingsDialog::updateSyncDeviceSelection(){
		QString selectedDevice = pParent->getSyncDeviceName();

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
		}

		if (FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT800 ||
			FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801) {

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
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 &&
			FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT815) {

			ME813AUWH50C->setVisible(true);

			if (selectedDevice == "ME813AU_WH50C(800x480)")
				ME813AUWH50C->setChecked(true);
		}

		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
		{
			VM816C50A->setVisible(true);

			if (selectedDevice == "VM816C50A(800x480)")
				VM816C50A->setChecked(true);

		}
	}

	void DeviceDisplaySettingsDialog::execute(){
		updateSyncDeviceSelection();
		show();
	}


	void DeviceDisplaySettingsDialog::saveInputValues(){
		if (VM800B35A->isChecked()) {
			pParent->setDeviceandScreenSize("320x240", "VM800B35A");
		}
		else if (VM800C35A->isChecked()){
			pParent->setDeviceandScreenSize("320x240", "VM800C35A");
		}
		else if (VM800BU35A->isChecked()){
			pParent->setDeviceandScreenSize("320x240", "VM800BU35A");
		}
		else if (VM800B43A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800B43A");
		}
		else if (VM800C43A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800C43A");
		}
		else if (VM800BU43A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800BU43A");
		}
		else if (VM800B50A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800B50A");
		}
		else if (VM800C50A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800C50A");
		}
		else if (VM800BU50A->isChecked()){
			pParent->setDeviceandScreenSize("480x272", "VM800BU50A");
		}else if (ME813AUWH50C->isChecked()){
			pParent->setDeviceandScreenSize("800x480", "ME813AU_WH50C(800x480)");
		}
		else if (VM816C50A->isChecked()) {
			pParent->setDeviceandScreenSize("800x480", "VM816C50A(800x480)");
		}

		this->accept();
	}

#endif

}


