/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
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

	// Pixmap can be used after each frame() signal as updated source for repaint etc. It does not permanently remain valid
	const QPixmap &getPixMap() const;

	int hsize();
	int vsize();

	int screenLeft(); // Left in display coordinates
	int screenTop();
	int screenScale(); // Scale in 1/16

	int screenBottom();
	int screenRight();

	void setScreenScale(int screenScale);

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
	int m_ScreenScale;

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
