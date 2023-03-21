/*!
 * @file RamReg.h
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMREG_H
#define RAMREG_H

#include "RamBase.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace FTEDITOR {
class Inspector;

class RamReg : public RamBase
{
	Q_OBJECT

public:
	RamReg(Inspector *widget);
	~RamReg() = default;
	void initDisplayReg();
	bool wantRegister(int regEnum);
	void releaseDisplayReg();
	bool eventFilter(QObject *watched, QEvent *event) override;
	
public slots:
	void updateView(int dlCMDCount);
	void onPrepareContextMenu(const QPoint &pos);
	void onCopy();
	void openDialog(bool checked) override;
	void dockBack(bool checked) override;

private:
	std::vector<uint32_t> m_RegisterCopy;
	std::vector<QTreeWidgetItem *> m_RegisterItems;
	QAction *m_CopyAct;
	QMenu *m_ContextMenu;
	QTreeWidget *m_Registers;
};
} // namespace FTEDITOR
#endif // RAMREG_H
