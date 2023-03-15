/*!
 * @file RamCMDDockWidget.cpp
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamCMDDockWidget.h"

#include <QBoxLayout>
#include <QDockWidget>
#include <QGroupBox>
#include <QTimer>

#include "customize/QHexView.h"
#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamCMDDockWidget::RamCMDDockWidget(Inspector *parent)
    : RamCMD(parent)
{
	setObjectName("RamCMD");
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("RAM_CMD");
	setFloating(true);
	QWidget *w = new QWidget(this);
	w->setLayout(setupComponents());
	setWidget(w);
	resize(500, 300);
	parent->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);

	QTimer::singleShot(0, this, [this]() { activateWindow(); });
}
QBoxLayout *RamCMDDockWidget::setupComponents()
{
	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	auto groupBox = new QGroupBox(this);
	auto vBoxLayout = new QVBoxLayout;
	vBoxLayout->setContentsMargins(5, 5, 5, 5);
	vBoxLayout->addWidget(m_HexView);

	layout->setAlignment(Qt::AlignRight);
	vBoxLayout->addLayout(m_SearchLayout);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);

	return layout;
}
} // namespace FTEDITOR
