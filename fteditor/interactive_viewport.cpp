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

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

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
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QStatusBar>
#include <QActionGroup>
#include <QRegularExpression>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSinglePointEvent>
#include <QMenu>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "code_editor.h"
#include "constant_common.h"
#include "constant_mapping.h"
#include "content_manager.h"
#include "script/Script.h"
#include "inspector/Inspector.h"
#include "main_window.h"
#include "properties_editor.h"
#include "toolbox.h"
#include "utils/DLUtil.h"
#include "utils/LoggerUtil.h"
#include "utils/ReadWriteUtil.h"

namespace FTEDITOR {

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

extern QMutex g_ViewportMutex;

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
#define POINTER_EDIT_GRADIENT_MOVE_1 0x0400
#define POINTER_EDIT_GRADIENT_MOVE_2 0x0800
#define POINTER_EDIT_GRADIENT_MOVE (POINTER_EDIT_GRADIENT_MOVE_1 | POINTER_EDIT_GRADIENT_MOVE_2)

// Value outside display space
#define FTED_SNAP_HISTORY_NONE 4096

static QVector<float> ZoomRange({ 6.25, 12.5, 25, 37.5, 50, 62.5, 75, 100, 125, 150, 200, 300, 400, 600, 800, 1200, 1600 });
static const int ZOOM_INIT_INDEX = 7; // index 7 means zooming at 100%

InteractiveViewport::InteractiveViewport(MainWindow *parent)
    : EmulatorViewport(parent, parent->applicationDataDir())
    , m_MainWindow(parent)
    , m_PreferTraceCursor(false)
    , m_TraceEnabled(false)
    , m_MouseOver(false)
    , m_MouseStackWritten(false)
    , m_MouseStackValid(false)
    , m_PointerFilter(POINTER_ALL)
    , m_PointerMethod(0)
    , m_MouseTouch(false)
    , m_MouseOverVertex(false)
    , m_MouseOverVertexLine(-1)
    , m_MouseMovingVertex(false)
    , m_WidgetXY(false)
    , m_WidgetWH(false)
    , m_WidgetR(false)
    , m_WidgetGradient(false)
    , m_MouseMovingWidget(0)
    , m_LineEditor(NULL)
    , m_LineNumber(0)
    , m_DrawMultipleSelection(false)
    , m_SnapHistoryCur(0)
    , m_IsDrawAlgn(false)
    , m_selectable(true)
{
	for (int i = 0; i < FTED_SNAP_HISTORY; ++i)
	{
		m_SnapHistoryX[i] = FTED_SNAP_HISTORY_NONE;
		m_SnapHistoryY[i] = FTED_SNAP_HISTORY_NONE;
	}

	// m_Label->setCursor(Qt::PointingHandCursor);
	setMouseTracking(true);
	setAcceptDrops(true);

	QActionGroup *cursorGroup = new QActionGroup(this);

	QAction *automatic = new QAction(cursorGroup);
	connect(automatic, SIGNAL(triggered()), this, SLOT(automaticChecked()));
	automatic->setText(tr("Cursor"));
	automatic->setIcon(QIcon(":/icons/cursor.png"));
	automatic->setStatusTip(tr("Context dependent cursor"));
	automatic->setCheckable(true);
	automatic->setChecked(true);
	automatic->setShortcut(Qt::ALT | Qt::Key_C);

	QAction *touch = new QAction(cursorGroup);
	connect(touch, SIGNAL(triggered()), this, SLOT(touchChecked()));
	touch->setText(tr("Touch"));
	touch->setIcon(QIcon(":/icons/hand-point-090.png"));
	touch->setStatusTip(tr("Use to cursor to touch the emulated display"));
	touch->setCheckable(true);
	touch->setShortcut(Qt::ALT | Qt::Key_T);

	QAction *trace = new QAction(cursorGroup);
	connect(trace, SIGNAL(triggered()), this, SLOT(traceChecked()));
	trace->setText(tr("Trace"));
	trace->setIcon(QIcon(":/icons/trace.png"));
	trace->setStatusTip(tr("Select a pixel to trace display commands"));
	trace->setCheckable(true);
	trace->setShortcut(Qt::ALT | Qt::Key_R);

	QAction *edit = new QAction(cursorGroup);
	connect(edit, SIGNAL(triggered()), this, SLOT(editChecked()));
	edit->setText(tr("Edit"));
	edit->setIcon(QIcon(":/icons/arrow-move.png"));
	edit->setStatusTip(tr("Interactive editing tools"));
	edit->setCheckable(true);
	edit->setShortcut(Qt::ALT | Qt::Key_E);

	QToolBar *cursorToolBar = m_MainWindow->addToolBar(tr("Cursor"));
	cursorToolBar->setIconSize(QSize(16, 16));
	cursorToolBar->addAction(automatic);
	cursorToolBar->addSeparator();
	cursorToolBar->addAction(touch);
	cursorToolBar->addAction(trace);
	cursorToolBar->addAction(edit);

	// TODO: Sexy toolbar icons.

	// icon something
	m_Insert = new QAction(this);
	m_Insert->setText(tr("Insert"));
	m_Insert->setIcon(QIcon(":/icons/layer--plus.png"));
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
	toolBar->setIconSize(QSize(16, 16));
	toolBar->addAction(m_Insert);
	// toolBar->addAction(decrease);
	// toolBar->addAction(increase);

	m_ActionRuler = new QAction;
	m_ActionRuler->setChecked(true);
	m_ActionRuler->setCheckable(true);
	m_ActionRuler->setText(tr("Ruler"));
	m_ActionRuler->setIconVisibleInMenu(false);
	m_ActionRuler->setIcon(QIcon(":/icons/ruler.png"));
	m_ActionRuler->setStatusTip(tr("Show/Hide the ruler"));
	connect(m_ActionRuler, &QAction::triggered, this, &EmulatorViewport::toggleViewRuler);

	m_ActionAlgn = new QAction;
	m_ActionAlgn->setChecked(true);
	m_ActionAlgn->setCheckable(true);
	m_ActionAlgn->setText(tr("Alignment"));
	m_ActionAlgn->setIconVisibleInMenu(false);
	m_ActionAlgn->setIcon(QIcon(":/icons/align.png"));
	m_ActionAlgn->setStatusTip(tr("Show/Hide alignment"));

	QToolBar *toolBarOthers = m_MainWindow->addToolBar("Others");
	toolBarOthers->setIconSize(QSize(16, 16));
	toolBarOthers->addAction(m_ActionRuler);
	toolBarOthers->addSeparator();
	toolBarOthers->addAction(m_ActionAlgn);

	QAction *zoomIn = new QAction(this);
	connect(zoomIn, &QAction::triggered, this, &InteractiveViewport::zoomIn);
	zoomIn->setText(tr("Zoom In"));
	zoomIn->setIcon(QIcon(":/icons/magnifier-zoom-in.png"));
	zoomIn->setStatusTip(tr("Zoom In"));

	QAction *zoomOut = new QAction(this);
	connect(zoomOut, &QAction::triggered, this, &InteractiveViewport::zoomOut);
	zoomOut->setText(tr("Zoom Out"));
	zoomOut->setIcon(QIcon(":/icons/magnifier-zoom-out.png"));
	zoomOut->setStatusTip(tr("Zoom Out"));

	m_ZoomCB = new QComboBox(this);
	m_ZoomCB->setEditable(true);
	m_ZoomCB->setInsertPolicy(QComboBox::NoInsert);

	m_ZoomCB->lineEdit()->installEventFilter(this);

	connect(m_ZoomCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &InteractiveViewport::zoomChanged);
	connect(m_ZoomCB->lineEdit(), &QLineEdit::returnPressed, this, &InteractiveViewport::zoomEditTextChanged);
	connect(m_ZoomCB, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::textActivated),
	    [=](const QString &text) { m_ZoomCB->lineEdit()->selectAll(); });

	QStringList zoomList;

	for (int i = 0; i < ZoomRange.count(); i++)
	{
		zoomList << QString("%1%").arg(ZoomRange[i]);
	}

	m_ZoomCB->addItems(zoomList);
	m_ZoomCB->setCurrentIndex(ZOOM_INIT_INDEX);
	m_ZoomCB->setMaxVisibleItems(zoomList.count());

	QRegularExpression rx("(\\d{1,4}\\.)?\\d{1,4}%?");
	QValidator *validator = new QRegularExpressionValidator(rx, this);
	m_ZoomCB->setValidator(validator);

	QToolBar *zoomToolBar = m_MainWindow->addToolBar(tr("Zoom"));
	zoomToolBar->setIconSize(QSize(16, 16));
	zoomToolBar->addAction(zoomOut);
	zoomToolBar->addWidget(m_ZoomCB);
	zoomToolBar->addAction(zoomIn);

	m_ContextMenu = new QMenu(this);
	m_ContextMenu->addAction(m_ActionRuler);
	m_ContextMenu->addAction(m_ActionAlgn);

	connect(this, &QWidget::customContextMenuRequested, this,
	    [this](const QPoint &pos) {
		    m_ContextMenu->exec(this->mapToGlobal(pos));
	    });
	
	ComponentBase::finishedSetup(this, m_MainWindow);
}

QAction *InteractiveViewport::ActionRuler() const
{
	return m_ActionRuler;
}

InteractiveViewport::~InteractiveViewport()
{
}

void InteractiveViewport::setupConnections(QObject *obj)
{
	if (auto script = m_MainWindow->script();
	    script && (obj == script || obj == nullptr))
	{
		connect(script, &Script::started, this, [this]() {
			m_selectable = false;
		});
		connect(script, &Script::finished, this, [this]() {
			m_selectable = true;
		});
	}
}

void InteractiveViewport::zoomIn()
{
	int scaleFactor = screenScale() * 100;
	float currentScale = scaleFactor / 16.0;

	// find next index
	for (int i = 0; i < ZoomRange.count(); i++)
	{
		if (currentScale < ZoomRange[i])
		{
			m_ZoomCB->setCurrentIndex(i);
			break;
		}
	}
}

void InteractiveViewport::zoomOut()
{
	int scaleFactor = screenScale() * 100;
	float currentScale = (float)scaleFactor / 16.0f;

	// find previous index
	for (int i = ZoomRange.count() - 1; i >= 0; i--)
	{
		if (currentScale > ZoomRange[i])
		{
			m_ZoomCB->setCurrentIndex(i);
			break;
		}
	}
}

void InteractiveViewport::zoomChanged(int index)
{
	if (index < 0 || index >= ZoomRange.count())
		return;

	// zoom 100% equal to scale = 16
	setScreenScale(ZoomRange[index] * 0.16);
}

void InteractiveViewport::zoomEditTextChanged()
{
	bool ok = false;
	QString text = m_ZoomCB->lineEdit()->text();
	float zoom = text.remove('%').toFloat(&ok);

	if (zoom < ZoomRange.first()) zoom = ZoomRange.first();
	if (zoom > ZoomRange.last()) zoom = ZoomRange.last();

	if (!ok) return;

	text.append("%");
	m_ZoomCB->lineEdit()->setText(text);

	// zoom 100% equal to scale = 16
	setScreenScale(zoom * 0.16);
}

bool InteractiveViewport::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_ZoomCB->lineEdit() && event->type() == QEvent::MouseButtonPress)
	{
		m_ZoomCB->lineEdit()->selectAll();
		return true;
	}

	return EmulatorViewport::eventFilter(watched, event);
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
		if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II
		    || parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
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
			case CMD_CSKETCH:
				return FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801; // Deprecated in FT810
			case CMD_ANIMFRAME:
				return FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815;
			case CMD_ANIMSTARTRAM:
			case CMD_ANIMFRAMERAM:
			case CMD_RUNANIM:
			case CMD_WAIT:
			case CMD_NEWLIST:
			case CMD_ENDLIST:
			case CMD_CALLLIST:
			case CMD_RETURN:
			case CMD_APILEVEL:
			case CMD_CALIBRATESUB:
			case CMD_TESTCARD:
			case CMD_FONTCACHE:
			case CMD_FONTCACHEQUERY:
			case CMD_GETIMAGE:
				return FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT817;
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
	m_TraceStackDl.clear();
	m_TraceStackCmd.clear();
	bool cmdLast = false;
	if (m_TraceEnabled)
	{
		m_TraceStackSize = FTEDITOR_TRACE_STACK_SIZE;
		BT8XXEMU_processTrace(g_Emulator, m_TraceStack, &m_TraceStackSize, m_TraceX, m_TraceY, hsize());
		for (int i = 0; i < m_TraceStackSize; ++i)
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
			int size = FTEDITOR_TRACE_STACK_SIZE;
			m_MouseStackWrite.resize(FTEDITOR_TRACE_STACK_SIZE);
			BT8XXEMU_processTrace(g_Emulator, &m_MouseStackWrite[0], &size, m_NextMouseX, m_NextMouseY, hsize());
			m_MouseStackWrite.resize(size);
			if (m_MouseStackWrite.size())
			{
				m_MouseStackDlTop = m_MouseStackWrite[m_MouseStackWrite.size() - 1];
				m_MouseStackCmdTop = m_MainWindow->getDlCmd()[m_MouseStackDlTop];
				m_MouseStackValid = true;
			}
			else m_MouseStackValid = false;
			m_MouseStackWritten = true;
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
	return; // This callback can be used to make the navview
}

