/*!
 * @file RamRegDockWidget.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamRegDockWidget.h"

#include <QBoxLayout>
#include <QDockWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QTimer>
#include <QTreeWidget>

#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamRegDockWidget::RamRegDockWidget(Inspector *parent)
    : RamReg(parent)
{
	setObjectName("RamReg");
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("RAM_REG");
	setFloating(true);
	QWidget *w = new QWidget(this);
	w->setLayout(setupComponents());
	setWidget(w);
	resize(500, 350);
	parent->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);
	QTimer::singleShot(0, this, [this]() { activateWindow(); });
}

QBoxLayout *RamRegDockWidget::setupComponents()
{
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	QGroupBox *groupBox = new QGroupBox(this);
	QVBoxLayout *vBoxLayout = new QVBoxLayout();

	vBoxLayout->addWidget(m_Registers);
	groupBox->setLayout(vBoxLayout);

	initDisplayReg();

	layout->addWidget(groupBox);
	return layout;
}
} // namespace FTEDITOR
