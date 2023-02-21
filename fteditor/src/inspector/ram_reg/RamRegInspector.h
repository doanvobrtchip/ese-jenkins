/*!
 * @file RamRegInspector.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMREGINSPECTOR_H
#define RAMREGINSPECTOR_H

#include "RamReg.h"

class QBoxLayout;
class QPushButton;

namespace FTEDITOR {
class Inspector;

class RamRegInspector : public RamReg
{
	Q_OBJECT

public:
	RamRegInspector(Inspector *parent);
	~RamRegInspector() = default;
	QBoxLayout *setupComponents();

private:
	QPushButton *m_OpenDlgBtn;
};
} // namespace FTEDITOR
#endif // RAMREGINSPECTOR_H
