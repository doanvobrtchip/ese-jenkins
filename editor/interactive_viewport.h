/**
 * interactive_viewport.h
 * $Id$
 * \file interactive_viewport.h
 * \brief interactive_viewport.h
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INTERACTIVE_VIEWPORT_H
#define FT800EMUQT_INTERACTIVE_VIEWPORT_H

// STL includes
#include <vector>

// Qt includes

// Emulator includes

// Project includes
#include "emulator_viewport.h"
#include "dl_editor.h"

namespace FT800EMUQT {

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

private:
	void updatePointerMethod();

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void enterEvent(QEvent *e);
	virtual void leaveEvent(QEvent *e);

private slots:
	void automaticChecked();
	void touchChecked();
	void traceChecked();
	void editChecked();

private:
	MainWindow *m_MainWindow;

	bool m_TraceEnabled;
	uint32_t m_TraceX, m_TraceY;
	std::vector<int> m_TraceStack;
	
	bool m_MouseOver;
	uint32_t m_NextMouseX, m_NextMouseY;
	uint32_t m_MouseX, m_MouseY;
	std::vector<int> m_MouseStackWrite;
	std::vector<int> m_MouseStackRead;
	
	// User selected pointer mode
	int m_PointerFilter;
	// Actual current pointer method, depending on stack under cursor
	int m_PointerMethod;

	bool m_MouseTouch;
	
	bool m_MouseOverVertex;
	int m_MouseOverVertexLine;
	
	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

private:
	InteractiveViewport(const InteractiveViewport &);
	InteractiveViewport &operator=(const InteractiveViewport &);
	
}; /* class InteractiveViewport */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INTERACTIVE_VIEWPORT_H */

/* end of file */
