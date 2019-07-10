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

// Emulator includes
#include <EVE_Platform.h>
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "content_manager.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

#include "device_display_settings_dialog.h"

namespace FTEDITOR
{

extern BT8XXEMU_Emulator *g_Emulator;
extern ContentManager *g_ContentManager;

#if FT800_DEVICE_MANAGER

DeviceManager::DeviceManager(MainWindow *parent)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_displaySettingsDialog(NULL)
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

	buttons->addStretch();

	// Upload RAM_G and RAM_DL
	// Upload RAM and Coprocessor Commands
	// Upload Flash

	m_SendImageButton = new QPushButton(this);
	m_SendImageButton->setText(tr("Upload RAM_G and RAM_DL"));
	m_SendImageButton->setToolTip(tr("Sends the current memory and display list to the selected device"));
	m_SendImageButton->setVisible(false);
	connect(m_SendImageButton, &QPushButton::clicked, this, &DeviceManager::uploadRamDl);
	buttons->addWidget(m_SendImageButton);

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

	setLayout(layout);

	//Init MPSSE lib
	// Init_libMPSSE();
	EVE_HalPlatform *platform = EVE_Hal_initialize();
	// Initial refresh of devices
	refreshDevices();

	m_displaySettingsDialog = new DeviceDisplaySettingsDialog(this);
}

DeviceManager::~DeviceManager()
{
}

/*
void DeviceManager::setDeviceandScreenSize(QString displaySize, QString syncDevice)
{
	QStringList pieces = displaySize.split("x");
	if ((currScreenSize != displaySize) && m_DisconnectButton->isVisible())
	{
		disconnectDevice();
	}

	setCurrentDisplaySize(displaySize);

	setSyncDeviceName(syncDevice);
	m_MainWindow->userChangeResolution(pieces[0].toUInt(), pieces[1].toUInt());
	
}
*/

void DeviceManager::deviceDisplaySettings()
{
	if (m_displaySettingsDialog == NULL)
	{
		m_displaySettingsDialog = new DeviceDisplaySettingsDialog(this);
	}

	m_displaySettingsDialog->execute();
}