void InteractiveViewport::paintEvent(QPaintEvent *e)
{
	horizontalRuler()->setScale(screenScale());
	horizontalRuler()->setScreenLeft(screenLeft());
	horizontalRuler()->update();

	verticalRuler()->setScale(screenScale());
	verticalRuler()->setScreenTop(screenTop());
	verticalRuler()->update();

	EmulatorViewport::paintEvent(e);
	QPainter p(this);

	// Update frame dependent gui
	g_ViewportMutex.lock();
	m_MainWindow->dlEditor()->codeEditor()->setTraceHighlights(m_TraceStackDl);
	m_MainWindow->cmdEditor()->codeEditor()->setTraceHighlights(m_TraceStackCmd);
	m_MouseX = m_NextMouseX;
	m_MouseY = m_NextMouseY;
	if (m_MouseStackWritten)
	{
		m_MouseStackRead.swap(m_MouseStackWrite);
		m_MouseStackWritten = false;
	}
	g_ViewportMutex.unlock();

	int mvx = screenLeft();
	int mvy = screenTop();
	int scl = screenScale();
#define TFX(x) ((((x)*scl) / 16) + mvx)
#define TFY(y) ((((y)*scl) / 16) + mvy)
#define SCX(x) ((((x)*scl) / 16))
#define SCY(y) ((((y)*scl) / 16))
// #define UNTFX(x) ((((x - mvx) * 16) / scl))
// #define UNTFY(y) ((((y - mvy) * 16) / scl))
#define DRAWLINE(x1, y1, x2, y2) p.drawLine(TFX(x1), TFY(y1), TFX(x2), TFY(y2))
#define DRAWRECT(x, y, w, h) p.drawRect(TFX(x), TFY(y), SCX(w), SCY(h))
#define EPOSPOINT(e) (e->position().toPoint())

	if (m_MouseTouch && m_MouseX >= 0 && m_MouseX < getPixMap().width() && m_MouseY >= 0 && m_MouseY < getPixMap().height())
	{
		BT8XXEMU_touchSetXY(g_Emulator, 0, m_MouseX, m_MouseY, 0);
	}
	else
	{
		BT8XXEMU_touchResetXY(g_Emulator, 0);
	}

	// Draw image overlays
	/*QPainter p;
	p.begin(image);*/
	if (m_LineEditor && m_selectable)
	{
		auto drawAlignment = [&](DlParsed &parsed, int specX = -1, int specY = -1) {
#define COLOR_FIT_VERTICALLY Qt::green
#define COLOR_FIT_HORIZONTALLY Qt::cyan
#define COLOR_CLOSE_FIT_VERTICALLY Qt::red
#define COLOR_CLOSE_FIT_HORIZONTALLY Qt::darkYellow
#define DISTANCE 8
#define AUTO_ALIGN_DISTANCE 4
			int x, y;
			DlParsed tempParsed;
			const DlState *tempState = NULL;
			static const QSet<uint32_t> dragableItems = {
				CMD_TEXT, CMD_BUTTON, CMD_KEYS, CMD_PROGRESS,
				CMD_SLIDER, CMD_SCROLLBAR, CMD_TOGGLE, CMD_GAUGE,
				CMD_CLOCK, CMD_SPINNER, CMD_TRACK, CMD_DIAL, CMD_NUMBER,
				CMD_SKETCH, CMD_CSKETCH, CMD_ANIMFRAME, CMD_ANIMFRAMERAM
			};
			static const QList<uint32_t> otherItems = { FTEDITOR_DL_VERTEX2F, FTEDITOR_DL_VERTEX2II };

			bool autoAlignVertical = false, autoAlignHorizontal = false;
			auto drawVerticalAlignment = [&](int x1, int x2) {
				p.setPen(QPen(QBrush(x1 == x2 ? COLOR_FIT_VERTICALLY : COLOR_CLOSE_FIT_VERTICALLY), 1.0, Qt::DashLine));
				if (abs(x1 - x2) > DISTANCE)
					return false;
				DRAWLINE(x2, 0, x2, vsize());
				return true;
			};
			auto drawHorizontalAlignment = [&](int y1, int y2) {
				p.setPen(QPen(QBrush(y1 == y2 ? COLOR_FIT_HORIZONTALLY : COLOR_CLOSE_FIT_HORIZONTALLY), 1.0, Qt::DashLine));
				if (abs(y1 - y2) > DISTANCE)
					return false;
				DRAWLINE(0, y2, hsize(), y2);
				return true;
			};
			auto checkClosest = [](int &closest, int &minDelta, int value1, int value2) {
				int newDelta = abs(value1 - value2);
				if (newDelta < minDelta || minDelta < 0)
				{
					closest = value2;
					minDelta = newDelta;
				}
			};
			auto getClosestVertical = [&](int &minDelta, int &closest) {
				if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2F)
				{
					int tempX = tempParsed.Parameter[0].I >> tempState->Graphics.VertexFormat;
					tempX += tempState->Graphics.VertexTranslateX >> 4;
					checkClosest(closest, minDelta, x, tempX);
					return;
				}
				if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2II)
				{
					int tempX = tempParsed.Parameter[0].U;
					tempX += tempState->Graphics.VertexTranslateX >> 4;
					checkClosest(closest, minDelta, x, tempX);
					return;
				}

				bool tempM_WidgetWH = false;
				switch (tempParsed.IdRight | 0xFFFFFF00)
				{
				case CMD_BUTTON:
				case CMD_KEYS:
				case CMD_PROGRESS:
				case CMD_SLIDER:
				case CMD_SCROLLBAR:
				case CMD_TRACK:
				case CMD_SKETCH:
				case CMD_CSKETCH:
					tempM_WidgetWH = true;
				}
				if (tempM_WidgetWH)
				{
					checkClosest(closest, minDelta, x, tempParsed.Parameter[0].I + tempParsed.Parameter[2].I / 2); // mid
				}
				checkClosest(closest, minDelta, x, tempParsed.Parameter[0].I); // left
				if (tempM_WidgetWH)
				{
					checkClosest(closest, minDelta, x, tempParsed.Parameter[0].I + tempParsed.Parameter[2].I); // right
				}
			};
			auto getClosestHorizontal = [&](int &minDelta, int &closest) {
				if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2F)
				{
					int tempY = tempParsed.Parameter[1].I >> tempState->Graphics.VertexFormat;
					tempY += tempState->Graphics.VertexTranslateY >> 4;
					checkClosest(closest, minDelta, y, tempY);
					return;
				}
				if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2II)
				{
					int tempY = tempParsed.Parameter[1].U;
					tempY += tempState->Graphics.VertexTranslateY >> 4;
					checkClosest(closest, minDelta, y, tempY);
					return;
				}

				bool tempM_WidgetWH = false;
				switch (tempParsed.IdRight | 0xFFFFFF00)
				{
				case CMD_BUTTON:
				case CMD_KEYS:
				case CMD_PROGRESS:
				case CMD_SLIDER:
				case CMD_SCROLLBAR:
				case CMD_TRACK:
				case CMD_SKETCH:
				case CMD_CSKETCH:
					tempM_WidgetWH = true;
				}
				if (tempM_WidgetWH)
				{
					checkClosest(closest, minDelta, y, tempParsed.Parameter[1].I + tempParsed.Parameter[3].I / 2); // mid
				}
				checkClosest(closest, minDelta, y, tempParsed.Parameter[1].I); // top
				if (tempM_WidgetWH)
				{
					checkClosest(closest, minDelta, y, tempParsed.Parameter[1].I + tempParsed.Parameter[3].I); // bottom
				}
			};

			for (int i = 0; i < FTEDITOR_DL_SIZE && i < m_LineEditor->getLineCount() ; i++)
			{
				if (i == m_LineNumber)
					continue;
				tempParsed = m_LineEditor->getLine(i);
				int tempRightID = tempParsed.IdRight | 0xFFFFFF00;
				if (!dragableItems.contains(tempRightID) && !otherItems.contains(tempParsed.IdLeft))
					continue;

				tempState = &m_LineEditor->getState(i);
				int selPos = 0, closestOfSelPos = 0, closestOfPos = -1;
				int minDelta = -1, delta = -1;
				auto setSelPos = [&](int pos, int closestOfPos, int delta) {
					selPos = pos;
					closestOfSelPos = closestOfPos;
					minDelta = delta;
				};

				if (m_WidgetWH)
				{
					x = specX != -1 ? specX : parsed.Parameter[0].I + parsed.Parameter[2].I / 2; // mid of the main item
					getClosestVertical(delta, closestOfPos);
					setSelPos(x, closestOfPos, delta);
				}
				if (true)
				{
					x = specX != -1 ? specX : parsed.Parameter[0].I; // left of the main item
					getClosestVertical(delta, closestOfPos);
					if (delta < minDelta || minDelta < 0)
					{
						setSelPos(x, closestOfPos, delta);
					}
				}
				if (m_WidgetWH)
				{
					x = specX != -1 ? specX : parsed.Parameter[0].I + parsed.Parameter[2].I; // right of the main item
					getClosestVertical(delta, closestOfPos);
					if (delta < minDelta || minDelta < 0)
					{
						setSelPos(x, closestOfPos, delta);
					}
				}
				// Auto align vertical
				if (abs(selPos - closestOfSelPos) < AUTO_ALIGN_DISTANCE && !autoAlignVertical)
				{
					int delta1 = closestOfSelPos - selPos;
					selPos = closestOfSelPos;
					autoAlignVertical = true;
					mouseMoveEvent(m_MovingLastX + delta1, m_MovingLastY);
				}
				drawVerticalAlignment(selPos, closestOfSelPos);

				delta = -1;
				closestOfPos = 0;
				setSelPos(0, -1, -1);
				if (m_WidgetWH)
				{
					y = specY != -1
					    ? specY
					    : parsed.Parameter[1].I + parsed.Parameter[3].I / 2; // mid of the main item
					getClosestHorizontal(delta, closestOfPos);
					setSelPos(y, closestOfPos, delta);
				}
				if (true)
				{
					y = specY != -1 ? specY
					                : parsed.Parameter[1].I; // left of the main item
					getClosestHorizontal(delta, closestOfPos);
					if (delta < minDelta || minDelta < 0)
					{
						setSelPos(y, closestOfPos, delta);
					}
				}
				if (m_WidgetWH)
				{
					y = specY != -1
					    ? specY
					    : parsed.Parameter[1].I + parsed.Parameter[3].I; // right of the main item
					getClosestHorizontal(delta, closestOfPos);
					if (delta < minDelta || minDelta < 0)
					{
						setSelPos(y, closestOfPos, delta);
					}
				}
				// Auto align horizontal
				if (abs(selPos - closestOfSelPos) < AUTO_ALIGN_DISTANCE && !autoAlignHorizontal)
				{
					int delta2 = closestOfSelPos - selPos;
					selPos = closestOfSelPos;
					autoAlignHorizontal = true;
					mouseMoveEvent(m_MovingLastX, m_MovingLastY + delta2);
				}
				drawHorizontalAlignment(selPos, closestOfSelPos);
			}
		};

		for (auto &line : m_SelectedLines)
		{
			DlParsed parsed = m_LineEditor->getLine(line);
			if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F || parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
			{
				m_WidgetXY = false;
				m_WidgetGradient = false;
				if (m_PointerFilter & POINTER_EDIT_VERTEX_MOVE)
				{
					QPen outer;
					QPen inner;
					outer.setWidth(3);
					outer.setColor(QColor(Qt::black));
					inner.setWidth(1);
					inner.setColor(QColor(Qt::gray));
					// Find first line
					int firstLine = line;
					for (int l = firstLine - 1; l > 0; --l)
					{
						const DlParsed &pa = m_LineEditor->getLine(l);
						if (pa.IdLeft == 0 && (pa.IdRight == FTEDITOR_DL_BEGIN || pa.IdRight == FTEDITOR_DL_END || pa.IdRight == FTEDITOR_DL_RETURN || pa.IdRight == FTEDITOR_DL_JUMP))
						{
							break;
						}
						else
						{
							firstLine = l;
						}
					}
					// Iterate over neighbouring vertices
					for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l) // FIXME
					{
						if (l == line) continue;
						const DlParsed &pa = m_LineEditor->getLine(l);
						const DlState &st = m_LineEditor->getState(l);
						if (pa.IdLeft == FTEDITOR_DL_VERTEX2F || pa.IdLeft == FTEDITOR_DL_VERTEX2II)
						{
							int x, y;
							if (pa.IdLeft == FTEDITOR_DL_VERTEX2F)
							{
								x = pa.Parameter[0].I >> st.Graphics.VertexFormat;
								y = pa.Parameter[1].I >> st.Graphics.VertexFormat;
							}
							else
							{
								x = pa.Parameter[0].U;
								y = pa.Parameter[1].U;
							}
							x += st.Graphics.VertexTranslateX >> 4;
							y += st.Graphics.VertexTranslateY >> 4;
							p.setPen(outer);
							DRAWLINE(x - 4, y - 4, x + 4, y - 4);
							DRAWLINE(x - 4, y + 4, x + 4, y + 4);
							DRAWLINE(x - 4, y - 4, x - 4, y + 4);
							DRAWLINE(x + 4, y - 4, x + 4, y + 4);
							p.setPen(inner);
							DRAWLINE(x - 4, y - 4, x + 4, y - 4);
							DRAWLINE(x - 4, y + 4, x + 4, y + 4);
							DRAWLINE(x - 4, y - 4, x - 4, y + 4);
							DRAWLINE(x + 4, y - 4, x + 4, y + 4);
						}
						else if (pa.IdRight == FTEDITOR_DL_BEGIN || pa.IdRight == FTEDITOR_DL_END || pa.IdRight == FTEDITOR_DL_RETURN || pa.IdRight == FTEDITOR_DL_JUMP)
						{
							break;
						}
					}
					const DlState &state = m_LineEditor->getState(line);
					// Show central vertex
					int x, y;

					if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
					{
						x = parsed.Parameter[0].I >> state.Graphics.VertexFormat;
						y = parsed.Parameter[1].I >> state.Graphics.VertexFormat;
					}
					else
					{
						x = parsed.Parameter[0].U;
						y = parsed.Parameter[1].U;
					}

					x += state.Graphics.VertexTranslateX >> 4;
					y += state.Graphics.VertexTranslateY >> 4;

					if (m_IsDrawAlgn) drawAlignment(parsed, x, y);

					p.setPen(outer);
					DRAWLINE(x, y - 5, x, y - 12);
					DRAWLINE(x, y + 5, x, y + 12);
					DRAWLINE(x - 5, y, x - 12, y);
					DRAWLINE(x + 5, y, x + 12, y);
					DRAWLINE(x - 4, y - 4, x + 4, y - 4);
					DRAWLINE(x - 4, y + 4, x + 4, y + 4);
					DRAWLINE(x - 4, y - 4, x - 4, y + 4);
					DRAWLINE(x + 4, y - 4, x + 4, y + 4);
					inner.setColor(QColor(Qt::red));
					p.setPen(inner);
					DRAWLINE(x, y - 5, x, y - 12);
					DRAWLINE(x, y + 5, x, y + 12);
					DRAWLINE(x - 5, y, x - 12, y);
					DRAWLINE(x + 5, y, x + 12, y);
					DRAWLINE(x - 4, y - 4, x + 4, y - 4);
					DRAWLINE(x - 4, y + 4, x + 4, y + 4);
					DRAWLINE(x - 4, y - 4, x - 4, y + 4);
					DRAWLINE(x + 4, y - 4, x + 4, y + 4);

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
			else if (parsed.IdLeft == 0xFFFFFF00 && (((parsed.IdRight | 0xFFFFFF00) == CMD_GRADIENT) || ((parsed.IdRight | 0xFFFFFF00) == CMD_GRADIENTA)))
			{
				const DlState &state = m_LineEditor->getState(line);

				m_WidgetXY = false;
				m_WidgetGradient = true;

				// TODO: Cleanup drawing code
				QPen outer;
				QPen inner;
				outer.setWidth(3);
				outer.setColor(QColor(Qt::black));
				inner.setWidth(1);
				inner.setColor(QColor(Qt::gray));

				// Show first vertex
				int x, y;
				x = parsed.Parameter[0].I;
				y = parsed.Parameter[1].I;
				x += state.Graphics.VertexTranslateX >> 4;
				y += state.Graphics.VertexTranslateY >> 4;
				p.setPen(outer);
				DRAWLINE(x, y - 5, x, y - 12);
				DRAWLINE(x, y + 5, x, y + 12);
				DRAWLINE(x - 5, y, x - 12, y);
				DRAWLINE(x + 5, y, x + 12, y);
				DRAWLINE(x - 4, y - 4, x + 4, y - 4);
				DRAWLINE(x - 4, y + 4, x + 4, y + 4);
				DRAWLINE(x - 4, y - 4, x - 4, y + 4);
				DRAWLINE(x + 4, y - 4, x + 4, y + 4);
				inner.setColor(QColor(Qt::red));
				p.setPen(inner);
				DRAWLINE(x, y - 5, x, y - 12);
				DRAWLINE(x, y + 5, x, y + 12);
				DRAWLINE(x - 5, y, x - 12, y);
				DRAWLINE(x + 5, y, x + 12, y);
				DRAWLINE(x - 4, y - 4, x + 4, y - 4);
				DRAWLINE(x - 4, y + 4, x + 4, y + 4);
				DRAWLINE(x - 4, y - 4, x - 4, y + 4);
				DRAWLINE(x + 4, y - 4, x + 4, y + 4);

				// Show second vertex
				x = parsed.Parameter[3].I;
				y = parsed.Parameter[4].I;
				x += state.Graphics.VertexTranslateX >> 4;
				y += state.Graphics.VertexTranslateY >> 4;
				p.setPen(outer);
				DRAWLINE(x, y - 5, x, y - 12);
				DRAWLINE(x, y + 5, x, y + 12);
				DRAWLINE(x - 5, y, x - 12, y);
				DRAWLINE(x + 5, y, x + 12, y);
				DRAWLINE(x - 4, y - 4, x + 4, y - 4);
				DRAWLINE(x - 4, y + 4, x + 4, y + 4);
				DRAWLINE(x - 4, y - 4, x - 4, y + 4);
				DRAWLINE(x + 4, y - 4, x + 4, y + 4);
				inner.setColor(QColor(Qt::red));
				p.setPen(inner);
				DRAWLINE(x, y - 5, x, y - 12);
				DRAWLINE(x, y + 5, x, y + 12);
				DRAWLINE(x - 5, y, x - 12, y);
				DRAWLINE(x + 5, y, x + 12, y);
				DRAWLINE(x - 4, y - 4, x + 4, y - 4);
				DRAWLINE(x - 4, y + 4, x + 4, y + 4);
				DRAWLINE(x - 4, y - 4, x - 4, y + 4);
				DRAWLINE(x + 4, y - 4, x + 4, y + 4);
			}
			else if (parsed.IdLeft == 0xFFFFFF00) // Coprocessor
			{
				m_WidgetGradient = false;
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
				case CMD_CSKETCH:
				case CMD_ANIMFRAME:
				case CMD_ANIMFRAMERAM: {
					QPen outer;
					QPen inner;
					outer.setWidth(3);
					outer.setColor(QColor(Qt::black));
					inner.setWidth(1);
					inner.setColor(QColor(Qt::red));
					m_WidgetXY = true;
					isWidgetWHR(parsed, m_WidgetWH, m_WidgetR);

					const DlState &state = m_LineEditor->getState(line);
					if (m_IsDrawAlgn) drawAlignment(parsed);
					int x = parsed.Parameter[0].I;
					int y = parsed.Parameter[1].I;
					x += state.Graphics.VertexTranslateX >> 4;
					y += state.Graphics.VertexTranslateY >> 4;

					if (m_IsDrawAlgnHorizontal)
					{
						p.setPen(QPen(QBrush(Qt::red), 1.0, Qt::DashLine));
						DRAWLINE(0, y, hsize(), y);
					}

					if (m_IsDrawAlgnVertical)
					{
						p.setPen(QPen(QBrush(Qt::red), 1.0, Qt::DashLine));
						DRAWLINE(x, 0, x, vsize());
					}

					// CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
					// Draw...
					if (m_WidgetWH == false && m_WidgetR == false)
					{
						// Only have vertex control
						p.setPen(outer);
						DRAWLINE(x, y - 5, x, y - 12);
						DRAWLINE(x, y + 5, x, y + 12);
						DRAWLINE(x - 5, y, x - 12, y);
						DRAWLINE(x + 5, y, x + 12, y);
						DRAWLINE(x - 4, y - 4, x + 4, y - 4);
						DRAWLINE(x - 4, y + 4, x + 4, y + 4);
						DRAWLINE(x - 4, y - 4, x - 4, y + 4);
						DRAWLINE(x + 4, y - 4, x + 4, y + 4);
						p.setPen(inner);
						DRAWLINE(x, y - 5, x, y - 12);
						DRAWLINE(x, y + 5, x, y + 12);
						DRAWLINE(x - 5, y, x - 12, y);
						DRAWLINE(x + 5, y, x + 12, y);
						DRAWLINE(x - 4, y - 4, x + 4, y - 4);
						DRAWLINE(x - 4, y + 4, x + 4, y + 4);
						DRAWLINE(x - 4, y - 4, x - 4, y + 4);
						DRAWLINE(x + 4, y - 4, x + 4, y + 4);
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
						DRAWRECT(x, y, w, h);
						DRAWRECT(x1 - 1, y1 - 1, 2, 2);
						DRAWRECT(x1 - 1, y2 - 1, 2, 2);
						DRAWRECT(x2 - 1, y2 - 1, 2, 2);
						DRAWRECT(x2 - 1, y1 - 1, 2, 2);
						p.setPen(inner);
						DRAWRECT(x, y, w, h);
						DRAWRECT(x1 - 1, y1 - 1, 2, 2);
						DRAWRECT(x1 - 1, y2 - 1, 2, 2);
						DRAWRECT(x2 - 1, y2 - 1, 2, 2);
						DRAWRECT(x2 - 1, y1 - 1, 2, 2);
					}
					break;
				}
				default: {
					m_WidgetXY = false;
					break;
				}
				}
			}
			else
			{
				m_WidgetXY = false;
				m_WidgetGradient = false;
			}
		}
	}
	if (m_DrawMultipleSelection)
	{
		p.fillRect(m_FirstPoint.x(), m_FirstPoint.y(),
		    m_SecondPoint.x() - m_FirstPoint.x(),
		    m_SecondPoint.y() - m_FirstPoint.y(),
		    QColor(0x99, 0x9d, 0xa3, 0x9F));
	}
	if (m_TraceEnabled)
	{
		QPen outer;
		QPen inner;
		outer.setWidth(3);
		outer.setColor(QColor(Qt::black));
		inner.setWidth(1);
		inner.setColor(QColor(Qt::green).lighter(160));
		p.setPen(outer);
		DRAWLINE(m_TraceX, m_TraceY - 7, m_TraceX, m_TraceY - 14);
		DRAWLINE(m_TraceX, m_TraceY + 7, m_TraceX, m_TraceY + 14);
		DRAWLINE(m_TraceX - 7, m_TraceY, m_TraceX - 14, m_TraceY);
		DRAWLINE(m_TraceX + 7, m_TraceY, m_TraceX + 14, m_TraceY);
		p.setPen(inner);
		DRAWLINE(m_TraceX, m_TraceY - 7, m_TraceX, m_TraceY - 14);
		DRAWLINE(m_TraceX, m_TraceY + 7, m_TraceX, m_TraceY + 14);
		DRAWLINE(m_TraceX - 7, m_TraceY, m_TraceX - 14, m_TraceY);
		DRAWLINE(m_TraceX + 7, m_TraceY, m_TraceX + 14, m_TraceY);
	}
	p.end();

	// Update pointer method
	updatePointerMethod();

	m_MainWindow->frameQt();
	m_MainWindow->inspector()->frameQt();
}

