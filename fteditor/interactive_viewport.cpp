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

#pragma warning(disable : 26812)  // Unscoped enum
#pragma warning(disable : 26495)  // Uninitialized member
#pragma warning(disable : 26444)  // Unnamed objects

#include "interactive_viewport.h"

// STL includes

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QRegularExpression>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "code_editor.h"
#include "constant_common.h"
#include "constant_mapping.h"
#include "content_manager.h"
#include "inspector.h"
#include "main_window.h"
#include "properties_editor.h"
#include "toolbox.h"
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
#define POINTER_EDIT_WIDGET_MOVE                                    \
  (POINTER_EDIT_WIDGET_TRANSLATE | POINTER_EDIT_WIDGET_SIZE_TOP |   \
   POINTER_EDIT_WIDGET_SIZE_RIGHT | POINTER_EDIT_WIDGET_SIZE_LEFT | \
   POINTER_EDIT_WIDGET_SIZE_BOTTOM)
#define POINTER_EDIT_WIDGET_SIZE_TOPLEFT \
  (POINTER_EDIT_WIDGET_SIZE_TOP | POINTER_EDIT_WIDGET_SIZE_LEFT)
#define POINTER_EDIT_WIDGET_SIZE_TOPRIGHT \
  (POINTER_EDIT_WIDGET_SIZE_TOP | POINTER_EDIT_WIDGET_SIZE_RIGHT)
#define POINTER_EDIT_WIDGET_SIZE_BOTTOMLEFT \
  (POINTER_EDIT_WIDGET_SIZE_BOTTOM | POINTER_EDIT_WIDGET_SIZE_LEFT)
#define POINTER_EDIT_WIDGET_SIZE_BOTTOMRIGHT \
  (POINTER_EDIT_WIDGET_SIZE_BOTTOM | POINTER_EDIT_WIDGET_SIZE_RIGHT)
#define POINTER_INSERT 0x0200
#define POINTER_EDIT_GRADIENT_MOVE_1 0x0400
#define POINTER_EDIT_GRADIENT_MOVE_2 0x0800
#define POINTER_EDIT_GRADIENT_MOVE \
  (POINTER_EDIT_GRADIENT_MOVE_1 | POINTER_EDIT_GRADIENT_MOVE_2)

// Value outside display space
#define FTED_SNAP_HISTORY_NONE 4096

static QVector<float> ZoomRange({6.25, 12.5, 25, 37.5, 50, 62.5, 75, 100, 125,
                                 150, 200, 300, 400, 600, 800, 1200, 1600});
static const int ZOOM_INIT_INDEX = 7;  // index 7 means zooming at 100%

