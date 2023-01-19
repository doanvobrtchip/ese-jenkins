/*
Copyright (C) 2014-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2022  Bridgetek Pte Lte
*/

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6262) // Large stack
#endif

#define _USE_MATH_DEFINES
#include "device_manager.h"

// STL includes
#include <cstdio>
#include <memory>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QFileInfo>
#include <QProgressBar>
#include <QTimer>

// Emulator includes
#ifdef _USE_MATH_DEFINES
#undef _USE_MATH_DEFINES
#endif
#include <EVE_Hal.h>
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "content_manager.h"
#include "dl_parser.h"
#include "dl_editor.h"
#include "constant_mapping.h"
#include "device_info_custom.h"

#include "device_display_settings_dialog.h"
#include "device_manage_dialog.h"

namespace FTEDITOR {

extern BT8XXEMU_Emulator *g_Emulator;
extern ContentManager *g_ContentManager;

extern volatile int g_HSize;
extern volatile int g_VSize;
extern volatile int g_Rotate;

// Editors
extern DlEditor *g_DlEditor;
extern DlEditor *g_CmdEditor;
extern DlEditor *g_Macro;

#if FT800_DEVICE_MANAGER

CustomDeviceInfo::CustomDeviceInfo()
    : EVE_Type(0)
    , FlashSize(0)
    , SystemClock(60)
    , isBuiltin(false)
    , configParams({0})
{
	// no op
}

DeviceManager::DeviceManager(MainWindow *parent)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_DisplaySettingsDialog(NULL)
    , m_DeviceManageDialog(NULL)
    , m_IsCustomDevice(false)
	, m_CDI(std::make_unique<CustomDeviceInfo>())
    , m_DeviceJsonPath("")
    , m_Busy(false)
    , m_Abort(false)
    , m_StreamProgress(NULL)
{
	QVBoxLayout *layout = new QVBoxLayout();

	m_DeviceList = new QTreeWidget(this);
	m_DeviceList->setColumnCount(2);
	QStringList headers;
	headers.push_back(tr("Connected"));
	headers.push_back(tr("Device Name"));
	m_DeviceList->setHeaderLabels(headers);
	layout->addWidget(m_DeviceList);
	m_DeviceList->resizeColumnToContents(0);

	connect(m_DeviceList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	QHBoxLayout *buttons = new QHBoxLayout();

	m_RefreshButton = new QPushButton(this);
	m_RefreshButton->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
	m_RefreshButton->setToolTip(tr("Refresh the device list"));
	connect(m_RefreshButton, SIGNAL(clicked()), this, SLOT(refreshDevices()));
	buttons->addWidget(m_RefreshButton);
	m_RefreshButton->setMaximumWidth(m_RefreshButton->height());

	m_DeviceDisplayButton = new QPushButton(this);
	m_DeviceDisplayButton->setIcon(QIcon(":/icons/wrench-screwdriver.png"));
	m_DeviceDisplayButton->setToolTip(tr("Device display settings"));
	connect(m_DeviceDisplayButton, SIGNAL(clicked()), this, SLOT(deviceDisplaySettings()));
	buttons->addWidget(m_DeviceDisplayButton);
	m_DeviceDisplayButton->setMaximumWidth(m_DeviceDisplayButton->height());

	m_DeviceManageButton = new QPushButton(this);
	m_DeviceManageButton->setIcon(QIcon(":/icons/category.png"));
	m_DeviceManageButton->setToolTip(tr("Manage Device"));
	connect(m_DeviceManageButton, SIGNAL(clicked()), this, SLOT(deviceManage()));
	buttons->addWidget(m_DeviceManageButton);
	m_DeviceManageButton->setMaximumWidth(m_DeviceManageButton->height());

	buttons->addStretch();

	m_ConnectButton = new QPushButton(this);
	m_ConnectButton->setIcon(QIcon(":/icons/plus-circle-frame-20x16.png"));
	m_ConnectButton->setIconSize(QSize(20, 16));
	m_ConnectButton->setText(tr("Connect"));
	m_ConnectButton->setToolTip(tr("Connect the selected device"));
	m_ConnectButton->setVisible(false);
	connect(m_ConnectButton, SIGNAL(clicked()), this, SLOT(connectDevice()));
	buttons->addWidget(m_ConnectButton);

	m_DisconnectButton = new QPushButton(this);
	m_DisconnectButton->setIcon(QIcon(":/icons/minus-circle-frame-20x16.png"));
	m_DisconnectButton->setIconSize(QSize(20, 16));
	m_DisconnectButton->setText(tr("Disconnect"));
	m_DisconnectButton->setToolTip(tr("Disconnect from the selected device"));
	m_DisconnectButton->setVisible(false);
	connect(m_DisconnectButton, SIGNAL(clicked()), this, SLOT(disconnectDevice()));
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(disconnectDevice()));
	buttons->addWidget(m_DisconnectButton);

	layout->addLayout(buttons);

	// Upload RAM_G and RAM_DL
	// Upload RAM and Coprocessor Commands
	// Upload Flash

	m_UploadRamDlButton = new QPushButton(this);
	m_UploadRamDlButton->setIcon(QIcon(":/icons/arrow-curve-090-left.png"));
	m_UploadRamDlButton->setText(tr("Upload RAM_G and RAM_DL"));
	m_UploadRamDlButton->setToolTip(tr("Sends the current memory and display list to the selected device"));
	m_UploadRamDlButton->setVisible(false);
	connect(m_UploadRamDlButton, &QPushButton::clicked, this, &DeviceManager::uploadRamDl);
	layout->addWidget(m_UploadRamDlButton);

	m_UploadCoprocessorContentButton = new QPushButton(this);
	m_UploadCoprocessorContentButton->setIcon(QIcon(":/icons/arrow-curve-090-left.png"));
	m_UploadCoprocessorContentButton->setText(tr("Upload RAM and Coprocessor"));
	m_UploadCoprocessorContentButton->setToolTip(tr(""));
	m_UploadCoprocessorContentButton->setVisible(false);
	connect(m_UploadCoprocessorContentButton, &QPushButton::clicked, this, &DeviceManager::uploadCoprocessorContent);
	layout->addWidget(m_UploadCoprocessorContentButton);

	m_UploadFlashContentButton = new QPushButton(this);
	m_UploadFlashContentButton->setIcon(QIcon(":/icons/lightning--pencil.png"));
	m_UploadFlashContentButton->setText(tr("Write Flash Content"));
	m_UploadFlashContentButton->setToolTip(tr(""));
	m_UploadFlashContentButton->setVisible(false);
	connect(m_UploadFlashContentButton, &QPushButton::clicked, this, &DeviceManager::uploadFlashContent);
	layout->addWidget(m_UploadFlashContentButton);

	m_UploadFlashBlobButton = new QPushButton(this);
	m_UploadFlashBlobButton->setText(tr("Write Flash Firmware"));
	m_UploadFlashBlobButton->setToolTip(tr(""));
	m_UploadFlashBlobButton->setVisible(false);
	connect(m_UploadFlashBlobButton, &QPushButton::clicked, this, &DeviceManager::uploadFlashBlob);
	layout->addWidget(m_UploadFlashBlobButton);

	setLayout(layout);

	//Init MPSSE lib
	// Init_libMPSSE();
	EVE_HalPlatform *platform = EVE_Hal_initialize();
	// Initial refresh of devices
	// refreshDevices(); // TODO: Move to when device manager is first shown for faster bootup
	QTimer::singleShot(0, this, &DeviceManager::refreshDevices);

	// m_DisplaySettingsDialog = new DeviceDisplaySettingsDialog(this);
}

DeviceManager::~DeviceManager()
{
	// Close all open contexts
	for (std::pair<DeviceId, DeviceInfo *> p : m_DeviceInfo)
	{
		if (p.second->EveHalContext)
		{
			EVE_HalContext *phost = (EVE_HalContext *)p.second->EveHalContext;
			EVE_Util_clearScreen(phost);
			EVE_Util_shutdown(phost);
			p.second->EveHalContext = NULL;
			EVE_Hal_close(phost);
		}
	}

	// Release HAL
	EVE_Hal_release();

	// Release dialogs
	if (m_DeviceManageDialog)
	{
		delete m_DeviceManageDialog;
		m_DeviceManageDialog = NULL;
	}

	if (m_DisplaySettingsDialog)
	{
		delete m_DisplaySettingsDialog;
	}
}

struct BusyLock
{
public:
	BusyLock(bool *busy)
	{
		if (!*busy)
		{
			m_Busy = busy;
			*busy = true;
		}
		else
		{
			m_Busy = NULL;
		}
	}

