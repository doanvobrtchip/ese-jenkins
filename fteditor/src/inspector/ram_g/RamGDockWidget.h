/*!
 * @file RamGDockWidget.h
 * @date 2/7/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMGDOCKWIDGET_H
#define RAMGDOCKWIDGET_H

#include "RamG.h"

class QBoxLayout;

namespace FTEDITOR {
class Inspector;

class RamGDockWidget : public RamG
{
	Q_OBJECT

public:
	RamGDockWidget(Inspector *parent);
	~RamGDockWidget() = default;

	QBoxLayout *setupComponents();
};
} // namespace FTEDITOR
#endif // RAMGDOCKWIDGET_H
