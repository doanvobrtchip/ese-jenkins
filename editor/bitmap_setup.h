/**
 * bitmap_setup.h
 * $Id$
 * \file bitmap_setup.h
 * \brief bitmap_setup.h
 * \date 2014-02-25 13:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_BITMAP_SETUP_H
#define FT800EMUQT_BITMAP_SETUP_H

// STL includes
#include <vector>
#include <set>
#include <stdio.h>

// Qt includes
#include <QFrame>
#include <QWidget>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QImage>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes
// ...

class QResizeEvent;
class QLabel;

namespace FT800EMUQT {

class MainWindow;
class BitmapWidgetThread;

/**
 * BitmapWidget
 * \brief BitmapWidget
 * \date 2014-02-25 13:59GMT
 * \author Jan Boon (Kaetemi)
 */
class BitmapWidget : public QFrame
{
	Q_OBJECT

public:
	BitmapWidget(MainWindow *parent, int index);
	virtual ~BitmapWidget();

	void select();
	void deselect();

	void setImage(const QString &name);

private:
	void reloadInternal();

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void resizeEvent(QResizeEvent *event);

private slots:
	void threadFinished();

private:
	MainWindow *m_MainWindow;
	int m_Index;
	QLabel *m_Label;
	QPixmap m_Pixmap;
	bool m_PixmapOkay;
	QString m_ImageName;
	QMutex m_Mutex;
	bool m_ThreadRunning;
	QPalette m_DefaultPalette;
	QPalette m_SelectedPalette;

	BitmapWidgetThread *m_ReloadThread;

	bool m_ReloadRequested;
	int m_ReloadWidth;
	int m_ReloadHeight;
	QString m_ReloadName;

	QImage m_ReloadImage;
	bool m_ReloadSuccess;

private:
	BitmapWidget(const BitmapWidget &);
	BitmapWidget &operator=(const BitmapWidget &);
	friend class BitmapWidgetThread;

}; /* class BitmapWidget */

class BitmapWidgetThread : public QThread
{
	Q_OBJECT

protected:
	void run();

private:
	BitmapWidget *m_BitmapWidget;

	friend class BitmapWidget;

};

/**
 * BitmapSetup
 * \brief BitmapSetup
 * \date 2014-02-25 13:59GMT
 * \author Jan Boon (Kaetemi)
 */
class BitmapSetup : public QWidget
{
	Q_OBJECT

public:
	BitmapSetup(MainWindow *parent);
	virtual ~BitmapSetup();

	void select(int i);
	void deselect();

private slots:
	void propertiesSetterChanged(QWidget *setter);

private:
	MainWindow *m_MainWindow;
	BitmapWidget *m_Bitmaps[32];
	int m_Selected;

protected:
	virtual void resizeEvent(QResizeEvent *event);

private:
	BitmapSetup(const BitmapSetup &);
	BitmapSetup &operator=(const BitmapSetup &);

}; /* class BitmapSetup */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_BITMAP_SETUP_H */

/* end of file */
