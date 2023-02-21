/*!
 * @file RamG.h
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMG_H
#define RAMG_H

#include <QDockWidget>

class QTreeWidgetItem;
class QLabel;
class QLineEdit;
class QPushButton;
class QHBoxLayout;

namespace FTEDITOR {
class QHexView;
struct ContentInfo;
class Inspector;

class RamG : public QDockWidget
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
	void updateView();
	void goToAddress();
	void setLabelUint(uint value);

protected:
	QHexView *m_HexView;
	bool m_Visible;
	QLabel *m_UintLabel;
	QLineEdit *m_AddressJumpEdit;
	QPushButton *m_SearchButton;
	QLabel *m_AddressLabel;
	QHBoxLayout *m_SearchLayout;
	Inspector *m_Inspector;

signals:
	void updateCurrentInfo(FTEDITOR::ContentInfo *contentInfo);
};
} // namespace FTEDITOR

#endif // RAMG_H
