/*!
 * @file RamDL.h
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMDL_H
#define RAMDL_H

#include <QDockWidget>

// Project includes
#include "dl_editor.h"
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;

namespace FTEDITOR {
class Inspector;

class RamDL : public QDockWidget
{
	Q_OBJECT

public:
	RamDL(Inspector *parent);
	void updateData();
	void updateView();
	QByteArray getDLBinary(bool isBigEndian);
	QString getDLContent(bool isBigEndian);
	QTreeWidget *DisplayList() const;
	bool *handleUsage() const;
	int dlCMDCount() const;
	int firstDisplayCMD() const;
	bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
	void onCopy();
	void onPrepareContextMenu(const QPoint &pos);

protected:
	QTreeWidget *m_DisplayList;
	QTreeWidgetItem *m_DisplayListItems[FTEDITOR_DL_SIZE];
	uint32_t m_DisplayListCopy[FTEDITOR_DL_SIZE];
	bool m_DisplayListUpdate[FTEDITOR_DL_SIZE];
	QMenu *m_ContextMenu;
	QAction *m_CopyAct;
	bool *m_HandleUsage;
	int m_DLCMDCount;
	int m_FirstDisplayCMD;
	Inspector *m_Inspector;
};
} // namespace FTEDITOR
#endif // RAMDL_H
