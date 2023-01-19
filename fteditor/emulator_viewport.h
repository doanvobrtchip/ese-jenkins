/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_EMULATOR_VIEWPORT_H
#define FTEDITOR_EMULATOR_VIEWPORT_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes

// Qt includes
#include <QWidget>
#include <QThread>
#include <QScrollBar>
#include <QScrollArea>
#include <QVBoxLayout>
#ifdef FTEDITOR_OPENGL_VIEWPORT
#	include <QOpenGLWidget>
#endif

// Emulator includes
#include <bt8xxemu.h>
#include "src/customize/QRuler.h"

// Project includes

class QAction;
class QImage;
class QPixmap;
class QLabel;

namespace FTEDITOR {

/**
 * EmulatorViewport
 * \brief EmulatorViewport
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */
class EmulatorViewport 
#ifdef FTEDITOR_OPENGL_VIEWPORT
	: public QOpenGLWidget
#else
	: public QWidget
#endif
{
	Q_OBJECT

public:
	EmulatorViewport(QWidget *parent, const QString &applicationDataDir);
	virtual ~EmulatorViewport();
	
	// Runs the emulator on a new thread and connects it with this viewport
	void run(const BT8XXEMU_EmulatorParameters &params);
	void stop();

	// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
	virtual void graphics() { }

	// Graphics callback synchronized to Qt thread, use to overlay graphics
	virtual void graphics(QImage *image) { }

	// Pixmap can be used after each frame() signal as updated source for repaint etc. It does not permanently remain valid
	const QPixmap &getPixMap() const;

	void fetchColorAsync(int x, int y);
	QColor fetchColorAsync();

	int hsize();
	int vsize();

	int screenLeft(); // Left in display coordinates
	int screenTop();
	int screenScale(); // Scale in 1/16

	int screenBottom();
	int screenRight();

	QScrollBar *horizontalScrollbar() { return m_Horizontal; }
	QScrollBar *verticalScrollbar() { return m_Vertical; }

	QRuler *horizontalRuler() { return m_HorizontalRuler; }
	QRuler *verticalRuler() { return m_VerticalRuler; }

protected:
	void setScreenScale(int screenScale);

	virtual void paintEvent(QPaintEvent *e);

public slots:		
	// void saveScreenshot();
	void threadRepaint();
	void toggleViewRuler(bool show);

	virtual void zoomIn();
	virtual void zoomOut();

signals:
	void frame();
	void visibleChanged(bool visible);

private:
	QScrollBar *m_Horizontal;
	QScrollBar *m_Vertical;
	int m_ScreenScale;

	QString m_ApplicationDataDir;

	QRuler *m_HorizontalRuler;
	QRuler *m_VerticalRuler;

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

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_EMULATOR_VIEWPORT_H */

/* end of file */
