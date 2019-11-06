/*
Copyright (C) 2014-2015  Future Technology Devices International Ltd
*/

#include "device_manager.h"

// STL includes
#include <stdio.h>

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
#include <EVE_Platform.h>
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "content_manager.h"
#include "dl_parser.h"
#include "dl_editor.h"
#include "constant_mapping.h"

#include "device_display_settings_dialog.h"
#include "device_manage_dialog.h"

namespace FTEDITOR
{

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
    , isBuiltin(false)
    , CUS_REG_HCYCLE(0)
    , CUS_REG_HOFFSET(0)
    , CUS_REG_HSYNC0(0)
    , CUS_REG_HSYNC1(0)
    , CUS_REG_VCYCLE(0)
    , CUS_REG_VOFFSET(0)
    , CUS_REG_VSYNC0(0)
    , CUS_REG_VSYNC1(0)
    , CUS_REG_SWIZZLE(0)
    , CUS_REG_PCLK_POL(0)
    , CUS_REG_HSIZE(0)
    , CUS_REG_VSIZE(0)
    , CUS_REG_CSPREAD(0)
    , CUS_REG_DITHER(0)
    , CUS_REG_PCLK(0)
    , ExternalOsc(true)
{
	// no-op
}

DeviceManager::DeviceManager(MainWindow *parent)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_DisplaySettingsDialog(NULL)
    , m_DeviceManageDialog(NULL)
    , m_IsCustomDevice(false)
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

