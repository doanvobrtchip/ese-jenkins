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
#include <ft8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "content_manager.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

#if FT800_DEVICE_MANAGER
//mpsse lib includes -- Windows
#undef POINTS
#include <Windows.h>
#include "LibMPSSE_spi.h"

#include "FT_DataTypes.h"
#include "FT_Gpu_Hal.h"

ft_int16_t FT_DispWidth = 480;
ft_int16_t FT_DispHeight = 272;
ft_int16_t FT_DispHCycle =  548;
ft_int16_t FT_DispHOffset = 43;
ft_int16_t FT_DispHSync0 = 0;
ft_int16_t FT_DispHSync1 = 41;
ft_int16_t FT_DispVCycle = 292;
ft_int16_t FT_DispVOffset = 12;
ft_int16_t FT_DispVSync0 = 0;
ft_int16_t FT_DispVSync1 = 10;
ft_uint8_t FT_DispPCLK = 5;
ft_char8_t FT_DispSwizzle = 0;
ft_char8_t FT_DispPCLKPol = 1;
#endif

using namespace std;

namespace FTEDITOR {

#if FT800_DEVICE_MANAGER

DeviceManager::DeviceManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
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
	//uint icon[2] = { 0x27F2, 0 };
	//refreshButton->setText(QString::fromUcs4(icon));//tr("Refresh"));
	refreshButton->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
	refreshButton->setToolTip(tr("Refresh the device list"));
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshDevices()));
	buttons->addWidget(refreshButton);
	refreshButton->setMaximumWidth(refreshButton->height());

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

	layout->addLayout(buttons);

	setLayout(layout);

    //Init MPSSE lib
    Init_libMPSSE();
	// Initial refresh of devices
	refreshDevices();
}

DeviceManager::~DeviceManager()
{

}

void DeviceManager::refreshDevices()
{
	printf("Refresh devices\n");

	// Swap device list to local
	std::map<DeviceId, DeviceInfo *> deviceInfo;
	deviceInfo.swap(m_DeviceInfo);

	uint32 deviceNum;
	SPI_GetNumChannels(&deviceNum);

	// For each device that is found
	for (uint32_t i = 0;i < deviceNum;i++)
	{
		FT_DEVICE_LIST_INFO_NODE devListInfoNode;
		SPI_GetChannelInfo(i,&devListInfoNode);

		DeviceId devId = i;
		QString devName(devListInfoNode.Description);

		if (deviceInfo.find(devId) == deviceInfo.end())
		{
			// The device was not added yet, create the gui
			QTreeWidgetItem *view = new QTreeWidgetItem(m_DeviceList);
			view->setText(0, "");
			view->setText(1, devName);

			// Store this device
			DeviceInfo *devInfo = new DeviceInfo();
			devInfo->Id = devId;
			devInfo->View = view;
			devInfo->Connected = false;
			m_DeviceInfo[devId] = devInfo;
			view->setData(0, Qt::UserRole, qVariantFromValue((quintptr)(void *)devInfo));
		}
		else
		{
			// Already know this device, store back into list
			m_DeviceInfo[devId] = deviceInfo[devId];
			deviceInfo.erase(devId);
		}
	}

	// Erase devices that are gone from the list
	for (std::map<DeviceId, DeviceInfo *>::iterator it = deviceInfo.begin(), end = deviceInfo.end(); it != end; ++it)
	{
		// Delete anything in the DeviceInfo that needs to be deleted
		delete it->second->View;
		// TODO ...
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
	if (!m_DeviceList->currentItem()) return;

	printf("connectDevice\n");


	DeviceInfo *devInfo = (DeviceInfo *)(void *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<quintptr>();

	if (devInfo->Connected) return;

	Ft_Gpu_Hal_Context_t *phost = new Ft_Gpu_Hal_Context_t;

	phost->hal_config.channel_no = devInfo->Id;
	phost->hal_config.spi_clockrate_khz = 12000; //in KHz



	Ft_Gpu_Hal_Open(phost);

	/* Do a power cycle for safer side */
	Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);
	Ft_Gpu_Hal_Sleep(20);

	/* Access address 0 to wake up the FT800 */
	Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);
	Ft_Gpu_Hal_Sleep(20);

	/* Set the clk to external clock */
	Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
	Ft_Gpu_Hal_Sleep(10);


	/* Switch PLL output to 48MHz */
	Ft_Gpu_HostCommand(phost,FT_GPU_PLL_48M);
	Ft_Gpu_Hal_Sleep(10);

	/* Do a core reset for safer side */
	Ft_Gpu_HostCommand(phost,FT_GPU_CORE_RESET);
	Ft_Gpu_Hal_Sleep(10);

	if (0x7C == Ft_Gpu_Hal_Rd8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ID)))
	{
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HCYCLE), FT_DispHCycle);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HOFFSET), FT_DispHOffset);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSYNC0), FT_DispHSync0);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSYNC1), FT_DispHSync1);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VCYCLE), FT_DispVCycle);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VOFFSET), FT_DispVOffset);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSYNC0), FT_DispVSync0);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSYNC1), FT_DispVSync1);
		Ft_Gpu_Hal_Wr8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_SWIZZLE), FT_DispSwizzle);
		Ft_Gpu_Hal_Wr8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PCLK_POL), FT_DispPCLKPol);
		Ft_Gpu_Hal_Wr8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PCLK), FT_DispPCLK);//after this display is visible on the LCD
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE), FT_DispWidth);
		Ft_Gpu_Hal_Wr16(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE), FT_DispHeight);



		Ft_Gpu_Hal_Wr8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_GPIO_DIR),0x83 | Ft_Gpu_Hal_Rd8(phost,reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_GPIO_DIR)));
		Ft_Gpu_Hal_Wr8(phost, reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_GPIO),0x083 | Ft_Gpu_Hal_Rd8(phost,reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_GPIO)));


		Ft_Gpu_Hal_WrCmd32(phost, CMD_DLSTART);
		//Ft_Gpu_Hal_WrCmd32(phost,CLEAR_COLOR_RGB(31, 63, 127));
		Ft_Gpu_Hal_WrCmd32(phost,CLEAR(1,1,1));
		Ft_Gpu_Hal_WrCmd32(phost,DISPLAY());
		Ft_Gpu_Hal_WrCmd32(phost, CMD_SWAP);

		devInfo->Connected = true;
	}

	devInfo->handle = (void*)phost;
	updateSelection();
}