	~BusyLock()
	{
		if (m_Busy)
		{
			*m_Busy = false;
			m_Busy = NULL;
		}
	}

	inline bool locked() const { return m_Busy; /* != NULL */ }

private:
	bool *m_Busy;
};

bool DeviceManager::cbCmdWait(void *ph)
{
	EVE_HalContext *phost = reinterpret_cast<EVE_HalContext *>(ph);
	DeviceManager *deviceManager = reinterpret_cast<DeviceManager *>(phost->UserContext);
	if (deviceManager->m_StreamProgress && deviceManager->m_StreamProgress->value() != deviceManager->m_StreamTransfered)
		deviceManager->m_StreamProgress->setValue(deviceManager->m_StreamTransfered);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
	return !deviceManager->m_Abort;
}

void DeviceManager::initProgressDialog(QDialog *progressDialog, QLabel *progressLabel, QProgressBar *progressBar, QProgressBar *progressSubBar)
{
	progressDialog->setMinimumSize(350, 150);
	progressDialog->setWindowModality(Qt::ApplicationModal);
	progressDialog->setWindowFlags(progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout *progressLayout = new QVBoxLayout(progressDialog);
	progressDialog->setLayout(progressLayout);
	progressLayout->addWidget(progressLabel);
	progressLayout->addWidget(progressBar);
	progressLayout->addWidget(progressSubBar);
	QPushButton *abortButton = new QPushButton(progressDialog);
	abortButton->setText("Abort");
	QHBoxLayout *buttonLayout = new QHBoxLayout(progressDialog);
	progressLayout->addLayout(buttonLayout);
	buttonLayout->addStretch();
	buttonLayout->addWidget(abortButton);
	connect(abortButton, &QPushButton::clicked, this, &DeviceManager::abortRequest);
	progressLayout->addStretch();
}

void DeviceManager::abortRequest()
{
	m_Abort = true;
}

void DeviceManager::setDeviceAndScreenSize(QString displaySize, QString syncDevice, QString jsonPath, bool isCustomDevice)
{
	m_IsCustomDevice = isCustomDevice;
	m_DeviceJsonPath = jsonPath;

	QStringList pieces = displaySize.split("x");
	if ((m_SelectedDisplaySize != displaySize) && m_DisconnectButton->isVisible())
	{
		disconnectDevice();
	}

	m_SelectedDisplaySize = displaySize;
	m_SelectedDeviceName = syncDevice;

	m_MainWindow->userChangeResolution(pieces[0].toUInt(), pieces[1].toUInt());
}

void DeviceManager::deviceManage()
{
	if (m_DeviceManageDialog)
	{
		delete m_DeviceManageDialog;
	}

	m_DeviceManageDialog = new DeviceManageDialog(this);
	m_DeviceManageDialog->execute();
}

void DeviceManager::deviceDisplaySettings()
{
	if (m_DisplaySettingsDialog)
	{
		delete m_DisplaySettingsDialog;
	}

	m_DisplaySettingsDialog = new DeviceDisplaySettingsDialog(this);
	m_DisplaySettingsDialog->execute();
}

void DeviceManager::refreshDevices()
{
	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is still in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	printf("Refresh devices\n");

	/*
	std::unique_ptr<QDialog> progressDialog = std::make_unique<QDialog>(this);
	progressDialog->setWindowTitle("EVE Screen Editor");
	QLabel *progressLabel = new QLabel(progressDialog.get());
	QProgressBar *progressBar = new QProgressBar(progressDialog.get());
	QProgressBar *progressSubBar = new QProgressBar(progressDialog.get());
	progressLabel->setText("Refresh devices...");
	progressBar->setVisible(false);
	progressSubBar->setVisible(false);
	initProgressDialog(progressDialog.get(), progressLabel, progressBar, progressSubBar);
	progressDialog->setVisible(true);
	*/

	size_t eveDeviceCount = EVE_Hal_list();

	// Swap device list to local
	std::map<DeviceId, DeviceInfo *> deviceInfo;
	deviceInfo.swap(m_DeviceInfo);

	// For each device that is found
	for (size_t i = 0; i < eveDeviceCount; ++i)
	{
		EVE_DeviceInfo info;
		EVE_Hal_info(&info, i);
		if (!info.Host)
			continue; // Skip unknown

		std::map<DeviceId, DeviceInfo *>::iterator it = std::find_if(deviceInfo.begin(), deviceInfo.end(),
		    [&](std::pair<const DeviceId, DeviceInfo *> &di) -> bool {
			    // Match by serial number
			    // return di.second->SerialNumber == info.SerialNumber;
			    return EVE_Hal_isDevice((EVE_HalContext *)di.second->EveHalContext, i);
		    });

		DeviceInfo *di;
		if (it != deviceInfo.end())
		{
			deviceInfo.erase(it->first);
			di = it->second;
		}
		else
		{
			// The device was not in use yet, create the gui
			di = new DeviceInfo();
			di->EveHalContext = NULL;
			di->View = new QTreeWidgetItem(m_DeviceList);
			di->View->setText(0, "No");
			di->View->setData(0, Qt::UserRole, QVariant::fromValue<DeviceInfo *>(di));
			di->DeviceIntf = 0;
		}

		// Store this device
		di->DeviceIdx = i;
		di->Host = info.Host;
		// di->DisplayName = info->DisplayName;
		di->View->setText(1, QString(info.DisplayName) + " (" + info.SerialNumber + ")");
		di->Id = (DeviceId)i;
		m_DeviceInfo[(DeviceId)i] = di;
	}

	// Erase devices that are gone from the list
	for (std::map<DeviceId, DeviceInfo *>::iterator it = deviceInfo.begin(), end = deviceInfo.end(); it != end; ++it)
	{
		// Delete anything in the DeviceInfo that needs to be deleted
		EVE_HalContext *phost = (EVE_HalContext *)it->second->EveHalContext;
		if (phost)
			EVE_Hal_close(phost);
		delete it->second->View;
		delete it->second;
	}
}

void DeviceManager::selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	printf("selectionChanged\n");

	updateSelection();
}

void DeviceManager::connectDevice()
{
	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is still in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	if (!m_DeviceList->currentItem())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Please select a device from the list.", QMessageBox::Ok);
		return;
	}