	QPushButton *refreshButton = new QPushButton(this);
	refreshButton->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
	refreshButton->setToolTip(tr("Refresh the device list"));
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshDevices()));
	buttons->addWidget(refreshButton);
	refreshButton->setMaximumWidth(refreshButton->height());

	QPushButton *deviceDisplayButton = new QPushButton(this);
	deviceDisplayButton->setIcon(QIcon(":/icons/wrench-screwdriver.png"));
	deviceDisplayButton->setToolTip(tr("Device display settings"));
	connect(deviceDisplayButton, SIGNAL(clicked()), this, SLOT(deviceDisplaySettings()));
	buttons->addWidget(deviceDisplayButton);
	deviceDisplayButton->setMaximumWidth(deviceDisplayButton->height());

	QPushButton *deviceManageButton = new QPushButton(this);
	deviceManageButton->setIcon(QIcon(":/icons/category.png"));
	deviceManageButton->setToolTip(tr("Manage Device"));
	connect(deviceManageButton, SIGNAL(clicked()), this, SLOT(deviceManage()));
	buttons->addWidget(deviceManageButton);
	deviceManageButton->setMaximumWidth(deviceManageButton->height());

	buttons->addStretch();

	m_ConnectButton = new QPushButton(this);
	m_ConnectButton->setText(tr("Connect"));
	m_ConnectButton->setToolTip(tr("Connect the selected device"));
	m_ConnectButton->setVisible(false);
	connect(m_ConnectButton, SIGNAL(clicked()), this, SLOT(connectDevice()));
	buttons->addWidget(m_ConnectButton);

	m_DisconnectButton = new QPushButton(this);
	m_DisconnectButton->setText(tr("Disconnect"));
	m_DisconnectButton->setToolTip(tr("Disconnect from the selected device"));
	m_DisconnectButton->setVisible(false);
	connect(m_DisconnectButton, SIGNAL(clicked()), this, SLOT(disconnectDevice()));
	buttons->addWidget(m_DisconnectButton);

	layout->addLayout(buttons);

	// Upload RAM_G and RAM_DL
	// Upload RAM and Coprocessor Commands
	// Upload Flash

	m_UploadRamDlButton = new QPushButton(this);
	m_UploadRamDlButton->setText(tr("Upload RAM_G and RAM_DL"));
	m_UploadRamDlButton->setToolTip(tr("Sends the current memory and display list to the selected device"));
	m_UploadRamDlButton->setVisible(false);
	connect(m_UploadRamDlButton, &QPushButton::clicked, this, &DeviceManager::uploadRamDl);
	layout->addWidget(m_UploadRamDlButton);

	m_UploadCoprocessorContentButton = new QPushButton(this);
	m_UploadCoprocessorContentButton->setText(tr("Upload RAM and Coprocessor"));
	m_UploadCoprocessorContentButton->setToolTip(tr(""));
	m_UploadCoprocessorContentButton->setVisible(false);
	connect(m_UploadCoprocessorContentButton, &QPushButton::clicked, this, &DeviceManager::uploadCoprocessorContent);
	layout->addWidget(m_UploadCoprocessorContentButton);

	m_UploadFlashContentButton = new QPushButton(this);
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

	if (m_IsCustomDevice)
	{
		DeviceManageDialog::getCustomDeviceInfo(jsonPath, m_CDI);
	}

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
			di->View->setData(0, Qt::UserRole, qVariantFromValue<DeviceInfo *>(di));
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
	EVE_Hal_defaultsEx(&params, (EVE_CHIPID_T)deviceToEnum(FTEDITOR_CURRENT_DEVICE), devInfo->DeviceIdx);
	devInfo->DeviceIntf = deviceToIntf((BT8XXEMU_EmulatorMode)params.ChipId);

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

	EVE_BootupParameters bootupParams;
	EVE_Util_bootupDefaults(phost, &bootupParams);

	if (m_IsCustomDevice)
	{
		bootupParams.Width = m_CDI.CUS_REG_HSIZE;
		bootupParams.Height = m_CDI.CUS_REG_VSIZE;
		bootupParams.HCycle = m_CDI.CUS_REG_HCYCLE;
		bootupParams.HOffset = m_CDI.CUS_REG_HOFFSET;
		bootupParams.HSync0 = m_CDI.CUS_REG_HSYNC0;
		bootupParams.HSync1 = m_CDI.CUS_REG_HSYNC1;
		bootupParams.VCycle = m_CDI.CUS_REG_VCYCLE;
		bootupParams.VOffset = m_CDI.CUS_REG_VOFFSET;
		bootupParams.VSync0 = m_CDI.CUS_REG_VSYNC0;
		bootupParams.VSync1 = m_CDI.CUS_REG_VSYNC1;
		bootupParams.PCLK = m_CDI.CUS_REG_PCLK;
		bootupParams.Swizzle = m_CDI.CUS_REG_SWIZZLE;
		bootupParams.PCLKPol = m_CDI.CUS_REG_PCLK_POL;
		bootupParams.CSpread = m_CDI.CUS_REG_CSPREAD;
		bootupParams.Dither = m_CDI.CUS_REG_DITHER;
	}
	else if (m_SelectedDisplaySize == "480x272")
	{
		bootupParams.Width = 480;
		bootupParams.Height = 272;
		bootupParams.HCycle = 548;
		bootupParams.HOffset = 43;
		bootupParams.HSync0 = 0;
		bootupParams.HSync1 = 41;
		bootupParams.VCycle = 292;
		bootupParams.VOffset = 12;
		bootupParams.VSync0 = 0;
		bootupParams.VSync1 = 10;
		bootupParams.PCLK = 5;
		bootupParams.Swizzle = 0;
		bootupParams.PCLKPol = 1;
		bootupParams.CSpread = 1;
		bootupParams.Dither = 1;
	}
	else if (m_SelectedDisplaySize == "800x480")
	{
		bootupParams.Width = 800;
		bootupParams.Height = 480;
		bootupParams.HCycle = 928;
		bootupParams.HOffset = 88;
		bootupParams.HSync0 = 0;
		bootupParams.HSync1 = 48;
		bootupParams.VCycle = 525;
		bootupParams.VOffset = 32;
		bootupParams.VSync0 = 0;
		bootupParams.VSync1 = 3;
		bootupParams.PCLK = 2;
		bootupParams.Swizzle = 0;
		bootupParams.PCLKPol = 1;
		bootupParams.CSpread = 0;
		bootupParams.Dither = 1;
	}
	else if (m_SelectedDisplaySize == "320x240")
	{
		bootupParams.Width = 320;
		bootupParams.Height = 240;
		bootupParams.HCycle = 408;
		bootupParams.HOffset = 70;
		bootupParams.HSync0 = 0;
		bootupParams.HSync1 = 10;
		bootupParams.VCycle = 263;
		bootupParams.VOffset = 13;
		bootupParams.VSync0 = 0;
		bootupParams.VSync1 = 2;
		bootupParams.PCLK = 8;
		bootupParams.Swizzle = 2;
		bootupParams.PCLKPol = 0;
		bootupParams.CSpread = 1;
		bootupParams.Dither = 1;
	}

	bootupParams.ExternalOsc = m_CDI.ExternalOsc;

	devInfo->EveHalContext = phost;

	progressLabel->setText("Boot up...");
	if (!EVE_Util_bootup(phost, &bootupParams))
	{
		EVE_Hal_close(phost);
		devInfo->EveHalContext = NULL;
		delete phost;
		QMessageBox::critical(this, "Failed", "Failed to boot up EVE", QMessageBox::Ok);
		return;
	}

	EVE_Hal_displayMessage(phost, "EVE Screen Editor ", sizeof("EVE Screen Editor "));

	updateSelection();
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
		scope
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
			uint32_t size = cmdParamCache[cmdParamIdx[i] + 1];
			EVE_MediaFifo_set(phost, address, size);
		}
		else if (cmdList[i] == CMD_LOADIMAGE)
		{
			useFlash = (devInfo->DeviceIntf >= FTEDITOR_BT815)
			    && (cmdParamCache[cmdParamIdx[i] + 1] & OPT_FLASH);
			useFileStream = useFlash ? NULL : cmdStrParamCache[strParamRead].c_str();
			++strParamRead;
			useMediaFifo = (devInfo->DeviceIntf >= FTEDITOR_FT810)
			    && (cmdParamCache[cmdParamIdx[i] + 1] & OPT_MEDIAFIFO);
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
			if (!QFileInfo(useFileStream).exists())
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
		scope
		{
			binFile.open(QIODevice::ReadOnly);
			progressLabel->setText("Writing \"" + info->DestName + "\"");
			progressSubBar->setValue(0);
			progressSubBar->setRange(0, binFile.size());
			QDataStream in(&binFile);
			// char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_Flash_data(g_Flash)));
			// int s = in.readRawData(&ram[loadAddr], binSize);
			// BT8XXEMU_poke(g_Emulator);
			uint8_t buffer[64 * 4096];
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
		scope
		{
			binFile.open(QIODevice::ReadOnly);
			progressLabel->setText("Verifying \"" + info->DestName + "\"");
			progressSubBar->setValue(0);
			progressSubBar->setRange(0, binFile.size());
			QDataStream in(&binFile);
			// char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_Flash_data(g_Flash)));
			// int s = in.readRawData(&ram[loadAddr], binSize);
			// BT8XXEMU_poke(g_Emulator);
			uint8_t buffer[64 * 4096];
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

#endif /* FT800_DEVICE_MANAGER */

} /* namespace FTEDITOR */

/* end of file */
