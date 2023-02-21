/*!
 * @file RamGDockWidget.cpp
 * @date 2/7/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamGDockWidget.h"

#include <QBoxLayout>
#include <QDialog>
#include <QDockWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "customize/QHexView.h"
#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamGDockWidget::RamGDockWidget(Inspector *parent)
    : RamG(parent)
{
	setObjectName("RamG");
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("RAM_G");
	setFloating(true);
	QWidget *w = new QWidget(this);
	w->setLayout(setupComponents());
	setWidget(w);
	resize(640, 377);
	m_Inspector->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);
	QTimer::singleShot(0, this, [this]() { activateWindow(); });
}

QBoxLayout *RamGDockWidget::setupComponents()
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