	printf("connectDevice\n");

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (devInfo->EveHalContext)
		return;

	std::unique_ptr<QDialog> progressDialog = std::make_unique<QDialog>(this);
	progressDialog->setWindowTitle("Connecting to Device");
	QLabel *progressLabel = new QLabel(progressDialog.get());
	QProgressBar *progressBar = new QProgressBar(progressDialog.get());
	QProgressBar *progressSubBar = new QProgressBar(progressDialog.get());
	progressLabel->setText("Connecting...");
	progressBar->setVisible(false);
	progressSubBar->setVisible(false);
	initProgressDialog(progressDialog.get(), progressLabel, progressBar, progressSubBar);
	progressDialog->setVisible(true);

	// Get parameters to open the selected device
	EVE_HalParameters params = { 0 };
	EVE_Hal_defaultsEx(&params, devInfo->DeviceIdx);
	if (params.Host == EVE_HOST_BT8XXEMU)
		params.EmulatorMode = (BT8XXEMU_EmulatorMode)deviceToEnum(FTEDITOR_CURRENT_DEVICE);
	devInfo->DeviceIntf = FTEDITOR_CURRENT_DEVICE;

	params.CbCmdWait = (EVE_Callback)cbCmdWait;
	params.UserContext = reinterpret_cast<void *>(this);

	EVE_HalContext *phost = new EVE_HalContext{ 0 };

	

	bool ok = EVE_Hal_open(phost, &params);
	if (!ok)
	{
		QMessageBox::critical(this, "Failed", "Failed to open HAL context", QMessageBox::Ok);
		delete phost;
		return;
	}

	phost->ChipId = (EVE_CHIPID_T)projectChipID();
	devInfo->EveHalContext = phost;

	EVE_BootupParameters bootupParams;
	EVE_Util_bootupDefaults(phost, &bootupParams);

	if (m_IsCustomDevice) {
		DeviceManageDialog::getCustomDeviceInfo(m_DeviceJsonPath, *m_CDI);
		bootupParams.ExternalOsc = m_CDI->ExternalClock;

		switch (m_CDI->SystemClock) {
		case 24: bootupParams.SystemClock = EVE_SYSCLK_24M;			break;
		case 36: bootupParams.SystemClock = EVE_SYSCLK_36M;			break;
		case 48: bootupParams.SystemClock = EVE_SYSCLK_48M;			break;
		case 60: bootupParams.SystemClock = EVE_SYSCLK_60M;			break;
		case 72: bootupParams.SystemClock = EVE_SYSCLK_72M;			break;
		case 84: bootupParams.SystemClock = EVE_SYSCLK_84M;			break;
		default: bootupParams.SystemClock = EVE_SYSCLK_DEFAULT;		break;
		}
	}

	progressLabel->setText("Boot up...");
	if (!EVE_Util_bootup(phost, &bootupParams))	{
		EVE_Hal_close(phost);
		devInfo->EveHalContext = NULL;
		delete phost;
		QMessageBox::critical(this, "Failed", "Failed to boot up EVE", QMessageBox::Ok);
		return;
	}

	devInfo->DeviceIntf = deviceToIntf((BT8XXEMU_EmulatorMode)(phost->ChipId & 0xFFFF));
	EVE_ConfigParameters configParams;
	

	QString projectDisplaySize = m_MainWindow->getDisplaySize();

	if (m_IsCustomDevice)
	{
		configParams = m_CDI->configParams;		
	}	
	else if (m_SelectedDisplaySize == "1280x800" || projectDisplaySize == "1280x800")
	{
		EVE_Util_configDefaults(phost, &configParams, EVE_DISPLAY_WXGA_1280x800_65Hz);		
	}
	else if (m_SelectedDisplaySize == "1024x600" || projectDisplaySize == "1024x600")
	{
		EVE_Util_configDefaults(phost, &configParams, EVE_DISPLAY_WSVGA_1024x600_83Hz);		
	}
	else if (m_SelectedDisplaySize == "800x480" || projectDisplaySize == "800x480")
	{
		configParams.Width = 800;
		configParams.Height = 480;
		configParams.HCycle = 928;
		configParams.HOffset = 88;
		configParams.HSync0 = 0;
		configParams.HSync1 = 48;
		configParams.VCycle = 525;
		configParams.VOffset = 32;
		configParams.VSync0 = 0;
		configParams.VSync1 = 3;
		configParams.PCLK = 2;
		configParams.Swizzle = 0;
		configParams.PCLKPol = 1;
		configParams.CSpread = 0;
		configParams.Dither = 1;
	}
	else if (m_SelectedDisplaySize == "480x272" || projectDisplaySize == "480x272")
	{
		configParams.Width = 480;
		configParams.Height = 272;
		configParams.HCycle = 548;
		configParams.HOffset = 43;
		configParams.HSync0 = 0;
		configParams.HSync1 = 41;
		configParams.VCycle = 292;
		configParams.VOffset = 12;
		configParams.VSync0 = 0;
		configParams.VSync1 = 10;
		configParams.PCLK = 5;
		configParams.Swizzle = 0;
		configParams.PCLKPol = 1;
		configParams.CSpread = 1;
		configParams.Dither = 1;
	}
	else if (m_SelectedDisplaySize == "320x240" || projectDisplaySize == "320x240")
	{
		configParams.Width = 320;
		configParams.Height = 240;
		configParams.HCycle = 408;
		configParams.HOffset = 70;
		configParams.HSync0 = 0;
		configParams.HSync1 = 10;
		configParams.VCycle = 263;
		configParams.VOffset = 13;
		configParams.VSync0 = 0;
		configParams.VSync1 = 2;
		configParams.PCLK = 8;
		configParams.Swizzle = 2;
		configParams.PCLKPol = 0;
		configParams.CSpread = 1;
		configParams.Dither = 1;
	}
	else {
		EVE_Util_configDefaults(phost, &configParams, EVE_DISPLAY_DEFAULT);
	}