InteractiveViewport::InteractiveViewport(MainWindow *parent)
    : EmulatorViewport(parent, parent->applicationDataDir()),
      m_MainWindow(parent),
      m_PreferTraceCursor(false),
      m_TraceEnabled(false),
      m_MouseOver(false),
      m_MouseTouch(false),
      m_MouseStackValid(false),
      m_MouseStackWritten(false),
      m_PointerFilter(POINTER_ALL),
      m_PointerMethod(0),
      m_LineEditor(NULL),
      m_LineNumber(0),
      m_MouseOverVertex(false),
      m_MouseOverVertexLine(-1),
      m_MouseMovingVertex(false),
      m_WidgetXY(false),
      m_WidgetWH(false),
      m_WidgetR(false),
      m_WidgetGradient(false),
      m_MouseMovingWidget(0),
      m_SnapHistoryCur(0) {
  for (int i = 0; i < FTED_SNAP_HISTORY; ++i) {
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
  m_Insert->setStatusTip(
      tr("Place a new vertex or clone the selected widget directly on the "
         "screen"));
  m_Insert->setCheckable(true);

  /*
  // icon arrow left
  QAction *decrease = new QAction(this);
  // connect(decrease, SIGNAL(triggered()), this, SLOT(backTriggered()));
  decrease->setText(tr("Back"));
  decrease->setStatusTip(tr("Move selected vertex or widget behind the preceding
  operation"));

  // icon arrow right
  QAction *increase = new QAction(this);
  // connect(increase, SIGNAL(triggered()), this, SLOT(frontTriggered()));
  increase->setText(tr("Front"));
  increase->setStatusTip(tr("Move selected vertex or widget in front of the
  following operation"));
  */

  QToolBar *toolBar = m_MainWindow->addToolBar(tr("Toolbar"));
  toolBar->setIconSize(QSize(16, 16));
  toolBar->addAction(m_Insert);
  // toolBar->addAction(decrease);
  // toolBar->addAction(increase);

  QAction *ruler = new QAction;
  ruler->setText(tr("Ruler Alt+C"));
  ruler->setIcon(QIcon(":/icons/ruler.png"));
  ruler->setStatusTip(tr("Show/Hide the ruler"));
  ruler->setCheckable(true);
  ruler->setChecked(true);
  ruler->setShortcut(Qt::ALT | Qt::Key_Y);
  QToolBar *toolRulerBar = m_MainWindow->addToolBar(tr("Ruler"));
  toolRulerBar->setIconSize(QSize(16, 16));
  toolRulerBar->addAction(ruler);
  connect(ruler, &QAction::triggered, this, &EmulatorViewport::toggleViewRuler);
  connect(verticalRuler(), &QRuler::visibleChanged, ruler,
          &QAction::setChecked);

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

  connect(
      m_ZoomCB,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, &InteractiveViewport::zoomChanged);
  connect(m_ZoomCB->lineEdit(), &QLineEdit::returnPressed, this,
          &InteractiveViewport::zoomEditTextChanged);
  connect(m_ZoomCB,
          static_cast<void (QComboBox::*)(const QString &)>(
              &QComboBox::textActivated),
          [=](const QString &text) { m_ZoomCB->lineEdit()->selectAll(); });

  QStringList zoomList;

  for (int i = 0; i < ZoomRange.count(); i++) {
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
  QAction *actShowRuler = new QAction;
  actShowRuler->setCheckable(true);
  actShowRuler->setChecked(true);
  actShowRuler->setText(tr("Ruler"));
  m_ContextMenu->addAction(actShowRuler);
  connect(actShowRuler, &QAction::triggered, this,
          &EmulatorViewport::toggleViewRuler);
  connect(verticalRuler(), &QRuler::visibleChanged, actShowRuler,
          &QAction::setChecked);
  connect(this, &QWidget::customContextMenuRequested, this,
          [this](const QPoint &pos) {
            m_ContextMenu->exec(this->mapToGlobal(pos));
          });
}

InteractiveViewport::~InteractiveViewport() {}

void InteractiveViewport::zoomIn() {
  int scaleFactor = screenScale() * 100;
  float currentScale = scaleFactor / 16.0;

  // find next index
  for (int i = 0; i < ZoomRange.count(); i++) {
    if (currentScale < ZoomRange[i]) {
      m_ZoomCB->setCurrentIndex(i);
      break;
    }
  }
}

void InteractiveViewport::zoomOut() {
  int scaleFactor = screenScale() * 100;
  float currentScale = (float)scaleFactor / 16.0f;

  // find previous index
  for (int i = ZoomRange.count() - 1; i >= 0; i--) {
    if (currentScale > ZoomRange[i]) {
      m_ZoomCB->setCurrentIndex(i);
      break;
    }
  }
}

void InteractiveViewport::zoomChanged(int index) {
  if (index < 0 || index >= ZoomRange.count()) return;

  // zoom 100% equal to scale = 16
  setScreenScale(ZoomRange[index] * 0.16);
}

void InteractiveViewport::zoomEditTextChanged() {
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

bool InteractiveViewport::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_ZoomCB->lineEdit() &&
      event->type() == QEvent::MouseButtonPress) {
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

static bool isValidInsert(const DlParsed &parsed) {
  if (parsed.ValidId) {
    if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II ||
        parsed.IdLeft == FTEDITOR_DL_VERTEX2F) {
      return true;
    } else if (parsed.IdLeft == 0xFFFFFF00) {
      switch (parsed.IdRight | 0xFFFFFF00) {
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
          return FTEDITOR_CURRENT_DEVICE ==
                 FTEDITOR_FT801;  // Deprecated in FT810
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

// Graphics callback synchronized to the emulator thread, use to get debug
// information for a frame
void InteractiveViewport::graphics() {
  // Get the trace stack
  if (!m_TraceEnabled && m_MainWindow->traceEnabled()) {
    m_PreferTraceCursor = true;
  } else if (m_PreferTraceCursor && !m_MainWindow->traceEnabled()) {
    m_PreferTraceCursor = false;
  }
  m_TraceEnabled = m_MainWindow->traceEnabled();
  m_TraceX = m_MainWindow->traceX();
  m_TraceY = m_MainWindow->traceY();
  m_TraceStackDl.clear();
  m_TraceStackCmd.clear();
  bool cmdLast = false;
  if (m_TraceEnabled) {
    m_TraceStackSize = FTEDITOR_TRACE_STACK_SIZE;
    BT8XXEMU_processTrace(g_Emulator, m_TraceStack, &m_TraceStackSize, m_TraceX,
                          m_TraceY, hsize());
    for (int i = 0; i < m_TraceStackSize; ++i) {
      int cmdIdx = m_MainWindow->getDlCmd()[m_TraceStack[i]];
      if (cmdIdx >= 0) {
        m_TraceStackCmd.push_back(cmdIdx);
        cmdLast = true;
      } else {
        m_TraceStackDl.push_back(m_TraceStack[i]);
        cmdLast = false;
      }
    }
    if (cmdLast) {
      m_TraceStackDl.push_back(-1);
    } else {
      m_TraceStackCmd.push_back(-1);
    }
  }

  // Get the stack under the mouse cursor
  m_MouseStackWrite.clear();
  if (m_MouseOver || m_DragMoving) {
    if (m_NextMouseY >= 0 && m_NextMouseY < vsize() && m_NextMouseX > 0 &&
        m_NextMouseX < hsize()) {
      int size = FTEDITOR_TRACE_STACK_SIZE;
      m_MouseStackWrite.resize(FTEDITOR_TRACE_STACK_SIZE);
      BT8XXEMU_processTrace(g_Emulator, &m_MouseStackWrite[0], &size,
                            m_NextMouseX, m_NextMouseY, hsize());
      m_MouseStackWrite.resize(size);
      if (m_MouseStackWrite.size()) {
        m_MouseStackDlTop = m_MouseStackWrite[m_MouseStackWrite.size() - 1];
        m_MouseStackCmdTop = m_MainWindow->getDlCmd()[m_MouseStackDlTop];
        m_MouseStackValid = true;
      } else
        m_MouseStackValid = false;
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
void InteractiveViewport::graphics(QImage *image) {
  return;  // This callback can be used to make the navview
}

void InteractiveViewport::paintEvent(QPaintEvent *e) {
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
  if (m_MouseStackWritten) {
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
#define UNTFX(x) ((((x - mvx) * 16) / scl))
#define UNTFY(y) ((((y - mvy) * 16) / scl))
#define DRAWLINE(x1, y1, x2, y2) p.drawLine(TFX(x1), TFY(y1), TFX(x2), TFY(y2))
#define DRAWRECT(x, y, w, h) p.drawRect(TFX(x), TFY(y), SCX(w), SCY(h))
#if QT_VERSION_MAJOR < 6
#define EPOSPOINT(e) (e->pos())
#else
#define EPOSPOINT(e) (e->position().toPoint())
#endif

  if (m_MouseTouch && m_MouseX >= 0 && m_MouseX < getPixMap().width() &&
      m_MouseY >= 0 && m_MouseY < getPixMap().height()) {
    BT8XXEMU_touchSetXY(g_Emulator, 0, m_MouseX, m_MouseY, 0);
  } else {
    BT8XXEMU_touchResetXY(g_Emulator, 0);
  }

  // Draw image overlays
  /*QPainter p;
  p.begin(image);*/
  if (m_LineEditor) {
    DlParsed parsed = m_LineEditor->getLine(m_LineNumber);
    auto drawAlignment = [&](int specX = -1, int specY = -1) {
#define COLOR_FIT_VERTICALLY Qt::green
#define COLOR_FIT_HORIZONTALLY Qt::cyan
#define COLOR_CLOSE_FIT_VERTICALLY Qt::red
#define COLOR_CLOSE_FIT_HORIZONTALLY Qt::darkYellow
#define DISTANCE 8
#define AUTO_ALIGN_DISTANCE 4
      int x, y;
      DlParsed tempParsed;
      DlState tempState;
      QList dragableItems = {
          CMD_TEXT,      CMD_BUTTON,      CMD_KEYS,   CMD_PROGRESS, CMD_SLIDER,
          CMD_SCROLLBAR, CMD_TOGGLE,      CMD_GAUGE,  CMD_CLOCK,    CMD_SPINNER,
          CMD_TRACK,     CMD_DIAL,        CMD_NUMBER, CMD_SKETCH,   CMD_CSKETCH,
          CMD_ANIMFRAME, CMD_ANIMFRAMERAM};
      QList otherItems = {FTEDITOR_DL_VERTEX2F, FTEDITOR_DL_VERTEX2II};

      auto autoAlignVertical = false, autoAlignHorizontal = false;
      auto drawVerticalAlignment = [&](int x1, int x2) {
        p.setPen(QPen(QBrush(x1 == x2 ? COLOR_FIT_VERTICALLY
                                      : COLOR_CLOSE_FIT_VERTICALLY),
                      1.0, Qt::DashLine));
        if (abs(x1 - x2) > DISTANCE) return false;
        DRAWLINE(x2, 0, x2, vsize());
        return true;
      };
      auto drawHorizontalAlignment = [&](int y1, int y2) {
        p.setPen(QPen(QBrush(y1 == y2 ? COLOR_FIT_HORIZONTALLY
                                      : COLOR_CLOSE_FIT_HORIZONTALLY),
                      1.0, Qt::DashLine));
        if (abs(y1 - y2) > DISTANCE) return false;
        DRAWLINE(0, y2, hsize(), y2);
        return true;
      };
      auto checkClosest = [](int &closest, int &minDelta, int value1,
                             int value2) {
        int newDelta = abs(value1 - value2);
        if (newDelta < minDelta || minDelta < 0) {
          closest = value2;
          minDelta = newDelta;
        }
      };
      auto getClosestVertical = [&](int &minDelta, int &closest) {
        if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2F) {
          int tempX =
              tempParsed.Parameter[0].I >> tempState.Graphics.VertexFormat;
          tempX += tempState.Graphics.VertexTranslateX >> 4;
          checkClosest(closest, minDelta, x, tempX);
          return;
        }
        if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2II) {
          int tempX = tempParsed.Parameter[0].U;
          tempX += tempState.Graphics.VertexTranslateX >> 4;
          checkClosest(closest, minDelta, x, tempX);
          return;
        }

        bool tempM_WidgetWH = false;
        switch (tempParsed.IdRight | 0xFFFFFF00) {
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
        if (tempM_WidgetWH) {
          checkClosest(closest, minDelta, x,
                       tempParsed.Parameter[0].I +
                           tempParsed.Parameter[2].I / 2);  // mid
        }
        checkClosest(closest, minDelta, x, tempParsed.Parameter[0].I);  // left
        if (tempM_WidgetWH) {
          checkClosest(
              closest, minDelta, x,
              tempParsed.Parameter[0].I + tempParsed.Parameter[2].I);  // right
        }
      };
      auto getClosestHorizontal = [&](int &minDelta, int &closest) {
        if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2F) {
          int tempY =
              tempParsed.Parameter[1].I >> tempState.Graphics.VertexFormat;
          tempY += tempState.Graphics.VertexTranslateY >> 4;
          checkClosest(closest, minDelta, y, tempY);
          return;
        }
        if (tempParsed.IdLeft == FTEDITOR_DL_VERTEX2II) {
          int tempY = tempParsed.Parameter[1].U;
          tempY += tempState.Graphics.VertexTranslateY >> 4;
          checkClosest(closest, minDelta, y, tempY);
          return;
        }

        bool tempM_WidgetWH = false;
        switch (tempParsed.IdRight | 0xFFFFFF00) {
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
        if (tempM_WidgetWH) {
          checkClosest(closest, minDelta, y,
                       tempParsed.Parameter[1].I +
                           tempParsed.Parameter[3].I / 2);  // mid
        }
        checkClosest(closest, minDelta, y, tempParsed.Parameter[1].I);  // top
        if (tempM_WidgetWH) {
          checkClosest(
              closest, minDelta, y,
              tempParsed.Parameter[1].I + tempParsed.Parameter[3].I);  // bottom
        }
      };

      for (int i = 0; i < m_LineEditor->getLineCount(); i++) {
        if (i == m_LineNumber) continue;
        tempParsed = m_LineEditor->getLine(i);
        auto tempRightID = tempParsed.IdRight | 0xFFFFFF00;
        if (!dragableItems.contains(tempRightID) &&
            !otherItems.contains(tempParsed.IdLeft))
          continue;

        tempState = m_LineEditor->getState(i);
        int selPos = 0, closestOfSelPos = 0, closestOfPos = -1;
        int minDelta = -1, delta = -1;
        auto setSelPos = [&](int pos, int closestOfPos, int delta) {
          selPos = pos;
          closestOfSelPos = closestOfPos;
          minDelta = delta;
        };

        if (m_WidgetWH) {
          x = specX != -1
                  ? specX
                  : parsed.Parameter[0].I +
                        parsed.Parameter[2].I / 2;  // mid of the main item
          getClosestVertical(delta, closestOfPos);
          setSelPos(x, closestOfPos, delta);
        }
        if (true) {
          x = specX != -1 ? specX
                          : parsed.Parameter[0].I;  // left of the main item
          getClosestVertical(delta, closestOfPos);
          if (delta < minDelta || minDelta < 0) {
            setSelPos(x, closestOfPos, delta);
          }
        }
        if (m_WidgetWH) {
          x = specX != -1
                  ? specX
                  : parsed.Parameter[0].I +
                        parsed.Parameter[2].I;  // right of the main item
          getClosestVertical(delta, closestOfPos);
          if (delta < minDelta || minDelta < 0) {
            setSelPos(x, closestOfPos, delta);
          }
        }
        // Auto align vertical
        if (abs(selPos - closestOfSelPos) < AUTO_ALIGN_DISTANCE &&
            !autoAlignVertical) {
          int delta1 = closestOfSelPos - selPos;
          selPos = closestOfSelPos;
          autoAlignVertical = true;
          mouseMoveEvent(m_MovingLastX + delta1, m_MovingLastY);
        }
        drawVerticalAlignment(selPos, closestOfSelPos);

        delta = -1;
        closestOfPos = 0;
        setSelPos(0, -1, -1);
        if (m_WidgetWH) {
          y = specY != -1
                  ? specY
                  : parsed.Parameter[1].I +
                        parsed.Parameter[3].I / 2;  // mid of the main item
          getClosestHorizontal(delta, closestOfPos);
          setSelPos(y, closestOfPos, delta);
        }
        if (true) {
          y = specY != -1 ? specY
                          : parsed.Parameter[1].I;  // left of the main item
          getClosestHorizontal(delta, closestOfPos);
          if (delta < minDelta || minDelta < 0) {
            setSelPos(y, closestOfPos, delta);
          }
        }
        if (m_WidgetWH) {
          y = specY != -1
                  ? specY
                  : parsed.Parameter[1].I +
                        parsed.Parameter[3].I;  // right of the main item
          getClosestHorizontal(delta, closestOfPos);
          if (delta < minDelta || minDelta < 0) {
            setSelPos(y, closestOfPos, delta);
          }
        }
        // Auto align horizontal
        if (abs(selPos - closestOfSelPos) < AUTO_ALIGN_DISTANCE &&
            !autoAlignHorizontal) {
          int delta2 = closestOfSelPos - selPos;
          selPos = closestOfSelPos;
          autoAlignHorizontal = true;
          mouseMoveEvent(m_MovingLastX, m_MovingLastY + delta2);
        }
        drawHorizontalAlignment(selPos, closestOfSelPos);
      }
    };
    if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F ||
        parsed.IdLeft == FTEDITOR_DL_VERTEX2II) {
      m_WidgetXY = false;
      m_WidgetGradient = false;
      if (m_PointerFilter & POINTER_EDIT_VERTEX_MOVE) {
        QPen outer;
        QPen inner;
        outer.setWidth(3);
        outer.setColor(QColor(Qt::black));
        inner.setWidth(1);
        inner.setColor(QColor(Qt::gray));
        // Find first line
        int firstLine = m_LineNumber;
        for (int l = firstLine - 1; l > 0; --l) {
          const DlParsed &pa = m_LineEditor->getLine(l);
          if (pa.IdLeft == 0 && (pa.IdRight == FTEDITOR_DL_BEGIN ||
                                 pa.IdRight == FTEDITOR_DL_END ||
                                 pa.IdRight == FTEDITOR_DL_RETURN ||
                                 pa.IdRight == FTEDITOR_DL_JUMP)) {
            break;
          } else {
            firstLine = l;
          }
        }
        // Iterate over neighbouring vertices
        for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l)  // FIXME
        {
          if (l == m_LineNumber) continue;
          const DlParsed &pa = m_LineEditor->getLine(l);
          const DlState &st = m_LineEditor->getState(l);
          if (pa.IdLeft == FTEDITOR_DL_VERTEX2F ||
              pa.IdLeft == FTEDITOR_DL_VERTEX2II) {
            int x, y;
            if (pa.IdLeft == FTEDITOR_DL_VERTEX2F) {
              x = pa.Parameter[0].I >> st.Graphics.VertexFormat;
              y = pa.Parameter[1].I >> st.Graphics.VertexFormat;
            } else {
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
          } else if (pa.IdRight == FTEDITOR_DL_BEGIN ||
                     pa.IdRight == FTEDITOR_DL_END ||
                     pa.IdRight == FTEDITOR_DL_RETURN ||
                     pa.IdRight == FTEDITOR_DL_JUMP) {
            break;
          }
        }
        const DlState &state = m_LineEditor->getState(m_LineNumber);
        // Show central vertex
        int x, y;

        if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F) {
          x = parsed.Parameter[0].I >> state.Graphics.VertexFormat;
          y = parsed.Parameter[1].I >> state.Graphics.VertexFormat;
        } else {
          x = parsed.Parameter[0].U;
          y = parsed.Parameter[1].U;
        }

        x += state.Graphics.VertexTranslateX >> 4;
        y += state.Graphics.VertexTranslateY >> 4;

        if (m_isDrawAlignment) drawAlignment(x, y);

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
    } else if (parsed.IdLeft == 0xFFFFFF00 &&
               (((parsed.IdRight | 0xFFFFFF00) == CMD_GRADIENT) ||
                ((parsed.IdRight | 0xFFFFFF00) == CMD_GRADIENTA))) {
      const DlState &state = m_LineEditor->getState(m_LineNumber);

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
    } else if (parsed.IdLeft == 0xFFFFFF00)  // Coprocessor
    {
      m_WidgetGradient = false;
      switch (parsed.IdRight | 0xFFFFFF00) {
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
          switch (parsed.IdRight | 0xFFFFFF00) {
            case CMD_BUTTON:
            case CMD_KEYS:
            case CMD_PROGRESS:
            case CMD_SLIDER:
            case CMD_SCROLLBAR:
            case CMD_TRACK:
            case CMD_SKETCH:
            case CMD_CSKETCH:
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

          const DlState &state = m_LineEditor->getState(m_LineNumber);
          if (m_isDrawAlignment) drawAlignment();
          int x = parsed.Parameter[0].I;
          int y = parsed.Parameter[1].I;
          x += state.Graphics.VertexTranslateX >> 4;
          y += state.Graphics.VertexTranslateY >> 4;

          if (m_isDrawAlignmentHorizontal) {
            p.setPen(QPen(QBrush(Qt::red), 1.0, Qt::DashLine));
            DRAWLINE(0, y, hsize(), y);
          }

          if (m_isDrawAlignmentVertical) {
            p.setPen(QPen(QBrush(Qt::red), 1.0, Qt::DashLine));
            DRAWLINE(x, 0, x, vsize());
          }

          // CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
          // Draw...
          if (m_WidgetWH == false && m_WidgetR == false) {
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
          } else {
            int w, h;
            if (m_WidgetWH) {
              w = parsed.Parameter[2].I;
              h = parsed.Parameter[3].I;
            } else {
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
    } else {
      m_WidgetXY = false;
      m_WidgetGradient = false;
    }
  }
  if (m_TraceEnabled) {
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

void InteractiveViewport::setEditorLine(DlEditor *editor, int line) {
  if (m_MouseMovingVertex && m_LineEditor) {
    m_LineEditor->codeEditor()->endUndoCombine();
  }
  m_PreferTraceCursor = false;
  m_LineEditor = editor;
  m_LineNumber = line;
  // printf("Set line editor\n");
}

void InteractiveViewport::unsetEditorLine() {
  m_LineEditor = NULL;
  // printf("Unset line editor\n");
  m_Insert->setChecked(false);
}

void InteractiveViewport::automaticChecked() {
  m_PreferTraceCursor = false;
  m_PointerFilter = POINTER_ALL;
}

void InteractiveViewport::touchChecked() { m_PointerFilter = POINTER_TOUCH; }

void InteractiveViewport::traceChecked() { m_PointerFilter = POINTER_TRACE; }

void InteractiveViewport::editChecked() {
  m_PointerFilter = POINTER_EDIT_VERTEX_MOVE     // vertex movement
                    | POINTER_EDIT_STACK_SELECT  // stack selection
                    | POINTER_EDIT_WIDGET_MOVE   // widget movement
                    | POINTER_EDIT_GRADIENT_MOVE;
}

void InteractiveViewport::updatePointerMethod() {
  if (m_MouseTouch || m_MouseMovingVertex || m_MouseMovingWidget) {
    // Cannot change now
  } else {
    if (m_MainWindow->waitingCoprocessorAnimation()) goto PreferTouchCursor;
    if (m_Insert->isChecked()) {
      if (m_LineEditor) {
        const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
        if (isValidInsert(parsed)) {
          // Special case override cursur (not a filter), strange...
          m_PointerMethod = POINTER_INSERT;
          setCursor(Qt::CrossCursor);
          return;
        } else {
          printf("Uncheck insert\n");
          m_Insert->setChecked(false);
        }
      } else {
        printf("Force uncheck insert, no line editor\n");
        m_Insert->setChecked(false);
      }
    }
    if (m_PreferTraceCursor) {
      goto PreferTraceCursor;
    }
    // Vertex movement
    if (m_PointerFilter & POINTER_EDIT_VERTEX_MOVE) {
      if (m_LineEditor) {
        m_MouseOverVertex = false;
        m_MouseOverVertexLine = -1;
        const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
        if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F ||
            parsed.IdLeft == FTEDITOR_DL_VERTEX2II) {
          int firstLine = m_LineNumber;
          int vertexType = -1;
          for (int l = firstLine - 1; l > 0; --l) {
            const DlParsed &pa = m_LineEditor->getLine(l);
            if (pa.IdLeft == 0 && (pa.IdRight == FTEDITOR_DL_BEGIN ||
                                   pa.IdRight == FTEDITOR_DL_END ||
                                   pa.IdRight == FTEDITOR_DL_RETURN ||
                                   pa.IdRight == FTEDITOR_DL_JUMP)) {
              if (pa.IdRight == FTEDITOR_DL_BEGIN)
                vertexType = pa.Parameter[0].I;
              break;
            } else {
              firstLine = l;
            }
          }
          // Iterate over neighbouring vertices
          for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l)  // FIXME
          {
            const DlParsed &pa = m_LineEditor->getLine(l);
            if (pa.IdLeft == FTEDITOR_DL_VERTEX2F ||
                pa.IdLeft == FTEDITOR_DL_VERTEX2II) {
              const DlState &state = m_LineEditor->getState(l);
              int x, y;
              if (pa.IdLeft == FTEDITOR_DL_VERTEX2F) {
                x = pa.Parameter[0].I >> state.Graphics.VertexFormat;
                y = pa.Parameter[1].I >> state.Graphics.VertexFormat;
              } else {
                x = pa.Parameter[0].U;
                y = pa.Parameter[1].U;
              }
              x += state.Graphics.VertexTranslateX >> 4;
              y += state.Graphics.VertexTranslateY >> 4;

              // Mouse over
              if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY &&
                  m_MouseY < y + 4) {
                m_MouseOverVertex = true;
                m_MouseOverVertexLine = l;
                if (l == m_LineNumber)
                  break;  // Currently selected line always has preference
              }
            } else if (pa.IdRight == FTEDITOR_DL_BEGIN ||
                       pa.IdRight == FTEDITOR_DL_END ||
                       pa.IdRight == FTEDITOR_DL_RETURN ||
                       pa.IdRight == FTEDITOR_DL_JUMP) {
              break;
            }
          }
          if (m_MouseOverVertex) {
            setCursor(Qt::SizeAllCursor);
            m_PointerMethod = POINTER_EDIT_VERTEX_MOVE;  // move vertex
            return;
          }
          if ((vertexType == BITMAPS || vertexType == POINTS) &&
              ((m_MouseStackValid && m_LineEditor->isCoprocessor() &&
                m_MouseStackCmdTop == m_LineNumber) ||
               (m_MouseStackValid && !m_LineEditor->isCoprocessor() &&
                m_MouseStackDlTop == m_LineNumber))) {
            m_MouseOverVertex = true;
            m_MouseOverVertexLine = m_LineNumber;
            setCursor(Qt::SizeAllCursor);
            m_PointerMethod = POINTER_EDIT_VERTEX_MOVE;  // move vertex
            return;
          }
        }
      }
    }
    // Gradient move
    if (m_PointerFilter & POINTER_EDIT_GRADIENT_MOVE) {
      if (m_LineEditor) {
        if (m_WidgetGradient) {
          const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
          const DlState &state = m_LineEditor->getState(m_LineNumber);
          int x, y;
          x = parsed.Parameter[0].I;
          y = parsed.Parameter[1].I;
          x += state.Graphics.VertexTranslateX >> 4;
          y += state.Graphics.VertexTranslateY >> 4;
          if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY &&
              m_MouseY < y + 4) {
            setCursor(Qt::SizeAllCursor);
            m_PointerMethod = POINTER_EDIT_GRADIENT_MOVE_1;
            return;
          }
          x = parsed.Parameter[3].I;
          y = parsed.Parameter[4].I;
          x += state.Graphics.VertexTranslateX >> 4;
          y += state.Graphics.VertexTranslateY >> 4;
          if (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY &&
              m_MouseY < y + 4) {
            setCursor(Qt::SizeAllCursor);
            m_PointerMethod = POINTER_EDIT_GRADIENT_MOVE_2;
            return;
          }
        }
      }
    }
    // Widget movement
    if (m_PointerFilter & POINTER_EDIT_WIDGET_MOVE) {
      if (m_LineEditor) {
        if (m_WidgetXY) {
          const DlParsed &parsed = m_LineEditor->getLine(m_LineNumber);
          const DlState &state = m_LineEditor->getState(m_LineNumber);
          int x = parsed.Parameter[0].I;
          int y = parsed.Parameter[1].I;
          x += state.Graphics.VertexTranslateX >> 4;
          y += state.Graphics.VertexTranslateY >> 4;
          if (m_WidgetWH || m_WidgetR) {
            int w, h;
            if (m_WidgetWH) {
              w = parsed.Parameter[2].I;
              h = parsed.Parameter[3].I;
            } else {
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
            if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_TOP) {
              if (x1 - 3 < m_MouseX && m_MouseX < x2 + 3 && y1 - 3 < m_MouseY &&
                  m_MouseY < y1 + 3) {
                m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_TOP;
              }
            }
            if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_BOTTOM) {
              if (x1 - 3 < m_MouseX && m_MouseX < x2 + 3 && y2 - 3 < m_MouseY &&
                  m_MouseY < y2 + 3) {
                m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_BOTTOM;
              }
            }
            if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_LEFT) {
              if (x1 - 3 < m_MouseX && m_MouseX < x1 + 3 && y1 - 3 < m_MouseY &&
                  m_MouseY < y2 + 3) {
                m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_LEFT;
              }
            }
            if (m_PointerFilter & POINTER_EDIT_WIDGET_SIZE_RIGHT) {
              if (x2 - 3 < m_MouseX && m_MouseX < x2 + 3 && y1 - 3 < m_MouseY &&
                  m_MouseY < y2 + 3) {
                m_PointerMethod |= POINTER_EDIT_WIDGET_SIZE_RIGHT;
              }
            }
            if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPLEFT ||
                m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMRIGHT)
              setCursor(Qt::SizeFDiagCursor);
            else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOPRIGHT ||
                     m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOMLEFT)
              setCursor(Qt::SizeBDiagCursor);
            else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_TOP ||
                     m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_BOTTOM)
              setCursor(Qt::SizeVerCursor);
            else if (m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_LEFT ||
                     m_PointerMethod == POINTER_EDIT_WIDGET_SIZE_RIGHT)
              setCursor(Qt::SizeHorCursor);
            if (m_PointerMethod) return;
            if (m_PointerFilter & POINTER_EDIT_WIDGET_TRANSLATE) {
              if (x1 < m_MouseX && m_MouseX < x2 && y1 < m_MouseY &&
                  m_MouseY < y2) {
                // m_MouseOverWidget = true;
                setCursor(Qt::SizeAllCursor);
                m_PointerMethod =
                    POINTER_EDIT_WIDGET_TRANSLATE;  // translate widget
                return;
                // CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
              }
            }
          } else {
            if (m_PointerFilter & POINTER_EDIT_WIDGET_TRANSLATE) {
              if ((m_MouseStackValid && m_LineEditor->isCoprocessor() &&
                   m_MouseStackCmdTop == m_LineNumber) ||
                  (m_MouseStackValid && !m_LineEditor->isCoprocessor() &&
                   m_MouseStackDlTop == m_LineNumber) ||
                  (x - 4 < m_MouseX && m_MouseX < x + 4 && y - 4 < m_MouseY &&
                   m_MouseY < y + 4)) {
                // m_MouseOverWidget = true;
                setCursor(Qt::SizeAllCursor);
                m_PointerMethod =
                    POINTER_EDIT_WIDGET_TRANSLATE;  // translate widget
                return;
                // CMD_CLOCK(50, 50, 50, 0, 0, 0, 0, 0)
              }
            }
          }
        }
      }
    }
    // Stack selection
    if (m_PointerFilter & POINTER_EDIT_STACK_SELECT) {
      setCursor(Qt::ArrowCursor);
      m_PointerMethod = POINTER_EDIT_STACK_SELECT;  // Stack selection
      return;
    }
  PreferTouchCursor:
    if (m_PointerFilter & POINTER_TOUCH) {
      // TODO: Get the TAG from stack trace and show hand or not depending on
      // TAG value (maybe also show tootip with tag?)
      setCursor(Qt::PointingHandCursor);
      m_PointerMethod = POINTER_TOUCH;
      return;
    }
    if (m_MainWindow->waitingCoprocessorAnimation()) goto NoCursor;
  PreferTraceCursor:
    if (m_PointerFilter & POINTER_TRACE) {
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

void InteractiveViewport::snapPos(int &xd, int &yd, int xref, int yref) {
  // Disabled
  xd = 0;
  yd = 0;
  return;

  if (QApplication::keyboardModifiers() &
      (Qt::ControlModifier | Qt::ShiftModifier)) {
    // Hold CTRL to disable snap
    xd = 0;
    yd = 0;
  } else {
    int xsnap = xref;
    int ysnap = yref;
    int xdist = FTED_SNAP_HISTORY_NONE;
    int ydist = FTED_SNAP_HISTORY_NONE;
    for (int i = 0; i < FTED_SNAP_HISTORY; ++i) {
      if (i == m_SnapHistoryCur) continue;
      int nxdist = abs(m_SnapHistoryX[i] - xref);
      int nydist = abs(m_SnapHistoryY[i] - yref);
      if (nxdist < xdist && nxdist < FTED_SNAP_DIST) {
        xdist = nxdist;
        xsnap = m_SnapHistoryX[i];
      }
      if (nydist < ydist && nydist < FTED_SNAP_DIST) {
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

void InteractiveViewport::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Right) {
    m_MovingLastX -= 1;
    mouseMoveEvent(m_MovingLastX + 1, m_MovingLastY);
  } else if (e->key() == Qt::Key_Left) {
    m_MovingLastX += 1;
    mouseMoveEvent(m_MovingLastX - 1, m_MovingLastY);
  } else if (e->key() == Qt::Key_Up) {
    m_MovingLastY += 1;
    mouseMoveEvent(m_MovingLastX, m_MovingLastY - 1);
  } else if (e->key() == Qt::Key_Down) {
    m_MovingLastY -= 1;
    mouseMoveEvent(m_MovingLastX, m_MovingLastY + 1);
  } else {
    printf("Surpress keyboard\n");
  }
}

void InteractiveViewport::mouseMoveEvent(int mouseX, int mouseY,
                                         Qt::KeyboardModifiers km) {
  // printf("pos: %i, %i\n", EPOSPOINT(e).x(), EPOSPOINT(e).y());
  m_NextMouseX = mouseX;
  m_NextMouseY = mouseY;
  fetchColorAsync(mouseX, mouseY);

  m_MainWindow->statusBar()->showMessage("");
  m_isDrawAlignmentHorizontal = m_isDrawAlignmentVertical = false;

  if (m_MouseTouch) {
    // BT8XXEMU_setTouchScreenXY(EPOSPOINT(e).x(), EPOSPOINT(e).y(), 0);
  } else if (m_MouseMovingVertex) {
    if (m_LineEditor) {
      bool reEnableUndoCombine = false;
      int xd = mouseX - m_MovingLastX;
      int yd = mouseY - m_MovingLastY;
      m_isDrawAlignment = true;
      m_MovingLastX = mouseX;
      m_MovingLastY = mouseY;
      DlParsed pa = m_LineEditor->getLine(m_LineNumber);
      int otherVertices[4];
      int numOtherV = 0;
      if (m_LineNumber > 0 && m_LineEditor->getLine(m_LineNumber - 1).ValidId &&
          m_LineEditor->getLine(m_LineNumber - 1).IdLeft == 0 &&
          m_LineEditor->getLine(m_LineNumber - 1).IdRight ==
              FTEDITOR_DL_PALETTE_SOURCE) {
        int firstLine = m_LineNumber;
        for (int l = firstLine - 1; l > 0; --l) {
          const DlParsed &parsed = m_LineEditor->getLine(l);
          if (parsed.IdLeft == 0 && (parsed.IdRight == FTEDITOR_DL_BEGIN ||
                                     parsed.IdRight == FTEDITOR_DL_END ||
                                     parsed.IdRight == FTEDITOR_DL_RETURN ||
                                     parsed.IdRight == FTEDITOR_DL_JUMP)) {
            break;
          } else {
            firstLine = l;
          }
        }
        for (int l = firstLine; l < FTEDITOR_DL_SIZE; ++l)  // FIXME
        {
          if (l == m_LineNumber) continue;
          const DlParsed &parsed = m_LineEditor->getLine(l);

          if (!parsed.ValidId) continue;

          if (parsed.IdLeft == pa.IdLeft &&
              parsed.Parameter[0].I == pa.Parameter[0].I &&
              parsed.Parameter[1].I == pa.Parameter[1].I) {
            if (l != m_LineNumber) {
              otherVertices[numOtherV] = l;
              ++numOtherV;
              if (numOtherV >= 4) break;
            }
          } else if (pa.IdRight == FTEDITOR_DL_BEGIN ||
                     pa.IdRight == FTEDITOR_DL_END ||
                     pa.IdRight == FTEDITOR_DL_RETURN ||
                     pa.IdRight == FTEDITOR_DL_JUMP) {
            break;
          }
        }
      }
      // In case automatic expansion is necessary
      // if (pa.IdLeft == FTEDITOR_DL_VERTEX2II && shift) change to
      // FTEDITOR_DL_VERTEX2F and add the HANDLE and CELL if necessary
      const DlState &state = m_LineEditor->getState(m_LineNumber);
      /*if (pa.IdLeft == FTEDITOR_DL_VERTEX2II &&
      (QApplication::keyboardModifiers() & Qt::ShiftModifier))
      {
              // Doesn't work directly due to issue with undo combining when
      changing IdLeft reEnableUndoCombine = true;
              m_LineEditor->codeEditor()->endUndoCombine();

              pa.IdLeft = FTEDITOR_DL_VERTEX2F;
              pa.Parameter[0].I <<= state.Graphics.VertexFormat;
              pa.Parameter[1].I <<= state.Graphics.VertexFormat;

              switch (state.Rendering.Primitive)
              {
              case BITMAPS: // Only do when current primitive is BITMAP
                      DlParsed npa;
                      npa.ValidId = true;
                      npa.IdLeft = 0;
                      npa.ExpectedStringParameter = false;
                      npa.ExpectedParameterCount = 1;
                      if (state.Graphics.BitmapHandle != pa.Parameter[2].U)
                      {
                              npa.IdRight = FTEDITOR_DL_BITMAP_HANDLE;
                              npa.Parameter[0].U = pa.Parameter[2].U;
                              m_LineEditor->insertLine(numOtherV &&
      otherVertices[0] < m_LineNumber ? otherVertices[0] : m_LineNumber, npa);
                              for (int i = 0; i < numOtherV; ++i)
                                      ++otherVertices[i];
                      }
                      if (state.Graphics.Cell != pa.Parameter[3].U)
                      {
                              npa.IdRight = FTEDITOR_DL_CELL;
                              npa.Parameter[0].U = pa.Parameter[3].U;
                              m_LineEditor->insertLine(numOtherV &&
      otherVertices[0] < m_LineNumber ? otherVertices[0] : m_LineNumber, npa);
                              for (int i = 0; i < numOtherV; ++i)
                                      ++otherVertices[i];
                      }
                      // TODO: Restore the state if necessary for in front of
      any following VERTEX2F! break;
              }
      }
      if (pa.IdLeft == FTEDITOR_DL_VERTEX2F &&
      !(QApplication::keyboardModifiers() & Qt::ShiftModifier))
      {
              xd <<= state.Graphics.VertexFormat;
              yd <<= state.Graphics.VertexFormat;
      }*/

      if (pa.IdLeft == FTEDITOR_DL_VERTEX2F) {
        xd <<= state.Graphics.VertexFormat;
        yd <<= state.Graphics.VertexFormat;
      }

      pa.Parameter[0].I += xd;
      pa.Parameter[1].I += yd;

      if (pa.IdLeft == FTEDITOR_DL_VERTEX2II) {
        // Snap ->
        int snapx, snapy;
        snapPos(snapx, snapy, pa.Parameter[0].I, pa.Parameter[1].I);
        m_MovingLastX += snapx;
        pa.Parameter[0].I += snapx;
        m_MovingLastY += snapy;
        pa.Parameter[1].I += snapy;
        // if (snapx != 0) printf("snapx: %i\n", snapx);
        // if (snapy != 0) printf("snapy: %i\n", snapy);
        //  <- Snap

        // Do not allow negative values
        if (pa.Parameter[0].I < 0) {
          m_MovingLastX -= pa.Parameter[0].I;
          pa.Parameter[0].I = 0;
        }
        if (pa.Parameter[0].I > 511) {
          int diff = pa.Parameter[0].I - 511;
          m_MovingLastX -= diff;
          pa.Parameter[0].I -= diff;
        }
        if (pa.Parameter[1].I < 0) {
          m_MovingLastY -= pa.Parameter[1].I;
          pa.Parameter[1].I = 0;
        }
        if (pa.Parameter[1].I > 511) {
          int diff = pa.Parameter[1].I - 511;
          m_MovingLastY -= diff;
          pa.Parameter[1].I -= diff;
        }
      } else {
        // Snap ->
        int snapx, snapy;
        snapPos(snapx, snapy, pa.Parameter[0].I >> 4, pa.Parameter[1].I >> 4);
        m_MovingLastX += snapx;
        pa.Parameter[0].I += snapx << 4;
        m_MovingLastY += snapy;
        pa.Parameter[1].I += snapy << 4;
        // <- Snap
      }
      m_LineEditor->replaceLine(m_LineNumber, pa);
      for (int i = 0; i < numOtherV; ++i)
        m_LineEditor->replaceLine(otherVertices[i], pa);
      if (reEnableUndoCombine) {
        m_LineEditor->codeEditor()->beginUndoCombine(tr("Move vertex"));
      }
    } else {
      m_MouseMovingVertex = false;
      updatePointerMethod();  // update because update is not done while
                              // m_MouseMovingVertex true
    }
  } else if (m_MouseMovingWidget) {
    if (m_LineEditor) {
      m_MainWindow->statusBar()->showMessage(
          "Press SHIFT for keeping constant x-coordinate, ALT for keeping "
          "constant y-coordinate");
      m_isDrawAlignment = true;
      // Apply action
      int xd = 0;
      int yd = 0;

      if (km != Qt::ShiftModifier) {
        xd = mouseX - m_MovingLastX;
        m_MovingLastX = mouseX;
      } else {
        m_isDrawAlignmentVertical = true;
      }

      if (km != Qt::AltModifier) {
        yd = mouseY - m_MovingLastY;
        m_MovingLastY = mouseY;
      } else {
        m_isDrawAlignmentHorizontal = true;
      }
      DlParsed pa = m_LineEditor->getLine(m_LineNumber);
      if (m_MouseMovingWidget == POINTER_EDIT_WIDGET_TRANSLATE ||
          m_MouseMovingWidget == POINTER_EDIT_GRADIENT_MOVE_1) {
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
      } else if (m_MouseMovingWidget == POINTER_EDIT_GRADIENT_MOVE_2) {
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
      } else  // resize, check top/bottom and left/right
      {
        int x = pa.Parameter[0].I;
        int y = pa.Parameter[1].I;
        if (m_WidgetWH) {
          const int minsize = 0;
          int w = pa.Parameter[2].I;
          int h = pa.Parameter[3].I;
          if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_TOP) {
            y += yd;
            h -= yd;

            // Snap ->
            int snapx, snapy;
            snapPos(snapx, snapy, x, y);
            m_MovingLastY += snapy;
            y += snapy;
            h -= snapy;
            // <- Snap

            if (h < minsize) {
              m_MovingLastY += (h - minsize);
              y += (h - minsize);
              h = minsize;
            }
          } else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM) {
            h += yd;

            // Snap ->
            int snapx, snapy;
            snapPos(snapx, snapy, x + w, y + h);
            m_MovingLastY += snapy;
            h += snapy;
            // <- Snap

            if (h < minsize) {
              m_MovingLastY -= (h - minsize);
              h = minsize;
            }
          }
          if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT) {
            x += xd;
            w -= xd;

            // Snap ->
            int snapx, snapy;
            snapPos(snapx, snapy, x, y);
            m_MovingLastX += snapx;
            x += snapx;
            w -= snapx;
            // <- Snap

            if (w < minsize) {
              m_MovingLastX += (w - minsize);
              x += (w - minsize);
              w = minsize;
            }
          } else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT) {
            w += xd;

            // Snap ->
            int snapx, snapy;
            snapPos(snapx, snapy, x + w, y + h);
            m_MovingLastX += snapx;
            w += snapx;
            // <- Snap

            if (w < minsize) {
              m_MovingLastX -= (w - minsize);
              w = minsize;
            }
          }
          pa.Parameter[0].I = x;
          pa.Parameter[1].I = y;
          pa.Parameter[2].I = w;
          pa.Parameter[3].I = h;
        } else {
          int r = pa.Parameter[2].I;
          if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_TOP) {
            r -= yd;
            if (r < 0) {
              m_MovingLastY += r;
              r = 0;
            }
          } else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_BOTTOM) {
            r += yd;
            if (r < 0) {
              m_MovingLastY -= r;
              r = 0;
            }
          } else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_LEFT) {
            r -= xd;
            if (r < 0) {
              m_MovingLastX += r;
              r = 0;
            }
          } else if (m_MouseMovingWidget & POINTER_EDIT_WIDGET_SIZE_RIGHT) {
            r += xd;
            if (r < 0) {
              m_MovingLastX -= r;
              r = 0;
            }
          }
          pa.Parameter[2].I = r;
        }
      }
      m_LineEditor->replaceLine(m_LineNumber, pa);
    } else {
      m_MouseMovingWidget = 0;
      updatePointerMethod();  // update because update is not done while
                              // m_MouseMovingWidget has a value
    }
  }
}

void InteractiveViewport::wheelEvent(QWheelEvent *e) {
  int mvx = screenLeft();
  int mvy = screenTop();
  int scl = screenScale();
  int curx = UNTFX(e->position().toPoint().x());
  int cury = UNTFY(e->position().toPoint().y());

  if (e->angleDelta().y() > 0) {
    zoomIn();
  } else if (e->angleDelta().y() < 0) {
    zoomOut();
  }

  mvx = screenLeft();
  mvy = screenTop();
  scl = screenScale();
  int newx = UNTFX(e->position().toPoint().x());
  int newy = UNTFY(e->position().toPoint().y());

  int nx = (curx - newx) * 16;
  int ny = (cury - newy) * 16;

  horizontalScrollbar()->setValue(horizontalScrollbar()->value() + nx);
  verticalScrollbar()->setValue(verticalScrollbar()->value() + ny);

  EmulatorViewport::wheelEvent(e);
}

void InteractiveViewport::mouseMoveEvent(QMouseEvent *e) {
  int mvx = screenLeft();
  int mvy = screenTop();
  int scl = screenScale();

  horizontalRuler()->setIndicator(EPOSPOINT(e).x());
  verticalRuler()->setIndicator(EPOSPOINT(e).y());
  mouseMoveEvent(UNTFX(EPOSPOINT(e).x()), UNTFY(EPOSPOINT(e).y()),
                 e->modifiers());
  EmulatorViewport::mouseMoveEvent(e);
}

void InteractiveViewport::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::RightButton) {
    emit this->customContextMenuRequested(e->position().toPoint());
    return;
  }
  m_MainWindow->cmdEditor()->codeEditor()->setKeyHandler(this);
  m_MainWindow->dlEditor()->codeEditor()->setKeyHandler(this);

  int mvx = screenLeft();
  int mvy = screenTop();
  int scl = screenScale();

  switch (m_PointerMethod) {
    case POINTER_TOUCH:  // touch
      if (e->button() == Qt::LeftButton) {
        m_MouseTouch = true;
        // BT8XXEMU_setTouchScreenXY(EPOSPOINT(e).x(), EPOSPOINT(e).y(), 0);
      }
      break;
    case POINTER_TRACE:  // trace
      switch (e->button()) {
        case Qt::LeftButton:
          m_MainWindow->setTraceX(UNTFX(EPOSPOINT(e).x()));
          m_MainWindow->setTraceY(UNTFY(EPOSPOINT(e).y()));
          m_MainWindow->setTraceEnabled(true);
          break;
        case Qt::MiddleButton:
          if (m_PointerFilter != POINTER_TRACE) {
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
      if (m_LineEditor) {
        if (m_MouseOverVertexLine != m_LineNumber) {
          m_LineEditor->selectLine(m_MouseOverVertexLine);
        }
        m_MovingLastX = UNTFX(EPOSPOINT(e).x());
        m_MovingLastY = UNTFY(EPOSPOINT(e).y());
        m_MouseMovingVertex = true;
        m_LineEditor->codeEditor()->beginUndoCombine(tr("Move vertex"));
      }
      break;
    case POINTER_EDIT_STACK_SELECT:
      if (m_MouseStackRead.size() > 0) {
        if (e->button() == Qt::LeftButton) {
          // Select topmost command
          printf("Select topmost command\n");
          int idxDl = m_MouseStackRead[m_MouseStackRead.size() - 1];  // DL
          int idxCmd = m_MainWindow->getDlCmd()[idxDl];
          printf("DL %i\n", idxDl);
          printf("CMD %i\n", idxCmd);
          // m_LineEditor->selectLine(idx);
          if (idxCmd >= 0) {
            m_MainWindow->focusCmdEditor();
            m_MainWindow->cmdEditor()->selectLine(idxCmd);
          } else {
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
      if (e->button() == Qt::LeftButton) {
        int line = m_LineNumber;
        DlParsed pa = m_LineEditor->getLine(line);
        if (isValidInsert(pa)) {
          ++line;
          pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x())
                              << m_LineEditor->getVertextFormat(line);
          pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y())
                              << m_LineEditor->getVertextFormat(line);
          m_LineEditor->insertLine(line, pa);
          m_LineEditor->selectLine(line);
        }
        break;
      } else if (e->button() == Qt::MiddleButton ||
                 e->button() == Qt::RightButton) {
        m_Insert->setChecked(false);
      }
    default:
      if (m_PointerMethod &
          (POINTER_EDIT_WIDGET_MOVE | POINTER_EDIT_GRADIENT_MOVE)) {
        // Works for any widget move action
        m_MovingLastX = UNTFX(EPOSPOINT(e).x());
        m_MovingLastY = UNTFY(EPOSPOINT(e).y());
        m_MouseMovingWidget = m_PointerMethod;
        m_LineEditor->codeEditor()->beginUndoCombine(tr("Move widget"));
      }
      break;
  }

  EmulatorViewport::mousePressEvent(e);
}

void InteractiveViewport::mouseReleaseEvent(QMouseEvent *e) {
  m_MainWindow->cmdEditor()->codeEditor()->setKeyHandler(NULL);
  m_MainWindow->dlEditor()->codeEditor()->setKeyHandler(NULL);

  m_MainWindow->statusBar()->showMessage("");
  m_isDrawAlignmentHorizontal = m_isDrawAlignmentVertical = false;
  m_isDrawAlignment = false;

  if (m_MouseTouch) {
    m_MouseTouch = false;
    // BT8XXEMU_resetTouchScreenXY();
    updatePointerMethod();  // update because update is not done while
                            // m_MouseTouch true
  } else if (m_MouseMovingVertex) {
    m_MouseMovingVertex = false;
    if (m_LineEditor) {
      m_LineEditor->codeEditor()->endUndoCombine();
    }
    ++m_SnapHistoryCur;
    m_SnapHistoryCur %= FTED_SNAP_HISTORY;
    updatePointerMethod();  // update because update is not done while
                            // m_MouseMovingVertex true
  } else if (m_MouseMovingWidget) {
    m_MouseMovingWidget = 0;
    if (m_LineEditor) {
      m_LineEditor->codeEditor()->endUndoCombine();
    }
    ++m_SnapHistoryCur;
    m_SnapHistoryCur %= FTED_SNAP_HISTORY;
    updatePointerMethod();  // update because update is not done while
                            // m_MouseMovingWidget has a value
  }

  EmulatorViewport::mouseReleaseEvent(e);
}

#if QT_VERSION_MAJOR < 6
void InteractiveViewport::enterEvent(QEvent *e)
#else
void InteractiveViewport::enterEvent(QEnterEvent *e)
#endif
{
  // printf("InteractiveViewport::enterEvent\n");

  m_MouseOver = true;
  horizontalRuler()->setShowIndicator(true);
  verticalRuler()->setShowIndicator(true);

  EmulatorViewport::enterEvent(e);
}

void InteractiveViewport::leaveEvent(QEvent *e) {
  // printf("InteractiveViewport::leaveEvent\n");

  m_MouseOver = false;
  horizontalRuler()->setShowIndicator(false);
  verticalRuler()->setShowIndicator(false);

  EmulatorViewport::leaveEvent(e);
}

bool InteractiveViewport::acceptableSource(QDropEvent *e) {
  if (e->source() == m_MainWindow->toolbox()->treeWidget()) return true;
  // if (e->source() == m_MainWindow->bitmapSetup()) return true;
  if (e->source() == m_MainWindow->contentManager()->contentList()) {
    auto currentItem = m_MainWindow->contentManager()->current();

    QStringList supportedList = {"flash", "ram_g", "raw", "xfont", "avi"};
    QString fileSuffix = QFileInfo(currentItem->SourcePath).suffix().toLower();
    if (fileSuffix == "raw" && currentItem->SourcePath.contains("_lut"))
      return false;
    else if (supportedList.contains(fileSuffix))
      return true;

    if (!currentItem->MemoryLoaded) return false;

    if (currentItem->Converter != ContentInfo::Image &&
        currentItem->Converter != ContentInfo::Font &&
        currentItem->Converter != ContentInfo::ImageCoprocessor)
      return false;
    return true;
  }
  return false;
}

inline bool requirePaletteAddress(ContentInfo *contentInfo) {
  switch (contentInfo->ImageFormat) {
    case PALETTED4444:
    case PALETTED565:
    case PALETTED8:
      return true;
  }
  return false;
}

void InteractiveViewport::dropEvent(QDropEvent *e) {
  // Should probably lock the display list at this point ... ?
  // TODO: Bitmaps from files, etc
  if (acceptableSource(e)) {
    if (m_LineEditor && !m_LineEditor->isMacro()) {
      e->accept();

      uint32_t selectionType;
      uint32_t selection;
      uint32_t bitmapHandle;
      ContentInfo *contentInfo;
      if (e->source() == m_MainWindow->toolbox()->treeWidget()) {
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
      else if (e->source() == m_MainWindow->contentManager()->contentList()) {
        selectionType = FTEDITOR_SELECTION_PRIMITIVE;  // PRIMITIVE
        selection = BITMAPS;
        bitmapHandle = 0;
        contentInfo = m_MainWindow->contentManager()->current();
      } else {
        printf("Unknown error. This code is unreachable\n");
        return;
      }

      if (m_LineEditor->isCoprocessor()) {
        m_MainWindow->focusCmdEditor();
      } else {
        m_MainWindow->focusDlEditor();
      }

      if (selectionType == FTEDITOR_SELECTION_PRIMITIVE ||
          selectionType == FTEDITOR_SELECTION_FUNCTION ||
          selectionType == FTEDITOR_SELECTION_VERTEX) {
        int line = m_LineNumber;
        if (m_LineEditor->getLine(line).ValidId) {
          // printf("Valid Id\n");
          if (m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_VERTEX2II ||
              m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_VERTEX2F) {
            // Search down for end of vertex set
            for (
                ;
                line <
                FTEDITOR_DL_SIZE /*&& line <
                                    m_LineEditor->codeEditor()->document()->blockCount()*/
                ;
                ++line) {
              printf("line %i\n", line);
              const DlParsed &pa = m_LineEditor->getLine(line);
              if (!pa.ValidId) {
                break;
              }
              if (pa.IdLeft == 0) {
                if (pa.IdRight == FTEDITOR_DL_END) {
                  ++line;
                  break;
                } else if (pa.IdRight == FTEDITOR_DL_BEGIN ||
                           pa.IdRight == FTEDITOR_DL_RETURN ||
                           pa.IdRight == FTEDITOR_DL_JUMP) {
                  break;
                }
              }
            }
          } else {
            // Write after current line
            ++line;
          }
        }

        // Skip past CLEAR commands
        while (
            m_LineEditor->getLine(line).ValidId &&
            m_LineEditor->getLine(line).IdLeft == FTEDITOR_DL_INSTRUCTION &&
            (m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR ||
             m_LineEditor->getLine(line).IdRight ==
                 FTEDITOR_DL_CLEAR_COLOR_RGB ||
             m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_COLOR_A ||
             m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_STENCIL ||
             m_LineEditor->getLine(line).IdRight == FTEDITOR_DL_CLEAR_TAG)) {
          ++line;
        }

        // printf("Dropped item from toolbox, type %i\n", selection);

        // void insertLine(int line, const DlParsed &parsed);
        if (selectionType == FTEDITOR_SELECTION_VERTEX) {
          int mvx = screenLeft();
          int mvy = screenTop();
          int scl = screenScale();

          m_LineEditor->codeEditor()->beginUndoCombine("Drag and drop vertex");
          DlParsed pa;
          pa.ValidId = true;
          pa.IdLeft = selection;
          pa.IdRight = 0;

          switch (selection) {
            case FTEDITOR_DL_VERTEX2F:
              pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x())
                                  << m_LineEditor->getVertextFormat(line);
              pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y())
                                  << m_LineEditor->getVertextFormat(line);
              pa.ExpectedParameterCount = 2;
              break;
            default:
              pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
              pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());
              pa.Parameter[2].I = 0;
              pa.Parameter[3].I = 0;
              pa.ExpectedParameterCount = 4;
              break;
          }
          pa.ExpectedStringParameter = false;
          m_LineEditor->insertLine(line, pa);
          m_LineEditor->selectLine(line);
          m_LineEditor->codeEditor()->endUndoCombine();
        } else if ((selection & 0xFFFFFF00) == 0xFFFFFF00)  // Coprocessor
        {
          int mvx = screenLeft();
          int mvy = screenTop();
          int scl = screenScale();

          m_LineEditor->codeEditor()->beginUndoCombine(
              "Drag and drop coprocessor widget");
          DlParsed pa;
          pa.ValidId = true;
          pa.IdLeft = 0xFFFFFF00;
          pa.IdRight = selection & 0xFF;
          pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
          pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());
          pa.ExpectedStringParameter = false;
          switch (selection) {
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
              pa.Parameter[0].I =
                  addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) -
                  (64 * 1024);
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
            case CMD_CALIBRATESUB:
              pa.Parameter[0].U = 0;
              pa.Parameter[1].U = 0;
              pa.Parameter[2].U = 0;
              pa.Parameter[3].U = 0;
              pa.ExpectedParameterCount = 4;
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
              pa.ExpectedParameterCount = 0;
              break;
            case CMD_GETIMAGE:
              pa.ExpectedParameterCount = 0;
              break;
          }

          if (selection == CMD_SKETCH) {
            line = m_LineEditor->getLineCount();
            if (m_LineEditor->getLineText(line - 1).isEmpty()) {
              --line;
            }

            int addr = m_MainWindow->contentManager()->getFreeMemoryAddress();
            if (addr < 0) {
              m_LineEditor->insertLine(++line, "// No free space in RAM_G.");
              m_MainWindow->propertyErrorSet("No free space in RAM_G.");
              return;
            }

            pa.Parameter[2].U = 480;
            pa.Parameter[3].U = 272;
            pa.Parameter[4].U = addr;
            pa.Parameter[5].U = L1;
            pa.ExpectedParameterCount = 6;

            m_LineEditor->insertLine(
                ++line, QString("CMD_MEMZERO(%1, 16320)").arg(addr));
            m_LineEditor->insertLine(++line, pa);  // insert cmd_sketch
            m_LineEditor->selectLine(line);
            m_LineEditor->insertLine(++line, "");
            m_LineEditor->insertLine(++line, "// Then to display the bitmap");

            int handleFree =
                m_MainWindow->contentManager()->editorFindFreeHandle(
                    m_LineEditor);
            if (handleFree == -1) {
              m_LineEditor->insertLine(++line, "// No free bitmap handle.");
              m_MainWindow->propertyErrorSet("No free bitmap handle.");
              return;
            }

            QString bh = QString("BITMAP_HANDLE(%1)").arg(handleFree);
            m_LineEditor->insertLine(++line, bh);

            if (FTEDITOR_CURRENT_DEVICE > FTEDITOR_FT810) {
              m_LineEditor->insertLine(
                  ++line, QString("CMD_SETBITMAP(%1, L1, 480, 272)").arg(addr));
            } else {
              m_LineEditor->insertLine(++line,
                                       QString("BITMAP_SOURCE(0)").arg(addr));
              m_LineEditor->insertLine(++line, "BITMAP_LAYOUT(L1, 60, 272)");
              m_LineEditor->insertLine(
                  ++line, "BITMAP_SIZE(NEAREST, BORDER, BORDER, 480, 272)");
            }

            m_LineEditor->insertLine(++line, "");
            m_LineEditor->insertLine(++line, "BEGIN(BITMAPS)");

            int vf = m_LineEditor->getVertextFormat(line);
            QString vt2f = QString("VERTEX2F(%1, %2)")
                               .arg(pa.Parameter[0].I << vf)
                               .arg(pa.Parameter[1].I << vf);
            m_LineEditor->insertLine(++line, vt2f);
            m_LineEditor->insertLine(++line, "END()");
          } else {
            m_LineEditor->insertLine(line, pa);
            m_LineEditor->selectLine(line);
          }

          m_LineEditor->codeEditor()->endUndoCombine();
        } else if (selectionType == FTEDITOR_SELECTION_FUNCTION) {
          if (!m_MouseStackValid)
            line = m_MainWindow->contentManager()->editorFindNextBitmapLine(
                m_LineEditor);
          m_LineEditor->codeEditor()->beginUndoCombine(
              tr("Drag and drop from toolbox"));
          DlParsed pa;
          pa.ValidId = true;
          pa.IdLeft = 0;
          pa.IdRight = selection;
          pa.ExpectedStringParameter = false;
          switch (selection) {
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
        } else if (selectionType == FTEDITOR_SELECTION_PRIMITIVE)  // Primitive
        {
          bool mustCreateHandle = true;
          if (contentInfo) {
            printf("Find or create handle for content item\n");
            int handleResult = m_MainWindow->contentManager()->editorFindHandle(
                contentInfo, m_LineEditor);
            if (handleResult != -1 && handleResult != 15) {
              bitmapHandle = handleResult;

              mustCreateHandle = false;
            }
            if (mustCreateHandle) {
              printf("Must create handle\n");
              mustCreateHandle = false;
              int handleFree =
                  m_MainWindow->contentManager()->editorFindFreeHandle(
                      m_LineEditor);
              if (handleFree != -1) {
                bitmapHandle = handleFree;
                mustCreateHandle = true;
              }
              if (!mustCreateHandle) {
                if (contentInfo->Converter == ContentInfo::Font) {
                  printf("No free handle available\n");
                  PropertiesEditor *props = m_MainWindow->propertiesEditor();
                  props->setError(
                      tr("<b>Error</b>: No free bitmap handle available"));
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
          // m_MainWindow->undoStack()->beginMacro(tr("Drag and drop
          // primitive"));
          m_LineEditor->codeEditor()->beginUndoCombine(
              tr("Drag and drop primitive"));
          DlParsed pa;
          pa.ValidId = true;
          pa.IdLeft = 0;
          pa.ExpectedStringParameter = false;
          if (contentInfo && mustCreateHandle) {
            printf("Create handle for content item\n");

            int hline =
                (bitmapHandle == 15)
                    ? line
                    : m_MainWindow->contentManager()->editorFindNextBitmapLine(
                          m_LineEditor);

            line = hline;
            // TODO: contentInfo->Converter == ContentInfo::Font &&
            // isCoprocessor && (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
            auto addPalettedSource = [&](int address) {
              pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
              pa.Parameter[0].I = address;
              pa.ExpectedParameterCount = 1;
              m_LineEditor->insertLine(hline, pa);
              ++hline;
              ++line;
            };
            auto addBitmapHandler = [&]() {
              pa.IdRight = FTEDITOR_DL_BITMAP_HANDLE;
              pa.Parameter[0].U = bitmapHandle;
              pa.ExpectedParameterCount = 1;
              m_LineEditor->insertLine(hline, pa);
              ++hline;
              ++line;

              bool useSetBitmap = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 &&
                                   m_LineEditor->isCoprocessor());

              if (useSetBitmap) {
                pa.IdLeft = 0xFFFFFF00;
                pa.IdRight = CMD_SETBITMAP & 0xFF;

                pa.Parameter[0].U = contentInfo->bitmapAddress();

                if (contentInfo->Converter == ContentInfo::ImageCoprocessor) {
                  pa.Parameter[0].U = contentInfo->bitmapAddress() +
                                      contentInfo->PalettedAddress;
                }

                pa.Parameter[1].U = contentInfo->ImageFormat;
                pa.Parameter[2].U = contentInfo->CachedImageWidth & 0x7FF;
                pa.Parameter[3].U = contentInfo->CachedImageHeight & 0x7FF;
                pa.ExpectedParameterCount = 4;
                m_LineEditor->insertLine(hline, pa);
                ++hline;
                ++line;
                pa.IdLeft = 0;
              } else {
                pa.IdRight = FTEDITOR_DL_BITMAP_SOURCE;
                pa.Parameter[0].U = contentInfo->bitmapAddress();
                pa.ExpectedParameterCount = 1;
                m_LineEditor->insertLine(hline, pa);
                ++hline;
                ++line;
              }

              if (!useSetBitmap) {
                pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT;
                pa.Parameter[0].U = contentInfo->ImageFormat;
                pa.Parameter[1].U = contentInfo->CachedImageStride & 0x3FF;
                pa.Parameter[2].U = contentInfo->CachedImageHeight & 0x1FF;
                pa.ExpectedParameterCount = 3;
                m_LineEditor->insertLine(hline, pa);
                ++hline;
                ++line;

                if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) {
                  // Add _H if necessary
                  pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT_H;
                  pa.Parameter[0].U = contentInfo->CachedImageStride >> 10;
                  pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
                  pa.ExpectedParameterCount = 2;
                  m_LineEditor->insertLine(hline, pa);
                  ++hline;
                  ++line;
                }

                pa.IdRight = FTEDITOR_DL_BITMAP_SIZE;
                pa.Parameter[0].U = 0;  // size filter
                pa.Parameter[1].U = 0;  // wrap x
                pa.Parameter[2].U = 0;  // wrap y
                pa.Parameter[3].U = contentInfo->CachedImageWidth & 0x1FF;
                pa.Parameter[4].U = contentInfo->CachedImageHeight & 0x1FF;
                pa.ExpectedParameterCount = 5;
                m_LineEditor->insertLine(hline, pa);
                ++hline;
                ++line;

                if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) {
                  // Add _H if necessary
                  pa.IdRight = FTEDITOR_DL_BITMAP_SIZE_H;
                  pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
                  pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
                  pa.ExpectedParameterCount = 2;
                  m_LineEditor->insertLine(hline, pa);
                  ++hline;
                  ++line;
                }
              }
            };
            auto addSetFont2Command = [&](int firstChar = 0) {
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_SETFONT2 & 0xFF;
              pa.Parameter[0].U = bitmapHandle;
              pa.Parameter[1].U = contentInfo->MemoryAddress;
              pa.Parameter[2].U = firstChar;
              pa.ExpectedParameterCount = 3;
              m_LineEditor->insertLine(hline, pa);
              ++hline;
              ++line;
              pa.IdLeft = 0;
            };
            auto addSetFontCommand = [&]() {
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_SETFONT & 0xFF;
              pa.Parameter[0].U = bitmapHandle;
              pa.Parameter[1].U = contentInfo->MemoryAddress;
              pa.ExpectedParameterCount = 2;
              m_LineEditor->insertLine(hline, pa);
              ++hline;
              ++line;
              pa.IdLeft = 0;
            };
            auto fileSuffix =
                QFileInfo(contentInfo->SourcePath).suffix().toLower();
            if ((FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) &&
                m_LineEditor->isCoprocessor() &&
                (contentInfo->Converter == ContentInfo::Font)) {
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_SETFONT2 & 0xFF;
              pa.Parameter[0].U = bitmapHandle;
              pa.Parameter[1].U = contentInfo->MemoryAddress;
              pa.Parameter[2].U = contentInfo->FontOffset;
              pa.ExpectedParameterCount = 3;
              m_LineEditor->insertLine(hline, pa);
              ++hline;
              ++line;
              pa.IdLeft = 0;
            } else if (contentInfo->Converter == ContentInfo::Raw) {
              if (fileSuffix == "xfont") {
                printf("Add handler for .glyph or .xfont content file\n");
                addSetFont2Command(0);
              } else if (fileSuffix == "raw") {
                if (contentInfo->MapInfoFileType.contains(fileSuffix)) {
                  QString fileType =
                      ContentInfo::MapInfoFileType.value(fileSuffix, "");
                  if (!fileType.isEmpty()) {
                    QString filePath =
                        contentInfo->SourcePath
                            .left(contentInfo->SourcePath.lastIndexOf('.') + 1)
                            .append(fileType);
                    if (contentInfo->SourcePath.contains("_index")) {
                      filePath.remove(filePath.lastIndexOf("_index"), 6);
                    }
                    auto infoJson = ReadWriteUtil::getJsonInfo(filePath);
                    if (infoJson.contains("type")) {
                      auto contentType = infoJson["type"].toString();
                      if (contentType == "bitmap") {
                        if (infoJson["format"].toString().toUpper().contains(
                                "PALETTED")) {
                          auto searchedFile = contentInfo->DestName;
                          searchedFile.replace("_index", "_lut");
                          auto searchedContent =
                              m_MainWindow->contentManager()->find(
                                  searchedFile);
                          addPalettedSource(
                              searchedContent ? searchedContent->bitmapAddress()
                                              : 0);
                        }
                        addBitmapHandler();
                      } else if (contentType == "legacyfont") {
                        if (infoJson.contains("eve_command")) {
                          if (infoJson["eve_command"].toString() ==
                              "cmd_setfont2") {
                            int firstChar = 1;
                            if (infoJson.contains("first_character"))
                              firstChar = infoJson["first_character"].toInt();
                            addSetFont2Command(firstChar);
                          } else {
                            addSetFontCommand();
                          }
                        } else {
                          addSetFont2Command(1);
                        }
                      }
                    }
                  }
                }
              }
            } else {
              if (fileSuffix != "avi") {
                addBitmapHandler();
              }
              if (contentInfo->Converter == ContentInfo::Font) {
                addSetFontCommand();
              }
            }
          }

          auto addBitmapCommands = [&]() {
            int mvx = screenLeft();
            int mvy = screenTop();
            int scl = screenScale();

            DlParsed pav;
            pav.ValidId = true;
            pav.IdLeft = FTEDITOR_DL_VERTEX2F;  // FIXME: Drag-drop outside
                                                // 512px does not work??
            pav.IdRight = 0;
            pav.Parameter[0].I = UNTFX(EPOSPOINT(e).x())
                                 << m_LineEditor->getVertextFormat(line);
            pav.Parameter[1].I = UNTFY(EPOSPOINT(e).y())
                                 << m_LineEditor->getVertextFormat(line);
            // pav.Parameter[2].I = bitmapHandle;
            // pav.Parameter[3].I = 0;
            pav.ExpectedParameterCount = 2;
            pav.ExpectedStringParameter = false;
            pa.IdRight = FTEDITOR_DL_BEGIN;
            pa.Parameter[0].U = selection;
            pa.ExpectedParameterCount = 1;
            m_LineEditor->insertLine(line, pa);
            ++line;
            int lastVertex;

            if (contentInfo &&
                contentInfo->Converter == ContentInfo::ImageCoprocessor) {
              pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
              pa.Parameter[0].U = contentInfo->bitmapAddress();
              m_LineEditor->insertLine(line, pa);
              ++line;
              m_LineEditor->insertLine(line, pav);
              ++line;
              lastVertex = -1;
            } else if (selection == BITMAPS && contentInfo &&
                       contentInfo->Converter == ContentInfo::Image &&
                       requirePaletteAddress(contentInfo)) {
              if (contentInfo->ImageFormat == PALETTED8) {
                pa.IdRight = FTEDITOR_DL_SAVE_CONTEXT;
                pa.ExpectedParameterCount = 0;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
                pa.Parameter[0].U = ONE;
                pa.Parameter[1].U = ZERO;
                pa.ExpectedParameterCount = 2;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_COLOR_MASK;
                pa.Parameter[0].U = 0;
                pa.Parameter[1].U = 0;
                pa.Parameter[2].U = 0;
                pa.Parameter[3].U = 1;
                pa.ExpectedParameterCount = 4;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
                pa.Parameter[0].U = contentInfo->MemoryAddress + 3;
                pa.ExpectedParameterCount = 1;
                m_LineEditor->insertLine(line, pa);
                ++line;
                m_LineEditor->insertLine(line, pav);
                ++line;
                pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
                pa.Parameter[0].U = DST_ALPHA;
                pa.Parameter[1].U = ONE_MINUS_DST_ALPHA;
                pa.ExpectedParameterCount = 2;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_COLOR_MASK;
                pa.Parameter[0].U = 1;
                pa.Parameter[1].U = 0;
                pa.Parameter[2].U = 0;
                pa.Parameter[3].U = 0;
                pa.ExpectedParameterCount = 4;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
                pa.Parameter[0].U = contentInfo->MemoryAddress + 2;
                pa.ExpectedParameterCount = 1;
                m_LineEditor->insertLine(line, pa);
                ++line;
                m_LineEditor->insertLine(line, pav);
                ++line;
                pa.IdRight = FTEDITOR_DL_COLOR_MASK;
                pa.Parameter[0].U = 0;
                pa.Parameter[1].U = 1;
                pa.Parameter[2].U = 0;
                pa.Parameter[3].U = 0;
                pa.ExpectedParameterCount = 4;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
                pa.Parameter[0].U = contentInfo->MemoryAddress + 1;
                pa.ExpectedParameterCount = 1;
                m_LineEditor->insertLine(line, pa);
                ++line;
                m_LineEditor->insertLine(line, pav);
                ++line;
                pa.IdRight = FTEDITOR_DL_COLOR_MASK;
                pa.Parameter[0].U = 0;
                pa.Parameter[1].U = 0;
                pa.Parameter[2].U = 1;
                pa.Parameter[3].U = 0;
                pa.ExpectedParameterCount = 4;
                m_LineEditor->insertLine(line, pa);
                ++line;
                pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
                pa.Parameter[0].U = contentInfo->MemoryAddress;
                pa.ExpectedParameterCount = 1;
                m_LineEditor->insertLine(line, pa);
                ++line;
                m_LineEditor->insertLine(line, pav);
                ++line;
                pa.IdRight = FTEDITOR_DL_RESTORE_CONTEXT;
                pa.ExpectedParameterCount = 0;
                m_LineEditor->insertLine(line, pa);
                ++line;
                lastVertex = -2;
              } else {
                pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
                pa.Parameter[0].U = contentInfo->MemoryAddress;
                m_LineEditor->insertLine(line, pa);
                ++line;
                m_LineEditor->insertLine(line, pav);
                ++line;
                lastVertex = -1;
              }
            } else {
              m_LineEditor->insertLine(line, pav);
              ++line;
              lastVertex = -1;
            }
            pa.IdLeft = 0;
            pa.IdRight = FTEDITOR_DL_END;
            pa.ExpectedParameterCount = 1;
            m_LineEditor->insertLine(line, pa);
            m_LineEditor->selectLine(line + lastVertex);
          };
          if (contentInfo && contentInfo->Converter == ContentInfo::Font) {
            int mvx = screenLeft();
            int mvy = screenTop();
            int scl = screenScale();

            pa.IdLeft = 0xFFFFFF00;
            pa.IdRight = CMD_TEXT & 0xFF;
            pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
            pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());
            pa.Parameter[2].I = bitmapHandle;
            pa.Parameter[3].I = 0;
            pa.StringParameter = "Text";
            pa.ExpectedStringParameter = true;
            pa.ExpectedParameterCount = 5;
            m_LineEditor->insertLine(line, pa);
            m_LineEditor->selectLine(line);
            pa.IdLeft = 0;
            pa.ExpectedStringParameter = false;
          } else if (contentInfo &&
                     contentInfo->Converter == ContentInfo::Raw) {
            int mvx = screenLeft();
            int mvy = screenTop();
            int scl = screenScale();

            auto addTextCommand = [&](QString text) {
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_TEXT & 0xFF;
              pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
              pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());
              pa.Parameter[2].I = bitmapHandle;
              pa.Parameter[3].I = 0;
              pa.StringParameter = text.toStdString();
              pa.ExpectedStringParameter = true;
              pa.ExpectedParameterCount = 5;
              m_LineEditor->insertLine(line, pa);
              ++line;
            };
            auto readConvertedCharsFile = [](QString &file) {
              QFileInfo fi(file);
              if (!fi.exists()) return QString();
              QString suffix = fi.suffix();
              QFile f(file);
              if (!f.open(QIODevice::ReadOnly)) return QString();
              QString data = f.readAll();
              f.close();
              return data;
            };
            auto readConvertedCharsIndexFile = [](QString &file, int &first,
                                                  int &last) {
              QFileInfo fi(file);
              if (!fi.exists()) return;
              QFile f(file);
              if (!f.open(QIODevice::ReadOnly)) return;
              QTextStream in(&f);
              while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList listItem = line.split(QRegularExpression("\\s+"));
                listItem.removeAll("");
                if (listItem.count() >= 3) {
                  bool ok;
                  int indexNo = listItem.at(1).toInt(&ok, 10);
                  if (!ok) continue;
                  last = indexNo;
                  if (first < 1) first = indexNo;
                }
              }
              f.close();
            };
            auto infoJson = QJsonObject();
            auto fileSuffix =
                QFileInfo(contentInfo->SourcePath).suffix().toLower();
            if (contentInfo->MapInfoFileType.contains(fileSuffix)) {
              QString fileType =
                  ContentInfo::MapInfoFileType.value(fileSuffix, "");
              if (!fileType.isEmpty()) {
                QString filePath =
                    contentInfo->SourcePath
                        .left(contentInfo->SourcePath.lastIndexOf('.') + 1)
                        .append(fileType);
                if (contentInfo->SourcePath.contains("_index")) {
                  filePath.remove(filePath.lastIndexOf("_index"), 6);
                }
                infoJson = ReadWriteUtil::getJsonInfo(filePath);
              }
            }
            if (fileSuffix == "ram_g") {
              debugLog("Create commands for .ram_g content file\n");
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_ANIMFRAMERAM & 0xFF;
              pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
              pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());

              if (infoJson.contains("object")) {
                auto obj = infoJson.value("object").toObject();
                pa.Parameter[2].I = obj.value("offset").toInt();
              } else
                pa.Parameter[2].I = 0;
              pa.ExpectedParameterCount = 3;
              m_LineEditor->insertLine(line, pa);
              ++line;
            } else if (fileSuffix == "flash") {
              debugLog("Create commands for .flash content file\n");
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_ANIMFRAME & 0xFF;
              pa.Parameter[0].I = UNTFX(EPOSPOINT(e).x());
              pa.Parameter[1].I = UNTFY(EPOSPOINT(e).y());

              if (infoJson.contains("object")) {
                auto obj = infoJson.value("object").toObject();
                pa.Parameter[2].I = obj.value("offset").toInt();
              } else
                pa.Parameter[2].I = 0;
              pa.ExpectedParameterCount = 3;
              m_LineEditor->insertLine(line, pa);
              ++line;
            } else if (fileSuffix == "xfont") {
              debugLog("Create commands for .xfont content file\n");
              QString savedCharsFile =
                  contentInfo->SourcePath
                      .left(contentInfo->SourcePath.lastIndexOf('.'))
                      .append("_converted_chars.txt");
              QString data = readConvertedCharsFile(savedCharsFile);
              addTextCommand(data.left(5));
            } else if (fileSuffix == "raw") {
              if (infoJson.contains("type")) {
                auto contentType = infoJson["type"].toString();
                if (contentType == "bitmap") {
                  debugLog("Create commands for .raw bitmap content file");
                  addBitmapCommands();
                } else if (contentType == "legacyfont") {
                  debugLog(
                      "Create commands for .raw legacy font content file\n");
                  QString savedCharsIndexFile =
                      contentInfo->SourcePath
                          .left(contentInfo->SourcePath.lastIndexOf('.'))
                          .append("_converted_char_index.txt");
                  int first = 0, last = 0;
                  readConvertedCharsIndexFile(savedCharsIndexFile, first, last);
                  QString defaultText;
                  if (first != 0 && last != 0) {
                    int defaultNoOfChar = 5;
                    int count = 0;
                    int index = first + 1;
                    while (count < defaultNoOfChar) {
                      if (index == 34)  // Character "
                        ++index;

                      defaultText += QChar(index);
                      ++count;
                      ++index;
                    }
                  }
                  addTextCommand(!defaultText.isEmpty() ? defaultText : "Text");
                }
              }
            }
          } else if (contentInfo) {
            auto fileSuffix =
                QFileInfo(contentInfo->SourcePath).suffix().toLower();
            if (fileSuffix == "avi") {
              printf("Create commands for .avi content file\n");
              pa.IdLeft = 0xFFFFFF00;
              pa.IdRight = CMD_PLAYVIDEO & 0xFF;
              pa.Parameter[0].I = 0;
              pa.StringParameter = contentInfo->SourcePath.toStdString();
              pa.ExpectedStringParameter = true;
              pa.ExpectedParameterCount = 2;
              m_LineEditor->insertLine(line, pa);
              ++line;
            } else {
              addBitmapCommands();
            }
          } else {
            addBitmapCommands();
          }
          m_LineEditor->codeEditor()->endUndoCombine();
          // m_MainWindow->undoStack()->endMacro();
          switch (selection) {
            case BITMAPS:
            case POINTS:
              break;
            default:
              // Used for linestrip style drawing
              m_Insert->setChecked(true);
              break;
          }
        }
      } else if (selectionType == FTEDITOR_SELECTION_DL_STATE ||
                 selectionType == FTEDITOR_SELECTION_CMD_STATE) {
        bool useMouseStack;
        int lineOverride = -1;
        if (selectionType == FTEDITOR_SELECTION_DL_STATE &&
            (selection == FTEDITOR_DL_CLEAR_COLOR_RGB ||
             selection == FTEDITOR_DL_CLEAR_COLOR_A ||
             selection == FTEDITOR_DL_CLEAR_STENCIL ||
             selection == FTEDITOR_DL_CLEAR_TAG)) {
          // Try to find CLEAR command backward from current pos
          int l = m_LineEditor->isCoprocessor() ? m_MouseStackCmdTop
                                                : m_MouseStackDlTop;
          for (int i = l; i >= 0; --i) {
            DlParsed pai = m_LineEditor->getLine(i);
            if (pai.ValidId && (pai.IdLeft == FTEDITOR_DL_INSTRUCTION) &&
                (pai.IdRight == FTEDITOR_DL_CLEAR)) {
              lineOverride = i;
              break;
            }
          }
          useMouseStack = m_MouseStackValid;
        } else {
          // Only use primitive under cursor if it's not the CLEAR primitive
          DlParsed pac = m_LineEditor->getLine(m_LineEditor->isCoprocessor()
                                                   ? m_MouseStackCmdTop
                                                   : m_MouseStackDlTop);
          useMouseStack =
              m_MouseStackValid &&
              !(pac.ValidId && (pac.IdLeft == FTEDITOR_DL_INSTRUCTION) &&
                (pac.IdRight == FTEDITOR_DL_CLEAR));
        }
        int line =
            lineOverride >= 0
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
        if (selectionType == FTEDITOR_SELECTION_CMD_STATE) {
          pa.IdLeft = 0xFFFFFF00;
          pa.IdRight = selection & 0xFF;
          switch (selection) {
            case CMD_REGREAD:
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
            case CMD_ROTATE:
              pa.Parameter[0].I = 0;
              pa.ExpectedParameterCount = 1;
              break;
            case CMD_TRANSLATE:
              pa.Parameter[0].I = 0;
              pa.Parameter[1].I = 0;
              pa.ExpectedParameterCount = 2;
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
              pa.Parameter[0].I =
                  m_MainWindow->contentManager()->editorFindFreeHandle(
                      m_LineEditor);
              if (pa.Parameter[0].I < 0) pa.Parameter[0].I = 31;
              if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT880 &&
                  FTEDITOR_CURRENT_DEVICE <= FTEDITOR_BT883)
                pa.Parameter[1].I = 31;
              else
                pa.Parameter[1].I = 34;
              pa.ExpectedParameterCount = 2;
              break;
            case CMD_SETFONT:
              pa.Parameter[0].I = 0;
              pa.Parameter[1].I = 0;
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
              for (size_t i = 0; i < 13; i++) {
                pa.Parameter[i].I = 0;
              }
              pa.ExpectedParameterCount = 13;
              break;
            case CMD_FLASHSOURCE:
              pa.Parameter[0].I = 0;
              pa.ExpectedParameterCount = 1;
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
        } else {
          pa.IdLeft = 0;
          pa.IdRight = selection;
          switch (selection) {
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
    } else {
      printf("Warning: No line editor\n");
    }
  }
}

void InteractiveViewport::dragMoveEvent(QDragMoveEvent *e) {
  // TODO: Bitmaps from files, etc
  horizontalRuler()->setIndicator(EPOSPOINT(e).x());
  verticalRuler()->setIndicator(EPOSPOINT(e).y());
  if (acceptableSource(e)) {
    if (m_LineEditor) {
      int mvx = screenLeft();
      int mvy = screenTop();
      int scl = screenScale();

      m_NextMouseX = UNTFX(EPOSPOINT(e).x());
      m_NextMouseY = UNTFY(EPOSPOINT(e).y());
      m_DragMoving = true;

      e->acceptProposedAction();
    } else {
      printf("Warning: No line editor\n");
    }
  } else {
    printf("Unknown dragMoveEvent from %p\n", e->source());
  }
}
void InteractiveViewport::dragEnterEvent(QDragEnterEvent *e) {
  // TODO: Bitmaps from files, etc
  if (acceptableSource(e)) {
    if (m_LineEditor) {
      e->acceptProposedAction();
    } else {
      printf("Warning: No line editor\n");
    }
  } else {
    printf("Unknown dragEnterEvent from %p\n", e->source());
  }
}
} /* namespace FTEDITOR */

/* end of file */
