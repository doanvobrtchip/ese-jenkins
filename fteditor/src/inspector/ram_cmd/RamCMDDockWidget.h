/*!
 * @file RamCMDDockWidget.h
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMCMDDOCKWIDGET_H
#define RAMCMDDOCKWIDGET_H

#include "RamCMD.h"

class QBoxLayout;

namespace FTEDITOR {

class RamCMDDockWidget : public RamCMD
{
	Q_OBJECT

public:
	RamCMDDockWidget(Inspector *parent);
	~RamCMDDockWidget() = default;
	QBoxLayout *setupComponents();
};

} // namespace FTEDITOR
#endif // RAMCMDDOCKWIDGET_H
