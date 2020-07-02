/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_EMULATOR_NAVIGATOR_H
#define FTEDITOR_EMULATOR_NAVIGATOR_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes

// Qt includes
#include <QWidget>

namespace FTEDITOR {

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

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_EMULATOR_NAVIGATOR_H */

/* end of file */
