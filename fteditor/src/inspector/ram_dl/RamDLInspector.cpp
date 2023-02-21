/*!
 * @file RamDLInspector.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamDLInspector.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QtEndian>

#include "RamDLDockWidget.h"
#include "inspector.h"

namespace FTEDITOR {

RamDLInspector::RamDLInspector(Inspector *parent)
    : RamDL(parent)
{
	setVisible(false);
}

QBoxLayout *RamDLInspector::setupComponents()
{
	m_OpenDlgBtn = new QPushButton;
	m_OpenDlgBtn->setContentsMargins(0, 0, 0, 0);
	m_OpenDlgBtn->setIconSize(QSize(10, 10));
	m_OpenDlgBtn->setIcon(QIcon(":/icons/external-link.png"));
	m_OpenDlgBtn->setStatusTip(tr("Show/Hide the RAM_DL Window"));

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	auto groupBox = new QGroupBox(this);
	auto vBoxLayout = new QVBoxLayout;
	vBoxLayout->setContentsMargins(5, 5, 5, 5);

	auto lb = new QLabel;
	lb->setText("RAM_DL");

	auto titleLayout = new QHBoxLayout;
	titleLayout->setAlignment(Qt::AlignLeft);
	titleLayout->addWidget(lb);
	titleLayout->addWidget(m_OpenDlgBtn);
	vBoxLayout->addLayout(titleLayout);
	vBoxLayout->addWidget(m_DisplayList);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);

	connect(m_OpenDlgBtn, &QPushButton::clicked, this, [this](bool checked) {
		auto dock = m_Inspector->ramDLDockWidget();
		if (dock == NULL)
		{
			m_Inspector->initRamDLDockWidget();
		}
		if (!dock) return;
		dock->toggleViewAction();
		dock->raise();
		dock->activateWindow();
	});
	return layout;
}
} // namespace FTEDITOR