void DeviceManager::disconnectDevice()
{
	if (!m_DeviceList->currentItem()) return;

	DeviceInfo *devInfo = (DeviceInfo *)(void *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<quintptr>();

	printf("disconnectDevice\n");

	Ft_Gpu_Hal_Context_t *phost = (Ft_Gpu_Hal_Context_t*)devInfo->handle;

	Ft_Gpu_Hal_WrCmd32(phost, CMD_DLSTART);
	//Ft_Gpu_Hal_WrCmd32(phost,CLEAR_COLOR_RGB(0, 0, 0));
	Ft_Gpu_Hal_WrCmd32(phost,CLEAR(1,1,1));
	Ft_Gpu_Hal_WrCmd32(phost,DISPLAY());
	Ft_Gpu_Hal_WrCmd32(phost, CMD_SWAP);


	Ft_Gpu_Hal_Close(phost);

	delete (phost);

	devInfo->Connected = false;
	updateSelection();
}

static void loadContent2Device(ContentManager *contentManager, Ft_Gpu_Hal_Context_t *phost)
{
	contentManager->lockContent();
	std::set<ContentInfo *> contentInfo;
	QTreeWidget *contentList = (QTreeWidget*)contentManager->contentList();
	ft_uint8_t *ram = static_cast<ft_uint8_t *>(FT8XXEMU_getRam());

	for (QTreeWidgetItemIterator it(contentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		if (info->MemoryLoaded && info->CachedSize && (info->MemoryAddress + info->CachedSize <= addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END)))
		{
            {
				Ft_Gpu_Hal_WrMem(phost,addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)+info->MemoryAddress,&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)+info->MemoryAddress],info->CachedSize);
			}

			if (FTEDITOR_CURRENT_DEVICE < FTEDITOR_FT810) // FIXME_FT810
			{
				if (info->ImageFormat == PALETTED){
					const ft_uint32_t PALSIZE = 1024;
					Ft_Gpu_Hal_WrMem(phost,addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL),&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL)],PALSIZE);
				}
			}
		}
	}
	contentManager->unlockContent();
}

void DeviceManager::syncDevice()
{
	if (!m_DeviceList->currentItem()) return;

	ft_uint8_t *ram = static_cast<ft_uint8_t *>(FT8XXEMU_getRam());
	const uint32_t *displayList = FT8XXEMU_getDisplayList();

	//Sync with selected device
	{
		DeviceInfo *devInfo = (DeviceInfo *)(void *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<quintptr>();
		if (devInfo->Connected)
		{
			Ft_Gpu_Hal_Context_t *phost = (Ft_Gpu_Hal_Context_t *)devInfo->handle;

			loadContent2Device(m_MainWindow->contentManager(),phost);

			Ft_Gpu_Hal_StartTransfer(phost, FT_GPU_WRITE, addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_DL));
			Ft_Gpu_Hal_Transfer32(phost,CLEAR(1,1,1));
			for (int i = 0;i< displayListSize(FTEDITOR_CURRENT_DEVICE);i++){
				Ft_Gpu_Hal_Transfer32(phost, displayList[i]);
				if (displayList[i] == DISPLAY())
					break;
			}
			Ft_Gpu_Hal_EndTransfer(phost);

			Ft_Gpu_Hal_Wr32(phost,reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_DLSWAP),DLSWAP_FRAME);
			Ft_Gpu_Hal_Wr32(phost,reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE),*(ft_uint32_t*)&ram[reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE)]);
			Ft_Gpu_Hal_Wr32(phost,reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE),*(ft_uint32_t*)&ram[reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE)]);
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
		DeviceInfo *devInfo = (DeviceInfo *)(void *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<quintptr>();
		m_ConnectButton->setVisible(!devInfo->Connected);
		m_DisconnectButton->setVisible(devInfo->Connected);
		m_SendImageButton->setVisible(devInfo->Connected);
		if (devInfo->Connected)
		{
			uint icon[2] = { 0x2714, 0 };
			devInfo->View->setText(0, QString::fromUcs4(icon));
		}
		else
		{
			devInfo->View->setText(0, "");
		}
	}
}

#endif /* FT800_DEVICE_MANAGER */

} /* namespace FTEDITOR */

/* end of file */