	progressLabel->setText("Configure...");
	if (!EVE_Util_config(phost, &configParams))
	{
		EVE_Util_shutdown(phost);
		EVE_Hal_close(phost);
		devInfo->EveHalContext = NULL;
		delete phost;
		QMessageBox::critical(this, "Failed", "Failed to configure EVE", QMessageBox::Ok);
		return;
	}

	// print reg values
	printf("%s: %d\n", "REG_HCYCLE", EVE_Hal_rd16(phost, REG_HCYCLE));
	printf("%s: %d\n", "REG_HOFFSET", EVE_Hal_rd16(phost, REG_HOFFSET));
	printf("%s: %d\n", "REG_HSYNC0", EVE_Hal_rd16(phost, REG_HSYNC0));
	printf("%s: %d\n", "REG_HSYNC1", EVE_Hal_rd16(phost, REG_HSYNC1));
	printf("%s: %d\n", "REG_VCYCLE", EVE_Hal_rd16(phost, REG_VCYCLE));
	printf("%s: %d\n", "REG_VOFFSET", EVE_Hal_rd16(phost, REG_VOFFSET));
	printf("%s: %d\n", "REG_VSYNC0", EVE_Hal_rd16(phost, REG_VSYNC0));
	printf("%s: %d\n", "REG_VSYNC1", EVE_Hal_rd16(phost, REG_VSYNC1));
	printf("%s: %d\n", "REG_SWIZZLE", EVE_Hal_rd16(phost, REG_SWIZZLE));
	printf("%s: %d\n", "REG_PCLK_POL", EVE_Hal_rd16(phost, REG_PCLK_POL));
	printf("%s: %d\n", "REG_HSIZE", EVE_Hal_rd16(phost, REG_HSIZE));
	printf("%s: %d\n", "REG_VSIZE", EVE_Hal_rd16(phost, REG_VSIZE));

	printf("%s: %d\n", "REG_CSPREAD", EVE_Hal_rd16(phost, REG_CSPREAD));
	printf("%s: %d\n", "REG_DITHER", EVE_Hal_rd16(phost, REG_DITHER));
	printf("%s: %d\n", "REG_OUTBITS", EVE_Hal_rd16(phost, REG_OUTBITS));
	printf("%s: %d\n", "REG_PCLK_FREQ", EVE_Hal_rd16(phost, REG_PCLK_FREQ));
	printf("%s: %d\n", "REG_FREQUENCY", EVE_Hal_rd32(phost, REG_FREQUENCY));

	printf("%s: %d\n", "REG_ADAPTIVE_FRAMERATE", EVE_Hal_rd8(phost, REG_ADAPTIVE_FRAMERATE));
	printf("%s: %d\n", "REG_AH_HCYCLE_MAX", EVE_Hal_rd16(phost, REG_AH_HCYCLE_MAX));
	printf("%s: %d\n", "REG_PCLK_2X", EVE_Hal_rd8(phost, REG_PCLK_2X));
	printf("%s: %d\n", "REG_PCLK", EVE_Hal_rd8(phost, REG_PCLK));

	EVE_Hal_displayMessage(phost, "EVE Screen Editor ", sizeof("EVE Screen Editor "));





	updateSelection();

	m_RefreshButton->setDisabled(true);
	m_DeviceDisplayButton->setDisabled(true);
	m_DeviceManageButton->setDisabled(true);
}

void DeviceManager::disconnectDevice()
{
	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is still in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	if (!m_DeviceList->currentItem())
		return;

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (!devInfo->EveHalContext)
		return;

	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;

	EVE_Util_clearScreen(phost);
	EVE_Util_shutdown(phost);

	devInfo->EveHalContext = NULL;
	EVE_Hal_close(phost);

	updateSelection();

	m_RefreshButton->setEnabled(true);
	m_DeviceDisplayButton->setEnabled(true);
	m_DeviceManageButton->setEnabled(true);
}

void DeviceManager::uploadRamDl()
{
	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is still in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	if (!m_DeviceList->currentItem())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "No device selected.", QMessageBox::Ok);
		return;
	}

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (!devInfo->EveHalContext)
		return;

	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;

	g_ContentManager->lockContent();

	const uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
	const uint32_t *displayList = BT8XXEMU_getDisplayList(g_Emulator);

	EVE_Hal_wrMem(phost, addr(devInfo->DeviceIntf, FTEDITOR_RAM_G), ram, addr(devInfo->DeviceIntf, FTEDITOR_RAM_G_END) - addr(devInfo->DeviceIntf, FTEDITOR_RAM_G));
	EVE_Hal_wrMem(phost, addr(devInfo->DeviceIntf, FTEDITOR_RAM_DL), reinterpret_cast<const uint8_t *>(displayList), 4 * displayListSize(devInfo->DeviceIntf));

	EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_DLSWAP), DLSWAP_FRAME);

	// switch to next rotation
	EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_ROTATE), g_Rotate);

	g_Macro->lockDisplayList();

	if (g_Macro->getDisplayListParsed()[0].ValidId)
		EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_MACRO_0), g_Macro->getDisplayList()[0]);
	if (g_Macro->getDisplayListParsed()[1].ValidId)
		EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_MACRO_1), g_Macro->getDisplayList()[1]);

	g_Macro->unlockDisplayList();

	g_ContentManager->unlockContent();
}