void InteractiveViewport::setEditorLine(DlEditor *editor, int line,
    bool multiple)
{
	if (m_MouseMovingVertex && m_LineEditor)
	{
		m_LineEditor->codeEditor()->endUndoCombine();
	}
	m_PreferTraceCursor = false;
	m_LineEditor = editor;
	if (!multiple)
	{
		m_SelectedLines.clear();
	}
	const DlParsed &parsed = m_LineEditor->getLine(line);
	if (!m_SelectedLines.contains(line) && isSelectable(parsed))
	{
		m_SelectedLines.append(line);
	}
	emit selectedLinesChanged(m_SelectedLines);
	m_LineNumber = line;
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
	m_Insert->setChecked(false);
}

void InteractiveViewport::traceChecked()
{
	m_PointerFilter = POINTER_TRACE;
	m_Insert->setChecked(false);
}

void InteractiveViewport::editChecked()
{
	m_PointerFilter = POINTER_EDIT_VERTEX_MOVE // vertex movement
	    | POINTER_EDIT_STACK_SELECT // stack selection
	    | POINTER_EDIT_WIDGET_MOVE // widget movement
	    | POINTER_EDIT_GRADIENT_MOVE;
	m_Insert->setChecked(false);
}

void InteractiveViewport::updatePointerMethod()
{
	if (m_MouseTouch || m_MouseMovingVertex || m_MouseMovingWidget)
	{
		// Cannot change now
	}
	else
	{
		if (m_MainWindow->waitingCoprocessorAnimation()) goto PreferTouchCursor;
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
				for (auto &line : m_SelectedLines)
				{
					m_MouseOverVertex = false;
					m_MouseOverVertexLine = -1;
					const DlParsed &parsed = m_LineEditor->getLine(line);
					if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F || parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
					{
						int firstLine = line;
						int vertexType = -1;
						for (int l = firstLine - 1; l > 0; --l)
						{
							const DlParsed &pa = m_LineEditor->getLine(l);
							if (pa.IdLeft == 0 && (pa.IdRight == FTEDITOR_DL_BEGIN || pa.IdRight == FTEDITOR_DL_END || pa.IdRight == FTEDITOR_DL_RETURN || pa.IdRight == FTEDITOR_DL_JUMP))
							{
								if (pa.IdRight == FTEDITOR_DL_BEGIN)
									vertexType = pa.Parameter[0].I;
								break;
							}
							else
							{
								firstLine = l;
							}
						}
						// Iterate over neighbouring vertices
						for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l) // FIXME
						{
							const DlParsed &pa = m_LineEditor->getLine(l);
							if (pa.IdLeft == FTEDITOR_DL_VERTEX2F || pa.IdLeft == FTEDITOR_DL_VERTEX2II)
							{
								const DlState &state = m_LineEditor->getState(l);
								int x, y;
								if (pa.IdLeft == FTEDITOR_DL_VERTEX2F)
								{
									x = pa.Parameter[0].I >> state.Graphics.VertexFormat;
									y = pa.Parameter[1].I >> state.Graphics.VertexFormat;
								}
								else
								{
									x = pa.Parameter[0].U;
									y = pa.Parameter[1].U;
								}
								x += state.Graphics.VertexTranslateX >> 4;
								y += state.Graphics.VertexTranslateY >> 4;

								// Mouse over
								if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4)
								{
									m_MouseOverVertex = true;
									m_MouseOverVertexLine = l;
									if (l == line)
										break; // Currently selected line always has preference
								}
							}
							else if (pa.IdRight == FTEDITOR_DL_BEGIN || pa.IdRight == FTEDITOR_DL_END || pa.IdRight == FTEDITOR_DL_RETURN || pa.IdRight == FTEDITOR_DL_JUMP)
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
						if ((vertexType == BITMAPS || vertexType == POINTS) && ((m_MouseStackValid && m_LineEditor->isCoprocessor() && m_MouseStackCmdTop == line) || (m_MouseStackValid && !m_LineEditor->isCoprocessor() && m_MouseStackDlTop == line)))
						{
							m_MouseOverVertex = true;
							m_MouseOverVertexLine = line;
							setCursor(Qt::SizeAllCursor);
							m_PointerMethod = POINTER_EDIT_VERTEX_MOVE; // move vertex
							return;
						}
					}
				}
			}
		}
		// Gradient move
		if (m_PointerFilter & POINTER_EDIT_GRADIENT_MOVE)
		{
			if (m_LineEditor)
			{
				for (auto &line : m_SelectedLines)
				{
					const DlParsed &parsed = m_LineEditor->getLine(line);
					if (isWidgetGradient(parsed))
					{
						const DlState &state = m_LineEditor->getState(line);
						int x, y;
						x = parsed.Parameter[0].I;
						y = parsed.Parameter[1].I;
						x += state.Graphics.VertexTranslateX >> 4;
						y += state.Graphics.VertexTranslateY >> 4;
						if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4)
						{
							setCursor(Qt::SizeAllCursor);
							m_PointerMethod = POINTER_EDIT_GRADIENT_MOVE_1;
							return;
						}
						x = parsed.Parameter[3].I;
						y = parsed.Parameter[4].I;
						x += state.Graphics.VertexTranslateX >> 4;
						y += state.Graphics.VertexTranslateY >> 4;
						if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4)
						{
							setCursor(Qt::SizeAllCursor);
							m_PointerMethod = POINTER_EDIT_GRADIENT_MOVE_2;
							return;
						}
					}
				}
			}
		}
		// Widget movement
		if (m_PointerFilter & POINTER_EDIT_WIDGET_MOVE)
		{
			if (m_LineEditor)
			{
				for (auto &line : m_SelectedLines)
				{
					const DlParsed &parsed = m_LineEditor->getLine(line);
					if (isWidgetXY(parsed))
					{
						bool widgetWH, widgetR;
						isWidgetWHR(parsed, widgetWH, widgetR);
						if (widgetWH || widgetR)
						{
							const DlState &state = m_LineEditor->getState(line);
							int x = parsed.Parameter[0].I + (state.Graphics.VertexTranslateX >> 4);
							int y = parsed.Parameter[1].I + (state.Graphics.VertexTranslateY >> 4);
							int w, h;
							if (widgetWH)
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
							if (isSingleSelect())
							{
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
							}

							if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPLEFT || m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMRIGHT)
								setCursor(Qt::SizeFDiagCursor);
							else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPRIGHT || m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMLEFT)
								setCursor(Qt::SizeBDiagCursor);
							else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOP || m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOM)
								setCursor(Qt::SizeVerCursor);
							else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_LEFT || m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_RIGHT)
								setCursor(Qt::SizeHorCursor);
							if (m_PointerMethod) return;
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
								const DlState &state = m_LineEditor->getState(line);
								int x = parsed.Parameter[0].I + (state.Graphics.VertexTranslateX >> 4);
								int y = parsed.Parameter[1].I + (state.Graphics.VertexTranslateY >> 4);
								if ((m_MouseStackValid && m_LineEditor->isCoprocessor() && m_MouseStackCmdTop == line) || (m_MouseStackValid && !m_LineEditor->isCoprocessor() && m_MouseStackDlTop == line) || (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY && m_MouseY < y + 4))
								{
									// m_MouseOverWidget = true;
									setCursor(Qt::SizeAllCursor);
									m_PointerMethod = POINTER_EDIT_WIDGET_TRANSLATE; // translate widget
									return;
								}
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
			// TODO: Get the TAG from stack trace and show hand or not depending on
			// TAG value (maybe also show tootip with tag?)
			setCursor(Qt::PointingHandCursor);
			m_PointerMethod = POINTER_TOUCH;
			return;
		}
		if (m_MainWindow->waitingCoprocessorAnimation()) goto NoCursor;
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

