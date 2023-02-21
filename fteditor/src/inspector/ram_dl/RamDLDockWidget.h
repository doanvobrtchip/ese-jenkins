/*!
 * @file RamDLDockWidget.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMDLDOCKWIDGET_H
#define RAMDLDOCKWIDGET_H

// Project includes
#include "RamDL.h"

class QBoxLayout;

namespace FTEDITOR {

class RamDLDockWidget : public RamDL
{
	Q_OBJECT

public:
	RamDLDockWidget(Inspector *parent);
	~RamDLDockWidget() = default;

private:
	QBoxLayout *setupComponents();
};
} // namespace FTEDITOR

#endif // RAMDLDOCKWIDGET_H
