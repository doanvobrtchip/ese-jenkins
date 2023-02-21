/*!
 * @file RamRegInspector.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamRegInspector.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "RamRegDockWidget.h"
#include "inspector.h"

namespace FTEDITOR {

RamRegInspector::RamRegInspector(Inspector *parent)
    : RamReg(parent)
{
	setVisible(false);
}

QBoxLayout *RamRegInspector::setupComponents()
{
	m_OpenDlgBtn = new QPushButton;
	m_OpenDlgBtn->setContentsMargins(0, 0, 0, 0);
	m_OpenDlgBtn->setIconSize(QSize(10, 10));
	m_OpenDlgBtn->setIcon(QIcon(":/icons/external-link.png"));
	m_OpenDlgBtn->setStatusTip(tr("Show/Hide the RAM_REG Window"));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	QGroupBox *groupBox = new QGroupBox(this);
	QVBoxLayout *vBoxLayout = new QVBoxLayout();
	vBoxLayout->setContentsMargins(5, 5, 5, 5);

	auto lb = new QLabel;
	lb->setText("RAM_REG");

	auto titleLayout = new QHBoxLayout;
	titleLayout->setAlignment(Qt::AlignLeft);
	titleLayout->addWidget(lb);
	titleLayout->addWidget(m_OpenDlgBtn);
	vBoxLayout->addLayout(titleLayout);

	vBoxLayout->addWidget(m_Registers);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);

	connect(m_OpenDlgBtn, &QPushButton::clicked, this, [this](bool checked) {
		auto dock = m_Inspector->ramRegDockWidget();
		if (dock == NULL)
		{
			m_Inspector->initRamRegDockWidget();
		}
		if (!dock) return;
		dock->toggleViewAction();
		dock->raise();
		dock->activateWindow();
	});
	return layout;
}

} // namespace FTEDITOR
