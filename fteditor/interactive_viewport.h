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
#include <QList>

// Qt includes

// Emulator includes

// Project includes
#include "ComponentBase.h"
#include "dl_editor.h"
#include "emulator_viewport.h"

class QAction;
class QComboBox;
class QSinglePointEvent;
class QMenu;

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
class InteractiveViewport : public EmulatorViewport, public ComponentBase
{
	Q_OBJECT

public:
	InteractiveViewport(MainWindow *parent);
	virtual ~InteractiveViewport();

	// Graphics callback synchronized to the emulator thread, use to get debug
	// information for a frame
	virtual void graphics();

	// Graphics callback synchronized to Qt thread, use to overlay graphics
	virtual void graphics(QImage *image);

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line, bool multiple = false);
	void unsetEditorLine();

	virtual void keyPressEvent(QKeyEvent *e);

	int mouseX() const { return m_MouseX; }
	int mouseY() const { return m_MouseY; }
	bool mouseOver() const { return m_MouseOver; }

	int32_t mappingX(QDropEvent *e);
	int32_t mappingY(QDropEvent *e);
	int32_t mappingX(int x);
	int32_t mappingY(int y);
	int32_t mappingX(QSinglePointEvent *e);
	int32_t mappingY(QSinglePointEvent *e);
	void selectItems();
	bool isSingleSelect();
	bool isSelectable(const DlParsed &parsed);
	void isWidgetWHR(const DlParsed &parsed, bool &widgetWH, bool &widgetR);
	bool isWidgetXY(const DlParsed &parsed);
	bool isWidgetGradient(const DlParsed &parsed);
	
	QAction *ActionRuler() const;
	
protected:
	virtual void paintEvent(QPaintEvent *e);

	bool eventFilter(QObject *watched, QEvent *event);

private:
	void updatePointerMethod();
	bool acceptableSource(QDropEvent *e);
	void mouseMoveEvent(int mouseX, int mouseY,
	    Qt::KeyboardModifiers km = Qt::NoModifier);

protected:
	virtual void mouseMoveEvent(QMouseEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	virtual void enterEvent(QEnterEvent *e) override;
	virtual void leaveEvent(QEvent *e) override;
	virtual void wheelEvent(QWheelEvent *e) override;

	virtual void dropEvent(QDropEvent *e) override;
	virtual void dragMoveEvent(QDragMoveEvent *e) override;
	virtual void dragEnterEvent(QDragEnterEvent *e) override;

public slots:
	void setupConnections(QObject *obj) override;
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
	QComboBox *m_ZoomCB;
	MainWindow *m_MainWindow;
	QAction *m_Insert;
	QMenu *m_ContextMenu;
	QAction *m_ActionAlgn;
	QAction *m_ActionRuler;

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
	QList<int> m_SelectedLines;
	QPoint m_FirstPoint;
	QPoint m_SecondPoint;
	bool m_DrawMultipleSelection;

	bool m_DragMoving;

	int m_SnapHistoryX[FTED_SNAP_HISTORY];
	int m_SnapHistoryY[FTED_SNAP_HISTORY];
	int m_SnapHistoryCur;
	void snapPos(int &xd, int &yd, int xref, int yref);

	bool m_IsDrawAlgnHorizontal;
	bool m_IsDrawAlgnVertical;
	bool m_IsDrawAlgn;
	bool m_selectable;

signals:
	void selectedLinesChanged(const QList<int> &newSelectedLines);
}; /* class InteractiveViewport */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INTERACTIVE_VIEWPORT_H */

/* end of file */