void InteractiveViewport::snapPos(int &xd, int &yd, int xref, int yref)
{
	// Disabled
	xd = 0;
	yd = 0;
	return;

	if (QApplication::keyboardModifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
	{
		// Hold CTRL to disable snap
		xd = 0;
		yd = 0;
	}
	else
	{
		int xsnap = xref;
		int ysnap = yref;
		int xdist = FTED_SNAP_HISTORY_NONE;
		int ydist = FTED_SNAP_HISTORY_NONE;
		for (int i = 0; i < FTED_SNAP_HISTORY; ++i)
		{
			if (i == m_SnapHistoryCur)
				continue;
			int nxdist = abs(m_SnapHistoryX[i] - xref);
			int nydist = abs(m_SnapHistoryY[i] - yref);
			if (nxdist < xdist && nxdist < FTED_SNAP_DIST)
			{
				xdist = nxdist;
				xsnap = m_SnapHistoryX[i];
			}
			if (nydist < ydist && nydist < FTED_SNAP_DIST)
			{
				ydist = nydist;
				ysnap = m_SnapHistoryY[i];
			}
		}
		xd = xsnap - xref;
		yd = ysnap - yref;
	}
	m_SnapHistoryX[m_SnapHistoryCur] = xref;
	m_SnapHistoryY[m_SnapHistoryCur] = yref;
}

void InteractiveViewport::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Right)
	{
		m_MovingLastX -= 1;
		mouseMoveEvent(m_MovingLastX + 1, m_MovingLastY);
	}
	else if (e->key() == Qt::Key_Left)
	{
		m_MovingLastX += 1;
		mouseMoveEvent(m_MovingLastX - 1, m_MovingLastY);
	}
	else if (e->key() == Qt::Key_Up)
	{
		m_MovingLastY += 1;
		mouseMoveEvent(m_MovingLastX, m_MovingLastY - 1);
	}
	else if (e->key() == Qt::Key_Down)
	{
		m_MovingLastY -= 1;
		mouseMoveEvent(m_MovingLastX, m_MovingLastY + 1);
	}
	else
	{
		printf("Surpress keyboard\n");
	}
}

int32_t InteractiveViewport::mappingX(QDropEvent *e)
{
	QPoint p = e->position().toPoint();
	return ((p.x() - screenLeft()) * 16) / screenScale();
}

int32_t InteractiveViewport::mappingX(int x)
{
	return ((x - screenLeft()) * 16) / screenScale();
}

int32_t InteractiveViewport::mappingY(QDropEvent *e)
{
	QPoint p = e->position().toPoint();
	return ((p.y() - screenTop()) * 16) / screenScale();
}

int32_t InteractiveViewport::mappingY(int y)
{
	return ((y - screenTop()) * 16) / screenScale();
}

int32_t InteractiveViewport::mappingX(QSinglePointEvent *e)
{
	QPoint p = e->position().toPoint();
	return ((p.x() - screenLeft()) * 16) / screenScale();
}

int32_t InteractiveViewport::mappingY(QSinglePointEvent *e)
{
	QPoint p = EPOSPOINT(e);
	return ((p.y() - screenTop()) * 16) / screenScale();
}

void InteractiveViewport::selectItems()
{
	int lineCount = m_LineEditor->getLineCount();
	m_SelectedLines.clear();
	emit selectedLinesChanged(m_SelectedLines);
	for (int line = 0; line < lineCount && line < FTEDITOR_DL_SIZE; line++)
	{
		const DlParsed &parsed = m_LineEditor->getLine(line);
		const DlState &state = m_LineEditor->getState(line);
		int x = 0;
		int y = 0;
		if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
		{
			x = parsed.Parameter[0].I >> state.Graphics.VertexFormat;
			y = parsed.Parameter[1].I >> state.Graphics.VertexFormat;
			x += state.Graphics.VertexTranslateX >> 4;
			y += state.Graphics.VertexTranslateY >> 4;
		}
		else if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
		{
			x = parsed.Parameter[0].U;
			y = parsed.Parameter[1].U;
			x += state.Graphics.VertexTranslateX >> 4;
			y += state.Graphics.VertexTranslateY >> 4;
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
			case CMD_CSKETCH:
			case CMD_ANIMFRAME:
			case CMD_ANIMFRAMERAM: {
				x = parsed.Parameter[0].I;
				y = parsed.Parameter[1].I;
				x += state.Graphics.VertexTranslateX >> 4;
				y += state.Graphics.VertexTranslateY >> 4;
			}
			}
		}
		if (isSelectable(parsed))
		{
			QPoint topRight;
			QPoint botLeft;
			topRight.setX(mappingX(m_FirstPoint.x() > m_SecondPoint.x()
			        ? m_FirstPoint.x()
			        : m_SecondPoint.x()));
			topRight.setY(mappingY(m_FirstPoint.y() < m_SecondPoint.y()
			        ? m_FirstPoint.y()
			        : m_SecondPoint.y()));
			botLeft.setX(mappingX(m_FirstPoint.x() < m_SecondPoint.x()
			        ? m_FirstPoint.x()
			        : m_SecondPoint.x()));
			botLeft.setY(mappingY(m_FirstPoint.y() > m_SecondPoint.y()
			        ? m_FirstPoint.y()
			        : m_SecondPoint.y()));
			// Check if this item is inside the rectangle
			if (x > botLeft.x() && x < topRight.x() && y > topRight.y() && y < botLeft.y())
			{
				setEditorLine(m_LineEditor, line, true);
			}
		}
	}
	if (m_SelectedLines.count() > 0)
	{
		m_LineEditor->codeEditor()->setInteractiveDelete(true);
		if (m_SelectedLines.count() == 1)
		{
			m_LineEditor->selectLine(m_SelectedLines.at(0));
		}
	}
}

bool InteractiveViewport::isSingleSelect()
{
	return m_SelectedLines.size() == 1;
}