void DeviceManager::uploadCoprocessorContent()
{
	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is still in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	std::unique_ptr<QDialog> progressDialog = std::make_unique<QDialog>(this);
	progressDialog->setWindowTitle("Uploading to Coprocessor");
	QLabel *progressLabel = new QLabel(progressDialog.get());
	QProgressBar *progressBar = new QProgressBar(progressDialog.get());
	QProgressBar *progressSubBar = new QProgressBar(progressDialog.get());
	progressLabel->setText("Preparing...");
	progressBar->setVisible(false);
	progressSubBar->setVisible(false);
	initProgressDialog(progressDialog.get(), progressLabel, progressBar, progressSubBar);
	progressDialog->setVisible(true);

	if (!m_DeviceList->currentItem())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "No device selected.", QMessageBox::Ok);
		return;
	}

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (!devInfo->EveHalContext)
		return;

	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;

	if (!EVE_Util_resetCoprocessor(phost))
	{
		if (m_Abort)
		{
			QMessageBox::critical(this, "Request Aborted", "The request has been aborted.", QMessageBox::Ok);
		}
		else if (devInfo->DeviceIntf >= FTEDITOR_BT815)
		{
			char err[128];
			EVE_Hal_rdMem(phost, (uint8_t *)err, 0x309800, 128);
			QMessageBox::critical(this, "Coprocessor Reset Failed", err[0] ? QString::fromUtf8(err) : "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		else
		{
			QMessageBox::critical(this, "Coprocessor Reset Failed", "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		return;
	}

	progressLabel->setText("Entering fast flash mode...");

	if (devInfo->DeviceIntf >= FTEDITOR_BT815)
	{
		uint32_t flashStatus = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_STATUS));
		if (flashStatus == FLASH_STATUS_BASIC)
		{
			EVE_Cmd_startFunc(phost);
			EVE_Cmd_wr32(phost, CMD_FLASHFAST);
			uint32_t resAddr = EVE_Cmd_moveWp(phost, 4); // Get the address where the coprocessor will write the result
			EVE_Cmd_endFunc(phost);
			if (!waitFlush(devInfo)) // Wait for command completion
				return;
			uint32_t flashRes = EVE_Hal_rd32(phost, RAM_CMD + resAddr); // Fetch result
		}
		flashStatus = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_STATUS));
		uint32_t flashSize = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_SIZE)) * 1024 * 1024;
	}

	g_ContentManager->lockContent();

	std::vector<ContentInfo *> ramContent = g_ContentManager->allRam();
	progressLabel->setText("Writing RAM_G...");
	for (const ContentInfo *info : ramContent)
	{
		if (m_Abort)
			break;

		int loadAddr = (info->Converter == ContentInfo::Image) ? info->bitmapAddress() : info->MemoryAddress;
		QString fileName = info->DestName + ".raw";
		QFile binFile(fileName);
		if (!binFile.exists())
			continue;
		bool imageCoprocessor = (info->Converter == ContentInfo::ImageCoprocessor);
		int binSize = imageCoprocessor ? info->CachedMemorySize : binFile.size();
		if (binSize + loadAddr > addr(devInfo->DeviceIntf, FTEDITOR_RAM_G_END))
			continue;
		if (imageCoprocessor)
		{
			EVE_Util_loadImageFileW(phost, loadAddr, fileName.toStdWString().c_str(), NULL);
			continue;
		}
		eve_scope
		{
			binFile.open(QIODevice::ReadOnly);
			QByteArray ba = binFile.readAll();
			EVE_Hal_wrMem(phost, loadAddr, reinterpret_cast<const uint8_t *>(ba.data()), ba.size());
		}
		if (info->Converter == ContentInfo::Font)
		{
			// Write bitmap address
			EVE_Hal_wr32(phost, loadAddr + 144, loadAddr + 148);
		}
		if (devInfo->DeviceIntf < FTEDITOR_FT810)
		{
			if (info->Converter == ContentInfo::Image && info->ImageFormat == PALETTED)
			{
				QString palName = info->DestName + ".lut.raw";
				QFile palFile(palName);
				if (!palFile.exists())
					continue;
				int palSize = (int)palFile.size();
				if (palSize != 1024)
					continue;

				palFile.open(QIODevice::ReadOnly);
				QByteArray ba = palFile.readAll();
				EVE_Hal_wrMem(phost, addr(devInfo->DeviceIntf, FTEDITOR_RAM_PAL), reinterpret_cast<const uint8_t *>(ba.data()), ba.size());
			}
		}
		if (devInfo->DeviceIntf >= FTEDITOR_FT810)
		{
			if (info->Converter == ContentInfo::Image && (info->ImageFormat == PALETTED8 || info->ImageFormat == PALETTED565 || info->ImageFormat == PALETTED4444))
			{
				int palSize;
				switch (info->ImageFormat)
				{
				case PALETTED565:
				case PALETTED4444:
					palSize = 256 * 2;
					break;
				default:
					palSize = 256 * 4;
					break;
				}
				QString palName = info->DestName + ".lut.raw";
				QFile palFile(palName);
				if (!palFile.exists())
					continue;

				{
					palFile.open(QIODevice::ReadOnly);
					QByteArray ba = palFile.readAll();
					EVE_Hal_wrMem(phost, info->MemoryAddress, reinterpret_cast<const uint8_t *>(ba.data()), ba.size());
				}
			}
		}
	}

	g_ContentManager->unlockContent();

	// switch to next rotation
	EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_ROTATE), g_Rotate);

	g_Macro->lockDisplayList();

	if (g_Macro->getDisplayListParsed()[0].ValidId)
		EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_MACRO_0), g_Macro->getDisplayList()[0]);
	if (g_Macro->getDisplayListParsed()[1].ValidId)
		EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_MACRO_1), g_Macro->getDisplayList()[1]);

	g_Macro->unlockDisplayList();

	g_DlEditor->lockDisplayList();

	uint32_t *displayList = g_DlEditor->getDisplayList();

	EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, addr(devInfo->DeviceIntf, FTEDITOR_RAM_DL));
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
		EVE_Hal_transfer32(phost, displayList[i]);
	EVE_Hal_endTransfer(phost);

	g_DlEditor->unlockDisplayList();

	g_CmdEditor->lockDisplayList();

	bool validCmd = false;
	uint32_t cmdList[FTEDITOR_DL_SIZE];
	std::vector<uint32_t> cmdParamCache;
	std::vector<std::string> cmdStrParamCache;
	int strParamRead = 0;
	int cmdParamIdx[FTEDITOR_DL_SIZE + 1];
	bool cmdValid[FTEDITOR_DL_SIZE];
	QString cmdText[FTEDITOR_DL_SIZE];
	uint32_t *cmdListPtr = g_CmdEditor->getDisplayList();
	const DlParsed *cmdParsedPtr = g_CmdEditor->getDisplayListParsed();

	// Make local copy, necessary in case of blocking commands
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		cmdList[i] = cmdListPtr[i];
		// cmdParsed[i] = cmdParsedPtr[i];
		cmdParamIdx[i] = (int)cmdParamCache.size();
		DlParser::compile(devInfo->DeviceIntf, cmdParamCache, cmdParsedPtr[i]);
		cmdValid[i] = cmdParsedPtr[i].ValidId;
		if (cmdValid[i])
		{
			cmdText[i] = g_CmdEditor->getLineText(i);
			switch (cmdList[i])
			{
			case CMD_MEMWRITE:
			case CMD_INFLATE:
			case CMD_INFLATE2:
			case CMD_LOADIMAGE:
			case CMD_PLAYVIDEO:
				cmdStrParamCache.push_back(cmdParsedPtr[i].StringParameter);
				break;
			}
		}
	}
	cmdParamIdx[FTEDITOR_DL_SIZE] = (int)cmdParamCache.size();

	g_CmdEditor->unlockDisplayList();

	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_Abort)
			break;
		if (!cmdValid[i])
			continue;
		progressLabel->setText(cmdText[i]);
		bool useMediaFifo = false;
		bool useFlash = false;
		const char *useFileStream = NULL;
		if ((devInfo->DeviceIntf >= FTEDITOR_FT810) && (cmdList[i] == CMD_MEDIAFIFO))
		{
			validCmd = true;
			uint32_t address = cmdParamCache[cmdParamIdx[i]];
			uint32_t size = cmdParamCache[(size_t)cmdParamIdx[i] + 1];
			EVE_MediaFifo_set(phost, address, size);
		}
		else if (cmdList[i] == CMD_LOADIMAGE || cmdList[i] == CMD_INFLATE2)
		{
			useFlash = (devInfo->DeviceIntf >= FTEDITOR_BT815)
			    && (cmdParamCache[(size_t)cmdParamIdx[i] + 1] & OPT_FLASH);
			useFileStream = useFlash ? NULL : cmdStrParamCache[strParamRead].c_str();
			++strParamRead;
			useMediaFifo = (devInfo->DeviceIntf >= FTEDITOR_FT810)
			    && (cmdParamCache[(size_t)cmdParamIdx[i] + 1] & OPT_MEDIAFIFO);
		}
		else if (cmdList[i] == CMD_INFLATE )
		{
			useFileStream = cmdStrParamCache[strParamRead].c_str();
			++strParamRead;
		}
		else if (cmdList[i] == CMD_MEMWRITE )
		{
			useFileStream = cmdStrParamCache[strParamRead].c_str();
			++strParamRead;
		}
		else if (cmdList[i] == CMD_PLAYVIDEO)
		{
			useFlash = (devInfo->DeviceIntf >= FTEDITOR_BT815)
			    && (cmdParamCache[cmdParamIdx[i]] & OPT_FLASH);
			useFileStream = useFlash ? NULL : cmdStrParamCache[strParamRead].c_str();
			++strParamRead;
			useMediaFifo = (devInfo->DeviceIntf >= FTEDITOR_FT810)
			    && ((cmdParamCache[cmdParamIdx[i]] & OPT_MEDIAFIFO) == OPT_MEDIAFIFO);
		}
		else if (cmdList[i] == CMD_SNAPSHOT)
		{
			// Validate snapshot address range
			uint32_t addr = cmdParamCache[cmdParamIdx[i]];
			uint32_t ramGEnd = FTEDITOR::addr(devInfo->DeviceIntf, FTEDITOR_RAM_G_END);
			uint32_t imgSize = (g_VSize * g_HSize) * 2;
			if (addr + imgSize > ramGEnd)
				continue;
		}
		if (useFileStream)
		{
			if (!QFileInfo::exists(useFileStream))
				continue;
		}
		validCmd = true;
		int paramNb = cmdParamIdx[i + 1] - cmdParamIdx[i];
		int cmdLen = 4 + (paramNb * 4);
		EVE_Cmd_startFunc(phost);
		EVE_Cmd_wr32(phost, cmdList[i]);
		for (int j = cmdParamIdx[i]; j < cmdParamIdx[i + 1]; ++j)
			EVE_Cmd_wr32(phost, cmdParamCache[j]);
		EVE_Cmd_endFunc(phost);
		if (cmdList[i] == CMD_LOGO)
		{
			// printf("Waiting for CMD_LOGO...\n");
			EVE_Cmd_waitLogo(phost);

			EVE_Cmd_wr32(phost, CMD_DLSTART);
			EVE_Cmd_wr32(phost, CMD_COLDSTART);
		}
		else if (cmdList[i] == CMD_CALIBRATE)
		{
			EVE_Cmd_waitFlush(phost);

			EVE_Cmd_wr32(phost, CMD_DLSTART);
			EVE_Cmd_wr32(phost, CMD_COLDSTART);
		}
		else if ((cmdList[i] == CMD_PLAYVIDEO) && useFlash)
		{
			EVE_Cmd_waitFlush(phost);

			EVE_Cmd_wr32(phost, CMD_DLSTART);
			EVE_Cmd_wr32(phost, CMD_COLDSTART);
		}
		if (useFileStream)
		{
			if (useMediaFifo)
			{
				if (phost->MediaFifoSize)
				{
					// Load entire file into media fifo
					m_StreamTransfered = 0;
					m_StreamProgress = progressSubBar;
					progressSubBar->setValue(0);
					progressSubBar->setRange(0, QFile(QString::fromUtf8(useFileStream)).size());
					progressSubBar->setVisible(true);
					EVE_Util_loadMediaFileW(phost, QString::fromUtf8(useFileStream).toStdWString().c_str(), &m_StreamTransfered);
					progressSubBar->setVisible(false);
					m_StreamTransfered = 0;
					m_StreamProgress = NULL;
				}
				else
				{
					// Media fifo not set up
					EVE_Util_resetCoprocessor(phost);
					continue;
				}
			}
			else
			{
				// Load entire file into cmd fifo
				m_StreamTransfered = 0;
				m_StreamProgress = progressSubBar;
				progressSubBar->setValue(0);
				progressSubBar->setRange(0, QFile(QString::fromUtf8(useFileStream)).size());
				progressSubBar->setVisible(true);
				EVE_Util_loadCmdFileW(phost, QString::fromUtf8(useFileStream).toStdWString().c_str(), &m_StreamTransfered);
				progressSubBar->setVisible(false);
				m_StreamTransfered = 0;
				m_StreamProgress = NULL;
			}
		}
	}

	if (validCmd)
	{
		EVE_Cmd_wr32(phost, DISPLAY());
		EVE_Cmd_wr32(phost, CMD_SWAP);

		waitFlush(devInfo);
	}
	else
	{
		EVE_Hal_wr32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_DLSWAP), DLSWAP_FRAME);
	}
}

