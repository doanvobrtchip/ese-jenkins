/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "emulator_navigator.h"

// Qt includes
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>

// Project includes
#include "emulator_viewport.h"

namespace FT800EMUQT {

EmulatorNavigator::EmulatorNavigator(QWidget *parent, EmulatorViewport *emulatorViewport) : QWidget(parent), m_EmulatorViewport(emulatorViewport)
{
	connect(emulatorViewport, SIGNAL(frame()), this, SLOT(repaint()));
}

EmulatorNavigator::~EmulatorNavigator()
{

}

static void calcpos(EmulatorNavigator &me, int &x, int &y, int &w, int &h, const QPixmap &pixmap)
{
	int hforw = pixmap.height() * me.width() / pixmap.width();
	if (hforw > me.height())
	{
		int wforh = pixmap.width() * me.height() / pixmap.height();
		w = wforh;
		h = me.height();
		x = (me.width() - wforh) / 2;
		y = 0;
	}
	else
	{
		w = me.width();
		h = hforw;
		x = 0;
		y = (me.height() - hforw) / 2;
	}
}

void EmulatorNavigator::mouseMoveEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::LeftButton)
	{
		int mx = e->pos().x();
		int my = e->pos().y();
		const QPixmap &pixmap = m_EmulatorViewport->getPixMap();
		int x, y, w, h;
		calcpos(*this, x, y, w, h, pixmap);
		int ex = (((mx - x) * pixmap.width() / w) - (pixmap.width() / 2)) * 16;
		int ey = (((my - y) * pixmap.height() / h) - (pixmap.height() / 2)) * 16;
		m_EmulatorViewport->horizontalScrollbar()->setValue(ex);
		m_EmulatorViewport->verticalScrollbar()->setValue(ey);
	}
}

void EmulatorNavigator::mousePressEvent(QMouseEvent *e)
{

}

void EmulatorNavigator::mouseReleaseEvent(QMouseEvent *e)
{

}

void EmulatorNavigator::wheelEvent(QWheelEvent* e)
{
	if (e->delta() > 0)
	{
		m_EmulatorViewport->setScreenScale(m_EmulatorViewport->screenScale() * 2);
	}
	else if (e->delta() < 0)
	{
		m_EmulatorViewport->setScreenScale(m_EmulatorViewport->screenScale() / 2);
	}
	QWidget::wheelEvent(e);
}

void EmulatorNavigator::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	const QPixmap &pixmap = m_EmulatorViewport->getPixMap();
	int x, y, w, h;
	calcpos(*this, x, y, w, h, pixmap);
	painter.drawPixmap(x, y, w, h, pixmap,
		0, 0, pixmap.width(), pixmap.height());

	const int screenScale = m_EmulatorViewport->screenScale();

	const int screenLeft = m_EmulatorViewport->screenLeft();
	const int screenLeftD = -screenLeft;
	const int screenLeftS = screenLeftD * 16 / screenScale;
	int screenLeftLine = screenLeftS * w / pixmap.width();
	screenLeftLine += x;

	const int screenTop = m_EmulatorViewport->screenTop();
	const int screenTopD = -screenTop;
	const int screenTopS = screenTopD * 16 / screenScale;
	int screenTopLine = screenTopS * h / pixmap.height();
	screenTopLine += y;

	const int screenRight = m_EmulatorViewport->screenRight();
	const int screenRightD = (m_EmulatorViewport->width() - screenRight);
	const int screenRightS = pixmap.width() + (screenRightD * 16 / screenScale);
	int screenRightLine = screenRightS * w / pixmap.width();
	screenRightLine = x + screenRightLine;

	const int screenBottom = m_EmulatorViewport->screenBottom();
	const int screenBottomD = (m_EmulatorViewport->height() - screenBottom);
	const int screenBottomS = pixmap.height() + (screenBottomD * 16 / screenScale);
	int screenBottomLine = screenBottomS * h / pixmap.height();
	screenBottomLine = y + screenBottomLine;

	QPen box;
	box.setWidth(1);
	box.setColor(QColor(Qt::red));
	painter.setPen(box);
	painter.drawLine(screenLeftLine, screenTopLine, screenLeftLine, screenBottomLine);
	painter.drawLine(screenRightLine, screenTopLine, screenRightLine, screenBottomLine);
	painter.drawLine(screenLeftLine, screenTopLine, screenRightLine, screenTopLine);
	painter.drawLine(screenLeftLine, screenBottomLine, screenRightLine, screenBottomLine);
}

} /* namespace FT800EMUQT */

/* end of file */
