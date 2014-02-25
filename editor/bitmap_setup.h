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

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes
// ...

class QResizeEvent;

namespace FT800EMUQT {

class MainWindow;

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

private:
	MainWindow *m_MainWindow;
	int m_Index;

private:
	BitmapWidget(const BitmapWidget &);
	BitmapWidget &operator=(const BitmapWidget &);
}; /* class BitmapWidget */

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

private:
	MainWindow *m_MainWindow;
	BitmapWidget *m_Bitmaps[32];

protected:
	virtual void resizeEvent(QResizeEvent *event);

private:
	BitmapSetup(const BitmapSetup &);
	BitmapSetup &operator=(const BitmapSetup &);

}; /* class BitmapSetup */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_BITMAP_SETUP_H */

/* end of file */