bool DeviceManager::waitFlush(DeviceInfo *devInfo)
{
	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;
	if (!EVE_Cmd_waitFlush(phost))
	{
		if (m_Abort)
		{
			QMessageBox::critical(this, "Request Aborted", "The request has been aborted.", QMessageBox::Ok);
		}
		else if (devInfo->DeviceIntf >= FTEDITOR_BT815)
		{
			char err[128];
			EVE_Hal_rdMem(phost, (uint8_t *)err, 0x309800, 128);
			QMessageBox::critical(this, "Coprocessor Error", err[0] ? QString::fromUtf8(err) : "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		else
		{
			QMessageBox::critical(this, "Coprocessor Error", "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		return false;
	}
	return true;
}

void DeviceManager::uploadFlashContent()
{
	uint8_t buffer[64 * 4096];

	BusyLock busyLock(&m_Busy);
	if (!busyLock.locked())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "Another request is already in progress.", QMessageBox::Ok);
		return;
	}
	m_Abort = false;

	if (!m_DeviceList->currentItem())
	{
		QMessageBox::warning(this, "EVE Screen Editor", "No device selected.", QMessageBox::Ok);
		return;
	}

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (!devInfo->EveHalContext)
		return;

	std::unique_ptr<QDialog> progressDialog = std::make_unique<QDialog>(this);
	progressDialog->setWindowTitle("Writing to Flash");
	QLabel *progressLabel = new QLabel(progressDialog.get());
	QProgressBar *progressBar = new QProgressBar(progressDialog.get());
	QProgressBar *progressSubBar = new QProgressBar(progressDialog.get());
	progressLabel->setText("Preparing...");
	progressBar->setVisible(false);
	progressSubBar->setVisible(false);
	initProgressDialog(progressDialog.get(), progressLabel, progressBar, progressSubBar);
	progressDialog->setVisible(true);

	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;

	EVE_Host_powerModeSwitch(phost, EVE_POWERDOWN_M);
	EVE_sleep(50);
	EVE_Util_bootupConfig(phost);

	if (!EVE_Util_resetCoprocessor(phost))
	{
		if (m_Abort)
		{
			QMessageBox::critical(this, "Request Aborted", "The request has been aborted.", QMessageBox::Ok);
		}
		else if (devInfo->DeviceIntf >= FTEDITOR_BT815)
		{
			char err[128];
			EVE_Hal_rdMem(phost, (uint8_t *)err, 0x309800, 128);
			QMessageBox::critical(this, "Coprocessor Reset Failed", err[0] ? QString::fromUtf8(err) : "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		else
		{
			QMessageBox::critical(this, "Coprocessor Reset Failed", "Coprocessor has signaled an error.", QMessageBox::Ok);
		}
		return;
	}

	uint32_t flashStatus = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_STATUS));
	if (flashStatus == FLASH_STATUS_DETACHED)
	{
		EVE_Cmd_wr32(phost, CMD_FLASHATTACH);
		if (!waitFlush(devInfo)) // Wait for command completion
			return;
	}
	flashStatus = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_STATUS));
	if (flashStatus < FLASH_STATUS_BASIC)
	{
		QMessageBox::critical(this, "Flash Error", "Flash could not be attached.", QMessageBox::Ok);
		return;
	}
	if (flashStatus == FLASH_STATUS_BASIC)
	{
		EVE_Cmd_startFunc(phost);
		EVE_Cmd_wr32(phost, CMD_FLASHFAST);
		uint32_t resAddr = EVE_Cmd_moveWp(phost, 4); // Get the address where the coprocessor will write the result
		EVE_Cmd_endFunc(phost);
		if (!waitFlush(devInfo)) // Wait for command completion
			return;
		uint32_t flashRes = EVE_Hal_rd32(phost, RAM_CMD + resAddr); // Fetch result
		// TODO: Show flashRes error messages
	}
	flashStatus = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_STATUS));
	if (flashStatus < FLASH_STATUS_FULL)
	{
		QMessageBox::critical(this, "Flash Error", "Flash could not enter fast writing mode. Has the BT81X flash firmware blob been written yet?", QMessageBox::Ok);
		return;
	}
	uint32_t flashSize = EVE_Hal_rd32(phost, reg(devInfo->DeviceIntf, FTEDITOR_REG_FLASH_SIZE)) * 1024 * 1024;

	g_ContentManager->lockContent();

	std::vector<ContentInfo *> flashContent = g_ContentManager->allFlash();
	progressBar->setRange(0, (int)flashContent.size());
	progressBar->setValue(0);
	progressBar->setVisible(true);
	progressSubBar->setRange(0, 1);
	progressSubBar->setValue(0);
	progressSubBar->setVisible(true);
	progressLabel->setText("Writing...");
	for (const ContentInfo *info : flashContent)
	{
		if (m_Abort)
			break;
		progressBar->setValue(progressBar->value() + 1);
		progressBar->setMinimum(1);
		int loadAddr = info->FlashAddress; // (info->Converter == ContentInfo::Image) ? info->bitmapAddress() : info->MemoryAddress;
		if (loadAddr < FTEDITOR_FLASH_FIRMWARE_SIZE)
		{
			// Safety to avoid breaking functionality, never allow overriding the provided firmware from the content manager
			printf("[WriteFlash] Error: Load address not permitted for '%s' to '%i'\n", info->DestName.toLocal8Bit().data(), loadAddr);
			continue;
		}
		bool dataCompressed = (info->Converter != ContentInfo::ImageCoprocessor && info->Converter != ContentInfo::FlashMap)
		    ? info->DataCompressed
		    : false;
		QString fileName = info->DestName + (dataCompressed ? ".bin" : ".raw");
		printf("[WriteFlash] Load: '%s' to '%i'\n", fileName.toLocal8Bit().constData(), loadAddr);
		QFile binFile(fileName);
		if (!binFile.exists())
		{
			printf("[WriteFlash] Error: File '%s' does not exist\n", fileName.toLocal8Bit().constData());
			continue;
		}
		int binSize = binFile.size();
		if ((uint32_t)(binSize + loadAddr) > flashSize)
		{
			printf("[WriteFlash] Error: File of size '%i' exceeds flash size\n", binSize);
			continue;
		}
		if (loadAddr & (64 - 1))
		{
			printf("[WriteFlash] Error: Flash address '%i' not aligned\n", loadAddr);
			continue;
		}
		eve_scope
		{
			binFile.open(QIODevice::ReadOnly);
			progressLabel->setText("Writing \"" + info->DestName + "\"");
			progressSubBar->setValue(0);
			progressSubBar->setRange(0, binFile.size());
			QDataStream in(&binFile);
			// char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_Flash_data(g_Flash)));
			// int s = in.readRawData(&ram[loadAddr], binSize);
			// BT8XXEMU_poke(g_Emulator);
			int sz = 0;
			int preread = (loadAddr & (4096 - 1)); // Read previously written data
			loadAddr -= preread;
			for (;;)
			{
				if (m_Abort)
					break;
				sz = preread;
				int l;
				do
				{
					l = in.readRawData((char *)&buffer[sz], sizeof(buffer) - sz);
					sz += l;
					progressSubBar->setValue(progressSubBar->value() + l);
				} while (l != 0 && sz < sizeof(buffer));
				if (sz)
				{
					int pad = (64 - (sz & (64 - 1))) & (64 - 1);
					if (pad)
					{
						for (int i = 0; i < pad; ++i)
							buffer[sz++] = 0x00;
					}
					int szn = (sz + 4095) & ~4095;
					if (szn != sz)
					{
						EVE_Cmd_startFunc(phost);
						EVE_Cmd_wr32(phost, CMD_FLASHREAD);
						EVE_Cmd_wr32(phost, szn - 4096);
						EVE_Cmd_wr32(phost, loadAddr + szn - 4096);
						EVE_Cmd_wr32(phost, 4096);
						EVE_Cmd_endFunc(phost);
					}
					if (preread)
					{
						EVE_Cmd_startFunc(phost);
						EVE_Cmd_wr32(phost, CMD_FLASHREAD);
						EVE_Cmd_wr32(phost, 0);
						EVE_Cmd_wr32(phost, loadAddr);
						EVE_Cmd_wr32(phost, (preread + 3) & ~3);
						EVE_Cmd_endFunc(phost);
					}
					if (!waitFlush(devInfo)) // Wait for command completion
					{
						binFile.close();
						g_ContentManager->unlockContent();
						return;
					}
					EVE_Hal_wrMem(phost, preread, &buffer[preread], sz - preread);
					preread = 0;
					EVE_Cmd_startFunc(phost);
					EVE_Cmd_wr32(phost, CMD_FLASHUPDATE);
					EVE_Cmd_wr32(phost, loadAddr);
					EVE_Cmd_wr32(phost, 0);
					EVE_Cmd_wr32(phost, szn);
					EVE_Cmd_endFunc(phost);
					if (!waitFlush(devInfo)) // Wait for command completion
					{
						binFile.close();
						g_ContentManager->unlockContent();
						return;
					}
				}
				else
				{
					break;
				}
				loadAddr += sizeof(buffer);
			};
			binFile.close();
		}
	}

	EVE_Cmd_wr32(phost, CMD_CLEARCACHE);
	if (!waitFlush(devInfo)) // Wait for command completion
	{
		g_ContentManager->unlockContent();
		return;
	}

	progressBar->setMinimum(0);
	progressBar->setValue(0);
	progressLabel->setText("Verifying...");
	for (const ContentInfo *info : flashContent)
	{
		if (m_Abort)
			break;
		progressBar->setValue(progressBar->value() + 1);
		progressBar->setMinimum(1);
		int loadAddr = info->FlashAddress; // (info->Converter == ContentInfo::Image) ? info->bitmapAddress() : info->MemoryAddress;
		if (loadAddr < FTEDITOR_FLASH_FIRMWARE_SIZE)
		{
			// Safety to avoid breaking functionality, never allow overriding the provided firmware from the content manager
			printf("[WriteFlash] Error: Load address not permitted for '%s' to '%i'\n", info->DestName.toLocal8Bit().data(), loadAddr);
			continue;
		}
		bool dataCompressed = (info->Converter != ContentInfo::ImageCoprocessor && info->Converter != ContentInfo::FlashMap)
		    ? info->DataCompressed
		    : false;
		QString fileName = info->DestName + (dataCompressed ? ".bin" : ".raw");
		printf("[WriteFlash] Load: '%s' to '%i'\n", fileName.toLocal8Bit().constData(), loadAddr);
		QFile binFile(fileName);
		if (!binFile.exists())
		{
			printf("[WriteFlash] Error: File '%s' does not exist\n", fileName.toLocal8Bit().constData());
			continue;
		}
		int binSize = binFile.size();
		if ((uint32_t)(binSize + loadAddr) > flashSize)
		{
			printf("[WriteFlash] Error: File of size '%i' exceeds flash size\n", binSize);
			continue;
		}
		if (loadAddr & (64 - 1))
		{
			printf("[WriteFlash] Error: Flash address '%i' not aligned\n", loadAddr);
			continue;
		}
		eve_scope
		{
			binFile.open(QIODevice::ReadOnly);
			progressLabel->setText("Verifying \"" + info->DestName + "\"");
			progressSubBar->setValue(0);
			progressSubBar->setRange(0, binFile.size());
			QDataStream in(&binFile);
			// char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_Flash_data(g_Flash)));
			// int s = in.readRawData(&ram[loadAddr], binSize);
			// BT8XXEMU_poke(g_Emulator);
			uint8_t match[sizeof(buffer)];
			int sz;
			for (;;)
			{
				if (m_Abort)
					break;
				sz = 0;
				int l;
				do
				{
					l = in.readRawData((char *)&buffer[sz], sizeof(buffer) - sz);
					sz += l;
					progressSubBar->setValue(progressSubBar->value() + l);
				} while (l != 0 && sz < sizeof(buffer));
				if (sz)
				{
					EVE_Cmd_startFunc(phost);
					EVE_Cmd_wr32(phost, CMD_MEMSET);
					EVE_Cmd_wr32(phost, 0);
					EVE_Cmd_wr32(phost, 0xCB);
					EVE_Cmd_wr32(phost, sz);
					EVE_Cmd_wr32(phost, CMD_FLASHREAD);
					EVE_Cmd_wr32(phost, 0);
					EVE_Cmd_wr32(phost, loadAddr);
					EVE_Cmd_wr32(phost, sz);
					EVE_Cmd_endFunc(phost);
					if (!waitFlush(devInfo)) // Wait for command completion
					{
						binFile.close();
						g_ContentManager->unlockContent();
						return;
					}
					EVE_Hal_rdMem(phost, match, 0, sz);
					for (int i = 0; i < sz; ++i)
						if (buffer[i] != match[i])
							printf("[WriteFlash] Error: Validation failed at address %i\n", loadAddr + i);
				}
				else
				{
					break;
				}
				loadAddr += sizeof(buffer);
			};
			binFile.close();
		}
	}

	g_ContentManager->unlockContent();
}

