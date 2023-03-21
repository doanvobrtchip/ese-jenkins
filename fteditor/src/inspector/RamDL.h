/*!
 * @file RamDL.h
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMDL_H
#define RAMDL_H

#include "RamBase.h"

// Project includes
#include "dl_editor.h"
class QTreeWidget;
class QTreeWidgetItem;

namespace FTEDITOR {
class Inspector;

class RamDL : public RamBase
{
	Q_OBJECT

public:
	RamDL(Inspector *parent);
	void updateData();
	QByteArray getDLBinary(bool isBigEndian);
	QString getDLContent(bool isBigEndian);
	QTreeWidget *DisplayList() const;
	bool *handleUsage() const;
	int dlCMDCount() const;
	int firstDisplayCMD() const;
	bool eventFilter(QObject *watched, QEvent *event) override;
	
public slots:
	void onCopy();
	void updateView();
	void onPrepareContextMenu(const QPoint &pos);
	void openDialog(bool checked) override;
	void dockBack(bool checked) override;

private:
	bool m_DisplayListUpdate[FTEDITOR_DL_SIZE];
	uint32_t m_DisplayListCopy[FTEDITOR_DL_SIZE];
	int m_DLCMDCount;
	bool *m_HandleUsage;
	int m_FirstDisplayCMD;
	QAction *m_CopyAct;
	QMenu *m_ContextMenu;
	QTreeWidget *m_DisplayList;
	QTreeWidgetItem *m_DisplayListItems[FTEDITOR_DL_SIZE];
};
} // namespace FTEDITOR
#endif // RAMDL_H
