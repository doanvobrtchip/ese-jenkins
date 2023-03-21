/*!
 * @file RamG.h
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMG_H
#define RAMG_H

#include "RamBase.h"

class QTreeWidgetItem;
class QLineEdit;
class QLabel;

namespace FTEDITOR {
class QHexView;
struct ContentInfo;
class Inspector;

class RamG : public RamBase
{
	Q_OBJECT

public:
	RamG(Inspector *parent);
	~RamG() = default;

public slots:
	void setupConnections(QObject *obj = nullptr);
	void currItemContentChanged(QTreeWidgetItem *current,
	    QTreeWidgetItem *previous = nullptr);
	void handleContentItemPressed(QTreeWidgetItem *item);
	void handleCurrentInfoChanged(FTEDITOR::ContentInfo *contentInfo);
	void bindVisible(bool visible);
	void removeContentItem(FTEDITOR::ContentInfo *contentInfo);
	void addContentItem(FTEDITOR::ContentInfo *contentInfo);
	void goToAddress();
	void setLabelUint(uint value);
	void updateView();
	void openDialog(bool checked) override;
	void dockBack(bool checked) override;

private:
	bool m_Visible;
	QHexView *m_HexView;
	QLabel *m_UintLabel;
	QLabel *m_AddressLabel;
	QPushButton *m_SearchButton;
	QHBoxLayout *m_SearchLayout;
	QLineEdit *m_AddressJumpEdit;

signals:
	void updateCurrentInfo(FTEDITOR::ContentInfo *contentInfo);
};
} // namespace FTEDITOR

#endif // RAMG_H
