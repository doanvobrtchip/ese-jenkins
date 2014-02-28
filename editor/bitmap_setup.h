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
#include <QJsonArray>

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

#define BITMAP_SETUP_HANDLES_NB 16

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
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
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
	bool m_MouseDown;

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

	// Utility
	// Clear all settings. This is not undostack friendly
	void clear();
	// Save
	QJsonArray toJson(bool extended) const;
	// Load. This is not undostack friendly
	void fromJson(QJsonArray &bitmaps);

	// Changes
	void changeSourceContent(int bitmapHandle, ContentInfo *value);
	void changeLayoutFormat(int bitmapHandle, int value);
	void changeLayoutStride(int bitmapHandle, int value);
	void changeLayoutHeight(int bitmapHandle, int value);
	void changeSizeFilter(int bitmapHandle, int value);
	void changeSizeWrapX(int bitmapHandle, int value);
	void changeSizeWrapY(int bitmapHandle, int value);
	void changeSizeWidth(int bitmapHandle, int value);
	void changeSizeHeight(int bitmapHandle, int value);

	// Lock to call when reading bitmap info from emulator thread
	// void lockBitmaps();
	// void unlockBitmaps();

	inline int getModificationNb() const { return m_ModificationNb; }
	inline const FT800EMU::BitmapInfo *getBitmapInfos() const { return m_BitmapInfo; }
	inline const ContentInfo *const *getBitmapSources() const { return m_BitmapSource; }
	inline bool bitmapSourceExists(int bitmapHandle) const { return m_BitmapSourceExists[bitmapHandle]; }
	inline int selected() const { return m_Selected; }

	void reloadContent(ContentInfo *contentInfo);

private:
	class ChangeSourceContent;
	class ChangeLayoutFormat;
	class ChangeLayoutStride;
	class ChangeLayoutHeight;
	class ChangeSizeFilter;
	class ChangeSizeWrapX;
	class ChangeSizeWrapY;
	class ChangeSizeWidth;
	class ChangeSizeHeight;

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
	void propSizeFilterChanged(int value);
	void propSizeWrapXChanged(int value);
	void propSizeWrapYChanged(int value);
	void propSizeWidthChanged(int value);
	void propSizeHeightChanged(int value);

private:
	MainWindow *m_MainWindow;
	BitmapWidget *m_Bitmaps[BITMAP_SETUP_HANDLES_NB];
	FT800EMU::BitmapInfo m_BitmapInfo[BITMAP_SETUP_HANDLES_NB];
	ContentInfo *m_BitmapSource[BITMAP_SETUP_HANDLES_NB]; // NOTE: Must check with ContentManager if pointer is still valid!
	volatile bool m_BitmapSourceExists[BITMAP_SETUP_HANDLES_NB]; // Caches above mentioned check.
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
	QComboBox *m_PropSizeFilter;
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
