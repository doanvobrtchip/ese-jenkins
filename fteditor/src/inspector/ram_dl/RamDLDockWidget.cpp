/*!
 * @file RamDLDockWidget.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamDLDockWidget.h"

#include <QBoxLayout>
#include <QDialog>
#include <QDockWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QTreeWidget>

#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamDLDockWidget::RamDLDockWidget(Inspector *parent)
    : RamDL(parent)
{
	setObjectName("RamDL");
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("RAM_DL");
	setFloating(true);
	QWidget *w = new QWidget(this);
	w->setLayout(setupComponents());
	setWidget(w);
	resize(500, 300);
	parent->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);

	QTimer::singleShot(0, this, [this]() { activateWindow(); });
}

QBoxLayout *RamDLDockWidget::setupComponents()
{
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	QGroupBox *groupBox = new QGroupBox(this);
	QVBoxLayout *vBoxLayout = new QVBoxLayout();
	vBoxLayout->setContentsMargins(5, 5, 5, 5);

	vBoxLayout->addWidget(m_DisplayList);
	groupBox->setLayout(vBoxLayout);
	layout->addWidget(groupBox);
	return layout;
}
} // namespace FTEDITOR