void InteractiveViewport::isWidgetWHR(const DlParsed &parsed, bool &widgetWH,
    bool &widgetR)
{
	switch (parsed.IdRight | 0xFFFFFF00)
	{
	case CMD_BUTTON:
	case CMD_KEYS:
	case CMD_PROGRESS:
	case CMD_SLIDER:
	case CMD_SCROLLBAR:
	case CMD_TRACK:
	case CMD_SKETCH:
	case CMD_CSKETCH:
		widgetWH = true;
		widgetR = false;
		break;
	case CMD_GAUGE:
	case CMD_CLOCK:
	case CMD_DIAL:
		widgetWH = false;
		widgetR = true;
		break;
	default:
		widgetWH = false;
		widgetR = false;
		break;
	}
}

bool InteractiveViewport::isSelectable(const DlParsed &parsed)
{
	if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
	{
		return true;
	}
	else if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
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
		case CMD_CSKETCH:
		case CMD_ANIMFRAME:
		case CMD_GRADIENT:
		case CMD_GRADIENTA:
		case CMD_ANIMFRAMERAM: {
			return true;
		}
		}
	}
	return false;
}

bool InteractiveViewport::isWidgetXY(const DlParsed &parsed)
{
	if (parsed.IdLeft == 0xFFFFFF00)
	{ // Coprocessor
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
		case CMD_CSKETCH:
		case CMD_ANIMFRAME:
		case CMD_ANIMFRAMERAM:
			return true;
		}
	}
	return false;
}

bool InteractiveViewport::isWidgetGradient(const DlParsed &parsed)
{
	if (parsed.IdLeft == 0xFFFFFF00)
	{ // Coprocessor
		switch (parsed.IdRight | 0xFFFFFF00)
		{
		case CMD_GRADIENT:
		case CMD_GRADIENTA:
			return true;
		}
	}
	return false;
}

void InteractiveViewport::mouseMoveEvent(int mouseX, int mouseY,
    Qt::KeyboardModifiers km)
{
	m_NextMouseX = mouseX;
	m_NextMouseY = mouseY;
	fetchColorAsync(mouseX, mouseY);

	m_MainWindow->statusBar()->showMessage("");
	m_IsDrawAlgnHorizontal = m_IsDrawAlgnVertical = false;

	if (m_MouseTouch)
	{
		// BT8XXEMU_setTouchScreenXY(EPOSPOINT(e).x(), EPOSPOINT(e).y(), 0);
	}
	else if (m_MouseMovingVertex || (!isSingleSelect() && m_MouseMovingWidget))
	{
		if (m_LineEditor)
		{
			if (isSingleSelect() && m_ActionAlgn->isChecked())
				m_IsDrawAlgn = true;
			int xd = mouseX - m_MovingLastX;
			int yd = mouseY - m_MovingLastY;
			int otherVertices[4];
			m_MovingLastX = mouseX;
			m_MovingLastY = mouseY;

			for (auto &line : m_SelectedLines)
			{
				DlParsed pa = m_LineEditor->getLine(line);
				int numOtherV = 0;
				if (line > 0 && m_LineEditor->getLine(line - 1).ValidId && m_LineEditor->getLine(line - 1).IdLeft == 0 && m_LineEditor->getLine(line - 1).IdRight == FTEDITOR_DL_PALETTE_SOURCE)
				{
					int firstLine = line;
					for (int l = firstLine - 1; l > 0; --l)
					{
						const DlParsed &parsed = m_LineEditor->getLine(l);
						if (parsed.IdLeft == 0 && (parsed.IdRight == FTEDITOR_DL_BEGIN || parsed.IdRight == FTEDITOR_DL_END || parsed.IdRight == FTEDITOR_DL_RETURN || parsed.IdRight == FTEDITOR_DL_JUMP))
						{
							break;
						}
						else
						{
							firstLine = l;
						}
					}
					for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l) // FIXME
					{
						if (l == line) continue;
						const DlParsed &parsed = m_LineEditor->getLine(l);

						if (!parsed.ValidId) continue;

						if (parsed.IdLeft == pa.IdLeft && parsed.Parameter[0].I == pa.Parameter[0].I && parsed.Parameter[1].I == pa.Parameter[1].I)
						{
							if (l != line)
							{
								otherVertices[numOtherV] = l;
								++numOtherV;
								if (numOtherV >= 4) break;
							}
						}
						else if (pa.IdRight == FTEDITOR_DL_BEGIN || pa.IdRight == FTEDITOR_DL_END || pa.IdRight == FTEDITOR_DL_RETURN || pa.IdRight == FTEDITOR_DL_JUMP)
						{
							break;
						}
					}
				}
				// In case automatic expansion is necessary
				const DlState &state = m_LineEditor->getState(line);
				if (pa.IdLeft == FTEDITOR_DL_VERTEX2F)
				{
					pa.Parameter[0].I += xd << state.Graphics.VertexFormat;
					pa.Parameter[1].I += yd << state.Graphics.VertexFormat;
				}
				else
				{
					pa.Parameter[0].I += xd;
					pa.Parameter[1].I += yd;
				}

				int snapx, snapy;
				if (pa.IdLeft == FTEDITOR_DL_VERTEX2II)
				{
					snapPos(snapx, snapy, pa.Parameter[0].I, pa.Parameter[1].I);
					pa.Parameter[0].I += snapx;
					pa.Parameter[1].I += snapy;
				}
				else
				{
					snapPos(snapx, snapy, pa.Parameter[0].I >> 4, pa.Parameter[1].I >> 4);
					pa.Parameter[0].I += snapx << 4;
					pa.Parameter[1].I += snapy << 4;
				}
				m_MovingLastX += snapx;
				m_MovingLastY += snapy;
				m_LineEditor->replaceLine(line, pa);
				for (int i = 0; i < numOtherV; ++i)
					m_LineEditor->replaceLine(otherVertices[i], pa);
			}
		}
		else
		{
			m_MouseMovingVertex = false;
			updatePointerMethod(); // update because update is not done while
			                       // m_MouseMovingVertex true
		}
	}
	else if (m_MouseMovingWidget)
	{
		if (m_LineEditor)
		{
			if (isSingleSelect() && m_ActionAlgn->isChecked())
				m_IsDrawAlgn = true;
			m_MainWindow->statusBar()->showMessage(
			    "Press SHIFT for keeping constant x-coordinate, ALT for keeping "
			    "constant y-coordinate");
			// Apply action
			int xd = 0, yd = 0;

			if (km != Qt::ShiftModifier)
			{
				xd = mouseX - m_MovingLastX;
				m_MovingLastX = mouseX;
			}
			else
			{
				m_IsDrawAlgnVertical = true;
			}

			if (km != Qt::AltModifier)
			{
				yd = mouseY - m_MovingLastY;
				m_MovingLastY = mouseY;
			}
			else
			{
				m_IsDrawAlgnHorizontal = true;
			}

			for (auto &line : m_SelectedLines)
			{
				DlParsed pa = m_LineEditor->getLine(line);
				if (m_MouseMovingWidget == POINTER_EDIT_WIDGET_TRANSLATE || m_MouseMovingWidget == POINTER_EDIT_GRADIENT_MOVE_1)
				{
					pa.Parameter[0].I += xd;
					pa.Parameter[1].I += yd;

					// Snap ->
					int snapx, snapy;
					snapPos(snapx, snapy, pa.Parameter[0].I, pa.Parameter[1].I);
					m_MovingLastX += snapx;
					pa.Parameter[0].I += snapx;
					m_MovingLastY += snapy;
					pa.Parameter[1].I += snapy;
					// <- Snap
				}
				else if (m_MouseMovingWidget == POINTER_EDIT_GRADIENT_MOVE_2)
				{
					pa.Parameter[3].I += xd;
					pa.Parameter[4].I += yd;

					// Snap ->
					int snapx, snapy;
					snapPos(snapx, snapy, pa.Parameter[3].I, pa.Parameter[4].I);
					m_MovingLastX += snapx;
					pa.Parameter[3].I += snapx;
					m_MovingLastY += snapy;
					pa.Parameter[4].I += snapy;
					// <- Snap
				}
				else // resize, check top/bottom and left/right
				{
					int x = pa.Parameter[0].I;
					int y = pa.Parameter[1].I;
					if (m_WidgetWH)
					{
						const int minsize = 0;
						int w = pa.Parameter[2].I;
						int h = pa.Parameter[3].I;
						if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_TOP)
						{
							y += yd;
							h -= yd;

							// Snap ->
							int snapx, snapy;
							snapPos(snapx, snapy, x, y);
							m_MovingLastY += snapy;
							y += snapy;
							h -= snapy;
							// <- Snap

							if (h < minsize)
							{
								m_MovingLastY += (h - minsize);
								y += (h - minsize);
								h = minsize;
							}
						}
						else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM)
						{
							h += yd;

							// Snap ->
							int snapx, snapy;
							snapPos(snapx, snapy, x + w, y + h);
							m_MovingLastY += snapy;
							h += snapy;
							// <- Snap

							if (h < minsize)
							{
								m_MovingLastY -= (h - minsize);
								h = minsize;
							}
						}
						if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT)
						{
							x += xd;
							w -= xd;

							// Snap ->
							int snapx, snapy;
							snapPos(snapx, snapy, x, y);
							m_MovingLastX += snapx;
							x += snapx;
							w -= snapx;
							// <- Snap

							if (w < minsize)
							{
								m_MovingLastX += (w - minsize);
								x += (w - minsize);
								w = minsize;
							}
						}
						else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT)
						{
							w += xd;

							// Snap ->
							int snapx, snapy;
							snapPos(snapx, snapy, x + w, y + h);
							m_MovingLastX += snapx;
							w += snapx;
							// <- Snap

							if (w < minsize)
							{
								m_MovingLastX -= (w - minsize);
								w = minsize;
							}
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
							if (r < 0)
							{
								m_MovingLastY += r;
								r = 0;
							}
						}
						else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM)
						{
							r += yd;
							if (r < 0)
							{
								m_MovingLastY -= r;
								r = 0;
							}
						}
						else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT)
						{
							r -= xd;
							if (r < 0)
							{
								m_MovingLastX += r;
								r = 0;
							}
						}
						else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT)
						{
							r += xd;
							if (r < 0)
							{
								m_MovingLastX -= r;
								r = 0;
							}
						}
						pa.Parameter[2].I = r;
					}
				}
				m_LineEditor->replaceLine(line, pa);
			}
		}
		else
		{
			m_MouseMovingWidget = 0;
			updatePointerMethod(); // update because update is not done while
			                       // m_MouseMovingWidget has a value
		}
	}
}

void InteractiveViewport::wheelEvent(QWheelEvent *e)
{
	int curx = mappingX(e);
	int cury = mappingY(e);

	if (e->angleDelta().y() > 0)
	{
		zoomIn();
	}
	else if (e->angleDelta().y() < 0)
	{
		zoomOut();
	}

	int newx = mappingX(e);
	int newy = mappingY(e);

	int nx = (curx - newx) * 16;
	int ny = (cury - newy) * 16;

	horizontalScrollbar()->setValue(horizontalScrollbar()->value() + nx);
	verticalScrollbar()->setValue(verticalScrollbar()->value() + ny);

	EmulatorViewport::wheelEvent(e);
}

