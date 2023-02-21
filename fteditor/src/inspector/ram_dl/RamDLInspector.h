/*!
 * @file RamDLInspector.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMDLINSPECTOR_H
#define RAMDLINSPECTOR_H

#include <QWidget>
// Project includes
#include "RamDL.h"

class QPushButton;
class QBoxLayout;

namespace FTEDITOR {
class Inspector;

class RamDLInspector : public RamDL
{
	Q_OBJECT

public:
	RamDLInspector(Inspector *parent);
	QBoxLayout *setupComponents();

private:
	QPushButton *m_OpenDlgBtn;
};
} // namespace FTEDITOR
#endif // RAMDLINSPECTOR_H
