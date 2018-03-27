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
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "content_manager.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

#include "device_display_settings_dialog.h"



namespace FTEDITOR {

extern BT8XXEMU_Emulator *g_Emulator;

#if FT800_DEVICE_MANAGER

DeviceManager::DeviceManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent),
currScreenSize("480x272"), selectedSyncDevice("VM800B43A"), m_displaySettingsDialog(NULL)
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

	m_SendImageButton = new QPushButton(this);
	m_SendImageButton->setText(tr("Sync With Device"));
	m_SendImageButton->setToolTip(tr("Sends the current memory and display list to the selected device"));
	m_SendImageButton->setVisible(false);
	connect(m_SendImageButton, SIGNAL(clicked()), this, SLOT(syncDevice()));
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

    syncDeviceEVEType = FTEDITOR_FT800;

	layout->addLayout(buttons);

	setLayout(layout);

	//Init MPSSE lib
	Init_libMPSSE();
	// Initial refresh of devices
	refreshDevices();

	m_displaySettingsDialog = new DeviceDisplaySettingsDialog(this);
}

DeviceManager::~DeviceManager()
{

}

void DeviceManager::setDeviceandScreenSize(QString displaySize, QString syncDevice){
	QStringList pieces = displaySize.split("x");
	if ((currScreenSize != displaySize) && m_DisconnectButton->isVisible()){
		disconnectDevice();
	}

    setCurrentDisplaySize(displaySize);

    setSyncDeviceName(syncDevice);
	m_MainWindow->userChangeResolution(pieces[0].toUInt(),pieces[1].toUInt());
}

void DeviceManager::deviceDisplaySettings(){
	if (m_displaySettingsDialog == NULL){
		m_displaySettingsDialog = new DeviceDisplaySettingsDialog(this);
	}

	m_displaySettingsDialog->execute();
	
}

void DeviceManager::refreshDevices()
{
	printf("Refresh devices\n");

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

}

void DeviceManager::selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	printf("selectionChanged\n");

	updateSelection();
}

