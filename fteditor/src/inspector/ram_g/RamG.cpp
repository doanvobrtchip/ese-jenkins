/*!
 * @file RamG.cpp
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamG.h"

// Emulator includes
#include <bt8xxemu_diag.h>

#include <QDockWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include "content_manager.h"
#include "customize/QHexView.h"
#include "inspector.h"
#include "main_window.h"
#include "utils/LoggerUtil.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamG::RamG(Inspector *parent)
    : QDockWidget(parent->mainWindow())
    , m_Inspector(parent)
{
	m_HexView = new QHexView;
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

	connect(m_SearchButton, &QPushButton::clicked, this, &RamG::goToAddress);
	connect(m_AddressJumpEdit, &QLineEdit::returnPressed, this,
	    &RamG::goToAddress);
	connect(m_HexView, &QHexView::uintChanged, this, &RamG::setLabelUint);

	connect(m_Inspector, &Inspector::updateView, this, &RamG::updateView);
	setupConnections();

	connect(m_Inspector->mainWindow(), SIGNAL(readyToSetup(QObject *)), this,
	    SLOT(setupConnections(QObject *)));
	emit m_Inspector->mainWindow()->readyToSetup(this);
}

void RamG::setupConnections(QObject *obj)
{
	if (auto contentMgr = m_Inspector->mainWindow()->contentManager();
	    contentMgr && (obj == contentMgr || obj == nullptr))
	{
		auto currContentList = contentMgr->contentInfoList();
		for (auto &info : currContentList)
		{
			addContentItem(info);
		}

		auto contentList = contentMgr->contentList();
		connect(contentList, &QTreeWidget::currentItemChanged, this,
		    &RamG::currItemContentChanged, Qt::UniqueConnection);
		connect(contentList, &QTreeWidget::itemPressed, this,
		    &RamG::handleContentItemPressed, Qt::UniqueConnection);
		connect(contentList, &ContentTreeWidget::removeItem, this,
		    &RamG::removeContentItem, Qt::UniqueConnection);
		connect(contentList, &ContentTreeWidget::addItem, this,
		    &RamG::addContentItem, Qt::UniqueConnection);

		connect(contentMgr, &ContentManager::ramGlobalUsageChanged, this,
		    &RamG::updateView, Qt::UniqueConnection);
		connect(m_HexView, &QHexView::currentInfoChanged, this,
		    &RamG::handleCurrentInfoChanged, Qt::UniqueConnection);
	}

	if (auto inspDock = m_Inspector->mainWindow()->inspectorDock();
	    inspDock && (obj == m_Inspector->mainWindow()->inspectorDock() || obj == nullptr))
	{
		connect(inspDock, &QDockWidget::visibilityChanged, this, &RamG::bindVisible,
		    Qt::UniqueConnection);
	}
}

void RamG::currItemContentChanged(QTreeWidgetItem *current,
    QTreeWidgetItem *previous)
{
	if (!current)
	{
		debugLog("No selected content");
		m_HexView->setSelected(0, 0);
		return;
	};
}

void RamG::handleContentItemPressed(QTreeWidgetItem *item)
{
	if (!item)
	{
		m_HexView->setSelected(0, 0);
		return;
	};
	auto contentInfo = (ContentInfo *)(void *)item->data(0, Qt::UserRole).value<quintptr>();

	auto size = m_Inspector->mainWindow()->contentManager()->getContentSize(contentInfo);
	if (!contentInfo->MemoryLoaded || size <= 0 || !contentInfo->BuildError.isEmpty())
	{
		debugLog("Selected a non-memory content");
		m_HexView->setSelected(0, 0);
		return;
	}

	debugLog(QString("Selected a memory content | Addr: %1 | Size: %2 | Name: %3")
	             .arg(contentInfo->MemoryAddress)
	             .arg(size)
	             .arg(contentInfo->DestName));
	m_HexView->showFromOffset(contentInfo->MemoryAddress);
	m_HexView->setSelected(contentInfo->MemoryAddress, size);
	if (m_Visible)
	{
		m_HexView->setFocus(Qt::MouseFocusReason);
	}
}

void RamG::handleCurrentInfoChanged(ContentInfo *contentInfo)
{
	emit updateCurrentInfo(contentInfo);
}

void RamG::removeContentItem(ContentInfo *contentInfo)
{
	auto size = m_Inspector->mainWindow()->contentManager()->getContentSize(contentInfo);
	if (contentInfo->DataStorage == ContentInfo::Flash || size < 0)
	{
		debugLog("Remove a non-memory content");
		return;
	}
	debugLog("Remove a memory content");
	m_HexView->removeContentArea(contentInfo);
}

void RamG::bindVisible(bool visible) { m_Visible = visible; }

void RamG::addContentItem(ContentInfo *contentInfo)
{
	m_HexView->addContentArea(contentInfo);
}

void RamG::updateView()
{
	int ramUsage = -1;
	auto ramG = BT8XXEMU_getRam(g_Emulator);
	int ramSize = g_Addr[FTEDITOR_CURRENT_DEVICE][FTEDITOR_RAM_G_END];
	QByteArray byteArr;
	auto temp = static_cast<char *>(static_cast<void *>(ramG));
	byteArr.append(temp, ramSize);
	auto data = m_HexView->data();
	if (data && byteArr == data->getData(0, data->size()) && ramUsage == -1)
	{
		return;
	}
	debugLog("RAM_G - Content has a change");
	m_HexView->setData(new QHexView::DataStorageArray(byteArr));
	m_HexView->viewport()->update();
	handleContentItemPressed(m_Inspector->mainWindow()
	                             ->contentManager()
	                             ->contentList()
	                             ->currentItem());
}

void RamG::goToAddress()
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
		m_HexView->showFromOffset(address);
		m_HexView->setSelected(address, 1);
		m_HexView->updateUint();
		m_HexView->setFocus();
	}
}

void RamG::setLabelUint(uint value)
{
	QString text = QString(tr("Uint: %1")).arg(value);
	m_UintLabel->setText(text);
}

} // namespace FTEDITOR
