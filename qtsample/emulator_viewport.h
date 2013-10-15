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

#ifndef FTQT_EMULATOR_VIEWPORT_H
#define FTQT_EMULATOR_VIEWPORT_H

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes

// Project includes

class QAction;
class QPixmap;
class QLabel;

namespace FTQT {
	// class GraphicsConfig;

/**
 * EmulatorViewport
 * \brief EmulatorViewport
 * \date 2010-02-06 10:11GMT
 * \author Jan Boon (Kaetemi)
 */
class EmulatorViewport : public QWidget
{
	Q_OBJECT

public:
	EmulatorViewport(QWidget *parent);
	virtual ~EmulatorViewport();

	// virtual QPaintEngine* paintEngine() const { return NULL; }

	// QAction *createSaveScreenshotAction(QObject *parent);

public slots:		
	// void saveScreenshot();

private:
	// EmulatorConfig *m_EmulatorConfig;
	QPixmap *m_Pixmap;
	QLabel *m_Label;

private:
	EmulatorViewport(const EmulatorViewport &);
	EmulatorViewport &operator=(const EmulatorViewport &);
	
}; /* class EmulatorViewport */

} /* namespace FTQT */

#endif /* #ifndef FTQT_EMULATOR_VIEWPORT_H */

/* end of file */