void DeviceManager::refreshDevices()
{
	printf("Refresh devices\n");

	size_t eveDeviceCount;
	EVE_DeviceInfo *eveDeviceInfo = EVE_Hal_list(&eveDeviceCount);

	// Swap device list to local
	std::map<DeviceId, DeviceInfo *> deviceInfo;
	deviceInfo.swap(m_DeviceInfo);

	// For each device that is found
	for (size_t i = 0; i < eveDeviceCount; ++i)
	{
		EVE_DeviceInfo *info = &eveDeviceInfo[i];
		std::map<DeviceId, DeviceInfo *>::iterator it = std::find_if(deviceInfo.begin(), deviceInfo.end(),
		    [&](std::pair<const DeviceId, DeviceInfo *> &di) -> bool {
			    // Match by serial number
			    return di.second->SerialNumber == info->SerialNumber;
		    });

		DeviceInfo *di;
		if (it != deviceInfo.end())
		{
			deviceInfo.erase(it->first);
			di = it->second;
		}
		else
		{
			// The device was not added yet, create the gui
			di = new DeviceInfo();
			di->EveHalContext = NULL;
			di->SerialNumber = info->SerialNumber; // Store serial number for matching
			di->View = new QTreeWidgetItem(m_DeviceList);
			di->View->setText(0, "No");
			di->View->setData(0, Qt::UserRole, qVariantFromValue<DeviceInfo *>(di));
			di->DeviceIntf = 0;
		}

		// Store this device
		di->EveDeviceInfo = info;
		di->Type = info->Type;
		// di->DisplayName = info->DisplayName;
		di->View->setText(1, info->DisplayName);
		di->Id = (DeviceId)i;
		m_DeviceInfo[(DeviceId)i] = di;
	}

	// Erase devices that are gone from the list
	for (std::map<DeviceId, DeviceInfo *>::iterator it = deviceInfo.begin(), end = deviceInfo.end(); it != end; ++it)
	{
		// Delete anything in the DeviceInfo that needs to be deleted
		delete it->second->View;
		delete it->second;
	}

#if 0 /* move this to hal */
	FT_STATUS ftstatus = FT_OTHER_ERROR;
	DWORD num_devs = 0;		
    DWORD library_ver;


    if(FT_OK == FT_GetLibraryVersion(&library_ver))
        printf("FTGetLibraryVersion = 0x%x\n",library_ver);

    FT_STATUS status = FT_ListDevices(&num_devs, NULL, FT_LIST_NUMBER_ONLY);
    if (FT_OK != status)
    {
        printf("FT_ListDevices failed");
        //ret = FALSE;
    }

	if ( FT_OK != (ftstatus = FT_CreateDeviceInfoList(&num_devs)))
		printf("FT_CreateDeviceInfoList failed , status %d\n", ftstatus);

	// Swap device list to local
	std::map<DeviceId, DeviceInfo *> deviceInfo;
	deviceInfo.swap(m_DeviceInfo);
	
	// For each device that is found
	for (uint32_t i = 0; i < num_devs; i++)
	{
		FT_DEVICE_LIST_INFO_NODE devNodeInfo;
		ftstatus = FT_GetDeviceInfoDetail(
											i,
											&devNodeInfo.Flags,
											&devNodeInfo.Type,
											&devNodeInfo.ID,
											&devNodeInfo.LocId,
											devNodeInfo.SerialNumber,
											devNodeInfo.Description,
											&devNodeInfo.ftHandle
										);
		if (FT_OK != ftstatus)
			printf("FT_GetDeviceInfoDetail failed , status %d\n", ftstatus);

		if (!strstr(devNodeInfo.Description,"FT4222 B"))
		{
			DeviceId devId = i;
			QString devName(devNodeInfo.Description);

			bool IsDeviceFound = FALSE;
			for (std::map<DeviceId, DeviceInfo *>::iterator index = deviceInfo.begin(); index != deviceInfo.end(); ++index)
			{
				if (strcmp(index->second->description, devNodeInfo.Description))
					continue;
				else
				{
					//device description match
					IsDeviceFound = TRUE;
					m_DeviceInfo[devId] = deviceInfo[index->first];
					m_DeviceInfo[devId]->Id = devId;
					deviceInfo.erase(index);
					break;
				}
			}
			if (IsDeviceFound == FALSE)
			{
				// The device was not added yet, create the gui
				QTreeWidgetItem *view = new QTreeWidgetItem(m_DeviceList);
				view->setText(0, "No");
				view->setText(1, devName);

				// Store this device
				DeviceInfo *devInfo = new DeviceInfo();
				devInfo->Id = devId;
				devInfo->View = view;
				strcpy(devInfo->description, devNodeInfo.Description);
				devInfo->Connected = false;
				m_DeviceInfo[devId] = devInfo;
				view->setData(0, Qt::UserRole, qVariantFromValue<DeviceInfo *>(devInfo));
			}
		}
	}

	// Erase devices that are gone from the list
	for (std::map<DeviceId, DeviceInfo *>::iterator it = deviceInfo.begin(), end = deviceInfo.end(); it != end; ++it)
	{
		// Delete anything in the DeviceInfo that needs to be deleted
		delete it->second->View;
		delete it->second;		
	}
#endif
}

void DeviceManager::selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	printf("selectionChanged\n");

	updateSelection();
}

