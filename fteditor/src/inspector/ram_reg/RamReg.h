/*!
 * @file RamReg.h
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMREG_H
#define RAMREG_H

#include <QDockWidget>

class QTreeWidget;
class QTreeWidgetItem;

namespace FTEDITOR {
class Inspector;

class RamReg : public QDockWidget
{
	Q_OBJECT

public:
	RamReg(Inspector *widget);
	~RamReg() = default;
	void initDisplayReg();
	bool wantRegister(int regEnum);
	void updateView(int dlCMDCount);
	void releaseDisplayReg();
	bool eventFilter(QObject *watched, QEvent *event);

public slots:
	void onPrepareContextMenu(const QPoint &pos);
	void onCopy();

protected:
	QTreeWidget *m_Registers;
	std::vector<QTreeWidgetItem *> m_RegisterItems;
	std::vector<uint32_t> m_RegisterCopy;
	QMenu *m_ContextMenu;
	QAction *m_CopyAct;
	Inspector *m_Inspector;
};
} // namespace FTEDITOR
#endif // RAMREG_H
