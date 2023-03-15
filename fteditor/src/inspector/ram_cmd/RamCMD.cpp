/*!
 * @file RamCMD.cpp
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamCMD.h"

// Emulator includes
#include <bt8xxemu_diag.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "constant_mapping.h"
#include "customize/QHexView.h"
#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamCMD::RamCMD(Inspector *parent)
    : QDockWidget(parent->mainWindow())
    , m_Inspector(parent)
{
	m_HexView = new QHexView;
	m_HexView->setUseContentArea(false);
	m_HexView->setSelected(0, 0);

	m_UintLabel = new QLabel;
	m_UintLabel->setText(tr("Uint:"));

	// Set default width for QLineEdit
	m_AddressJumpEdit = new QLineEdit;
	m_AddressJumpEdit->setFocus();
	QString placeholderText = tr("Enter decimal or hexadecimal");
	auto textSize = m_AddressJumpEdit->fontMetrics().size(0, placeholderText);
	m_AddressJumpEdit->setPlaceholderText(placeholderText);
	m_AddressJumpEdit->setMaximumWidth(textSize.width() + 10);

	m_AddressLabel = new QLabel;
	m_AddressLabel->setText(tr("Address:"));
	m_AddressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	m_SearchButton = new QPushButton;
	m_SearchButton->setDefault(true);
	m_SearchButton->setMaximumWidth(90);
	m_SearchButton->setText(tr("Jump"));

	m_SearchLayout = new QHBoxLayout;
	m_SearchLayout->setAlignment(Qt::AlignRight);
	m_SearchLayout->addWidget(m_UintLabel);
	m_SearchLayout->addWidget(m_AddressLabel);
	m_SearchLayout->addWidget(m_AddressJumpEdit);
	m_SearchLayout->addWidget(m_SearchButton);

	connect(m_SearchButton, &QPushButton::clicked, this, &RamCMD::goToAddress);
	connect(m_AddressJumpEdit, &QLineEdit::returnPressed, this,
	    &RamCMD::goToAddress);
	connect(m_HexView, &QHexView::uintChanged, this, &RamCMD::setLabelUint);

	connect(m_Inspector, &Inspector::updateView, this, &RamCMD::updateView);
	//	setupConnections();

	emit m_Inspector->mainWindow()->readyToSetup(this);
}

void RamCMD::setupConnections(QObject *obj) { }

void RamCMD::goToAddress()
{
	bool ok;
	QString text = m_AddressJumpEdit->text();
	int address = text.toUInt(&ok, 10);
	if (!ok)
	{
		address = text.toUInt(&ok, 16);
	}
	if (ok)
	{
		if (!m_HexView->showFromOffset(address)) return;
		m_HexView->setSelected(address, 1);
		m_HexView->updateUint();
		m_HexView->setFocus();
	}
}

void RamCMD::setLabelUint(uint value)
{
	QString text = QString(tr("Uint: %1")).arg(value);
	m_UintLabel->setText(text);
}

void RamCMD::updateView()
{
	int startAddr = g_Addr[FTEDITOR_CURRENT_DEVICE][FTEDITOR_RAM_CMD];
	auto ram = BT8XXEMU_getRam(g_Emulator);
	uint8_t *ramCMD = ram + startAddr;
	QByteArray byteArr;
	auto ramCMDChar = static_cast<char *>(static_cast<void *>(ramCMD));
	byteArr.append(ramCMDChar, 4096); // 4KB
	auto data = m_HexView->data();
	if (data && byteArr == data->getData(0, data->size()))
	{
		return;
	}
	m_HexView->setStartAddress(startAddr);
	m_HexView->setData(new QHexView::DataStorageArray(byteArr));
	m_HexView->viewport()->update();
}

} // namespace FTEDITOR