#if 0 /* move this to hal */
bool DeviceManager::connectDeviceBT8xx(Gpu_Hal_Context_t *phost, DeviceInfo *devInfo)
{
	if (phost->lib_type == LIB_FT4222)
	{
		phost->hal_config.channel_no = devInfo->Id;
		phost->hal_config.pdn_pin_no = FT800_PD_N;
		phost->hal_config.spi_cs_pin_no = FT800_SEL_PIN;
		phost->hal_config.spi_clockrate_khz = 20000; //in KHz
	}
	else
	{
		phost->hal_config.channel_no = 0;
		phost->hal_config.pdn_pin_no = 7;
		phost->hal_config.spi_cs_pin_no = 0;
		phost->hal_config.spi_clockrate_khz = 12000; //in KHz
	}

	Gpu_Hal_Open(phost);

	// Bootup Config
	Gpu_Hal_Powercycle(phost, TRUE);

	/* FT81x will be in SPI Single channel after POR
    If we are here with FT4222 in multi channel, then
    an explicit switch to single channel is essential
    */
#ifdef FT81X_ENABLE
	Gpu_Hal_SetSPI(phost, GPU_SPI_SINGLE_CHANNEL, GPU_SPI_ONEDUMMY);
#endif

	Gpu_HostCommand(phost, GPU_EXTERNAL_OSC);
	Gpu_Hal_Sleep(10);

	/* Access address 0 to wake up the chip */
	Gpu_HostCommand(phost, GPU_ACTIVE_M);
	Gpu_Hal_Sleep(300);

	/* Read REG_CHIPID to confirm 0x7C is returned */
	{
		uint8_t chipid = 0x99;
		chipid = Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));

		int timeout_count = 0;
		const int MAX_READCOUNT = 200;
		while (0x7C != chipid)
		{
			Gpu_Hal_Sleep(10);
			chipid = Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));

			timeout_count++;
			if (timeout_count > MAX_READCOUNT)
			{
				qDebug("cannot connect to device\n");
				QMessageBox::warning(this, "Failed to connect device!", "cannot read chipID");

				Gpu_Hal_Close(phost);
				delete (phost);
				return false;
			}
		}

		printf("VC1 register ID after wake up %x\n", chipid);
	}

	/* Read REG_CPURESET to confirm 0 is returned */
	{
		uint8_t engine_status;
		/* Read REG_CPURESET to check if engines are ready.
        Bit 0 for coprocessor engine,
        Bit 1 for touch engine,
        Bit 2 for audio engine.
        */
		engine_status = Gpu_Hal_Rd8(phost, REG_CPURESET);
		while (engine_status != 0x00)
		{
			if (engine_status & 0x01)
			{
				printf("coprocessor engine is not ready \n");
			}
			if (engine_status & 0x02)
			{
				printf("touch engine is not ready \n");
			}
			if (engine_status & 0x04)
			{
				printf("audio engine is not ready \n");
			}

			engine_status = Gpu_Hal_Rd8(phost, REG_CPURESET);
			Gpu_Hal_Sleep(100);
		}
		printf("All engines are ready \n");
	}

	/* Configuration of LCD display */

	Gpu_Hal_Wr16(phost, REG_HCYCLE, DispHCycle);
	Gpu_Hal_Wr16(phost, REG_HOFFSET, DispHOffset);
	Gpu_Hal_Wr16(phost, REG_HSYNC0, DispHSync0);
	Gpu_Hal_Wr16(phost, REG_HSYNC1, DispHSync1);
	Gpu_Hal_Wr16(phost, REG_VCYCLE, DispVCycle);
	Gpu_Hal_Wr16(phost, REG_VOFFSET, DispVOffset);
	Gpu_Hal_Wr16(phost, REG_VSYNC0, DispVSync0);
	Gpu_Hal_Wr16(phost, REG_VSYNC1, DispVSync1);
	Gpu_Hal_Wr8(phost, REG_SWIZZLE, DispSwizzle);
	Gpu_Hal_Wr8(phost, REG_PCLK_POL, DispPCLKPol);
	Gpu_Hal_Wr16(phost, REG_HSIZE, DispWidth);
	Gpu_Hal_Wr16(phost, REG_VSIZE, DispHeight);
	Gpu_Hal_Wr16(phost, REG_CSPREAD, DispCSpread);
	Gpu_Hal_Wr16(phost, REG_DITHER, DispDither);

	Gpu_Hal_Wr16(phost, REG_GPIOX_DIR, 0xffff);
	Gpu_Hal_Wr16(phost, REG_GPIOX, 0xffff);

	Gpu_ClearScreen(phost);
	Gpu_Hal_Wr8(phost, REG_PCLK, DispPCLK); //after this display is visible on the LCD

	if (phost->lib_type == LIB_FT4222)
	{
		Gpu_Hal_SetSPI(phost, GPU_SPI_QUAD_CHANNEL, GPU_SPI_TWODUMMY);
	}
	else
	{
		Gpu_Hal_SetSPI(phost, GPU_SPI_SINGLE_CHANNEL, GPU_SPI_ONEDUMMY);
	}

	phost->cmd_fifo_wp = Gpu_Hal_Rd16(phost, REG_CMD_WRITE);
	// End Bootup Config

	return true;
}

