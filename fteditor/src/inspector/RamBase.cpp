/*!
 * @file RamBase.cpp
 * @date 3/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamBase.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamBase::RamBase(Inspector *parent)
    : QDockWidget(parent->mainWindow())
    , m_Inspector(parent)
{
	setVisible(false);
	setFloating(true);
	setFocusPolicy(Qt::StrongFocus);

	m_OpenDlgBtn = new QPushButton;
	m_OpenDlgBtn->setContentsMargins(0, 0, 0, 0);
	m_OpenDlgBtn->setIconSize(QSize(10, 10));
	m_OpenDlgBtn->setIcon(QIcon(":/icons/external-link.png"));

	m_DockBackBtn = new QPushButton;
	m_DockBackBtn->setContentsMargins(0, 0, 0, 0);
	m_DockBackBtn->setIconSize(QSize(10, 10));
	m_DockBackBtn->setIcon(QIcon(":/icons/dock-back.png"));
	m_DockBackBtn->setVisible(false);

	m_TitleLabel = new QLabel;
	m_Widget = new QWidget(this);
	m_Inspector->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);

	m_TitleLayout = new QHBoxLayout;
	m_TitleLayout->setAlignment(Qt::AlignLeft);
	m_TitleLayout->addWidget(m_TitleLabel);
	m_TitleLayout->addWidget(m_OpenDlgBtn);
	m_TitleLayout->addWidget(m_DockBackBtn);
	
	connect(this, &QDockWidget::visibilityChanged, m_TitleLabel,
	    [this](bool visible) {
		    m_OpenDlgBtn->setVisible(!visible);
		    m_DockBackBtn->setVisible(visible);
		    m_TitleLabel->setVisible(!visible);
	    });
	connect(m_OpenDlgBtn, &QPushButton::clicked, this, &RamBase::openDialog);
	connect(m_DockBackBtn, &QPushButton::clicked, this, &RamBase::dockBack);
}

QWidget *RamBase::Widget() const { return m_Widget; }

void RamBase::openDialog(bool checked)
{
	setWidget(m_Widget);
	setVisible(true);
	activateWindow();
}

void RamBase::dockBack(bool checked)
{
	m_Inspector->addSplitter(m_Widget);
	close();
}

void RamBase::closeEvent(QCloseEvent *event) { dockBack(); }
} // namespace FTEDITOR