void InteractiveViewport::mouseMoveEvent(QMouseEvent *e)
{
	if (m_DrawMultipleSelection)
	{
		m_SecondPoint = e->position().toPoint();
	}
	horizontalRuler()->setIndicator(EPOSPOINT(e).x());
	verticalRuler()->setIndicator(EPOSPOINT(e).y());
	mouseMoveEvent(mappingX(e), mappingY(e), e->modifiers());
	EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton)
	{
		emit this->customContextMenuRequested(e->position().toPoint());
		return;
	}
	
	if (!m_selectable) return;
	m_FirstPoint = m_SecondPoint = e->position().toPoint();
	m_MainWindow->cmdEditor()->codeEditor()->setKeyHandler(this);
	m_MainWindow->dlEditor()->codeEditor()->setKeyHandler(this);

	switch (m_PointerMethod)
	{
	case POINTER_TOUCH: // touch
		if (e->button() == Qt::LeftButton)
		{
			m_MouseTouch = true;
			// BT8XXEMU_setTouchScreenXY(EPOSPOINT(e).x(), EPOSPOINT(e).y(), 0);
		}
		break;
	case POINTER_TRACE: // trace
		switch (e->button())
		{
		case Qt::LeftButton:
			m_MainWindow->setTraceX(mappingX(e));
			m_MainWindow->setTraceY(mappingY(e));
			m_MainWindow->setTraceEnabled(true);
			break;
		case Qt::MiddleButton:
			if (m_PointerFilter != POINTER_TRACE)
			{
				m_PreferTraceCursor = false;
				break;
			}
			// fallthrough to Qt::RightButton
		case Qt::RightButton:
			m_MainWindow->setTraceEnabled(false);
			break;
		default:
			break;
		}
		break;
	case POINTER_EDIT_VERTEX_MOVE:
		if (m_LineEditor)
		{
			m_MovingLastX = mappingX(e);
			m_MovingLastY = mappingY(e);
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
				debugLog("Select topmost command");
				int idxDl = m_MouseStackRead[m_MouseStackRead.size() - 1]; // DL
				int idxCmd = m_MainWindow->getDlCmd()[idxDl];
				debugLog(QString("DL: %1").arg(idxDl));
				debugLog(QString("CMD: %1").arg(idxCmd));

				if (idxCmd >= 0)
				{
					m_MainWindow->focusCmdEditor();
					bool force = !m_SelectedLines.contains(idxCmd);
					m_MainWindow->cmdEditor()->selectLine(idxCmd, force);
				}
				else
				{
					m_MainWindow->focusDlEditor();
					bool force = !m_SelectedLines.contains(idxCmd);
					m_MainWindow->dlEditor()->selectLine(idxDl, force);
				}
				emit selectedLinesChanged(m_SelectedLines);

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
		else
		{
			debugLog(QString("Select blank"));
			m_SelectedLines.clear();
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
				pa.Parameter[0].I = mappingX(e)
				    << m_LineEditor->getVertextFormat(line);
				pa.Parameter[1].I = mappingY(e)
				    << m_LineEditor->getVertextFormat(line);
				m_LineEditor->insertLine(line, pa);
				m_LineEditor->selectLine(line);
			}
			break;
		}
		else if (e->button() == Qt::MiddleButton || e->button() == Qt::RightButton)
		{
			m_Insert->setChecked(false);
		}
	default:
		if (m_PointerMethod & (POINTER_EDIT_WIDGET_MOVE | POINTER_EDIT_GRADIENT_MOVE))
		{
			// Works for any widget move action
			m_MovingLastX = mappingX(e);
			m_MovingLastY = mappingY(e);
			m_MouseMovingWidget = m_PointerMethod;
			m_LineEditor->codeEditor()->beginUndoCombine(tr("Move widget"));
		}
		break;
	}

	if (!m_MouseMovingVertex && !m_MouseMovingWidget)
	{
		m_DrawMultipleSelection = true;
	}
	EmulatorViewport::mousePressEvent(e);
}

void InteractiveViewport::mouseReleaseEvent(QMouseEvent *e)
{
	if (m_DrawMultipleSelection && m_FirstPoint != m_SecondPoint)
	{
		selectItems();
	}
	m_DrawMultipleSelection = false;
	m_MainWindow->cmdEditor()->codeEditor()->setKeyHandler(NULL);
	m_MainWindow->dlEditor()->codeEditor()->setKeyHandler(NULL);

	m_MainWindow->statusBar()->showMessage("");
	m_IsDrawAlgnHorizontal = m_IsDrawAlgnVertical = false;
	m_IsDrawAlgn = false;

	if (m_MouseTouch)
	{
		m_MouseTouch = false;
		// BT8XXEMU_resetTouchScreenXY();
		updatePointerMethod(); // update because update is not done while m_MouseTouch true
	}
	else if (m_MouseMovingVertex)
	{
		m_MouseMovingVertex = false;
		if (m_LineEditor)
		{
			m_LineEditor->codeEditor()->endUndoCombine();
		}
		++m_SnapHistoryCur;
		m_SnapHistoryCur %= FTED_SNAP_HISTORY;
		updatePointerMethod(); // update because update is not done while m_MouseMovingVertex true
	}
	else if (m_MouseMovingWidget)
	{
		m_MouseMovingWidget = 0;
		if (m_LineEditor)
		{
			m_LineEditor->codeEditor()->endUndoCombine();
		}
		++m_SnapHistoryCur;
		m_SnapHistoryCur %= FTED_SNAP_HISTORY;
		updatePointerMethod(); // update because update is not done while m_MouseMovingWidget has a value
	}

	EmulatorViewport::mouseReleaseEvent(e);
}

void InteractiveViewport::enterEvent(QEnterEvent *e)
{
	// printf("InteractiveViewport::enterEvent\n");

	m_MouseOver = true;

	horizontalRuler()->setShowIndicator(true);
	verticalRuler()->setShowIndicator(true);
	EmulatorViewport::enterEvent(e);
}

void InteractiveViewport::leaveEvent(QEvent *e)
{
	// printf("InteractiveViewport::leaveEvent\n");

	m_MouseOver = false;

	horizontalRuler()->setShowIndicator(false);
	verticalRuler()->setShowIndicator(false);

	EmulatorViewport::leaveEvent(e);
}