bool DeviceManager::connectDeviceFT8xx(Gpu_Hal_Context_t *phost, DeviceInfo *devInfo)
{
	QString deviceDescription = (QString)devInfo->description;

	if (deviceDescription == "FT4222 A")
	{
		qDebug("It is FT4222A device\n");
		qDebug("current EVE type: %d", syncDeviceEVEType);

		phost->hal_config.channel_no = devInfo->Id;
		phost->hal_config.spi_clockrate_khz = 20000; //in KHz
		phost->hal_config.pdn_pin_no = 0;
		phost->hal_config.spi_cs_pin_no = 1;
	}
	else
	{
		qDebug("It is MPSSE device\n");
		qDebug("current EVE type: %d", syncDeviceEVEType);

		phost->hal_config.channel_no = 0;
		phost->hal_config.pdn_pin_no = 7;
		phost->hal_config.spi_cs_pin_no = 0;
		phost->hal_config.spi_clockrate_khz = 12000; //in KHz
	}

	Gpu_Hal_Open(phost);

	/* Do a power cycle for safer side */
	Gpu_Hal_Powercycle(phost, TRUE);
	Gpu_Hal_Sleep(20);

	/* Access address 0 to wake up the FT800 */
	Gpu_HostCommand(phost, GPU_ACTIVE_M);
	Gpu_Hal_Sleep(300);

	if (deviceDescription != "FT4222 A")
	{
		/* Set the clk to external clock */
		Gpu_HostCommand(phost, GPU_EXTERNAL_OSC);
		Gpu_Hal_Sleep(10);
	}

	uint8_t chipid = 0x99;
	chipid = Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));

	int timeout_count = 0;
	const int MAX_READCOUNT = 200;
	while (0x7C != chipid)
	{
		Gpu_Hal_Sleep(10);
		chipid = Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));

		timeout_count++;
		if (timeout_count > MAX_READCOUNT)
		{
			qDebug("cannot connect to device\n");
			QMessageBox::warning(this, "Failed to connect device!", "cannot read chipID");

			Gpu_Hal_Close(phost);
			delete (phost);
			return false;
		}
	}

	printf("REG_ID detected as  %x\n", chipid);

	if (currScreenSize == "480x272")
	{
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 548);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 43);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 41);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 292);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 12);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 10);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 0);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 1);

		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 480);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 272);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 5); //after this display is visible on the LCD
	}
	else if (currScreenSize == "800x480")
	{
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 928);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 88);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 48);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 525);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 32);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 3);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 0);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 1);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 800);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 480);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_CSPREAD), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_DITHER), 1);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 2); //after this display is visible on the LCD
	}
	else if (currScreenSize == "320x240")
	{
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 408);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 70);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 10);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 263);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 13);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 2);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 2);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 0);
		Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 8); //after this display is visible on the LCD
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 320);
		Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 240);
	}

	Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO_DIR), 0x83 | Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO_DIR)));
	Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO), 0x083 | Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO)));

	return true;
}
#endif

void DeviceManager::connectDevice()
{
	if (!m_DeviceList->currentItem())
	{
		QMessageBox::warning(this, "select device first\n", "Please Select the device in the list", QMessageBox::Ok);
		return;
	}

	printf("connectDevice\n");

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (devInfo->EveHalContext)
		return;

	// Get parameters to open the selected device
	EVE_HalParameters params = { 0 };
	EVE_Hal_defaultsEx(&params, deviceToEnum(FTEDITOR_CURRENT_DEVICE), (EVE_DeviceInfo *)devInfo->EveDeviceInfo);
	devInfo->DeviceIntf = deviceToIntf((BT8XXEMU_EmulatorMode)params.Model);

	EVE_HalContext *phost = new EVE_HalContext{ 0 };
	bool ok = EVE_Hal_open(phost, &params);
	if (!ok)
	{
		QMessageBox::critical(this, "Failed", "Failed to open HAL context", QMessageBox::Ok);
		delete phost;
		return;
	}

	devInfo->EveHalContext = phost;

	const uint32_t connectedScreenCmds[] = {
		CMD_DLSTART,
		CLEAR_COLOR_RGB(31, 63, 0),
		CLEAR(1, 1, 1),
		DISPLAY(),
		CMD_SWAP
	};

	if (!EVE_Util_bootupConfig(phost))
	{
		EVE_Hal_close(phost);
		devInfo->EveHalContext = NULL;
		delete phost;
		QMessageBox::critical(this, "Failed", "Failed to boot up EVE", QMessageBox::Ok);
		return;
	}

	EVE_Cmd_wrMem(phost, (uint8_t *)connectedScreenCmds, sizeof(connectedScreenCmds));
	EVE_Hal_flush(phost);

	// TODO: Initialize with the specified screen resolution & the default settings for said resolution

	updateSelection();

#if 0
	QString deviceDescription = (QString)devInfo->description;

	Gpu_Hal_Context_t *phost = new Gpu_Hal_Context_t;
	phost->lib_type = (deviceDescription == "FT4222 A") ? LIB_FT4222 : LIB_MPSSE;

	bool ok = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815) ? connectDeviceBT8xx(phost, devInfo) : connectDeviceFT8xx(phost, devInfo);
	if (!ok)
		return;

	const uint32_t CONNECTED_SCREEN_CMDS[] = {
		CMD_DLSTART,
		CLEAR_COLOR_RGB(31, 63, 0),
		CLEAR(1, 1, 1),
		DISPLAY(),
		CMD_SWAP
	};

	Gpu_Hal_WrMem(phost, addr(syncDeviceEVEType, FTEDITOR_RAM_CMD), (uint8_t *)CONNECTED_SCREEN_CMDS, sizeof(CONNECTED_SCREEN_CMDS));

	Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_CMD_WRITE), sizeof(CONNECTED_SCREEN_CMDS));

	devInfo->Connected = true;
	devInfo->handle = (void *)phost;
	updateSelection();
