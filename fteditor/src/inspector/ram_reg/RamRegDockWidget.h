/*!
 * @file RamRegDockWidget.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMREGDOCKWIDGET_H
#define RAMREGDOCKWIDGET_H

#include "RamReg.h"

class QBoxLayout;

namespace FTEDITOR {
class Inspector;

class RamRegDockWidget : public RamReg
{
	Q_OBJECT

public:
	RamRegDockWidget(Inspector *parent);
	QBoxLayout *setupComponents();
};
} // namespace FTEDITOR

#endif // RAMREGDOCKWIDGET_H
