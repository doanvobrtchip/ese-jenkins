/**
 * bitmap_setup.cpp
 * $Id$
 * \file bitmap_setup.cpp
 * \brief bitmap_setup.cpp
 * \date 2014-02-25 13:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "bitmap_setup.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

BitmapWidget::BitmapWidget(MainWindow *parent, int index) : QFrame(parent), m_MainWindow(parent), m_Index(index)
{
	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *label = new QLabel(this);
	label->setText(/*QString("<i>") +*/  QString::number(index) + /*"<i>*/" ");
	layout->addWidget(label);
	label->setWordWrap(true);
	label->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	setLayout(layout);

	/*QMargins cm = layout->contentsMargins();
	cm.setTop(cm.top() / 2);
	cm.setBottom(cm.bottom() / 2);
	cm.setLeft(cm.left() / 2);
	cm.setRight(cm.right() / 2);
	layout->setContentsMargins(cm);*/

	QMargins cm = layout->contentsMargins();
	label->setMargin(cm.left() / 4);
	layout->setContentsMargins(0, 0, 0, 0);

	setFrameStyle(QFrame::Panel | QFrame::Sunken);

	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);

	effect->setBlurRadius(0);
	effect->setColor(palette().color(QPalette::Background));
	effect->setOffset(1,1);
	label->setGraphicsEffect(effect);
}

BitmapWidget::~BitmapWidget()
{


}

BitmapSetup::BitmapSetup(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
{
	QGridLayout *layout = new QGridLayout();

	for (int i = 0; i < 32; ++i)
	{
		m_Bitmaps[i] = new BitmapWidget(parent, i);
		layout->addWidget(m_Bitmaps[i], i / 4, i % 4);
	}

	layout->setRowStretch(32 / 4, 1);

	setLayout(layout);

	/*QMargins cm = layout->contentsMargins();
	cm.setTop(cm.top() / 2);
	cm.setBottom(cm.bottom() / 2);
	cm.setLeft(cm.left() / 2);
	cm.setRight(cm.right() / 2);
	layout->setContentsMargins(cm);*/
}

BitmapSetup::~BitmapSetup()
{

}

void BitmapSetup::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	int width = m_Bitmaps[0]->width();

	for (int i = 0; i < 32 / 4; ++i)
	{
		((QGridLayout *)layout())->setRowMinimumHeight(i, width);
	}
}

} /* namespace FT800EMUQT */

/* end of file */