#endif
}

void DeviceManager::disconnectDevice()
{
	if (!m_DeviceList->currentItem())
		return;

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
	if (!devInfo)
		return;
	if (!devInfo->EveHalContext)
		return;

	EVE_HalContext *phost = (EVE_HalContext *)devInfo->EveHalContext;

	const uint32_t connectedScreenCmds[] = {
		CMD_DLSTART,
		CLEAR_COLOR_RGB(31, 63, 0),
		CLEAR(1, 1, 1),
		DISPLAY(),
		CMD_SWAP
	};

	EVE_Cmd_wrMem(phost, (uint8_t *)connectedScreenCmds, sizeof(connectedScreenCmds));
	EVE_Hal_flush(phost);

	devInfo->EveHalContext = NULL;
	EVE_Hal_close(phost);

	updateSelection();

#if 0

	Gpu_Hal_WrMem(phost, addr(syncDeviceEVEType, FTEDITOR_RAM_CMD), (uint8_t *)CONNECTED_SCREEN_CMDS, sizeof(CONNECTED_SCREEN_CMDS));

	Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_CMD_WRITE), sizeof(CONNECTED_SCREEN_CMDS));

	Gpu_Hal_Close(phost);

	Gpu_Hal_DeInit(phost);

	delete (phost);

	devInfo->Connected = false;
	updateSelection();
#endif
}

#if 0
void DeviceManager::loadContent2Device(ContentManager *contentManager, Gpu_Hal_Context_t *phost)
{
	contentManager->lockContent();

	QTreeWidget *contentList = (QTreeWidget *)contentManager->contentList();
	uint8_t *ram = static_cast<uint8_t *>(BT8XXEMU_getRam(g_Emulator));

	for (QTreeWidgetItemIterator it(contentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		/*if (info->MemoryLoaded && info->CachedSize && (info->MemoryAddress + info->CachedSize <= addr(syncDeviceEVEType, FTEDITOR_RAM_G_END)))
		{
            {
			Gpu_Hal_WrMem(phost,addr(syncDeviceEVEType, FTEDITOR_RAM_G)+info->MemoryAddress,&ram[addr(syncDeviceEVEType, FTEDITOR_RAM_G)+info->MemoryAddress],info->CachedSize);
			}

			if (syncDeviceEVEType < FTEDITOR_FT810)
			{
				if (info->ImageFormat == PALETTED){
					const ft_uint32_t PALSIZE = 1024;
					Gpu_Hal_WrMem(phost,addr(syncDeviceEVEType, FTEDITOR_RAM_PAL),&ram[addr(syncDeviceEVEType, FTEDITOR_RAM_PAL)],PALSIZE);
				}
			}
		}*/
	}
	contentManager->unlockContent();
}
#endif

void DeviceManager::uploadRamDl()
{
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

	g_ContentManager->unlockContent();
}

void DeviceManager::uploadCoprocessorContent()
{
}

void DeviceManager::uploadFlash()
{
}

void DeviceManager::updateSelection()
{
	if (!m_DeviceList->currentItem())
	{
		m_ConnectButton->setVisible(false);
		m_DisconnectButton->setVisible(false);
		m_SendImageButton->setVisible(false);
	}
	else
	{
		DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
		m_ConnectButton->setVisible(!devInfo->EveHalContext);
		m_DisconnectButton->setVisible(devInfo->EveHalContext);
		m_SendImageButton->setVisible(devInfo->EveHalContext);
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
