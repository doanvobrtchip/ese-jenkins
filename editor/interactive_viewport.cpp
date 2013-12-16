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
#include <QAction>
#include <QToolBar>

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
	m_TraceEnabled(false), m_MouseOver(false), m_MouseTouch(false), 
	m_PointerMode(0), m_PointerMethod(0)
{
	// m_Label->setCursor(Qt::PointingHandCursor);
	setMouseTracking(true);
	
	QActionGroup *cursorGroup = new QActionGroup(this);
	
	QAction *automatic = new QAction(cursorGroup);
	connect(automatic, SIGNAL(triggered()), this, SLOT(automaticChecked()));
	automatic->setText(tr("Auto"));
	automatic->setStatusTip(tr("Context dependent cursor"));
	automatic->setCheckable(true);
	automatic->setChecked(true);
	
	QAction *touch = new QAction(cursorGroup);
	connect(touch, SIGNAL(triggered()), this, SLOT(touchChecked()));
	touch->setText(tr("Touch"));
	touch->setStatusTip(tr("Use to cursor to touch the emulated display"));
	touch->setCheckable(true);
	
	QAction *trace = new QAction(cursorGroup);
	connect(trace, SIGNAL(triggered()), this, SLOT(traceChecked()));
	trace->setText(tr("Trace"));
	trace->setStatusTip(tr("Select a pixel to trace display commands"));
	trace->setCheckable(true);
	
	QToolBar *toolBar = m_MainWindow->addToolBar(tr("Cursor"));
	toolBar->addAction(automatic);
	toolBar->addAction(touch);
	toolBar->addAction(trace);
}

InteractiveViewport::~InteractiveViewport()
{
	
}

// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
void InteractiveViewport::graphics()
{
	// Get the trace stack
	m_TraceEnabled = m_MainWindow->traceEnabled();
	m_TraceX = m_MainWindow->traceX();
	m_TraceY = m_MainWindow->traceY();
	m_TraceStack.clear();
	if (m_TraceEnabled) FT800EMU::GraphicsProcessor.processTrace(m_TraceStack, m_TraceX, m_TraceY, hsize());
	
	// Get the stack under the mouse cursor
	m_MouseStackWrite.clear();
	if (m_MouseOver) FT800EMU::GraphicsProcessor.processTrace(m_MouseStackWrite, m_NextMouseX, m_NextMouseY, hsize());
}

// Graphics callback synchronized to Qt thread, use to overlay graphics
void InteractiveViewport::graphics(QImage *image)
{
	// Update frame dependent gui
	m_MainWindow->dlEditor()->codeEditor()->setTraceHighlights(m_TraceStack);
	m_MouseX = m_NextMouseX;
	m_MouseY = m_NextMouseY;
	m_MouseStackRead.swap(m_MouseStackWrite);
	updatePointerMethod();

	// Draw image overlays
}

void InteractiveViewport::automaticChecked()
{
	m_PointerMode = 0;
}

void InteractiveViewport::touchChecked()
{
	m_PointerMode = 1;
}

void InteractiveViewport::traceChecked()
{
	m_PointerMode = 2;
}

int InteractiveViewport::updatePointerMethod()
{
	if (m_MouseTouch)
	{
		// Cannot change now
	}
	else
	{
		switch (m_PointerMode)
		{
		case 0: // none
			setCursor(Qt::ArrowCursor);
			m_PointerMethod = 0;
			break;
		case 1:
			setCursor(Qt::PointingHandCursor);
			m_PointerMethod = 1;
			break;
		case 2:
			setCursor(Qt::CrossCursor);
			m_PointerMethod = 2;
			break;
		case 3:
			setCursor(Qt::ArrowCursor); // todo
			break;
		}
	}
}

void InteractiveViewport::mouseMoveEvent(QMouseEvent *e)
{
	// printf("pos: %i, %i\n", e->pos().x(), e->pos().y());
	
	m_NextMouseX = e->pos().x();
	m_NextMouseY = e->pos().y();
	
	if (m_MouseTouch)
	{
		FT800EMU::Memory.setTouchScreenXY(e->pos().x(), e->pos().y(), 0);
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mousePressEvent(QMouseEvent *e)
{
	switch (m_PointerMethod)
	{
	case 1: // touch
		m_MouseTouch = true;
		FT800EMU::Memory.setTouchScreenXY(e->pos().x(), e->pos().y(), 0);
		break;
	case 2: // trace
		m_MainWindow->setTraceX(e->pos().x());
		m_MainWindow->setTraceY(e->pos().y());
		m_MainWindow->setTraceEnabled(true);
		break;
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mouseReleaseEvent(QMouseEvent *e)
{
	if (m_MouseTouch)
	{
		m_MouseTouch = false;
		FT800EMU::Memory.resetTouchScreenXY();
		updatePointerMethod(); // update because update is not done while m_MouseTouch true
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::enterEvent(QEvent *e)
{
	m_MouseOver = true;

	EmulatorViewport::enterEvent(e);
}

void InteractiveViewport::leaveEvent(QEvent *e)
{
	m_MouseOver = false;

	EmulatorViewport::leaveEvent(e);
}

} /* namespace FT800EMUQT */

/* end of file */
