/*!
 * @file RamGInspector.cpp
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamGInspector.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "RamGDockWidget.h"
#include "customize/QHexView.h"
#include "inspector.h"

namespace FTEDITOR {

RamGInspector::RamGInspector(Inspector *parent)
    : RamG(parent)
{
	setVisible(false);
}

QBoxLayout *RamGInspector::setupComponents()
{
	m_OpenDlgBtn = new QPushButton;
	m_OpenDlgBtn->setStatusTip(tr("Show/Hide the RAM_G Window"));
	m_OpenDlgBtn->setContentsMargins(0, 0, 0, 0);
	m_OpenDlgBtn->setIconSize(QSize(10, 10));
	m_OpenDlgBtn->setIcon(QIcon(":/icons/external-link.png"));

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	auto groupBox = new QGroupBox(this);
	auto vBoxLayout = new QVBoxLayout;
	vBoxLayout->setContentsMargins(5, 5, 5, 5);

	auto lb = new QLabel;
	lb->setText("RAM_G");

	auto titleLayout = new QHBoxLayout;
	titleLayout->setAlignment(Qt::AlignLeft);
	titleLayout->addWidget(lb);
	titleLayout->addWidget(m_OpenDlgBtn);
	vBoxLayout->addLayout(titleLayout);

	vBoxLayout->addWidget(m_HexView);

	layout->setAlignment(Qt::AlignRight);
	vBoxLayout->addLayout(m_SearchLayout);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);

	connect(m_OpenDlgBtn, &QPushButton::clicked, this, [this](bool checked) {
		auto dock = m_Inspector->ramGDockWidget();
		if (dock == NULL)
		{
			m_Inspector->initRamGDockWidget();
		}
		if (!dock) return;
		dock->toggleViewAction();
		dock->raise();
		dock->activateWindow();
	});
	return layout;
}
} // namespace FTEDITOR
