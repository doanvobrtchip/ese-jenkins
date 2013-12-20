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
#include <QPainter>
#include <QPen>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <ft800emu_memory.h>

// Project includes
#include "main_window.h"
#include "code_editor.h"

namespace FT800EMUQT {

#define POINTER_ALL 0xFF
#define POINTER_TOUCH 0x01
#define POINTER_TRACE 0x02
#define POINTER_EDIT_VERTEX_MOVE 0x04
#define POINTER_EDIT_STACK_SELECT 0x08

InteractiveViewport::InteractiveViewport(MainWindow *parent) 
	: EmulatorViewport(parent), m_MainWindow(parent),
	m_PreferTraceCursor(false), m_TraceEnabled(false), m_MouseOver(false), m_MouseTouch(false), 
	m_PointerFilter(POINTER_ALL), m_PointerMethod(0), m_LineEditor(NULL), m_LineNumber(0),
	m_MouseOverVertex(false), m_MouseOverVertexLine(-1), m_MouseMovingVertex(false)
{
	// m_Label->setCursor(Qt::PointingHandCursor);
	setMouseTracking(true);
	
	QActionGroup *cursorGroup = new QActionGroup(this);
	
	QAction *automatic = new QAction(cursorGroup);
	connect(automatic, SIGNAL(triggered()), this, SLOT(automaticChecked()));
	automatic->setText(tr("Cursor"));
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
	
	QAction *edit = new QAction(cursorGroup);
	connect(edit, SIGNAL(triggered()), this, SLOT(editChecked()));
	edit->setText(tr("Edit"));
	edit->setStatusTip(tr("Interactive editing tools"));
	edit->setCheckable(true);
	
	QToolBar *toolBar = m_MainWindow->addToolBar(tr("Cursor"));
	toolBar->addAction(automatic);
	toolBar->addAction(touch);
	toolBar->addAction(trace);
	toolBar->addAction(edit);
}

InteractiveViewport::~InteractiveViewport()
{
	
}

// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
void InteractiveViewport::graphics()
{
	// Get the trace stack
	if (!m_TraceEnabled && m_MainWindow->traceEnabled())
	{
		m_PreferTraceCursor = true;
	}
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

	// Draw image overlays
	QPainter p;
	p.begin(image);
	if (m_TraceEnabled)
	{
		QPen outer;
		QPen inner;
		outer.setWidth(3);
		outer.setColor(QColor(Qt::black));
		inner.setWidth(1);
		inner.setColor(QColor(Qt::green).lighter(160));
		p.setPen(outer);
		p.drawLine(m_TraceX, m_TraceY - 7, m_TraceX, m_TraceY - 14);
		p.drawLine(m_TraceX, m_TraceY + 7, m_TraceX, m_TraceY + 14);
		p.drawLine(m_TraceX - 7, m_TraceY, m_TraceX - 14, m_TraceY);
		p.drawLine(m_TraceX + 7, m_TraceY, m_TraceX + 14, m_TraceY);
		p.setPen(inner);
		p.drawLine(m_TraceX, m_TraceY - 7, m_TraceX, m_TraceY - 14);
		p.drawLine(m_TraceX, m_TraceY + 7, m_TraceX, m_TraceY + 14);
		p.drawLine(m_TraceX - 7, m_TraceY, m_TraceX - 14, m_TraceY);
		p.drawLine(m_TraceX + 7, m_TraceY, m_TraceX + 14, m_TraceY);
	}
	if (m_LineEditor)
	{
		const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
		if (parsed.IdLeft == FT800EMU_DL_VERTEX2F || parsed.IdLeft == FT800EMU_DL_VERTEX2II)
		{
			if (m_PointerFilter & POINTER_EDIT_VERTEX_MOVE)
			{
				QPen outer;
				QPen inner;
				outer.setWidth(3);
				outer.setColor(QColor(Qt::black));
				inner.setWidth(1);
				inner.setColor(QColor(Qt::gray));
				// Find first line
				int firstLine = m_LineNumber;
				for (int l = firstLine - 1; l > 0; --l)
				{
					const DlParsed &pa = m_LineEditor->getLine(l);
					if (pa.IdLeft == 0 && 
						(pa.IdRight == FT800EMU_DL_BEGIN 
						|| pa.IdRight == FT800EMU_DL_END
						|| pa.IdRight == FT800EMU_DL_RETURN
						|| pa.IdRight == FT800EMU_DL_JUMP))
					{
						break;
					}
					else
					{
						firstLine = l;
					}
				}
				// Iterate over neighbouring vertices
				for (int l = firstLine; l < FT800EMU_DL_SIZE; ++l) // FIXME
				{
					if (l == m_LineNumber) continue;
					const DlParsed &pa = m_LineEditor->getLine(l);
					if (pa.IdLeft == FT800EMU_DL_VERTEX2F || pa.IdLeft == FT800EMU_DL_VERTEX2II)
					{
						int x, y;
						if (pa.IdLeft == FT800EMU_DL_VERTEX2F)
						{
							x = pa.Parameter[0] >> 4;
							y = pa.Parameter[1] >> 4;
						}
						else
						{
							x = pa.Parameter[0];
							y = pa.Parameter[1];
						}
						p.setPen(outer);
						p.drawLine(x - 4, y - 4, x + 4, y - 4);
						p.drawLine(x - 4, y + 4, x + 4, y + 4);
						p.drawLine(x - 4, y - 4, x - 4, y + 4);
						p.drawLine(x + 4, y - 4, x + 4, y + 4);
						p.setPen(inner);
						p.drawLine(x - 4, y - 4, x + 4, y - 4);
						p.drawLine(x - 4, y + 4, x + 4, y + 4);
						p.drawLine(x - 4, y - 4, x - 4, y + 4);
						p.drawLine(x + 4, y - 4, x + 4, y + 4);
					}
					else if (pa.IdRight == FT800EMU_DL_BEGIN
						|| pa.IdRight == FT800EMU_DL_END
						|| pa.IdRight == FT800EMU_DL_RETURN
						|| pa.IdRight == FT800EMU_DL_JUMP)
					{
						break;
					}
				}
				// Show central vertex
				int x, y;
				if (parsed.IdLeft == FT800EMU_DL_VERTEX2F)
				{
					x = parsed.Parameter[0] >> 4;
					y = parsed.Parameter[1] >> 4;
				}
				else
				{
					x = parsed.Parameter[0];
					y = parsed.Parameter[1];
				}
				p.setPen(outer);
				p.drawLine(x, y - 5, x, y - 12);
				p.drawLine(x, y + 5, x, y + 12);
				p.drawLine(x - 5, y, x - 12, y);
				p.drawLine(x + 5, y, x + 12, y);
				p.drawLine(x - 4, y - 4, x + 4, y - 4);
				p.drawLine(x - 4, y + 4, x + 4, y + 4);
				p.drawLine(x - 4, y - 4, x - 4, y + 4);
				p.drawLine(x + 4, y - 4, x + 4, y + 4);
				inner.setColor(QColor(Qt::red));
				p.setPen(inner);
				p.drawLine(x, y - 5, x, y - 12);
				p.drawLine(x, y + 5, x, y + 12);
				p.drawLine(x - 5, y, x - 12, y);
				p.drawLine(x + 5, y, x + 12, y);
				p.drawLine(x - 4, y - 4, x + 4, y - 4);
				p.drawLine(x - 4, y + 4, x + 4, y + 4);
				p.drawLine(x - 4, y - 4, x - 4, y + 4);
				p.drawLine(x + 4, y - 4, x + 4, y + 4);
			
			/*

test dl

CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
BEGIN(RECTS)
VERTEX2II(100, 100, 0, 0)
VERTEX2II(220, 150, 0, 0)
END()

			 */
			}
		}
		else
		{
			// switch ...
		}
	}
	p.end();
	
	// Update pointer method
	updatePointerMethod();
}

void InteractiveViewport::setEditorLine(DlEditor *editor, int line)
{
	if (m_MouseMovingVertex && m_LineEditor)
	{
		m_LineEditor->codeEditor()->endUndoCombine();
	}
	m_PreferTraceCursor = false;
	m_LineEditor = editor;
	m_LineNumber = line;
}

void InteractiveViewport::unsetEditorLine()
{
	m_LineEditor = NULL;
}

void InteractiveViewport::automaticChecked()
{
	m_PreferTraceCursor = false;
	m_PointerFilter = POINTER_ALL;
}

void InteractiveViewport::touchChecked()
{
	m_PointerFilter = POINTER_TOUCH;
}

void InteractiveViewport::traceChecked()
{
	m_PointerFilter = POINTER_TRACE;
}

void InteractiveViewport::editChecked()
{
	m_PointerFilter = 
		POINTER_EDIT_VERTEX_MOVE // vertex movement
		| POINTER_EDIT_STACK_SELECT // stack selection
		;
}

void InteractiveViewport::updatePointerMethod()
{
	if (m_MouseTouch || m_MouseMovingVertex)
	{
		// Cannot change now
	}
	else
	{
		if (m_PreferTraceCursor)
		{
			goto PreferTraceCursor;
		}
		// Vertex movement
		if (m_PointerFilter & POINTER_EDIT_VERTEX_MOVE)
		{
			if (m_LineEditor)
			{
				m_MouseOverVertex = false;
				m_MouseOverVertexLine = -1;
				const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
				if (parsed.IdLeft == FT800EMU_DL_VERTEX2F || parsed.IdLeft == FT800EMU_DL_VERTEX2II)
				{
					int firstLine = m_LineNumber;
					for (int l = firstLine - 1; l > 0; --l)
					{
						const DlParsed &pa = m_LineEditor->getLine(l);
						if (pa.IdLeft == 0 && 
							(pa.IdRight == FT800EMU_DL_BEGIN 
							|| pa.IdRight == FT800EMU_DL_END
							|| pa.IdRight == FT800EMU_DL_RETURN
							|| pa.IdRight == FT800EMU_DL_JUMP))
						{
							break;
						}
						else
						{
							firstLine = l;
						}
					}
					// Iterate over neighbouring vertices
					for (int l = firstLine; l < FT800EMU_DL_SIZE; ++l) // FIXME
					{
						const DlParsed &pa = m_LineEditor->getLine(l);
						if (pa.IdLeft == FT800EMU_DL_VERTEX2F || pa.IdLeft == FT800EMU_DL_VERTEX2II)
						{
							int x, y;
							if (pa.IdLeft == FT800EMU_DL_VERTEX2F)
							{
								x = pa.Parameter[0] >> 4;
								y = pa.Parameter[1] >> 4;
							}
							else
							{
								x = pa.Parameter[0];
								y = pa.Parameter[1];
							}
							
							// Mouse over
							if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4)
							{
								m_MouseOverVertex = true;
								m_MouseOverVertexLine = l;
								if (l == m_LineNumber) break; // Currently selected line always has preference
							}
						}
						else if (pa.IdRight == FT800EMU_DL_BEGIN
							|| pa.IdRight == FT800EMU_DL_END
							|| pa.IdRight == FT800EMU_DL_RETURN
							|| pa.IdRight == FT800EMU_DL_JUMP)
						{
							break;
						}
					}
					if (m_MouseOverVertex)
					{
						setCursor(Qt::SizeAllCursor);
						m_PointerMethod = POINTER_EDIT_VERTEX_MOVE; // move vertex
						return;
					}
				}
			}
		}
		// Stack selection
		if (m_PointerFilter & POINTER_EDIT_STACK_SELECT)
		{
			// FIXME: Only works on display list.
			setCursor(Qt::ArrowCursor);
			m_PointerMethod = POINTER_EDIT_STACK_SELECT; // Stack selection
			return;
		}
		if (m_PointerFilter & POINTER_TOUCH)
		{
			// TODO: Get the TAG from stack trace and show hand or not depending on TAG value (maybe also show tootip with tag?)
			setCursor(Qt::PointingHandCursor);
			m_PointerMethod = POINTER_TOUCH;
			return;
		}
	PreferTraceCursor:
		if (m_PointerFilter & POINTER_TRACE)
		{
			setCursor(Qt::CrossCursor);
			m_PointerMethod = POINTER_TRACE;
			return;
		}
		setCursor(Qt::ArrowCursor);
		m_PointerMethod = 0;
		return;
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
	else if (m_MouseMovingVertex)
	{
		if (m_LineEditor)
		{
			int xd = e->pos().x() - m_MovingLastX;
			int yd = e->pos().y() - m_MovingLastY;
			m_MovingLastX = e->pos().x();
			m_MovingLastY = e->pos().y();
			DlParsed pa = m_LineEditor->getLine(m_LineNumber);
			if (pa.IdLeft == FT800EMU_DL_VERTEX2F)
			{
				xd >>= 4;
				yd >>= 4;
			}
			pa.Parameter[0] += xd;
			pa.Parameter[1] += yd;
			m_LineEditor->replaceLine(m_LineNumber, pa);
		}
		else
		{
			m_MouseMovingVertex = false;
			updatePointerMethod(); // update because update is not done while m_MouseMovingVertex true
		}
	}

	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mousePressEvent(QMouseEvent *e)
{
	switch (m_PointerMethod)
	{
	case POINTER_TOUCH: // touch
		if (e->button() == Qt::LeftButton)
		{
			m_MouseTouch = true;
			FT800EMU::Memory.setTouchScreenXY(e->pos().x(), e->pos().y(), 0);
		}
		break;
	case POINTER_TRACE: // trace
		switch (e->button())
		{
		case Qt::LeftButton:
			m_MainWindow->setTraceX(e->pos().x());
			m_MainWindow->setTraceY(e->pos().y());
			m_MainWindow->setTraceEnabled(true);
			break;
		case Qt::MidButton:
			if (m_PointerFilter != POINTER_TRACE)
			{
				m_PreferTraceCursor = false;
				break;
			}
			// fallthrough to Qt::RightButton
		case Qt::RightButton:
			m_MainWindow->setTraceEnabled(false);
			break;
		}
		break;
	case POINTER_EDIT_VERTEX_MOVE:
		if (m_LineEditor)
		{
			if (m_MouseOverVertexLine != m_LineNumber)
			{
				m_LineEditor->selectLine(m_MouseOverVertexLine);
			}
			m_MovingLastX = e->pos().x();
			m_MovingLastY = e->pos().y();
			m_MouseMovingVertex = true;
			m_LineEditor->codeEditor()->beginUndoCombine();
		}
		break;
	case POINTER_EDIT_STACK_SELECT:
		if (m_LineEditor && m_MouseStackRead.size() > 0)
		{
			if (e->button() == Qt::LeftButton)
			{
				// FIXME: Only works on display list.
				// Select topmost command
				m_LineEditor->selectLine(m_MouseStackRead[m_MouseStackRead.size() - 1]);
			}
		}
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
	else if (m_MouseMovingVertex)
	{
		m_MouseMovingVertex = false;
		if (m_LineEditor)
		{
			m_LineEditor->codeEditor()->endUndoCombine();
		}
		updatePointerMethod(); // update because update is not done while m_MouseMovingVertex true
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
