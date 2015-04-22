/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FT800EMUQT_EMULATOR_NAVIGATOR_H
#define FT800EMUQT_EMULATOR_NAVIGATOR_H

// STL includes

// Qt includes
#include <QWidget>

namespace FT800EMUQT {

class EmulatorViewport;

class EmulatorNavigator : public QWidget
{
	Q_OBJECT

public:
	EmulatorNavigator(QWidget *parent, EmulatorViewport *emulatorViewport);
	virtual ~EmulatorNavigator();

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent* e);

	virtual void paintEvent(QPaintEvent *e);

private:
	EmulatorViewport *m_EmulatorViewport;

}; /* class EmulatorNavigator */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_EMULATOR_NAVIGATOR_H */

/* end of file */
