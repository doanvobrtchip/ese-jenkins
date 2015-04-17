/**
 * emulator_viewport.h
 * $Id$
 * \file emulator_viewport.h
 * \brief emulator_viewport.h
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_EMULATOR_VIEWPORT_H
#define FT800EMUQT_EMULATOR_VIEWPORT_H

// STL includes

// Qt includes
#include <QWidget>
#include <QThread>
#include <QScrollBar>

// Emulator includes
#include <ft800emu_emulator.h>

// Project includes

class QAction;
class QImage;
class QPixmap;
class QLabel;

namespace FT800EMUQT {

/**
 * EmulatorViewport
 * \brief EmulatorViewport
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */
class EmulatorViewport : public QWidget
{
	Q_OBJECT

public:
	EmulatorViewport(QWidget *parent);
	virtual ~EmulatorViewport();
	
	// Runs the emulator on a new thread and connects it with this viewport
	void run(const FT8XXEMU_EmulatorParameters &params);
	void stop();

	// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
	virtual void graphics() { }

	// Graphics callback synchronized to Qt thread, use to overlay graphics
	virtual void graphics(QImage *image) { }

	int hsize();
	int vsize();

	int screenLeft();
	int screenTop();
	int screenScale();

	QScrollBar *horizontalScrollbar() { return m_Horizontal; }
	QScrollBar *verticalScrollbar() { return m_Vertical; }

protected:
	virtual void paintEvent(QPaintEvent *e);

public slots:		
	// void saveScreenshot();
	void threadRepaint();

signals:
	void frame();

private:
	QScrollBar *m_Horizontal;
	QScrollBar *m_Vertical;

private:
	EmulatorViewport(const EmulatorViewport &);
	EmulatorViewport &operator=(const EmulatorViewport &);
	
}; /* class EmulatorViewport */

class EmulatorThread : public QThread
{
	Q_OBJECT

protected:
	void run();
	
signals:
	void repaint();
	
};

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_EMULATOR_VIEWPORT_H */

/* end of file */
