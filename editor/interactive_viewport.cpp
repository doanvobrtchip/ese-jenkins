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
#include <QTreeWidget>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <ft800emu_memory.h>
#include <vc.h>

// Project includes
#include "main_window.h"
#include "code_editor.h"
#include "toolbox.h"
#include "inspector.h"
#include "bitmap_setup.h"
#include "content_manager.h"
#include "properties_editor.h"

namespace FT800EMUQT {

#define POINTER_ALL 0xFFFF
#define POINTER_TOUCH 0x0001
#define POINTER_TRACE 0x0002
#define POINTER_EDIT_VERTEX_MOVE 0x0004
#define POINTER_EDIT_STACK_SELECT 0x0008
#define POINTER_EDIT_WIDGET_TRANSLATE 0x0010
#define POINTER_EDIT_WIDGET_SIZE_TOP 0x0020
#define POINTER_EDIT_WIDGET_SIZE_RIGHT 0x0040
#define POINTER_EDIT_WIDGET_SIZE_LEFT 0x0080
#define POINTER_EDIT_WIDGET_SIZE_BOTTOM 0x0100
#define POINTER_EDIT_WIDGET_MOVE (POINTER_EDIT_WIDGET_TRANSLATE | POINTER_EDIT_WIDGET_SIZE_TOP | POINTER_EDIT_WIDGET_SIZE_RIGHT | POINTER_EDIT_WIDGET_SIZE_LEFT | POINTER_EDIT_WIDGET_SIZE_BOTTOM)
#define POINTER_EDIT_WIDGET_SIZE_TOPLEFT (POINTER_EDIT_WIDGET_SIZE_TOP | POINTER_EDIT_WIDGET_SIZE_LEFT)
#define POINTER_EDIT_WIDGET_SIZE_TOPRIGHT (POINTER_EDIT_WIDGET_SIZE_TOP | POINTER_EDIT_WIDGET_SIZE_RIGHT)
#define POINTER_EDIT_WIDGET_SIZE_BOTTOMLEFT (POINTER_EDIT_WIDGET_SIZE_BOTTOM | POINTER_EDIT_WIDGET_SIZE_LEFT)
#define POINTER_EDIT_WIDGET_SIZE_BOTTOMRIGHT (POINTER_EDIT_WIDGET_SIZE_BOTTOM | POINTER_EDIT_WIDGET_SIZE_RIGHT)
#define POINTER_INSERT 0x0200

InteractiveViewport::InteractiveViewport(MainWindow *parent)
	: EmulatorViewport(parent), m_MainWindow(parent),
	m_PreferTraceCursor(false), m_TraceEnabled(false), m_MouseOver(false), m_MouseTouch(false), m_MouseStackValid(false),
	m_PointerFilter(POINTER_ALL), m_PointerMethod(0), m_LineEditor(NULL), m_LineNumber(0),
	m_MouseOverVertex(false), m_MouseOverVertexLine(-1), m_MouseMovingVertex(false),
	m_WidgetXY(false), m_WidgetWH(false), m_WidgetR(false),
	m_MouseMovingWidget(0)
{
	// m_Label->setCursor(Qt::PointingHandCursor);
	setMouseTracking(true);
	setAcceptDrops(true);

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

	QToolBar *cursorToolBar = m_MainWindow->addToolBar(tr("Cursor"));
	cursorToolBar->addAction(automatic);
	cursorToolBar->addAction(touch);
	cursorToolBar->addAction(trace);
	cursorToolBar->addAction(edit);

	// TODO: Sexy toolbar icons.

	// icon something
	m_Insert = new QAction(this);
	m_Insert->setText(tr("Insert"));
	m_Insert->setStatusTip(tr("Place a new vertex or clone the selected widget directly on the screen"));
	m_Insert->setCheckable(true);

	/*
	// icon arrow left
	QAction *decrease = new QAction(this);
	// connect(decrease, SIGNAL(triggered()), this, SLOT(backTriggered()));
	decrease->setText(tr("Back"));
	decrease->setStatusTip(tr("Move selected vertex or widget behind the preceding operation"));

	// icon arrow right
	QAction *increase = new QAction(this);
	// connect(increase, SIGNAL(triggered()), this, SLOT(frontTriggered()));
	increase->setText(tr("Front"));
	increase->setStatusTip(tr("Move selected vertex or widget in front of the following operation"));
	*/

	QToolBar *toolBar = m_MainWindow->addToolBar(tr("Toolbar"));
	toolBar->addAction(m_Insert);
	// toolBar->addAction(decrease);
	// toolBar->addAction(increase);
}

InteractiveViewport::~InteractiveViewport()
{

}

/*

LINE_WIDTH(24)
POINT_SIZE(36)
BEGIN(LINE_STRIP)
CALL(8)
END()
BEGIN(POINTS)
CALL(8)
END()
VERTEX2II(50, 50, 0, 0)
RETURN()

*/

static bool isValidInsert(const DlParsed &parsed)
{
	if (parsed.ValidId)
	{
		if (parsed.IdLeft == FT800EMU_DL_VERTEX2II
			|| parsed.IdLeft == FT800EMU_DL_VERTEX2F)
		{
			return true;
		}
		else if (parsed.IdLeft == 0xFFFFFF00)
		{
			switch (parsed.IdRight | 0xFFFFFF00)
			{
			case CMD_TEXT:
			case CMD_BUTTON:
			case CMD_KEYS:
			case CMD_PROGRESS:
			case CMD_SLIDER:
			case CMD_SCROLLBAR:
			case CMD_TOGGLE:
			case CMD_GAUGE:
			case CMD_CLOCK:
			case CMD_SPINNER:
			case CMD_TRACK:
			case CMD_DIAL:
			case CMD_NUMBER:
			case CMD_SKETCH:
				return true;
			}
		}
	}
	return false;
}

// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
void InteractiveViewport::graphics()
{
	// Get the trace stack
	if (!m_TraceEnabled && m_MainWindow->traceEnabled())
	{
		m_PreferTraceCursor = true;
	}
	else if (m_PreferTraceCursor && !m_MainWindow->traceEnabled())
	{
		m_PreferTraceCursor = false;
	}
	m_TraceEnabled = m_MainWindow->traceEnabled();
	m_TraceX = m_MainWindow->traceX();
	m_TraceY = m_MainWindow->traceY();
	m_TraceStack.clear();
	m_TraceStackDl.clear();
	m_TraceStackCmd.clear();
	bool cmdLast = false;
	if (m_TraceEnabled)
	{
		FT800EMU::GraphicsProcessor.processTrace(m_TraceStack, m_TraceX, m_TraceY, hsize());
		for (int i = 0; i < m_TraceStack.size(); ++i)
		{
			int cmdIdx = m_MainWindow->getDlCmd()[m_TraceStack[i]];
			if (cmdIdx >= 0)
			{
				m_TraceStackCmd.push_back(cmdIdx);
				cmdLast = true;
			}
			else
			{
				m_TraceStackDl.push_back(m_TraceStack[i]);
				cmdLast = false;
			}
		}
		if (cmdLast)
		{
			m_TraceStackDl.push_back(-1);
		}
		else
		{
			m_TraceStackCmd.push_back(-1);
		}
	}

	// Get the stack under the mouse cursor
	m_MouseStackWrite.clear();
	if (m_MouseOver || m_DragMoving)
	{
		if (m_NextMouseY >= 0 && m_NextMouseY < vsize() && m_NextMouseX > 0 && m_NextMouseX < hsize())
		{
			FT800EMU::GraphicsProcessor.processTrace(m_MouseStackWrite, m_NextMouseX, m_NextMouseY, hsize());
			if (m_MouseStackWrite.size())
			{
				m_MouseStackDlTop = m_MouseStackWrite[m_MouseStackWrite.size() - 1];
				m_MouseStackCmdTop = m_MainWindow->getDlCmd()[m_MouseStackDlTop];
				m_MouseStackValid = true;
			}
			else m_MouseStackValid = false;
		}
		m_DragMoving = false;
	}

	m_MainWindow->frameEmu();
	m_MainWindow->inspector()->frameEmu();
}

/*
 *
CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
BEGIN(RECTS)
VERTEX2II(100, 100, 0, 0)
VERTEX2II(220, 150, 0, 0)
END()
CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
 *
 */

// Graphics callback synchronized to Qt thread, use to overlay graphics
void InteractiveViewport::graphics(QImage *image)
{
	// Update frame dependent gui
	m_MainWindow->dlEditor()->codeEditor()->setTraceHighlights(m_TraceStackDl);
	m_MainWindow->cmdEditor()->codeEditor()->setTraceHighlights(m_TraceStackCmd);
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
			m_WidgetXY = false;
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
							x = pa.Parameter[0].I >> 4;
							y = pa.Parameter[1].I >> 4;
						}
						else
						{
							x = pa.Parameter[0].U;
							y = pa.Parameter[1].U;
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
					x = parsed.Parameter[0].I >> 4;
					y = parsed.Parameter[1].I >> 4;
				}
				else
				{
					x = parsed.Parameter[0].U;
					y = parsed.Parameter[1].U;
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
*
*
*
*
CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
BEGIN(RECTS)
VERTEX2II(100, 100, 0, 0)
VERTEX2II(220, 150, 0, 0)
END()
*

CMD_LOGO()
CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
BEGIN(RECTS)
VERTEX2II(100, 100, 0, 0)
MACRO(0) // VERTEX2II(220, 150, 0, 0)
END()
CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
CMD_SCREENSAVER()
*
*
CLEAR_COLOR_RGB(50, 0, 0)
CLEAR(1, 1, 1)
CMD_SPINNER(240, 136, 1, 2)
*
*
CMD_MEMZERO(0, 16320)
*
CMD_SKETCH(0, 0, 480, 272, 0, L1)
BITMAP_SOURCE(0)
BITMAP_LAYOUT(L1, 60, 270)
BITMAP_SIZE(NEAREST, BORDER, BORDER, 480, 270)
BEGIN(BITMAPS)
VERTEX2II(0, 0, 0, 0)
CMD_NUMBER(80, 60, 31, OPT_CENTER, 42)




CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
BEGIN(LINES)
MACRO(0)
VERTEX2II(460, 253, 0, 0)
MACRO(0)
VERTEX2II(465, 15, 0, 0)
MACRO(0)
VERTEX2II(13, 251, 0, 0)
MACRO(0)
VERTEX2II(5, 10, 0, 0)
END()
CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
CMD_SCREENSAVER()


			 */
			}
		}
		else if (parsed.IdLeft == 0xFFFFFF00) // Coprocessor
		{
			switch (parsed.IdRight | 0xFFFFFF00)
			{
				case CMD_TEXT:
				case CMD_BUTTON:
				case CMD_KEYS:
				case CMD_PROGRESS:
				case CMD_SLIDER:
				case CMD_SCROLLBAR:
				case CMD_TOGGLE:
				case CMD_GAUGE:
				case CMD_CLOCK:
				case CMD_SPINNER:
				case CMD_TRACK:
				case CMD_DIAL:
				case CMD_NUMBER:
				case CMD_SKETCH:
				{
					QPen outer;
					QPen inner;
					outer.setWidth(3);
					outer.setColor(QColor(Qt::black));
					inner.setWidth(1);
					inner.setColor(QColor(Qt::red));
					m_WidgetXY = true;
					switch (parsed.IdRight | 0xFFFFFF00)
					{
					case CMD_BUTTON:
					case CMD_KEYS:
					case CMD_PROGRESS:
					case CMD_SLIDER:
					case CMD_SCROLLBAR:
					case CMD_TRACK:
					case CMD_SKETCH:
						m_WidgetWH = true;
						m_WidgetR = false;
						break;
					case CMD_GAUGE:
					case CMD_CLOCK:
					case CMD_DIAL:
						m_WidgetWH = false;
						m_WidgetR = true;
						break;
					default:
						m_WidgetWH = false;
						m_WidgetR = false;
						break;
					}

					int x = parsed.Parameter[0].I;
					int y = parsed.Parameter[1].I;

// CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
					// Draw...
					if (m_WidgetWH == false && m_WidgetR == false)
					{
						// Only have vertex control
						p.setPen(outer);
						p.drawLine(x, y - 5, x, y - 12);
						p.drawLine(x, y + 5, x, y + 12);
						p.drawLine(x - 5, y, x - 12, y);
						p.drawLine(x + 5, y, x + 12, y);
						p.drawLine(x - 4, y - 4, x + 4, y - 4);
						p.drawLine(x - 4, y + 4, x + 4, y + 4);
						p.drawLine(x - 4, y - 4, x - 4, y + 4);
						p.drawLine(x + 4, y - 4, x + 4, y + 4);
						p.setPen(inner);
						p.drawLine(x, y - 5, x, y - 12);
						p.drawLine(x, y + 5, x, y + 12);
						p.drawLine(x - 5, y, x - 12, y);
						p.drawLine(x + 5, y, x + 12, y);
						p.drawLine(x - 4, y - 4, x + 4, y - 4);
						p.drawLine(x - 4, y + 4, x + 4, y + 4);
						p.drawLine(x - 4, y - 4, x - 4, y + 4);
						p.drawLine(x + 4, y - 4, x + 4, y + 4);
					}
					else
					{
						int w, h;
						if (m_WidgetWH)
						{
							w = parsed.Parameter[2].I;
							h = parsed.Parameter[3].I;
						}
						else
						{
							x = x - parsed.Parameter[2].I;
							y = y - parsed.Parameter[2].I;
							w = parsed.Parameter[2].I * 2;
							h = parsed.Parameter[2].I * 2;
						}
						int x1 = x;
						int y1 = y;
						int x2 = x + w;
						int y2 = y + h;
						p.setPen(outer);
						p.drawRect(x, y, w, h);
						p.drawRect(x1 - 1, y1 - 1, 2, 2);
						p.drawRect(x1 - 1, y2 - 1, 2, 2);
						p.drawRect(x2 - 1, y2 - 1, 2, 2);
						p.drawRect(x2 - 1, y1 - 1, 2, 2);
						p.setPen(inner);
						p.drawRect(x, y, w, h);
						p.drawRect(x1 - 1, y1 - 1, 2, 2);
						p.drawRect(x1 - 1, y2 - 1, 2, 2);
						p.drawRect(x2 - 1, y2 - 1, 2, 2);
						p.drawRect(x2 - 1, y1 - 1, 2, 2);
					}
					break;
				}
				default:
				{
					m_WidgetXY = false;
					break;
				}
			}
		}
		else
		{
			m_WidgetXY = false;
		}
	}
	p.end();

	// Update pointer method
	updatePointerMethod();

	m_MainWindow->frameQt();
	m_MainWindow->inspector()->frameQt();
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
	// printf("Set line editor\n");
}