void DeviceManager::uploadFlashBlob()
{
}

void DeviceManager::updateSelection()
{
	if (!m_DeviceList->currentItem())
	{
		m_ConnectButton->setVisible(false);
		m_DisconnectButton->setVisible(false);
		m_UploadRamDlButton->setVisible(false);
		m_UploadCoprocessorContentButton->setVisible(false);
		m_UploadFlashContentButton->setVisible(false);
		m_UploadFlashBlobButton->setVisible(false);
	}
	else
	{
		DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
		m_ConnectButton->setVisible(!devInfo->EveHalContext);
		m_DisconnectButton->setVisible(devInfo->EveHalContext);
		m_UploadRamDlButton->setVisible(devInfo->EveHalContext);
		m_UploadCoprocessorContentButton->setVisible(devInfo->EveHalContext);
		m_UploadFlashContentButton->setVisible(devInfo->EveHalContext && flashSupport(devInfo->DeviceIntf));
		// TODO: m_UploadFlashBlobButton->setVisible(devInfo->EveHalContext && flashSupport(devInfo->DeviceIntf));
		if (devInfo->EveHalContext)
		{
			devInfo->View->setText(0, "Yes");
		}
		else
		{
			devInfo->View->setText(0, "No");
		}
	}
}

int DeviceManager::projectChipID() {
	return (int)EVE_extendedChipId((int)deviceToEnum(FTEDITOR_CURRENT_DEVICE));
}
#endif /* FT800_DEVICE_MANAGER */

} /* namespace FTEDITOR */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
