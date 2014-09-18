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

// Emulator includes
#include <ft800emu_graphics_driver.h>

// Project includes
// #include "emulator_config.h"

namespace FT800EMUQT {

static FT800EMU::EmulatorParameters s_EmulatorParameters;
EmulatorThread *s_EmulatorThread;
static QImage *s_Image = NULL;
static QPixmap *s_Pixmap = NULL;

bool ftqtGraphics(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT800EMU::FrameFlags flags)
{
	// TODO: Optimize using platform specific access to QImage so we 
	// don't need to copy the buffer each time.
	if (s_Image && s_Pixmap)
	{
		if (hsize != s_Image->width() || vsize != s_Image->height())
		{
			// printf("Graphics resize");
			QImage *image = new QImage(hsize, vsize, QImage::Format_RGB32);
			s_Image->swap(*image);
			delete image;
			QPixmap *pixmap = new QPixmap(hsize, vsize); // Probably not safe with the threading
			s_Pixmap->swap(*pixmap);
			delete pixmap;
		}
		if (output)
		{
			// printf("Graphics received");
			// This is just terrible code.
			for (int y = 0; y < vsize; ++y)
				memcpy(s_Image->scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
			s_Pixmap->convertFromImage(*s_Image); // Probably not safe with the threading
			s_EmulatorThread->repaint();
		}
		else
		{
			// TODO: Blank
			// ..
		}
	}
	return true; // g_EmulatorThread != NULL;
}

void EmulatorThread::run()
{
	FT800EMU::Emulator.run(s_EmulatorParameters);
}

EmulatorViewport::EmulatorViewport(QWidget *parent) 
	: QWidget(parent)
	// m_EmulatorConfig(NULL)
{
	m_Image = new QImage(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT, QImage::Format_RGB32);
	m_Pixmap = new QPixmap(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT);
	m_Label = new QLabel();
	m_Label->setPixmap(*m_Pixmap);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_Label);
	setLayout(layout);
}

EmulatorViewport::~EmulatorViewport()
{
	delete m_Label; m_Label = NULL;
	if (m_Pixmap == s_Pixmap) s_Pixmap = NULL;
	delete m_Pixmap; m_Pixmap = NULL;
	if (m_Image == s_Image) s_Image = NULL;
	delete m_Image; m_Image = NULL;
	// TODO: End the emulator thread in a clean way
}

void EmulatorViewport::run(const FT800EMU::EmulatorParameters &params)
{
	// There can be only one
	if (s_EmulatorThread == NULL)
	{
		// Copy pointers to Qt buffers
		s_Pixmap = m_Pixmap;
		s_Image = m_Image;
		
		// Copy the params for the new thread to use
		s_EmulatorParameters = params;
		
		// Add the graphics callback to the parameters
		s_EmulatorParameters.Graphics = ftqtGraphics;
		
		// Create the main thread for the emulator
		s_EmulatorThread = new EmulatorThread();
		s_EmulatorThread->start();
		
		// Connect the cross thread repaint event
		connect(s_EmulatorThread, SIGNAL(repaint()), m_Label, SLOT(repaint()));
	}
}

} /* namespace FT800EMUQT */

/* end of file */
