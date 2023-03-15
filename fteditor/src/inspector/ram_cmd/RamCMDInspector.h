/*!
 * @file RamCMDInspector.h
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMCMDINSPECTOR_H
#define RAMCMDINSPECTOR_H

#include "RamCMD.h"

class QBoxLayout;
class QPushButton;

namespace FTEDITOR {
class Inspector;

class RamCMDInspector : public RamCMD
{
public:
	RamCMDInspector(Inspector *parent);
	~RamCMDInspector() = default;
	QBoxLayout *setupComponents();

private:
	QPushButton *m_OpenDlgBtn;
};

} // namespace FTEDITOR
#endif // RAMCMDINSPECTOR_H