void DeviceManager::connectDevice()
{
	if (!m_DeviceList->currentItem()) {
		QMessageBox::warning(this, "select device first\n", "Please Select the device in the list", QMessageBox::Ok);
		return;
	}

	if (syncDeviceEVEType >= FTEDITOR_DEVICE_NB)
	{
		QMessageBox::warning(this, "Project Type Not correct\n", "Only FT80X project supported", QMessageBox::Ok);
		return;
	}

	printf("connectDevice\n");


	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();

	if (devInfo->Connected) return;


	Ft_Gpu_Hal_Context_t *phost = new Ft_Gpu_Hal_Context_t;

    QString deviceDescription = QString(devInfo->description);

    if (deviceDescription == "FT4222 A")
    {
        qDebug("It is FT4222A device\n");
        qDebug("current EVE type: %d",syncDeviceEVEType);

        phost->spi_host = SPIHOST_FT4222_SPI;

        phost->hal_config.channel_no = devInfo->Id;
        phost->hal_config.spi_clockrate_khz = 20000; //in KHz
        phost->hal_config.pdn_pin_no = GPIO_PORT0;
        phost->hal_config.spi_cs_pin_no = 1;

        Ft_Gpu_Hal_Open_FT4222Dev(phost);
    }else
    {
        qDebug("It is MPSSE device\n");
        qDebug("current EVE type: %d",syncDeviceEVEType);

        phost->spi_host = SPIHOST_MPSSE_VA800A_SPI;

        phost->hal_config.channel_no = 0;
        phost->hal_config.pdn_pin_no = 7;
        phost->hal_config.spi_cs_pin_no = 0;
        phost->hal_config.spi_clockrate_khz = 12000; //in KHz

        Ft_Gpu_Hal_Open_MPSSEDev(phost);
    }



	/* Do a power cycle for safer side */
	Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);
	Ft_Gpu_Hal_Sleep(20);

	/* Access address 0 to wake up the FT800 */
	Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);
	Ft_Gpu_Hal_Sleep(20);

    if (phost->spi_host == SPIHOST_MPSSE_VA800A_SPI){
        /* Set the clk to external clock */
        Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
        Ft_Gpu_Hal_Sleep(10);
    }

	ft_uint8_t chipid = 0x99;
	chipid = Ft_Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));	

    int timeout_count = 0; const int MAX_READCOUNT = 200;
	while (0x7C != chipid)
	{
		Ft_Gpu_Hal_Sleep(10);
		chipid = Ft_Gpu_Hal_Rd8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_ID));

        timeout_count ++;
        if (timeout_count > MAX_READCOUNT){
            qDebug("cannot connect to device\n");
            QMessageBox::warning(this,"Failed to connect device!","cannot read chipID");

            Ft_Gpu_Hal_Close(phost);
            delete(phost);
            return;
        }
	}
	
	printf("REG_ID detected as  %x\n", chipid);
	{
		if (currScreenSize == "480x272"){
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 548);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 43);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 41);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 292);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 12);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 10);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 0);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 1);

			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 480);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 272);
            Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 5);//after this display is visible on the LCD
		}
		else if (currScreenSize == "800x480"){
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 928);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 88);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 48);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 525);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 32);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 3);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 0);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 1);			
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 800);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 480);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_CSPREAD), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_DITHER), 1);
            Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 2);//after this display is visible on the LCD
		}
		else if (currScreenSize == "320x240"){
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HCYCLE), 408);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HOFFSET), 70);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSYNC1), 10);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VCYCLE), 263);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VOFFSET), 13);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC0), 0);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSYNC1), 2);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_SWIZZLE), 2);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK_POL), 0);
			Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_PCLK), 8);//after this display is visible on the LCD
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_HSIZE), 320);
			Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_VSIZE), 240);
		}


        Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO_DIR),0x83 | Ft_Gpu_Hal_Rd8(phost,reg(syncDeviceEVEType, FTEDITOR_REG_GPIO_DIR)));
        Ft_Gpu_Hal_Wr8(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIO),0x083 | Ft_Gpu_Hal_Rd8(phost,reg(syncDeviceEVEType, FTEDITOR_REG_GPIO)));
        //Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIOX_DIR), 0xffff);
        //Ft_Gpu_Hal_Wr16(phost, reg(syncDeviceEVEType, FTEDITOR_REG_GPIOX), 0xffff);


        const ft_uint32_t CONNECTED_SCREEN_CMDS [] =
        {
            CMD_DLSTART,
            CLEAR_COLOR_RGB(31, 63, 0),
            CLEAR(1,1,1),
            DISPLAY(),
            CMD_SWAP
        };

        Ft_Gpu_Hal_WrMem(phost, addr(syncDeviceEVEType,FTEDITOR_RAM_CMD),(ft_uint8_t *)CONNECTED_SCREEN_CMDS,sizeof(CONNECTED_SCREEN_CMDS));

        Ft_Gpu_Hal_Wr16(phost,reg(syncDeviceEVEType,FTEDITOR_REG_CMD_WRITE),sizeof(CONNECTED_SCREEN_CMDS));

        devInfo->Connected = true;
	}

	devInfo->handle = (void*)phost;
	updateSelection();
}

void DeviceManager::disconnectDevice()
{
	if (!m_DeviceList->currentItem()) return;

	DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();

    if (devInfo){
        Ft_Gpu_Hal_Context_t *phost = (Ft_Gpu_Hal_Context_t*)devInfo->handle;

        if (phost){

            const ft_uint32_t CONNECTED_SCREEN_CMDS [] =
            {
                CMD_DLSTART,
                CLEAR_COLOR_RGB(31, 63, 0),
                CLEAR(1,1,1),
                DISPLAY(),
                CMD_SWAP
            };

            Ft_Gpu_Hal_WrMem(phost, addr(syncDeviceEVEType,FTEDITOR_RAM_CMD),(ft_uint8_t *)CONNECTED_SCREEN_CMDS,sizeof(CONNECTED_SCREEN_CMDS));

            Ft_Gpu_Hal_Wr16(phost,reg(syncDeviceEVEType,FTEDITOR_REG_CMD_WRITE),sizeof(CONNECTED_SCREEN_CMDS));

			Ft_Gpu_Hal_Close(phost);

            delete (phost);
        }
    }

	devInfo->Connected = false;
	updateSelection();
}