void InteractiveViewport::unsetEditorLine()
{
	m_LineEditor = NULL;
	// printf("Unset line editor\n");
	m_Insert->setChecked(false);
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
		| POINTER_EDIT_WIDGET_MOVE // widget movement
		;
}

void InteractiveViewport::updatePointerMethod()
{
	if (m_MouseTouch || m_MouseMovingVertex || m_MouseMovingWidget)
	{
		// Cannot change now
	}
	else
	{
		if (m_MainWindow->waitingCoprocessorAnimation())
			goto PreferTouchCursor;
		if (m_Insert->isChecked())
		{
			if (m_LineEditor)
			{
				const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
				if (isValidInsert(parsed))
				{
					// Special case override cursur (not a filter), strange...
					m_PointerMethod = POINTER_INSERT;
					setCursor(Qt::CrossCursor);
					return;
				}
				else
				{
					printf("Uncheck insert\n");
					m_Insert->setChecked(false);
				}
			}
			else
			{
				printf("Force uncheck insert, no line editor\n");
				m_Insert->setChecked(false);
			}
		}
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
					int vertexType = -1;
					for (int l = firstLine - 1; l > 0; --l)
					{
						const DlParsed &pa = m_LineEditor->getLine(l);
						if (pa.IdLeft == 0 &&
							(pa.IdRight == FT800EMU_DL_BEGIN
							|| pa.IdRight == FT800EMU_DL_END
							|| pa.IdRight == FT800EMU_DL_RETURN
							|| pa.IdRight == FT800EMU_DL_JUMP))
						{
							if (pa.IdRight == FT800EMU_DL_BEGIN)
								vertexType = pa.Parameter[0].I;
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
								x = pa.Parameter[0].I >> 4;
								y = pa.Parameter[1].I >> 4;
							}
							else
							{
								x = pa.Parameter[0].U;
								y = pa.Parameter[1].U;
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
					if ((vertexType == BITMAPS || vertexType == POINTS) &&
						((m_MouseStackValid && m_LineEditor->isCoprocessor() && m_MouseStackCmdTop == m_LineNumber)
						|| (m_MouseStackValid && !m_LineEditor->isCoprocessor() && m_MouseStackDlTop == m_LineNumber)))
					{
						m_MouseOverVertex = true;
						m_MouseOverVertexLine = m_LineNumber;
						setCursor(Qt::SizeAllCursor);
						m_PointerMethod = POINTER_EDIT_VERTEX_MOVE; // move vertex
						return;
					}
				}
			}
		}
		// Widget movement
		if (m_PointerFilter & POINTER_EDIT_WIDGET_MOVE)
		{
			if (m_LineEditor)
			{
				if (m_WidgetXY)
				{
					const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
					int x = parsed.Parameter[0].I;
					int y = parsed.Parameter[1].I;
					if (m_WidgetWH || m_WidgetR)
					{
						int w, h;
						if (m_WidgetWH)
						{
							w = parsed.Parameter[2].I;
							h = parsed.Parameter[3].I;
						}
						else
						{
							x = x - parsed.Parameter[2].I;
							y = y - parsed.Parameter[2].I;
							w = parsed.Parameter[2].I * 2;
							h = parsed.Parameter[2].I * 2;
						}
						int x1 = x;
						int y1 = y;
						int x2 = x + w;
						int y2 = y + h;
						m_PointerMethod = 0;
						if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_TOP)
						{
							if (x1 - 3 < m_MouseX && m_MouseX < x2 + 3 && y1 - 3 < m_MouseY && m_MouseY < y1 + 3)
							{
								m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_TOP;
							}
						}
						if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_BOTTOM)
						{
							if (x1 - 3 < m_MouseX && m_MouseX < x2 + 3 && y2 - 3 < m_MouseY && m_MouseY < y2 + 3)
							{
								m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_BOTTOM;
							}
						}
						if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_LEFT)
						{
							if (x1 - 3 < m_MouseX && m_MouseX < x1 + 3 && y1 - 3 < m_MouseY && m_MouseY < y2 + 3)
							{
								m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_LEFT;
							}
						}
						if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_RIGHT)
						{
							if (x2 - 3 < m_MouseX && m_MouseX < x2 + 3 && y1 - 3 < m_MouseY && m_MouseY < y2 + 3)
							{
								m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_RIGHT;
							}
						}
						if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPLEFT
							|| m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMRIGHT)
							setCursor(Qt::SizeFDiagCursor);
						else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPRIGHT
							|| m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMLEFT)
							setCursor(Qt::SizeBDiagCursor);
						else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOP
							|| m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOM)
							setCursor(Qt::SizeVerCursor);
						else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_LEFT
							|| m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_RIGHT)
							setCursor(Qt::SizeHorCursor);
						if (m_PointerMethod)
							return;
						if (m_PointerFilter & POINTER_EDIT_WIDGET_TRANSLATE)
						{
							if (x1 < m_MouseX && m_MouseX < x2 && y1 < m_MouseY && m_MouseY < y2)
							{
								// m_MouseOverWidget = true;
								setCursor(Qt::SizeAllCursor);
								m_PointerMethod = POINTER_EDIT_WIDGET_TRANSLATE; // translate widget
								return;
								// CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
							}
						}
					}
					else
					{
						if (m_PointerFilter & POINTER_EDIT_WIDGET_TRANSLATE)
						{
							if ((m_MouseStackValid && m_LineEditor->isCoprocessor() && m_MouseStackCmdTop == m_LineNumber)
								|| (m_MouseStackValid && !m_LineEditor->isCoprocessor() && m_MouseStackDlTop == m_LineNumber)
								|| (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4))
							{
								// m_MouseOverWidget = true;
								setCursor(Qt::SizeAllCursor);
								m_PointerMethod = POINTER_EDIT_WIDGET_TRANSLATE; // translate widget
								return;
								// CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
							}
						}
					}
				}
			}
		}
		// Stack selection
		if (m_PointerFilter & POINTER_EDIT_STACK_SELECT)
		{
			setCursor(Qt::ArrowCursor);
			m_PointerMethod = POINTER_EDIT_STACK_SELECT; // Stack selection
			return;
		}
	PreferTouchCursor:
		if (m_PointerFilter & POINTER_TOUCH)
		{
			// TODO: Get the TAG from stack trace and show hand or not depending on TAG value (maybe also show tootip with tag?)
			setCursor(Qt::PointingHandCursor);
			m_PointerMethod = POINTER_TOUCH;
			return;
		}
		if (m_MainWindow->waitingCoprocessorAnimation())
			goto NoCursor;
	PreferTraceCursor:
		if (m_PointerFilter & POINTER_TRACE)
		{
			setCursor(Qt::CrossCursor);
			m_PointerMethod = POINTER_TRACE;
			return;
		}
	NoCursor:
		setCursor(Qt::ArrowCursor);
		m_PointerMethod = 0;
		return;
	}
}

// CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
// CMD_BUTTON(50, 50, 100, 40, 21, 0, "hello world")
// CMD_SPINNER(50, 50, 0, 0)

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
				xd <<= 4;
				yd <<= 4;
			}
			pa.Parameter[0].I += xd;
			pa.Parameter[1].I += yd;
			if (pa.IdLeft == FT800EMU_DL_VERTEX2II) // Do not allow negative values
			{
				if (pa.Parameter[0].I < 0)
				{
					m_MovingLastX -= pa.Parameter[0].I;
					pa.Parameter[0].I = 0;
				}
				if (pa.Parameter[0].I > 511)
				{
					int diff = pa.Parameter[0].I - 511;
					m_MovingLastX -= diff;
					pa.Parameter[0].I -= diff;
				}
				if (pa.Parameter[1].I < 0)
				{
					m_MovingLastY -= pa.Parameter[1].I;
					pa.Parameter[1].I = 0;
				}
				if (pa.Parameter[1].I > 511)
				{
					int diff = pa.Parameter[1].I - 511;
					m_MovingLastY -= diff;
					pa.Parameter[1].I -= diff;
				}
			}
			m_LineEditor->replaceLine(m_LineNumber, pa);
		}
		else
		{
			m_MouseMovingVertex = false;
			updatePointerMethod(); // update because update is not done while m_MouseMovingVertex true
		}
	}
	else if (m_MouseMovingWidget)
	{
		if (m_LineEditor)
		{
			// Apply action
			int xd = e->pos().x() - m_MovingLastX;
			int yd = e->pos().y() - m_MovingLastY;
			m_MovingLastX = e->pos().x();
			m_MovingLastY = e->pos().y();
			DlParsed pa = m_LineEditor->getLine(m_LineNumber);
			if (m_MouseMovingWidget == POINTER_EDIT_WIDGET_TRANSLATE)
			{
				pa.Parameter[0].I += xd;
				pa.Parameter[1].I += yd;
			}
			else // resize, check top/bottom and left/right
			{
				int x = pa.Parameter[0].I;
				int y = pa.Parameter[1].I;
				if (m_WidgetWH)
				{
					int w = pa.Parameter[2].I;
					int h = pa.Parameter[3].I;
					if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_TOP)
					{
						y += yd;
						h -= yd;
					}
					else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM)
					{
						h += yd;
					}
					if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT)
					{
						x += xd;
						w -= xd;
					}
					else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT)
					{
						w += xd;
					}
					pa.Parameter[0].I = x;
					pa.Parameter[1].I = y;
					pa.Parameter[2].I = w;
					pa.Parameter[3].I = h;
				}
				else
				{
					int r = pa.Parameter[2].I;
					if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_TOP)
					{
						r -= yd;
					}
					else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM)
					{
						r += yd;
					}
					else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT)
					{
						r -= xd;
					}
					else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT)
					{
						r += xd;
					}
					pa.Parameter[2].I = r;
				}
			}
			m_LineEditor->replaceLine(m_LineNumber, pa);
		}
		else
		{
			m_MouseMovingWidget = 0;
			updatePointerMethod(); // update because update is not done while m_MouseMovingWidget has a value
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
			m_LineEditor->codeEditor()->beginUndoCombine(tr("Move vertex"));
		}
		break;
	case POINTER_EDIT_STACK_SELECT:
		if (m_MouseStackRead.size() > 0)
		{
			if (e->button() == Qt::LeftButton)
			{
				// Select topmost command
				printf("Select topmost command\n");
				int idxDl = m_MouseStackRead[m_MouseStackRead.size() - 1]; // DL
				int idxCmd = m_MainWindow->getDlCmd()[idxDl];
				printf("DL %i\n", idxDl);
				printf("CMD %i\n", idxCmd);
				// m_LineEditor->selectLine(idx);
				if (idxCmd >= 0)
				{
					m_MainWindow->focusCmdEditor();
					m_MainWindow->cmdEditor()->selectLine(idxCmd);
				}
				else
				{
					m_MainWindow->focusDlEditor();
					m_MainWindow->dlEditor()->selectLine(idxDl);
				}
/*
 *
CLEAR_COLOR_RGB(50, 80, 160)
CLEAR(1, 1, 1)
CALL(100)
CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
*
BEGIN(RECTS)
VERTEX2II(100, 100, 0, 0)
VERTEX2II(220, 150, 0, 0)
END()
RETURN()
 *
 */
			}
		}
		break;
	case POINTER_INSERT:
		if (e->button() == Qt::LeftButton)
		{
			int line = m_LineNumber;
			DlParsed pa = m_LineEditor->getLine(line);
			if (isValidInsert(pa))
			{
				++line;
				pa.Parameter[0].I = e->pos().x();
				pa.Parameter[1].I = e->pos().y();
				m_LineEditor->insertLine(line, pa);
				m_LineEditor->selectLine(line);
			}
			break;
		}
		else if (e->button() == Qt::MidButton
			|| e->button() == Qt::RightButton)
		{
			m_Insert->setChecked(false);
		}
	default:
		if (m_PointerMethod & POINTER_EDIT_WIDGET_MOVE)
		{
			// Works for any widget move action
			m_MovingLastX = e->pos().x();
			m_MovingLastY = e->pos().y();
			m_MouseMovingWidget = m_PointerMethod;
			m_LineEditor->codeEditor()->beginUndoCombine(tr("Move widget"));
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
	else if (m_MouseMovingWidget)
	{
		m_MouseMovingWidget = 0;
		if (m_LineEditor)
		{
			m_LineEditor->codeEditor()->endUndoCombine();
		}
		updatePointerMethod(); // update because update is not done while m_MouseMovingWidget has a value
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

bool InteractiveViewport::acceptableSource(QDropEvent *e)
{
	if (e->source() == m_MainWindow->toolbox()->treeWidget()) return true;
	if (e->source() == m_MainWindow->bitmapSetup()) return true;
	if (e->source() == m_MainWindow->contentManager()->contentList())
	{
		if (!m_MainWindow->contentManager()->current()->MemoryLoaded) return false;
		if (m_MainWindow->contentManager()->current()->Converter != ContentInfo::Image) return false;
		return true;
	}
	return false;
}

void InteractiveViewport::dropEvent(QDropEvent *e)
{
	// Should probably lock the display list at this point ... ?
	// TODO: Bitmaps from files, etc
	if (acceptableSource(e))
	{
		if (m_LineEditor)
		{
			e->accept();

			uint32_t selectionType;
			uint32_t selection;
			uint32_t bitmapHandle;
			ContentInfo *contentInfo;
			if (e->source() == m_MainWindow->toolbox()->treeWidget())
			{
				selectionType = m_MainWindow->toolbox()->getSelectionType();
				selection = m_MainWindow->toolbox()->getSelectionId();
				bitmapHandle = 0;
				contentInfo = NULL;
			}
			else if (e->source() == m_MainWindow->bitmapSetup())
			{
				selectionType = 1; // PRIMITIVE
				selection = BITMAPS;
				bitmapHandle = m_MainWindow->bitmapSetup()->selected();
				contentInfo = NULL;
			}
			else if (e->source() == m_MainWindow->contentManager()->contentList())
			{
				selectionType = 1; // PRIMITIVE
				selection = BITMAPS;
				bitmapHandle = 0;
				contentInfo = m_MainWindow->contentManager()->current();
			}
			else
			{
				printf("Unknown error. This code is unreachable\n");
				return;
			}
			if (selectionType == 1 || selectionType == 2)
			{
				int line = m_LineNumber;
				if (m_LineEditor->getLine(line).ValidId)
				{
					// printf("Valid Id\n");
					if (m_LineEditor->getLine(line).IdLeft == FT800EMU_DL_VERTEX2II
						|| m_LineEditor->getLine(line).IdLeft == FT800EMU_DL_VERTEX2F)
					{
						// Search down for end of vertex set
						for (; line < FT800EMU_DL_SIZE /*&& line < m_LineEditor->codeEditor()->document()->blockCount()*/; ++line)
						{
							printf("line %i\n", line);
							const DlParsed &pa = m_LineEditor->getLine(line);
							if (!pa.ValidId)
							{
								break;
							}
							if (pa.IdLeft == 0)
							{
								if (pa.IdRight == FT800EMU_DL_END)
								{
									++line;
									break;
								}
								else if (pa.IdRight == FT800EMU_DL_BEGIN
									|| pa.IdRight == FT800EMU_DL_RETURN
									|| pa.IdRight == FT800EMU_DL_JUMP)
								{
									break;
								}
							}
						}
					}
					else
					{
						// Write after current line
						++line;
					}
				}

				// Skip past CLEAR commands
				while (m_LineEditor->getLine(line).ValidId && m_LineEditor->getLine(line).IdLeft == 0 && (
					m_LineEditor->getLine(line).IdRight == FT800EMU_DL_CLEAR
					|| m_LineEditor->getLine(line).IdRight == FT800EMU_DL_CLEAR_COLOR_RGB
					|| m_LineEditor->getLine(line).IdRight == FT800EMU_DL_CLEAR_COLOR_A
					|| m_LineEditor->getLine(line).IdRight == FT800EMU_DL_CLEAR_STENCIL
					|| m_LineEditor->getLine(line).IdRight == FT800EMU_DL_CLEAR_TAG
					))
				{
					++line;
				}

				// printf("Dropped item from toolbox, type %i\n", selection);

				// void insertLine(int line, const DlParsed &parsed);
				if ((selection & 0xFFFFFF00) == 0xFFFFFF00) // Coprocessor
				{
					m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop coprocessor widget");
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0xFFFFFF00;
					pa.IdRight = selection & 0xFF;
					pa.Parameter[0].I = e->pos().x();
					pa.Parameter[1].I = e->pos().y();
					pa.ExpectedStringParameter = false;
					switch (selection)
					{
					case CMD_TEXT:
						pa.Parameter[2].U = 28;
						pa.Parameter[3].U = 0;
						pa.StringParameter = "Text";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 5;
						break;
					case CMD_BUTTON:
						pa.Parameter[2].U = 120;
						pa.Parameter[3].U = 36;
						pa.Parameter[4].U = 27;
						pa.Parameter[5].U = 0;
						pa.StringParameter = "Button";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_KEYS:
						pa.Parameter[2].U = 160;
						pa.Parameter[3].U = 36;
						pa.Parameter[4].U = 29;
						pa.Parameter[5].U = 0;
						pa.StringParameter = "keys";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_PROGRESS:
						pa.Parameter[2].U = 180;
						pa.Parameter[3].U = 12;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = 20;
						pa.Parameter[6].U = 100;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_SLIDER:
						pa.Parameter[2].U = 80;
						pa.Parameter[3].U = 8;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = 30;
						pa.Parameter[6].U = 100;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_SCROLLBAR:
						pa.Parameter[2].U = 16;
						pa.Parameter[3].U = 160;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = 120;
						pa.Parameter[6].U = 60;
						pa.Parameter[7].U = 480;
						pa.ExpectedParameterCount = 8;
						break;
					case CMD_TOGGLE:
						pa.Parameter[2].U = 40;
						pa.Parameter[3].U = 27;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = 0;
						pa.StringParameter = "on\xFFoff";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_GAUGE:
						pa.Parameter[2].U = 36;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = 4;
						pa.Parameter[5].U = 8;
						pa.Parameter[6].U = 40;
						pa.Parameter[7].U = 100;
						pa.ExpectedParameterCount = 8;
						break;
					case CMD_CLOCK:
						pa.Parameter[2].U = 36;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = 13;
						pa.Parameter[5].U = 51;
						pa.Parameter[6].U = 17;
						pa.Parameter[7].U = 0;
						pa.ExpectedParameterCount = 8;
						break;
					case CMD_DIAL:
						pa.Parameter[2].U = 36;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = 6144;
						pa.ExpectedParameterCount = 5;
						break;
					case CMD_NUMBER:
						pa.Parameter[2].U = 28;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = 42;
						pa.ExpectedParameterCount = 5;
						break;
					case CMD_SPINNER:
						pa.Parameter[2].U = 0;
						pa.Parameter[3].U = 0;
						pa.ExpectedParameterCount = 4;
						break;
	/*

	CMD_TEXT(19, 23, 28, 0, "Text")
	CMD_BUTTON(15, 59, 120, 36, 27, 0, "Button")
	CMD_KEYS(14, 103, 160, 36, 29, 0, "keys")
	CMD_PROGRESS(15, 148, 180, 12, 0, 20, 100)
	CMD_SLIDER(15, 174, 80, 8, 0, 30, 100)
	CMD_SCROLLBAR(426, 53, 16, 160, 0, 120, 60, 480)
	CMD_TOGGLE(22, 199, 40, 27, 0, 0, "on\\xFFoff")
	CMD_GAUGE(274, 55, 36, 0, 4, 8, 40, 100)
	CMD_CLOCK(191, 55, 36, 0, 13, 51, 17, 0)
	CMD_DIAL(356, 55, 36, 0, 6144)
	CMD_NUMBER(14, 233, 28, 0, 42)
	CMD_SPINNER(305, 172, 0, 0)

	*/
					}
					m_LineEditor->insertLine(line, pa);
					m_LineEditor->selectLine(line);
					m_LineEditor->codeEditor()->endUndoCombine();
				}
				else if (selectionType == 2)
				{
					m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop background");
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0;
					pa.IdRight = selection;
					pa.ExpectedStringParameter = false;
					switch (selection)
					{
					case FT800EMU_DL_CLEAR:
						pa.Parameter[0].U = 1;
						pa.Parameter[1].U = 1;
						pa.Parameter[2].U = 1;
						pa.ExpectedParameterCount = 3;
						break;
					}
					m_LineEditor->insertLine(line, pa);
					m_LineEditor->selectLine(line);
					m_LineEditor->codeEditor()->endUndoCombine();
				}
				else if (selectionType == 1) // Primitive
				{
					bool mustCreateHandle = true;
					if (contentInfo)
					{
						printf("Find or create handle for image content\n");
						const ContentInfo *const *bitmapSources = m_MainWindow->bitmapSetup()->getBitmapSources();
						for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
						{
							if (bitmapSources[i] == contentInfo)
							{
								bitmapHandle = i;
								mustCreateHandle = false;
								break;
							}
						}
						if (mustCreateHandle)
						{
							mustCreateHandle = false;
							for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
							{
								if (bitmapSources[i] == NULL)
								{
									bitmapHandle = i;
									mustCreateHandle = true;
									break;
								}
							}
							if (!mustCreateHandle)
							{
								printf("No free handle available\n");
								PropertiesEditor *props = m_MainWindow->propertiesEditor();
								props->setInfo(tr("<b>Error</b>: No free bitmap handle available"));
								props->setEditWidget(NULL, false, NULL);
								m_MainWindow->focusProperties();
								return;
							}
						}
					}
					m_MainWindow->undoStack()->beginMacro(tr("Drag and drop primitive"));
					if (contentInfo && mustCreateHandle)
					{
						printf("Create handle for image content\n");
						m_MainWindow->bitmapSetup()->changeSourceContent(bitmapHandle, contentInfo);
					}
					m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop primitive");
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0;
					pa.IdRight = FT800EMU_DL_BEGIN;
					pa.Parameter[0].U = selection;
					m_LineEditor->insertLine(line, pa);
					++line;
					pa.IdLeft = FT800EMU_DL_VERTEX2II;
					pa.IdRight = 0;
					pa.Parameter[0].I = e->pos().x();
					pa.Parameter[1].I = e->pos().y();
					pa.Parameter[2].I = bitmapHandle;
					pa.Parameter[3].I = 0;
					m_LineEditor->insertLine(line, pa);
					++line;
					pa.IdLeft = 0;
					pa.IdRight = FT800EMU_DL_END;
					m_LineEditor->insertLine(line, pa);
					m_LineEditor->selectLine(line - 1);
					m_LineEditor->codeEditor()->endUndoCombine();
					m_MainWindow->undoStack()->endMacro();
					switch (selection)
					{
						case BITMAPS:
						case POINTS:
							break;
						default:
							// Used for linestrip style drawing
							m_Insert->setChecked(true);
							break;
					}
				}
			}
			else if (selectionType == 3 || selectionType == 4)
			{
				int line = m_MouseStackValid ? (m_LineEditor->isCoprocessor() ? m_MouseStackCmdTop : m_MouseStackDlTop) : 0;
				m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop property");
				DlParsed pa;
				pa.ValidId = true;
				pa.ExpectedStringParameter = false;
				if (selectionType == 4)
				{
					pa.IdLeft = 0xFFFFFF00;
					pa.IdRight = selection & 0xFF;
					switch (selection)
					{
					case CMD_BGCOLOR:
						pa.Parameter[0].U = 127;
						pa.Parameter[1].U = 63;
						pa.Parameter[2].U = 31;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_FGCOLOR:
						pa.Parameter[0].U = 255;
						pa.Parameter[1].U = 127;
						pa.Parameter[2].U = 63;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_GRADCOLOR:
						pa.Parameter[0].U = 255;
						pa.Parameter[1].U = 255;
						pa.Parameter[2].U = 127;
						pa.ExpectedParameterCount = 3;
						break;
					}
				}
				else
				{
					pa.IdLeft = 0;
					pa.IdRight = selection;
					switch (selection)
					{
					case FT800EMU_DL_CLEAR_COLOR_RGB:
						pa.Parameter[0].U = 31;
						pa.Parameter[1].U = 63;
						pa.Parameter[2].U = 127;
						pa.ExpectedParameterCount = 3;
						break;
					case FT800EMU_DL_CLEAR_COLOR_A:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_CLEAR_STENCIL:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_CLEAR_TAG:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_COLOR_RGB:
						pa.Parameter[0].U = 255;
						pa.Parameter[1].U = 255;
						pa.Parameter[2].U = 127;
						pa.ExpectedParameterCount = 3;
						break;
					case FT800EMU_DL_COLOR_A:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_COLOR_MASK:
						pa.Parameter[0].U = 1;
						pa.Parameter[1].U = 1;
						pa.Parameter[2].U = 1;
						pa.Parameter[3].U = 1;
						pa.ExpectedParameterCount = 4;
						break;
					case FT800EMU_DL_LINE_WIDTH:
						pa.Parameter[0].U = 64;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_POINT_SIZE:
						pa.Parameter[0].U = 256;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_BLEND_FUNC:
						pa.Parameter[0].U = SRC_ALPHA;
						pa.Parameter[1].U = ONE_MINUS_SRC_ALPHA;
						pa.ExpectedParameterCount = 2;
						break;
					case FT800EMU_DL_SCISSOR_SIZE:
						pa.Parameter[0].U = 512;
						pa.Parameter[1].U = 512;
						pa.ExpectedParameterCount = 2;
						break;
					case FT800EMU_DL_SCISSOR_XY:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case FT800EMU_DL_ALPHA_FUNC:
						pa.Parameter[0].U = ALWAYS;
						pa.Parameter[1].U = ZERO;
						pa.ExpectedParameterCount = 2;
						break;
					case FT800EMU_DL_STENCIL_FUNC:
						pa.Parameter[0].U = ALWAYS;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 255;
						pa.ExpectedParameterCount = 3;
						break;
					case FT800EMU_DL_STENCIL_MASK:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_STENCIL_OP:
						pa.Parameter[0].U = KEEP;
						pa.Parameter[1].U = KEEP;
						pa.ExpectedParameterCount = 2;
						break;
					case FT800EMU_DL_TAG:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FT800EMU_DL_TAG_MASK:
						pa.Parameter[0].U = 1;
						pa.ExpectedParameterCount = 1;
						break;
					}
				}
				m_LineEditor->insertLine(line, pa);
				m_LineEditor->selectLine(line);
				m_LineEditor->codeEditor()->endUndoCombine();
			}
		}
		else
		{
			printf("Warning: No line editor\n");
		}
	}
}

void InteractiveViewport::dragMoveEvent(QDragMoveEvent *e)
{
	// TODO: Bitmaps from files, etc
	if (acceptableSource(e))
	{
		if (m_LineEditor)
		{
			m_NextMouseX = e->pos().x();
			m_NextMouseY = e->pos().y();
			m_DragMoving = true;

			e->acceptProposedAction();
		}
		else
		{
			printf("Warning: No line editor\n");
		}
	}
	else
	{
		printf("Unknown dragMoveEvent from %p\n", e->source());
	}
}
void InteractiveViewport::dragEnterEvent(QDragEnterEvent *e)
{
	// TODO: Bitmaps from files, etc
	if (acceptableSource(e))
	{
		if (m_LineEditor)
		{
			e->acceptProposedAction();
		}
		else
		{
			printf("Warning: No line editor\n");
		}
	}
	else
	{
		printf("Unknown dragEnterEvent from %p\n", e->source());
	}
}

} /* namespace FT800EMUQT */

/* end of file */
