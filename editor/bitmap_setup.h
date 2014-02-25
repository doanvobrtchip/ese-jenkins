/**
 * bitmap_setup.h
 * $Id$
 * \file bitmap_setup.h
 * \brief bitmap_setup.h
 * \date 2014-01-31 16:56GMT
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

// Qt includes
#include <QWidget>
#include <QString>
#include <QMutex>
#include <QJsonObject>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes
// ...

namespace FT800EMUQT {

class MainWindow;

/**
 * BitmapSetup
 * \brief BitmapSetup
 * \date 2014-01-31 16:56GMT
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

private:
	BitmapSetup(const BitmapSetup &);
	BitmapSetup &operator=(const BitmapSetup &);

}; /* class BitmapSetup */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_BITMAP_SETUP_H */

/* end of file */
