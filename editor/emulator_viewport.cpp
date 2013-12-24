/**
 * emulator_viewport.cpp
 * $Id$
 * \file emulator_viewport.cpp
 * \brief emulator_viewport.cpp
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
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
#include <ft800emu_graphics_driver.h>

// Project includes
// #include "emulator_config.h"

namespace FT800EMUQT {

static FT800EMU::EmulatorParameters s_EmulatorParameters;
EmulatorThread *s_EmulatorThread;
static QImage *s_Image = NULL;
static QPixmap *s_Pixmap = NULL;
static QMutex s_Mutex;
static EmulatorViewport *s_EmulatorViewport = NULL;

bool ftqtGraphics(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize)
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
	: QWidget(parent)
{
	s_EmulatorViewport = this;
	s_Image = new QImage(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT, QImage::Format_RGB32);
	s_Pixmap = new QPixmap(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT);

	setMinimumWidth(FT800EMU_WINDOW_WIDTH_DEFAULT);
	setMinimumHeight(FT800EMU_WINDOW_HEIGHT_DEFAULT);
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

void EmulatorViewport::run(const FT800EMU::EmulatorParameters &params)
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
	FT800EMU::Emulator.stop();
}

void EmulatorViewport::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, *s_Pixmap);
}

void EmulatorViewport::threadRepaint()
{
	s_Mutex.lock();
	graphics(s_Image);
	if (s_Image->width() != s_Pixmap->width()
		|| s_Image->height() != s_Image->height())
	{
		QPixmap *pixmap = s_Pixmap;
		s_Pixmap = new QPixmap(s_Image->width(), s_Image->height());
		setMinimumWidth(s_Pixmap->width());
		setMinimumHeight(s_Image->height());
		delete pixmap;
	}
	s_Pixmap->convertFromImage(*s_Image);
	s_Mutex.unlock();
	repaint();
	frame();
}

} /* namespace FT800EMUQT */

/* end of file */
