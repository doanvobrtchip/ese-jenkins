/*!
 * @file RamGInspector.h
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMGINSPECTOR_H
#define RAMGINSPECTOR_H

#include "RamG.h"

class QBoxLayout;
class QPushButton;

namespace FTEDITOR {
class Inspector;

class RamGInspector : public RamG
{
	Q_OBJECT

public:
	RamGInspector(Inspector *parent);
	~RamGInspector() = default;

	QBoxLayout *setupComponents();

private:
	QPushButton *m_OpenDlgBtn;
};
} // namespace FTEDITOR
#endif // RAMGINSPECTOR_H