QString DeviceManager::getCurrentDisplaySize(){
	return currScreenSize;
}

QString DeviceManager::getSyncDeviceName(){
	return selectedSyncDevice;
}

void DeviceManager::setCurrentDisplaySize(QString displaySize){
	currScreenSize = displaySize;
}

void DeviceManager::setSyncDeviceName(QString deviceName){
	selectedSyncDevice = deviceName;

    if (selectedSyncDevice == "ME813AU_WH50C(800x480)")
    {
        syncDeviceEVEType = FTEDITOR_FT813;
    }else if (selectedSyncDevice == "VM800BU50A")
    {
        syncDeviceEVEType = FTEDITOR_FT800;
    }else
    {
        syncDeviceEVEType = FTEDITOR_FT800;
    }
}

void DeviceManager::loadContent2Device(ContentManager *contentManager, Ft_Gpu_Hal_Context_t *phost)
{
	contentManager->lockContent();

	QTreeWidget *contentList = (QTreeWidget*)contentManager->contentList();
	ft_uint8_t *ram = static_cast<ft_uint8_t *>(BT8XXEMU_getRam(g_Emulator));
	
	for (QTreeWidgetItemIterator it(contentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		/*if (info->MemoryLoaded && info->CachedSize && (info->MemoryAddress + info->CachedSize <= addr(syncDeviceEVEType, FTEDITOR_RAM_G_END)))
		{
            {
			Ft_Gpu_Hal_WrMem(phost,addr(syncDeviceEVEType, FTEDITOR_RAM_G)+info->MemoryAddress,&ram[addr(syncDeviceEVEType, FTEDITOR_RAM_G)+info->MemoryAddress],info->CachedSize);
			}

			if (syncDeviceEVEType < FTEDITOR_FT810)
			{
				if (info->ImageFormat == PALETTED){
					const ft_uint32_t PALSIZE = 1024;
					Ft_Gpu_Hal_WrMem(phost,addr(syncDeviceEVEType, FTEDITOR_RAM_PAL),&ram[addr(syncDeviceEVEType, FTEDITOR_RAM_PAL)],PALSIZE);
				}
			}
		}*/
	}
	contentManager->unlockContent();
}

void DeviceManager::syncDevice()
{
	if (!m_DeviceList->currentItem()){
		QMessageBox::question(this, "General confirmation", "No Content in the command list.",QMessageBox::Yes | QMessageBox::No);
		return;
	}

	ft_uint8_t *ram = static_cast<ft_uint8_t *>(BT8XXEMU_getRam(g_Emulator));
	const uint32_t *displayList = BT8XXEMU_getDisplayList(g_Emulator);
	//Sync with selected device
	{

		DeviceInfo *devInfo = m_DeviceList->currentItem()->data(0, Qt::UserRole).value<DeviceInfo *>();
		if (devInfo->Connected)
		{
			Ft_Gpu_Hal_Context_t *phost = (Ft_Gpu_Hal_Context_t *)devInfo->handle;

			loadContent2Device(m_MainWindow->contentManager(), phost);

            Ft_Gpu_Hal_WrMem(phost,addr(syncDeviceEVEType, FTEDITOR_RAM_DL),static_cast<const uint8_t *>(static_cast<const void *> (displayList)), 4 * displayListSize(syncDeviceEVEType));

			Ft_Gpu_Hal_Wr32(phost, reg(syncDeviceEVEType, FTEDITOR_REG_DLSWAP), DLSWAP_FRAME);
		}
	}
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
		m_ConnectButton->setVisible(!devInfo->Connected);
		m_DisconnectButton->setVisible(devInfo->Connected);
		m_SendImageButton->setVisible(devInfo->Connected);
		if (devInfo->Connected)
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
