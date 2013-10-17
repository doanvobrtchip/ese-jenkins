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
#include "main.h"
// #include "emulator_config.h"

extern EmulatorThread *g_EmulatorThread;
void ftqtRepaint();

namespace FTQT {
	
static QImage *s_Image = NULL;
static QPixmap *s_Pixmap = NULL;
bool ftqtGraphics(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize)
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
			QPixmap *pixmap = new QPixmap(hsize, vsize);
			s_Pixmap->swap(*pixmap);
			delete pixmap;
		}
		if (output)
		{
			// printf("Graphics received");
			// This is just terrible code.
			for (int y = 0; y < vsize; ++y)
				memcpy(s_Image->scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
			s_Pixmap->convertFromImage(*s_Image);
			ftqtRepaint();
		}
		else
		{
			// ..
		}
	}
	return true; // g_EmulatorThread != NULL;
}

EmulatorViewport::EmulatorViewport(QWidget *parent) 
	: QWidget(parent)
	// m_EmulatorConfig(NULL)
{
	m_Image = new QImage(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT, QImage::Format_RGB32);
	s_Image = m_Image;
	m_Pixmap = new QPixmap(FT800EMU_WINDOW_WIDTH_DEFAULT, FT800EMU_WINDOW_HEIGHT_DEFAULT);
	s_Pixmap = m_Pixmap;
	m_Label = new QLabel();
	m_Label->setPixmap(*m_Pixmap);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_Label);
	setLayout(layout);
	
	printf("Emu thread is %i\n", g_EmulatorThread);
	connect(g_EmulatorThread, SIGNAL(repaint()), m_Label, SLOT(repaint()));
}

EmulatorViewport::~EmulatorViewport()
{
	delete m_Label; m_Label = NULL;
	s_Pixmap = NULL;
	delete m_Pixmap; m_Pixmap = NULL;
	s_Image = NULL;
	delete m_Image; m_Image = NULL;
}

} /* namespace FTQT */

/* end of file */
