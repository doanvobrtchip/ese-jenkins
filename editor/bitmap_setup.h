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
#include <ft800emu_graphics_processor.h>

// Project includes
#include "content_manager.h"

class QResizeEvent;
class QLabel;
class QGroupBox;
class QComboBox;
class QSpinBox;

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
	void unsetImage();

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

	// Changes
	void changeSourceContent(int bitmapHandle, ContentInfo *value);
	void changeLayoutFormat(int bitmapHandle, int value);
	void changeLayoutStride(int bitmapHandle, int value);
	void changeLayoutHeight(int bitmapHandle, int value);

	// Lock to call when reading bitmap info from emulator thread
	// void lockBitmaps();
	// void unlockBitmaps();

	inline int getModificationNb() const { return m_ModificationNb; }

	void reloadContent(ContentInfo *contentInfo);

private:
	class ChangeSourceContent;
	class ChangeLayoutFormat;
	class ChangeLayoutStride;
	class ChangeLayoutHeight;

	void rebuildGUIInternal();
	void refreshGUIInternal(int bitmapHandle);
	void refreshGUIInternal();
	void refreshViewInternal(int bitmapHandle);

private slots:
	void propertiesSetterChanged(QWidget *setter);

	void propSourceContentChanged(int value);
	void propLayoutFormatChanged(int value);
	void propLayoutStrideChanged(int value);
	void propLayoutHeightChanged(int value);

private:
	MainWindow *m_MainWindow;
	BitmapWidget *m_Bitmaps[32];
	FT800EMU::BitmapInfo m_BitmapInfo[32];
	ContentInfo *m_BitmapSource[32]; // NOTE: Must check with ContentManager if pointer is still valid!
	int m_Selected;
	int m_ModificationNb;
	bool m_RebuildingPropSourceContent;

	QGroupBox *m_PropSource;
	QComboBox *m_PropSourceContent;

	QGroupBox *m_PropLayout; // BITMAP_LAYOUT(format, linestride, height)
	QComboBox *m_PropLayoutFormat;
	QSpinBox *m_PropLayoutStride;
	QSpinBox *m_PropLayoutHeight;

	QGroupBox *m_PropSize; // BITMAP_SIZE(filter, wrapx, wrapy, width, height)
	QComboBox *m_PropSizeWrapX;
	QComboBox *m_PropSizeWrapY;
	QSpinBox *m_PropSizeWidth;
	QSpinBox *m_PropSizeHeight;

	QMutex m_Mutex;

protected:
	virtual void resizeEvent(QResizeEvent *event);

private:
	BitmapSetup(const BitmapSetup &);
	BitmapSetup &operator=(const BitmapSetup &);

}; /* class BitmapSetup */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_BITMAP_SETUP_H */

/* end of file */
