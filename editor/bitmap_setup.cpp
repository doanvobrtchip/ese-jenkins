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
#include "properties_editor.h"
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

BitmapWidget::BitmapWidget(MainWindow *parent, int index) : QFrame(parent), m_MainWindow(parent), m_Index(index), m_PixmapOkay(false), m_ThreadRunning(false), m_ReloadRequested(false)
{
	QVBoxLayout *l = new QVBoxLayout();
	m_Label = new QLabel(this);
	l->addWidget(m_Label);
	setLayout(l);

	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *label = new QLabel(this);
	label->setText(QString("<i>") + QString::number(index) + "<i> ");
	layout->addWidget(label);
	label->setWordWrap(true);
	label->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	m_Label->setLayout(layout);

	/*QMargins cm = layout->contentsMargins();
	cm.setTop(cm.top() / 2);
	cm.setBottom(cm.bottom() / 2);
	cm.setLeft(cm.left() / 2);
	cm.setRight(cm.right() / 2);
	layout->setContentsMargins(cm);*/

	QMargins cm = layout->contentsMargins();
	label->setMargin(cm.left() / 4);
	layout->setContentsMargins(0, 0, 0, 0);
	l->setContentsMargins(0, 0, 0, 0);

	m_DefaultPalette = palette();
	m_SelectedPalette = m_DefaultPalette;
	m_SelectedPalette.setColor(QPalette::WindowText, Qt::red);

	deselect();

	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);

	effect->setBlurRadius(0);
	effect->setColor(palette().color(QPalette::Background));
	effect->setOffset(1,1);
	label->setGraphicsEffect(effect);

	m_ReloadThread = new BitmapWidgetThread();
	m_ReloadThread->m_BitmapWidget = this;
	connect(m_ReloadThread, SIGNAL(finished()), this, SLOT(threadFinished()));

	QSizePolicy policy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	setSizePolicy(policy);
}

BitmapWidget::~BitmapWidget()
{

}

void BitmapWidget::mousePressEvent(QMouseEvent *event)
{
	m_MainWindow->bitmapSetup()->select(m_Index);
}

void BitmapWidget::resizeEvent(QResizeEvent *event)
{
	QFrame::resizeEvent(event);

	if (m_PixmapOkay)
	{
		// Rescale locally
		/*m_Pixmap = m_Pixmap.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		m_Label->setPixmap(m_Pixmap);
		m_Label->repaint();*/
	}

	// Reload from file
	reloadInternal();
}

void BitmapWidget::select()
{
	setPalette(m_SelectedPalette);
	setFrameStyle(QFrame::Panel | QFrame::Plain);
}

void BitmapWidget::deselect()
{
	setPalette(m_DefaultPalette);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

void BitmapWidget::setImage(const QString &name)
{
	m_ImageName = name;
	reloadInternal();
}

void BitmapWidget::reloadInternal()
{
	// printf("BitmapWidget::reloadInternal()\n");

	m_Mutex.lock();
	m_ReloadRequested = true;
	m_ReloadWidth = m_Label->width();
	m_ReloadHeight = m_Label->height();
	m_ReloadName = m_ImageName;
	m_Mutex.unlock();
	if (!m_ThreadRunning && !m_ReloadName.isEmpty())
	{
		m_ThreadRunning = true;
		m_ReloadThread->start();
	}
}

void BitmapWidget::threadFinished()
{
	// printf("BitmapWidget::threadFinished()\n");

	m_ThreadRunning = false;
	if (m_ReloadSuccess)
	{
		m_Pixmap.convertFromImage(m_ReloadImage);
	}
	else
	{
		m_Pixmap = QPixmap();
	}
	m_Label->setPixmap(m_Pixmap);
	m_Label->repaint();
	m_PixmapOkay = m_ReloadSuccess;
	if (m_ReloadRequested)
	{
		m_ThreadRunning = true;
		m_ReloadThread->start();
	}
}

void BitmapWidgetThread::run()
{
	// printf("BitmapWidgetThread::run()\n");

	m_BitmapWidget->m_Mutex.lock();
	m_BitmapWidget->m_ReloadRequested = false;
	int width = m_BitmapWidget->m_ReloadWidth;
	int height = m_BitmapWidget->m_ReloadHeight;
	QString name = m_BitmapWidget->m_ReloadName;
	m_BitmapWidget->m_Mutex.unlock();

	QImage image;
	m_BitmapWidget->m_ReloadSuccess = image.load(name + "_converted.png");
	if (!m_BitmapWidget->m_ReloadSuccess) { printf("Bitmap widget reload failed\n"); return; }
	m_BitmapWidget->m_ReloadImage = image.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation).copy(0, 0, width, height);
}

BitmapSetup::BitmapSetup(MainWindow *parent) : QWidget(parent), m_MainWindow(parent), m_Selected(-1)
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

	connect(m_MainWindow->propertiesEditor(), SIGNAL(setterChanged(QWidget *)), this, SLOT(propertiesSetterChanged(QWidget *)));
}

BitmapSetup::~BitmapSetup()
{

}

void BitmapSetup::select(int i)
{
	printf("BitmapSetup::select(i)\n");

	if (m_Selected == i) return;

	deselect();
	m_Bitmaps[i]->select();

	m_Selected = i;

	// TEST
	// m_Bitmaps[i]->setImage("photo_redglasses_2");
}

void BitmapSetup::deselect()
{
	printf("BitmapSetup::deselect()\n");

	if (m_Selected == -1) return;

	for (int i = 0; i < 32; ++i)
		m_Bitmaps[i]->deselect();

	m_Selected = -1;
}

void BitmapSetup::propertiesSetterChanged(QWidget *setter)
{
	printf("BitmapSetup::propertiesSetterChanged(setter)\n");

	if (setter != this)
	{
		deselect();
	}
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
