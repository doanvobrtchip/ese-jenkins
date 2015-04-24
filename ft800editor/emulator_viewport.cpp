/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "emulator_viewport.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QAction>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QMutex>
#include <QPaintEvent>
#include <QPainter>

// Emulator includes
#include <ft8xxemu_graphics_driver.h>

// Project includes
// #include "emulator_config.h"

namespace FT800EMUQT {

static FT8XXEMU_EmulatorParameters s_EmulatorParameters;
EmulatorThread *s_EmulatorThread;
static QImage *s_Image = NULL;
static QPixmap *s_Pixmap = NULL;
static QMutex s_Mutex;
static EmulatorViewport *s_EmulatorViewport = NULL;

int ftqtGraphics(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags) // on Emulator thread
{
	// TODO: Optimize using platform specific access to QImage so we
	// don't need to copy the buffer each time.
	if (s_Image && s_Pixmap)
	{
		s_Mutex.lock();
		if (hsize != s_Image->width() || vsize != s_Image->height())
		{
			// printf("Graphics resize");
			QImage *image = s_Image;
			s_Image = new QImage(hsize, vsize, QImage::Format_RGB32);
			delete image;
		}
		if (output)
		{
			// printf("Graphics received");
			// This is just terrible code.
			for (uint32_t y = 0; y < vsize; ++y)
				memcpy(s_Image->scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
		}
		else
		{
			// TODO: Blank
			// ..
		}
		s_EmulatorViewport->graphics();
		s_Mutex.unlock();
		s_EmulatorThread->repaint();
	}
	return true; // g_EmulatorThread != NULL;
}

void EmulatorThread::run()
{
	FT800EMU::Emulator.run(s_EmulatorParameters);
}

EmulatorViewport::EmulatorViewport(QWidget *parent)
	: QWidget(parent), m_ScreenScale(16)
{
	s_EmulatorViewport = this;
	s_Image = new QImage(FT8XXEMU_WINDOW_WIDTH_DEFAULT, FT8XXEMU_WINDOW_HEIGHT_DEFAULT, QImage::Format_RGB32);
	s_Pixmap = new QPixmap(FT8XXEMU_WINDOW_WIDTH_DEFAULT, FT8XXEMU_WINDOW_HEIGHT_DEFAULT);

	m_Vertical = new QScrollBar(Qt::Vertical, this);
	m_Horizontal = new QScrollBar(Qt::Horizontal, this);

	m_Vertical->setMinimum(-FT8XXEMU_WINDOW_HEIGHT_DEFAULT * 8);
	m_Vertical->setMaximum(FT8XXEMU_WINDOW_HEIGHT_DEFAULT * 8);
	m_Vertical->setValue(0);
	m_Horizontal->setMinimum(-FT8XXEMU_WINDOW_WIDTH_DEFAULT * 8);
	m_Horizontal->setMaximum(FT8XXEMU_WINDOW_WIDTH_DEFAULT * 8);
	m_Horizontal->setValue(0);

	setMinimumWidth(FT8XXEMU_WINDOW_WIDTH_DEFAULT);
	setMinimumHeight(FT8XXEMU_WINDOW_HEIGHT_DEFAULT);
}

EmulatorViewport::~EmulatorViewport()
{
	stop();
	delete s_Pixmap; s_Pixmap = NULL;
	delete s_Image; s_Image = NULL;
	s_EmulatorViewport = NULL;
}

int EmulatorViewport::hsize()
{
	return s_Pixmap->width();
}

int EmulatorViewport::vsize()
{
	return s_Pixmap->height();
}

void EmulatorViewport::run(const FT8XXEMU_EmulatorParameters &params)
{
	// There can be only one
	if (s_EmulatorThread == NULL)
	{
		// Copy the params for the new thread to use
		s_EmulatorParameters = params;

		// Add the graphics callback to the parameters
		s_EmulatorParameters.Graphics = ftqtGraphics;

		// Create the main thread for the emulator
		s_EmulatorThread = new EmulatorThread();
		s_EmulatorThread->start();

		// Connect the cross thread repaint event
		connect(s_EmulatorThread, SIGNAL(repaint()), this, SLOT(threadRepaint()));
	}
}

void EmulatorViewport::stop()
{
	if (s_EmulatorThread != NULL)
	{
		FT800EMU::Emulator.stop();

		printf("Wait for emulator threads\n");
		s_EmulatorThread->wait();
		delete s_EmulatorThread;
		s_EmulatorThread = NULL;
		printf("Emulator threads finished\n");
	}
}

void EmulatorViewport::paintEvent(QPaintEvent* e) // on Qt thread
{
	QPainter painter(this);
	painter.drawPixmap(screenLeft(), screenTop(),
		s_Pixmap->width() * screenScale() / 16, 
		s_Pixmap->height() * screenScale() / 16, 
		*s_Pixmap,
		0, 0, s_Pixmap->width(), s_Pixmap->height());
}

int EmulatorViewport::screenLeft() // offset in screenspace pixels
{
	int center = width() / 2;
	int centerFrame = s_Pixmap->width() * m_ScreenScale / 32;
	int offset = m_Horizontal->value() * m_ScreenScale / 256;
	return center - centerFrame - offset;
}

int EmulatorViewport::screenTop()
{
	int center = height() / 2;
	int centerFrame = s_Pixmap->height() * m_ScreenScale / 32;
	int offset = m_Vertical->value() * m_ScreenScale / 256;
	return center - centerFrame - offset;
}

int EmulatorViewport::screenBottom()
{
	return screenTop() + (s_Pixmap->height() * m_ScreenScale / 16);
}

int EmulatorViewport::screenRight()
{
	return screenLeft() + (s_Pixmap->width() * m_ScreenScale / 16);
}

int EmulatorViewport::screenScale()
{
	return m_ScreenScale;
}

void EmulatorViewport::setScreenScale(int screenScale)
{
	if (screenScale < 1) m_ScreenScale = 1;
	else if (screenScale > 256) m_ScreenScale = 256;
	else m_ScreenScale = screenScale;
}

void EmulatorViewport::threadRepaint() // on Qt thread
{
	s_Mutex.lock();
	graphics(s_Image);
	if (s_Image->width() != s_Pixmap->width()
		|| s_Image->height() != s_Pixmap->height())
	{
		QPixmap *pixmap = s_Pixmap;
		s_Pixmap = new QPixmap(s_Image->width(), s_Image->height());
		/*setMinimumWidth(s_Pixmap->width());
		setMinimumHeight(s_Image->height());*/
		m_Vertical->setMinimum(-s_Pixmap->height() * 8);
		m_Vertical->setMaximum(s_Pixmap->height() * 8);
		m_Horizontal->setMinimum(-s_Pixmap->width() * 8);
		m_Horizontal->setMaximum(s_Pixmap->width() * 8);
		delete pixmap;
	}
	s_Pixmap->convertFromImage(*s_Image);
	s_Mutex.unlock();
	repaint();
	frame();
}

const QPixmap &EmulatorViewport::getPixMap() const
{
	return *s_Pixmap;
}

} /* namespace FT800EMUQT */

/* end of file */
