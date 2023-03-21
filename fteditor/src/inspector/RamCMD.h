/*!
 * @file RamCMD.h
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMCMD_H
#define RAMCMD_H

#include "RamBase.h"

class QLineEdit;

namespace FTEDITOR {
class Inspector;
class QHexView;

class RamCMD : public RamBase
{
	Q_OBJECT

public:
	RamCMD(Inspector *parent);
	~RamCMD() = default;

public slots:
	void goToAddress();
	void setLabelUint(uint value);
	void updateView();
	void openDialog(bool checked) override;
	void dockBack(bool checked) override;

private:
	QLabel *m_UintLabel;
	QHexView *m_HexView;
	QLabel *m_AddressLabel;
	QPushButton *m_SearchButton;
	QHBoxLayout *m_SearchLayout;
	QLineEdit *m_AddressJumpEdit;
};
} // namespace FTEDITOR
#endif // RAMCMD_H
