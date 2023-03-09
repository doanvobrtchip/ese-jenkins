/*!
 * @file RamCMD.h
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMCMD_H
#define RAMCMD_H

#include <QDockWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QHBoxLayout;

namespace FTEDITOR {
class Inspector;
class QHexView;

class RamCMD : public QDockWidget
{
	Q_OBJECT

public:
	RamCMD(Inspector *parent);
	~RamCMD() = default;

public slots:
	void setupConnections(QObject *obj = nullptr);
	void goToAddress();
	void setLabelUint(uint value);
	void updateView();

protected:
	Inspector *m_Inspector;
	QHexView *m_HexView;
	QLabel *m_UintLabel;
	QLineEdit *m_AddressJumpEdit;
	QPushButton *m_SearchButton;
	QLabel *m_AddressLabel;
	QHBoxLayout *m_SearchLayout;
};
} // namespace FTEDITOR
#endif // RAMCMD_H
