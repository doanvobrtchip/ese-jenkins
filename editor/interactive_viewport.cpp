/**
 * interactive_viewport.cpp
 * $Id$
 * \file interactive_viewport.cpp
 * \brief interactive_viewport.cpp
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */
 
#include "interactive_viewport.h"

// STL includes

// Qt includes
#include <QLabel>
#include <QMouseEvent>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <ft800emu_memory.h>

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "code_editor.h"

namespace FT800EMUQT {

InteractiveViewport::InteractiveViewport(MainWindow *parent) 
	: EmulatorViewport(parent), m_MainWindow(parent),
	m_MouseTouch(false)
{
	// m_Label->setCursor(Qt::PointingHandCursor);
	setMouseTracking(true);
}

InteractiveViewport::~InteractiveViewport()
{
	
}

// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
void InteractiveViewport::graphics()
{
	//FT800EMU::GraphicsProcessor.getDebugTrace(m_TraceEnabled, m_TraceX, m_TraceY);
	m_TraceEnabled = m_MainWindow->traceEnabled();
	m_TraceX = m_MainWindow->traceX();
	m_TraceY = m_MainWindow->traceY();
	m_TraceStack.clear();
	if (m_TraceEnabled) FT800EMU::GraphicsProcessor.processTrace(m_TraceStack, m_TraceX, m_TraceY, hsize());
}

// Graphics callback synchronized to Qt thread, use to overlay graphics
void InteractiveViewport::graphics(QImage *image)
{
	// Update frame dependent gui
	m_MainWindow->dlEditor()->codeEditor()->setTraceHighlights(m_TraceStack);

	// Draw image overlays
}

int InteractiveViewport::getPointerMethod(int x, int y)
{
	return 0;
}

void InteractiveViewport::mouseMoveEvent(QMouseEvent *e)
{
	// printf("pos: %i, %i\n", e->pos().x(), e->pos().y());

	if (m_MouseTouch)
	{
		FT800EMU::Memory.setTouchScreenXY(e->pos().x(), e->pos().y(), 0);
	}
	else // no ongoing action
	{
		int method = 1; // auto = 0, touch = 1, trace = 2, edit = 3

		// if auto, select correct method depending on positiongetPointerMethod

		switch (method)
		{
		case 0: // none
			setCursor(Qt::ArrowCursor);
			break;
		case 1:
			setCursor(Qt::PointingHandCursor);
			break;
		case 2:
			setCursor(Qt::CrossCursor);
			break;
		case 3:
			setCursor(Qt::ArrowCursor); // todo
			break;
		}
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mousePressEvent(QMouseEvent *e)
{
	// printf("pos: %i, %i\n", e->pos().x(), e->pos().y());

	int method = 1; // none = 0, touch = 1

	// Touch mode
	switch (method)
	{
	case 1: // touch
		m_MouseTouch = true;
		FT800EMU::Memory.setTouchScreenXY(e->pos().x(), e->pos().y(), 0);
		break;
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mouseReleaseEvent(QMouseEvent *e)
{
	// printf("pos: %i, %i\n", e->pos().x(), e->pos().y());

	if (m_MouseTouch)
	{
		m_MouseTouch = false;
		FT800EMU::Memory.resetTouchScreenXY();
	}

	EmulatorViewport::mouseMoveEvent(e);
}

} /* namespace FT800EMUQT */

/* end of file */