bool InteractiveViewport::acceptableSource(QDropEvent *e)
{
	if (e->source() == m_MainWindow->toolbox()->treeWidget())
		return true;
	// if (e->source() == m_MainWindow->bitmapSetup()) return true;
	if (e->source() == m_MainWindow->contentManager()->contentList())
	{
		ContentInfo *currentItem = m_MainWindow->contentManager()->current();

		static const QSet<QString> supportedList = { "flash", "ram_g", "raw", "xfont", "avi" };
		QString fileSuffix = QFileInfo(currentItem->SourcePath).suffix().toLower(); //Remove static const

		if (fileSuffix == "raw" && currentItem->SourcePath.contains("_lut"))
			return false;

		if (supportedList.contains(fileSuffix))
			return true;

		if (!currentItem->MemoryLoaded)
			return false;

		if (currentItem->Converter != ContentInfo::Image
		    && currentItem->Converter != ContentInfo::Font
		    && currentItem->Converter != ContentInfo::ImageCoprocessor)
			return false;
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
		if (m_LineEditor && !m_LineEditor->isMacro())
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
			/*else if (e->source() == m_MainWindow->bitmapSetup())
			{
			    selectionType = FTEDITOR_SELECTION_PRIMITIVE; // PRIMITIVE
			    selection = BITMAPS;
			    bitmapHandle = m_MainWindow->bitmapSetup()->selected();
			    contentInfo = NULL;
			}*/
			else if (e->source() == m_MainWindow->contentManager()->contentList())
			{
				selectionType = FTEDITOR_SELECTION_PRIMITIVE; // PRIMITIVE
				selection = BITMAPS;
				bitmapHandle = 0;
				contentInfo = m_MainWindow->contentManager()->current();
			}
			else
			{
				printf("Unknown error. This code is unreachable\n");
				return;
			}

			if (m_LineEditor->isCoprocessor())
			{
				m_MainWindow->focusCmdEditor();
			}
			else
			{
				m_MainWindow->focusDlEditor();
			}

			if (selectionType == FTEDITOR_SELECTION_PRIMITIVE
			    || selectionType == FTEDITOR_SELECTION_FUNCTION
			    || selectionType == FTEDITOR_SELECTION_VERTEX)
			{
				int line = m_LineNumber;
				if (m_LineEditor->getLine(line).ValidId)
				{
					// printf("Valid Id\n");
					if (m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_VERTEX2II
					    || m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_VERTEX2F)
					{
						// Search down for end of vertex set
						for (; line < FTEDITOR_DL_SIZE /*&& line < m_LineEditor->codeEditor()->document()->blockCount()*/; ++line)
						{
							printf("line %i\n", line);
							const DlParsed &pa = m_LineEditor->getLine(line);
							if (!pa.ValidId)
							{
								break;
							}
							if (pa.IdLeft == 0)
							{
								if (pa.IdRight == FTEDITOR_DL_END)
								{
									++line;
									break;
								}
								else if (pa.IdRight == FTEDITOR_DL_BEGIN
								    || pa.IdRight == FTEDITOR_DL_RETURN
								    || pa.IdRight == FTEDITOR_DL_JUMP)
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
				while (m_LineEditor->getLine(line).ValidId && m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_INSTRUCTION && (m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR || m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_COLOR_RGB || m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_COLOR_A || m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_STENCIL || m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_TAG))
				{
					++line;
				}

				// printf("Dropped item from toolbox, type %i\n", selection);

				// void insertLine(int line, const DlParsed &parsed);
				if (selectionType == FTEDITOR_SELECTION_VERTEX)
				{
					m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop vertex");
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = selection;
					pa.IdRight = 0;
					pa.ExpectedStringParameter = false;
					pa.VarArgCount = 0;
					switch (selection)
					{
					case FTEDITOR_DL_VERTEX2F:
						pa.Parameter[0].I = mappingX(e) << m_LineEditor->getVertextFormat(line);
						pa.Parameter[1].I = mappingY(e) << m_LineEditor->getVertextFormat(line);
						pa.ExpectedParameterCount = 2;
						break;
					default:
						pa.Parameter[0].I = mappingX(e);
						pa.Parameter[1].I = mappingY(e);
						pa.Parameter[2].I = 0;
						pa.Parameter[3].I = 0;
						pa.ExpectedParameterCount = 4;
						break;
					}
					m_LineEditor->insertLine(line, pa);
					m_LineEditor->selectLine(line);
					m_LineEditor->codeEditor()->endUndoCombine();
				}
				else if ((selection & 0xFFFFFF00) == 0xFFFFFF00) // Coprocessor
				{
					m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop coprocessor widget");
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0xFFFFFF00;
					pa.IdRight = selection & 0xFF;
					pa.Parameter[0].I = mappingX(e);
					pa.Parameter[1].I = mappingY(e);
					pa.ExpectedStringParameter = false;
					pa.VarArgCount = 0;
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
						pa.Parameter[5].U = 'y';
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
					case CMD_SCREENSAVER:
						pa.ExpectedParameterCount = 0;
						break;
					case CMD_SKETCH:
						pa.Parameter[2].U = 120;
						pa.Parameter[3].U = 48;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = L1;
						pa.ExpectedParameterCount = 6;
						break;
					case CMD_CSKETCH:
						pa.Parameter[2].U = 120;
						pa.Parameter[3].U = 48;
						pa.Parameter[4].U = 0;
						pa.Parameter[5].U = L1;
						pa.Parameter[6].U = 1500;
						pa.ExpectedParameterCount = 7;
						break;
					case CMD_GRADIENT:
						pa.Parameter[2].U = 0x007FFF;
						pa.Parameter[3].I = pa.Parameter[0].I + 32;
						pa.Parameter[4].I = pa.Parameter[1].I + 32;
						pa.Parameter[5].U = 0x7FFF00;
						pa.ExpectedParameterCount = 6;
						break;
					case CMD_TRACK:
						pa.Parameter[2].U = 72;
						pa.Parameter[3].U = 48;
						pa.Parameter[4].U = 1;
						pa.ExpectedParameterCount = 5;
						break;
					case CMD_INTERRUPT:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_COLDSTART:
						pa.ExpectedParameterCount = 0;
						break;
					case CMD_MEDIAFIFO:
						pa.Parameter[0].I = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - (64 * 1024);
						pa.Parameter[1].I = 64 * 1024;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_SNAPSHOT:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_SNAPSHOT2:
						pa.Parameter[0].U = RGB565;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = m_MainWindow->viewport()->hsize();
						pa.Parameter[5].U = m_MainWindow->viewport()->vsize();
						pa.ExpectedParameterCount = 6;
						break;
					case CMD_LOADIMAGE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = OPT_NODL;
						pa.StringParameter = "";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_GETPTR:
						pa.Parameter[0].U = 0;
						pa.ExpectedStringParameter = false;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_GETPROPS:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.ExpectedStringParameter = false;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_APPEND:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_PLAYVIDEO:
						pa.Parameter[0].U = OPT_FULLSCREEN | OPT_SOUND;
						pa.StringParameter = "";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_MEMCRC:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.ExpectedStringParameter = false;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_MEMWRITE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.StringParameter = "";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_MEMSET:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_MEMZERO:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_MEMCPY:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_INFLATE:
						pa.Parameter[0].U = 0;
						pa.StringParameter = "";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_INFLATE2:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].I = 0;
						pa.StringParameter = "";
						pa.ExpectedStringParameter = true;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_GRADIENTA:
						pa.Parameter[2].U = 0xFF007FFF;
						pa.Parameter[3].I = pa.Parameter[0].I + 32;
						pa.Parameter[4].I = pa.Parameter[1].I + 32;
						pa.Parameter[5].U = 0x7FFF00;
						pa.ExpectedParameterCount = 6;
						break;
					case CMD_ANIMSTART:
					case CMD_ANIMSTARTRAM:
						pa.Parameter[0].I = 0;
						pa.Parameter[1].I = 0;
						pa.Parameter[2].I = 0;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_ANIMSTOP:
						pa.Parameter[0].I = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_ANIMDRAW:
						pa.Parameter[0].I = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_ANIMFRAME:
					case CMD_ANIMFRAMERAM:
						pa.Parameter[2].I = 0;
						pa.Parameter[3].I = 0;
						pa.ExpectedParameterCount = 4;
						break;
					case CMD_VIDEOFRAME:
						pa.Parameter[0].I = 0;
						pa.Parameter[1].I = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_WAIT:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_NEWLIST:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_CALLLIST:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_RETURN:
						pa.ExpectedParameterCount = 0;
						break;
					case CMD_APILEVEL:
						pa.Parameter[0].U = 1;
						pa.ExpectedStringParameter = false;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_CALIBRATE:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_CALIBRATESUB:
						pa.Parameter[2].U = 100;
						pa.Parameter[3].U = 100;
						pa.Parameter[4].U = 0;
						pa.ExpectedParameterCount = 5;
						break;
					case CMD_TESTCARD:
						pa.ExpectedParameterCount = 0;
						break;
					case CMD_FONTCACHE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_FONTCACHEQUERY:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_GETIMAGE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.Parameter[3].U = 0;
						pa.Parameter[4].U = 0;
						pa.ExpectedParameterCount = 5;
						break;
					}

					if (selection == CMD_SKETCH)
					{
						line = m_LineEditor->getLineCount();
						if (m_LineEditor->getLineText(line - 1).isEmpty())
						{
							--line;
						}

						int addr = m_MainWindow->contentManager()->getFreeMemoryAddress();
						if (addr < 0)
						{
							m_LineEditor->insertLine(++line, "// No free space in RAM_G.");
							m_MainWindow->propertyErrorSet("No free space in RAM_G.");
							return;
						}

						pa.Parameter[2].U = 480;
						pa.Parameter[3].U = 272;
						pa.Parameter[4].U = addr;
						pa.Parameter[5].U = L1;
						pa.ExpectedParameterCount = 6;

						m_LineEditor->insertLine(++line, QString("CMD_MEMZERO(%1, 16320)").arg(addr));
						m_LineEditor->insertLine(++line, pa); // insert cmd_sketch
						m_LineEditor->selectLine(line);
						m_LineEditor->insertLine(++line, "");
						m_LineEditor->insertLine(++line, "// Then to display the bitmap");

						int handleFree = m_MainWindow->contentManager()->editorFindFreeHandle(m_LineEditor);
						if (handleFree == -1)
						{
							m_LineEditor->insertLine(++line, "// No free bitmap handle.");
							m_MainWindow->propertyErrorSet("No free bitmap handle.");
							return;
						}

						QString bh = QString("BITMAP_HANDLE(%1)").arg(handleFree);
						m_LineEditor->insertLine(++line, bh);

						if (FTEDITOR_CURRENT_DEVICE > FTEDITOR_FT810)
						{
							m_LineEditor->insertLine(++line, QString("CMD_SETBITMAP(%1, L1, 480, 272)").arg(addr));
						}
						else
						{
							m_LineEditor->insertLine(++line, QString("BITMAP_SOURCE(0)").arg(addr));
							m_LineEditor->insertLine(++line, "BITMAP_LAYOUT(L1, 60, 272)");
							m_LineEditor->insertLine(++line, "BITMAP_SIZE(NEAREST, BORDER, BORDER, 480, 272)");
						}

						m_LineEditor->insertLine(++line, "");
						m_LineEditor->insertLine(++line, "BEGIN(BITMAPS)");

						int vf = m_LineEditor->getVertextFormat(line);
						QString vt2f = QString("VERTEX2F(%1, %2)")
						                   .arg(pa.Parameter[0].I << vf)
						                   .arg(pa.Parameter[1].I << vf);
						m_LineEditor->insertLine(++line, vt2f);
						m_LineEditor->insertLine(++line, "END()");
					}
					else
					{
						m_LineEditor->insertLine(line, pa);
						m_LineEditor->selectLine(line);
					}

					m_LineEditor->codeEditor()->endUndoCombine();
				}
				else if (selectionType == FTEDITOR_SELECTION_FUNCTION)
				{
					if (!m_MouseStackValid) line = m_MainWindow->contentManager()->editorFindNextBitmapLine(m_LineEditor);
					m_LineEditor->codeEditor()->beginUndoCombine(tr("Drag and drop from toolbox"));
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0;
					pa.IdRight = selection;
					pa.ExpectedStringParameter = false;
					pa.VarArgCount = 0;
					switch (selection)
					{
					case FTEDITOR_DL_CLEAR:
						pa.Parameter[0].U = 1;
						pa.Parameter[1].U = 1;
						pa.Parameter[2].U = 1;
						pa.ExpectedParameterCount = 3;
						break;
					case FTEDITOR_DL_BEGIN:
						pa.Parameter[0].U = 1;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_END:
						pa.ExpectedParameterCount = 0;
						break;
					case FTEDITOR_DL_CALL:
						pa.Parameter[0].U = line + 1;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_DISPLAY:
						pa.ExpectedParameterCount = 0;
						break;
					case FTEDITOR_DL_JUMP:
						pa.Parameter[0].U = line + 1;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_MACRO:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_RETURN:
						pa.ExpectedParameterCount = 0;
						break;
					}
					m_LineEditor->insertLine(line, pa);
					m_LineEditor->selectLine(line);
					m_LineEditor->codeEditor()->endUndoCombine();
				}
				else if (selectionType == FTEDITOR_SELECTION_PRIMITIVE) // Primitive
				{
					bool mustCreateHandle = true;
					if (contentInfo)
					{
						debugLog("Find or create handle for content item");
						int handleResult = m_MainWindow->contentManager()->editorFindHandle(contentInfo, m_LineEditor);
						if (handleResult != -1 && handleResult != 15)
						{
							bitmapHandle = handleResult;

							mustCreateHandle = false;
						}
						if (mustCreateHandle)
						{
							debugLog("Must create handle");
							mustCreateHandle = false;
							int handleFree = m_MainWindow->contentManager()->editorFindFreeHandle(m_LineEditor);
							if (handleFree != -1)
							{
								bitmapHandle = handleFree;
								mustCreateHandle = true;
							}
							if (!mustCreateHandle)
							{
								if (contentInfo->Converter == ContentInfo::Font)
								{
									printf("No free handle available\n");
									PropertiesEditor *props = m_MainWindow->propertiesEditor();
									props->setError(tr("<b>Error</b>: No free bitmap handle available"));
									props->setEditWidget(NULL, false, NULL);
									m_MainWindow->focusProperties();
									return;
								}
								// Handle 15 is disposable...
								bitmapHandle = 15;
								mustCreateHandle = true;
							}
						}
					}
					// m_MainWindow->undoStack()->beginMacro(tr("Drag and drop primitive"));
					m_LineEditor->codeEditor()->beginUndoCombine(tr("Drag and drop primitive"));
					DlParsed pa;
					pa.ValidId = true;
					pa.IdLeft = 0;
					pa.ExpectedStringParameter = false;
					pa.VarArgCount = 0;
					if (contentInfo && mustCreateHandle)
					{
						printf("Create handle for content item\n");

						int hline = (bitmapHandle == 15) ? line : m_MainWindow->contentManager()->editorFindNextBitmapLine(m_LineEditor);

						line = hline;
						// TODO: contentInfo->Converter == ContentInfo::Font && isCoprocessor && (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)

						QString fileSuffix = QFileInfo(contentInfo->SourcePath).suffix().toLower();
						if ((FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
						    && m_LineEditor->isCoprocessor()
						    && (contentInfo->Converter == ContentInfo::Font))
						{
							DLUtil::addSetFont2Cmd(
							    m_LineEditor, pa, bitmapHandle, contentInfo->MemoryAddress,
							    contentInfo->FontOffset, line, hline);
						}
						else if (contentInfo->Converter == ContentInfo::Raw)
						{
							if (fileSuffix == "xfont")
							{
								printf("Add handler for .glyph or .xfont content file\n");
								DLUtil::addSetFont2Cmd(
								    m_LineEditor, pa, bitmapHandle, contentInfo->MemoryAddress,
								    contentInfo->FontOffset, line, hline);
							}
							else if (fileSuffix == "raw")
							{
								if (contentInfo->MapInfoFileType.contains(fileSuffix))
								{
									QStringList fileTypes = ContentInfo::MapInfoFileType.value(fileSuffix, {});
									bool existedFile = false;
									for (const auto &fileType : fileTypes) {
										if (existedFile) break;
										if (!fileType.isEmpty())
										{
											QString filePath = contentInfo->SourcePath.left(contentInfo->SourcePath.lastIndexOf('.') + 1).append(fileType);
											if (contentInfo->SourcePath.contains("_index"))
											{
												filePath.remove(filePath.lastIndexOf("_index"), 6);
											}
											QJsonObject infoJson = ReadWriteUtil::getJsonInfo(filePath);
											if (!infoJson.isEmpty()) existedFile = true;
											if (infoJson.contains("type"))
											{
												QString contentType = infoJson["type"].toString();
												if (contentType == "bitmap")
												{
													if (infoJson["format"].toString().toUpper().contains("PALETTED")
														&& !infoJson["format"].toString().toUpper().contains("PALETTED8"))
													{
														QString searchedFile = contentInfo->DestName;
														searchedFile.replace("_index", "_lut");
														const ContentInfo *searchedContent = m_MainWindow->contentManager()->find(
															searchedFile);
														DLUtil::addPalettedSrc(
															m_LineEditor, pa,
															(searchedContent
																	? searchedContent->bitmapAddress()
																	: 0),
															hline);
														line++;
													}
													DLUtil::addBitmapHandler(m_LineEditor, pa, contentInfo,
														bitmapHandle, line, hline);
												}
												else if (contentType == "legacyfont")
												{
													if (infoJson["eve_command"].toString() == "cmd_setfont")
													{
														DLUtil::addBitmapHandler(m_LineEditor, pa, contentInfo,
															bitmapHandle, line, hline);
														DLUtil::addSetFontCmd(m_LineEditor, pa,
															contentInfo->MemoryAddress,
															bitmapHandle, line, hline);
													}
													else
													{
														int firstChar = 1;
														if (infoJson.contains("first_character"))
															firstChar = infoJson["first_character"].toInt();
														DLUtil::addSetFont2Cmd(
															m_LineEditor, pa, bitmapHandle, contentInfo->MemoryAddress,
															firstChar, line, hline);
													}
												}
											}
										}
									}
								}
							}
						}
						else
						{
							if (fileSuffix != "avi")
							{
								DLUtil::addBitmapHandler(m_LineEditor, pa, contentInfo,
								    bitmapHandle, line, hline);
							}
							if (contentInfo->Converter == ContentInfo::Font)
							{
								DLUtil::addSetFontCmd(m_LineEditor, pa,
								    contentInfo->MemoryAddress, bitmapHandle,
								    line, hline);
							}
						}
					}

					if (!contentInfo)
					{
						int32_t x = mappingX(e) << m_LineEditor->getVertextFormat(line);
						int32_t y = mappingY(e) << m_LineEditor->getVertextFormat(line);
						DLUtil::addBitmapCmds(m_LineEditor, pa, contentInfo, selection,
						    line, x, y);
					}
					else if (contentInfo->Converter == ContentInfo::Font)
					{
						DLUtil::addTextCmd(m_LineEditor, pa, bitmapHandle, "Text", line,
						    mappingX(e), mappingY(e));
						m_LineEditor->selectLine(line - 1);
					}
					else if (contentInfo->Converter == ContentInfo::Raw)
					{
						auto infoJson = QJsonObject();
						auto fileSuffix = QFileInfo(contentInfo->SourcePath).suffix().toLower();
						if (contentInfo->MapInfoFileType.contains(fileSuffix))
						{
							QStringList fileTypes = ContentInfo::MapInfoFileType.value(fileSuffix, {});
							bool existedFile = false;
							for (const auto &fileType : fileTypes) {
								if (existedFile) break;
								if (!fileType.isEmpty())
								{
									QString filePath = contentInfo->SourcePath
														   .left(contentInfo->SourcePath.lastIndexOf('.') + 1)
														   .append(fileType);
									if (contentInfo->SourcePath.contains("_index"))
									{
										filePath.remove(filePath.lastIndexOf("_index"), 6);
									}
									infoJson = ReadWriteUtil::getJsonInfo(filePath);
									if (!infoJson.isEmpty()) existedFile = true;
								}
							}
						}

						if (fileSuffix == "ram_g")
						{
							debugLog("Create commands for .ram_g content file\n");
							int objAddr = 0;
							if (infoJson.contains("object"))
							{
								auto obj = infoJson.value("object").toObject();
								objAddr = obj.value("offset").toInt();
							}
							DLUtil::addRunAnimRamG(m_LineEditor, pa, line, objAddr,
							    mappingX(e), mappingY(e));
						}
						else if (fileSuffix == "flash")
						{
							debugLog("Create commands for .flash content file\n");
							int objAddr = 0;
							if (infoJson.contains("object"))
							{
								auto obj = infoJson.value("object").toObject();
								objAddr = obj.value("offset").toInt();
							}
							DLUtil::addRunAnimFlash(m_LineEditor, pa, line, contentInfo->FlashAddress,
							    objAddr, mappingX(e), mappingY(e));
						}
						else if (fileSuffix == "xfont")
						{
							debugLog("Create commands for .xfont content file\n");
							QString savedCharsFile = contentInfo->SourcePath
							                             .left(contentInfo->SourcePath.lastIndexOf('.'))
							                             .append("_converted_chars.txt");
							QString data = ReadWriteUtil::readConvertedCharsFile(savedCharsFile);
							DLUtil::addTextCmd(m_LineEditor, pa, bitmapHandle, data.left(5),
							    line, mappingX(e), mappingY(e));
						}
						else if (fileSuffix == "raw")
						{
							if (infoJson.contains("type"))
							{
								auto contentType = infoJson["type"].toString();
								if (contentType == "bitmap")
								{
									debugLog("Create commands for .raw bitmap content file");
									int32_t x = mappingX(e)
									    << m_LineEditor->getVertextFormat(line);
									int32_t y = mappingY(e)
									    << m_LineEditor->getVertextFormat(line);
									if (contentInfo->ImageFormat == PALETTED8)
									{
										debugLog("Create commands for PALETTED8 content file");
										auto searchedFile = contentInfo->DestName;
										searchedFile.replace("_index", "_lut");
										auto searchedContent = m_MainWindow->contentManager()->find(searchedFile);
										DLUtil::addPaletted8Cmds(
										    m_LineEditor, pa,
										    searchedContent ? searchedContent->bitmapAddress() : 0,
										    selection, line, x, y);
									}
									else
									{
										DLUtil::addBitmapCmds(m_LineEditor, pa, contentInfo,
										    selection, line, x, y);
									}
								}
								else if (contentType == "legacyfont")
								{
									debugLog(
									    "Create commands for .raw legacy font content file\n");
									QString savedCharsIndexFile = contentInfo->SourcePath
									                                  .left(contentInfo->SourcePath.lastIndexOf('.'))
									                                  .append("_converted_char_index.txt");
									int first = 0, last = 0;
									ReadWriteUtil::readConvertedCharsIndexFile(
									    savedCharsIndexFile, first, last);
									QString defaultText;
									if (first != 0 && last != 0)
									{
										int defaultNoOfChar = 5;
										int count = 0;
										int index = first + 1;
										while (count < defaultNoOfChar)
										{
											if (index == 34) // Character "
												++index;

											defaultText += QChar(index);
											++count;
											++index;
										}
									}
									DLUtil::addTextCmd(
									    m_LineEditor, pa, bitmapHandle,
									    !defaultText.isEmpty() ? defaultText : "Text", line,
									    mappingX(e), mappingY(e));
								}
							}
						}
					}
					else
					{
						auto fileSuffix = QFileInfo(contentInfo->SourcePath).suffix().toLower();
						if (fileSuffix == "avi")
						{
							printf("Create commands for .avi content file\n");
							pa.IdLeft = 0xFFFFFF00;
							pa.IdRight = CMD_PLAYVIDEO & 0xFF;
							pa.Parameter[0].I = 0;
							pa.StringParameter = contentInfo->SourcePath.toStdString();
							pa.ExpectedStringParameter = true;
							pa.ExpectedParameterCount = 2;
							m_LineEditor->insertLine(line, pa);
							++line;
						}
						else
						{
							int32_t x = mappingX(e) << m_LineEditor->getVertextFormat(line);
							int32_t y = mappingY(e) << m_LineEditor->getVertextFormat(line);
							DLUtil::addBitmapCmds(m_LineEditor, pa, contentInfo, selection,
							    line, x, y);
						}
					}

					m_LineEditor->codeEditor()->endUndoCombine();
					// m_MainWindow->undoStack()->endMacro();
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
			else if (selectionType == FTEDITOR_SELECTION_DL_STATE || selectionType == FTEDITOR_SELECTION_CMD_STATE)
			{
				bool useMouseStack;
				int lineOverride = -1;
				if (selectionType == FTEDITOR_SELECTION_DL_STATE && (selection == FTEDITOR_DL_CLEAR_COLOR_RGB || selection == FTEDITOR_DL_CLEAR_COLOR_A || selection == FTEDITOR_DL_CLEAR_STENCIL || selection == FTEDITOR_DL_CLEAR_TAG))
				{
					// Try to find CLEAR command backward from current pos
					int l = m_LineEditor->isCoprocessor() ? m_MouseStackCmdTop
					                                      : m_MouseStackDlTop;
					for (int i = l; i >= 0; --i)
					{
						DlParsed pai = m_LineEditor->getLine(i);
						if (pai.ValidId && (pai.IdLeft == FTEDITOR_DL_INSTRUCTION) && (pai.IdRight == FTEDITOR_DL_CLEAR))
						{
							lineOverride = i;
							break;
						}
					}
					useMouseStack = m_MouseStackValid;
				}
				else
				{
					// Only use primitive under cursor if it's not the CLEAR primitive
					DlParsed pac = m_LineEditor->getLine(m_LineEditor->isCoprocessor()
					        ? m_MouseStackCmdTop
					        : m_MouseStackDlTop);
					useMouseStack = m_MouseStackValid && !(pac.ValidId && (pac.IdLeft == FTEDITOR_DL_INSTRUCTION) && (pac.IdRight == FTEDITOR_DL_CLEAR));
				}
				int line = lineOverride >= 0
				    ? lineOverride
				    : (useMouseStack
				            ? (m_LineEditor->isCoprocessor() ? m_MouseStackCmdTop
				                                             : m_MouseStackDlTop)
				            : (m_LineNumber >= 0
				                    ? (m_LineEditor->getLine(m_LineNumber).ValidId
				                            ? m_LineNumber + 1
				                            : m_LineNumber)
				                    : 0));
				m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop property");
				DlParsed pa;
				pa.ValidId = true;
				pa.ExpectedStringParameter = false;
				pa.VarArgCount = 0;
				if (selectionType == FTEDITOR_SELECTION_CMD_STATE)
				{
					pa.IdLeft = 0xFFFFFF00;
					pa.IdRight = selection & 0xFF;
					switch (selection)
					{
					case CMD_REGREAD:
					case CMD_SETFONT:
					case CMD_TRANSLATE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_BGCOLOR:
						pa.Parameter[0].U = 0x7F3F1F;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_FGCOLOR:
						pa.Parameter[0].U = 0xFF7F3F;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_GRADCOLOR:
						pa.Parameter[0].U = 0xFFFF7F;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_LOADIDENTITY:
					case CMD_SETMATRIX:
						pa.ExpectedParameterCount = 0;
						break;
					case CMD_SCALE:
						pa.Parameter[0].I = 65536;
						pa.Parameter[1].I = 65536;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_FLASHSOURCE:
					case CMD_FILLWIDTH:
					case CMD_ROTATE:
						pa.Parameter[0].I = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_SETSCRATCH:
						pa.Parameter[0].I = 15;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_SETBASE:
						pa.Parameter[0].I = 10;
						pa.ExpectedParameterCount = 1;
						break;
					case CMD_ROMFONT:
						pa.Parameter[0].I = m_MainWindow->contentManager()->editorFindFreeHandle(
						    m_LineEditor);
						if (pa.Parameter[0].I < 0) pa.Parameter[0].I = 31;
						if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT880 && FTEDITOR_CURRENT_DEVICE <= FTEDITOR_BT883)
							pa.Parameter[1].I = 31;
						else
							pa.Parameter[1].I = 34;
						pa.ExpectedParameterCount = 2;
						break;
					case CMD_SETFONT2:
						pa.Parameter[0].I = 0;
						pa.Parameter[1].I = 0;
						pa.Parameter[2].I = 32;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_SETBITMAP:
						pa.Parameter[0].I = 0;
						pa.Parameter[1].I = 0;
						pa.Parameter[2].I = 64;
						pa.Parameter[3].I = 64;
						pa.ExpectedParameterCount = 4;
						break;
					case CMD_BITMAP_TRANSFORM:
						for (size_t i = 0; i < 13; i++)
						{
							pa.Parameter[i].I = 0;
						}
						pa.ExpectedParameterCount = 13;
						break;
					case CMD_ANIMXY:
						pa.Parameter[0].I = 0;
						pa.Parameter[1].I = 0;
						pa.Parameter[2].I = 0;
						pa.ExpectedParameterCount = 3;
						break;
					case CMD_RUNANIM:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].I = -1;
						pa.ExpectedParameterCount = 2;
						break;
					}
				}
				else
				{
					pa.IdLeft = 0;
					pa.IdRight = selection;
					switch (selection)
					{
					case FTEDITOR_DL_CLEAR_COLOR_RGB:
						pa.Parameter[0].U = 31;
						pa.Parameter[1].U = 63;
						pa.Parameter[2].U = 127;
						pa.ExpectedParameterCount = 3;
						break;
					case FTEDITOR_DL_CLEAR_COLOR_A:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_CLEAR_STENCIL:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_CLEAR_TAG:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_COLOR_RGB:
						pa.Parameter[0].U = 255;
						pa.Parameter[1].U = 255;
						pa.Parameter[2].U = 127;
						pa.ExpectedParameterCount = 3;
						break;
					case FTEDITOR_DL_COLOR_A:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_COLOR_MASK:
						pa.Parameter[0].U = 1;
						pa.Parameter[1].U = 1;
						pa.Parameter[2].U = 1;
						pa.Parameter[3].U = 1;
						pa.ExpectedParameterCount = 4;
						break;
					case FTEDITOR_DL_LINE_WIDTH:
						pa.Parameter[0].U = 64;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_POINT_SIZE:
						pa.Parameter[0].U = 256;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BLEND_FUNC:
						pa.Parameter[0].U = SRC_ALPHA;
						pa.Parameter[1].U = ONE_MINUS_SRC_ALPHA;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_SCISSOR_SIZE:
						pa.Parameter[0].U = 512;
						pa.Parameter[1].U = 512;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_SCISSOR_XY:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_ALPHA_FUNC:
						pa.Parameter[0].U = ALWAYS;
						pa.Parameter[1].U = ZERO;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_STENCIL_FUNC:
						pa.Parameter[0].U = ALWAYS;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 255;
						pa.ExpectedParameterCount = 3;
						break;
					case FTEDITOR_DL_STENCIL_MASK:
						pa.Parameter[0].U = 255;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_STENCIL_OP:
						pa.Parameter[0].U = KEEP;
						pa.Parameter[1].U = KEEP;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_TAG:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_TAG_MASK:
						pa.Parameter[0].U = 1;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_TRANSFORM_A:
					case FTEDITOR_DL_BITMAP_TRANSFORM_E:
						pa.Parameter[0].I = 256;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_TRANSFORM_B:
					case FTEDITOR_DL_BITMAP_TRANSFORM_C:
					case FTEDITOR_DL_BITMAP_TRANSFORM_D:
					case FTEDITOR_DL_BITMAP_TRANSFORM_F:
						pa.Parameter[0].I = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_HANDLE:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_SOURCE:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_LAYOUT:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 128;
						pa.Parameter[2].U = 64;
						pa.ExpectedParameterCount = 3;
						break;
					case FTEDITOR_DL_BITMAP_LAYOUT_H:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_BITMAP_SIZE:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.Parameter[2].U = 0;
						pa.Parameter[3].U = 64;
						pa.Parameter[4].U = 64;
						pa.ExpectedParameterCount = 5;
						break;
					case FTEDITOR_DL_BITMAP_SIZE_H:
						pa.Parameter[0].U = 0;
						pa.Parameter[1].U = 0;
						pa.ExpectedParameterCount = 2;
						break;
					case FTEDITOR_DL_PALETTE_SOURCE:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_CELL:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_RESTORE_CONTEXT:
					case FTEDITOR_DL_SAVE_CONTEXT:
						pa.ExpectedParameterCount = 0;
						break;
					case FTEDITOR_DL_VERTEX_FORMAT:
						pa.Parameter[0].U = 4;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_VERTEX_TRANSLATE_X:
					case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
						pa.Parameter[0].U = 256;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_EXT_FORMAT:
						pa.Parameter[0].U = 0;
						pa.ExpectedParameterCount = 1;
						break;
					case FTEDITOR_DL_BITMAP_SWIZZLE:
						pa.Parameter[0].U = 2;
						pa.Parameter[1].U = 3;
						pa.Parameter[2].U = 4;
						pa.Parameter[3].U = 5;
						pa.ExpectedParameterCount = 4;
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
	horizontalRuler()->setIndicator(EPOSPOINT(e).x());
	verticalRuler()->setIndicator(EPOSPOINT(e).y());
	if (acceptableSource(e))
	{
		if (m_LineEditor)
		{
			m_NextMouseX = mappingX(e);
			m_NextMouseY = mappingY(e);
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

} /* namespace FTEDITOR */

/* end of file */
