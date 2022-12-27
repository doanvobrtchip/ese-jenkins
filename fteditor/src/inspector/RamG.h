/*!
 * @file RamG.h
 * @date 12/27/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMG_H
#define RAMG_H

#include <QWidget>

class QTreeWidgetItem;
class QLabel;
class QGroupBox;
class QLineEdit;
class QMenu;

namespace FTEDITOR {
class QHexView;
class Inspector;
struct ContentInfo;

class RamG : public QWidget {
  Q_OBJECT

 public:
  RamG(Inspector *parent);
  ~RamG() = default;
  void setupComponents(QGroupBox *ramGGroup);

 public slots:
  void setupConnections(QObject *obj = nullptr);
  void currItemContentChanged(QTreeWidgetItem *current,
                              QTreeWidgetItem *previous = nullptr);
  void handleContentItemPressed(QTreeWidgetItem *item);
  void handleCurrentInfoChanged(FTEDITOR::ContentInfo *contentInfo);
  void bindVisible(bool visible);
  void removeContentItem(FTEDITOR::ContentInfo *contentInfo);
  void addContentItem(FTEDITOR::ContentInfo *contentInfo);
  void updateData(int ramUsage = -1);

 private:
  Inspector *m_Inspector;
  QHexView *m_HexView;
  bool m_Visible;
  QLabel *lbUint;
  QLineEdit *lineEdit;

 signals:
  void updateCurrentInfo(FTEDITOR::ContentInfo *contentInfo);
};

}  // namespace FTEDITOR
#endif  // RAMG_H
