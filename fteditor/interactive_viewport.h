/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_INTERACTIVE_VIEWPORT_H
#define FTEDITOR_INTERACTIVE_VIEWPORT_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>

// Qt includes

// Emulator includes

// Project includes
#include "emulator_viewport.h"
#include "dl_editor.h"

class QAction;
class QComboBox;

namespace FTEDITOR {

#define FTED_SNAP_HISTORY 8
#define FTED_SNAP_DIST 3
// #define FTED_SNAP_SPACING 8

class MainWindow;

/**
 * InteractiveViewport
 * \brief InteractiveViewport
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */
class InteractiveViewport : public EmulatorViewport
{
	Q_OBJECT

public:
	InteractiveViewport(MainWindow *parent);
	virtual ~InteractiveViewport();

	// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
	virtual void graphics();

	// Graphics callback synchronized to Qt thread, use to overlay graphics
	virtual void graphics(QImage *image);

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void unsetEditorLine();

	virtual void keyPressEvent(QKeyEvent *e);

	int mouseX() const { return m_MouseX; }
	int mouseY() const { return m_MouseY; }
	bool mouseOver() const { return m_MouseOver; }

	QColor getPixelColor();

protected:
	virtual void paintEvent(QPaintEvent *e);

	bool eventFilter(QObject *watched, QEvent *event);

private:
	void updatePointerMethod();
	bool acceptableSource(QDropEvent *e);
	void mouseMoveEvent(int mouseX, int mouseY, Qt::KeyboardModifiers km = Qt::NoModifier);

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void enterEvent(QEvent *e);
	virtual void leaveEvent(QEvent *e);
	virtual void wheelEvent(QWheelEvent* e);

	virtual void dropEvent(QDropEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *e);
	virtual void dragEnterEvent(QDragEnterEvent *e);

public slots:
	virtual void zoomIn() override;
	virtual void zoomOut() override;

private slots:
	void automaticChecked();
	void touchChecked();
	void traceChecked();
	void editChecked();

	void zoomChanged(int index);
	void zoomEditTextChanged();

private:
	InteractiveViewport(const InteractiveViewport &);
	InteractiveViewport &operator=(const InteractiveViewport &);

private:
	QComboBox  *m_ZoomCB;
	MainWindow *m_MainWindow;
	QAction *m_Insert;

	bool m_PreferTraceCursor;
	bool m_TraceEnabled;
	uint32_t m_TraceX, m_TraceY;
#define FTEDITOR_TRACE_STACK_SIZE 2048
	int m_TraceStack[FTEDITOR_TRACE_STACK_SIZE];
	int m_TraceStackSize;
	std::vector<int> m_TraceStackDl;
	std::vector<int> m_TraceStackCmd;

	bool m_MouseOver;
	int m_NextMouseX, m_NextMouseY;
	int m_MouseX, m_MouseY;
	std::vector<int> m_MouseStackWrite;
	std::vector<int> m_MouseStackRead;
	volatile bool m_MouseStackWritten;
	int m_MouseStackDlTop;
	int m_MouseStackCmdTop;
	bool m_MouseStackValid;

	// User selected pointer mode
	int m_PointerFilter;
	// Actual current pointer method, depending on stack under cursor
	int m_PointerMethod;

	bool m_MouseTouch;

	bool m_MouseOverVertex;
	int m_MouseOverVertexLine;
	bool m_MouseMovingVertex;

	int m_MovingLastX, m_MovingLastY;

	bool m_WidgetXY;
	bool m_WidgetWH;
	bool m_WidgetR;

	bool m_WidgetGradient;

	int m_MouseMovingWidget;

	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

	bool m_DragMoving;

	int m_SnapHistoryX[FTED_SNAP_HISTORY];
	int m_SnapHistoryY[FTED_SNAP_HISTORY];
	int m_SnapHistoryCur;
	void snapPos(int &xd, int &yd, int xref, int yref);

	bool m_isDrawAlignmentHorizontal;
	bool m_isDrawAlignmentVertical;
}; /* class InteractiveViewport */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INTERACTIVE_VIEWPORT_H */

/* end of file */
