/**
 * device_manager.cpp
 * $Id$
 * \file device_manager.cpp
 * \brief device_manager.cpp
 * \date 2014-01-27 18:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
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
#include <vc.h>
#include <ft800emu_memory.h>

// Project includes
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

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
	uint icon[2] = { 0x27F2, 0 };
	refreshButton->setText(QString::fromUcs4(icon));//tr("Refresh"));
	refreshButton->setToolTip(tr("Refresh the device list"));
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshDevices()));
	buttons->addWidget(refreshButton);
	refreshButton->setMaximumWidth(refreshButton->height());

	buttons->addStretch();

	m_SendImageButton = new QPushButton(this);
	m_SendImageButton->setText(tr("Send Image"));
	m_SendImageButton->setToolTip(tr("Sends the current memory and display list to the selected device"));
	m_SendImageButton->setVisible(false);
	connect(m_SendImageButton, SIGNAL(clicked()), this, SLOT(sendImage()));
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

	// Initial refresh of devices
	refreshDevices();
}

DeviceManager::~DeviceManager()
{

}

// TODO
void DeviceManager::refreshDevices()
{
	printf("Refresh devices\n");

	// Swap device list to local
	std::map<DeviceId, DeviceInfo *> deviceInfo;
	deviceInfo.swap(m_DeviceInfo);

	// Find all devices
	// TODO

	// For each device that is found
	// TODO
	{
		DeviceId devId = 123456; // TODO // Unique device identifier
		if (deviceInfo.find(devId) == deviceInfo.end())
		{
			// The device was not added yet, create the gui
			QString devName = "Dummy Device"; // TODO // Device name
			// Create the gui
			QTreeWidgetItem *view = new QTreeWidgetItem(m_DeviceList);
			view->setText(0, "");
			view->setText(1, devName);
			// Store this device
			DeviceInfo *devInfo = new DeviceInfo();
			devInfo->Id = devId;
			devInfo->View = view;
			devInfo->Connected = false;
			m_DeviceInfo[devId] = devInfo;
			view->setData(0, Qt::UserRole, qVariantFromValue((void *)devInfo));
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

	DeviceInfo *devInfo = (DeviceInfo *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<void *>();

	// Connect and initialize device
	// TODO

	devInfo->Connected = true;
	updateSelection();
}

void DeviceManager::disconnectDevice()
{
	if (!m_DeviceList->currentItem()) return;

	printf("disconnectDevice\n");

	DeviceInfo *devInfo = (DeviceInfo *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<void *>();

	// Disconnect device
	// TODO

	devInfo->Connected = false;
	updateSelection();
}

void DeviceManager::sendImage()
{
	if (!m_DeviceList->currentItem()) return;

	printf("sendImage\n");

	DeviceInfo *devInfo = (DeviceInfo *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<void *>();
	const uint8_t *ram = FT800EMU::Memory.getRam();
	const uint32_t *displayList = FT800EMU::Memory.getDisplayList();

	// Set height, width and macro registers
	// Copy global memory
	// Send display list
	// Swap frame
	// TODO
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
		DeviceInfo *devInfo = (DeviceInfo *)m_DeviceList->currentItem()->data(0, Qt::UserRole).value<void *>();
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

} /* namespace FT800EMUQT */

/* end of file */
