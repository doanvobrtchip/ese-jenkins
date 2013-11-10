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
 * \date 2010-02-06 10:11GMT
 * \author Jan Boon (Kaetemi)
 */
class EmulatorViewport : public QWidget
{
	Q_OBJECT

public:
	EmulatorViewport(QWidget *parent);
	virtual ~EmulatorViewport();
	
	// Runs the emulator on a new thread and connects it with this viewport
	void run(const FT800EMU::EmulatorParameters &params);

	// virtual QPaintEngine* paintEngine() const { return NULL; }

	// QAction *createSaveScreenshotAction(QObject *parent);

public slots:		
	// void saveScreenshot();
	void threadRepaint();

private:
	QLabel *m_Label;

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
