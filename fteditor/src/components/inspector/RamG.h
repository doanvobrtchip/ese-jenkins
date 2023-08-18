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

class RamG : public RamBase {
  Q_OBJECT

 public:
  RamG(Inspector *parent);
  ~RamG() = default;

 public slots:
  void setupConnections(QObject *obj) override;
  void currItemContentChanged(QTreeWidgetItem *current,
                              QTreeWidgetItem *previous = nullptr);
  void handleContentItemPressed(QTreeWidgetItem *item);
  void handleCurrentInfoChanged(FTEDITOR::ContentInfo *contentInfo);
  void bindVisible(bool visible);
  void removeContentItem(FTEDITOR::ContentInfo *contentInfo);
  void addContentItem(FTEDITOR::ContentInfo *contentInfo);
  void goToAddress();
  void setLabelUint(QString valueStr, uint value);
  void updateView();

 private:
  bool m_visible;
  QLabel *m_lbUint;
  QHexView *m_hexView;
  QLabel *m_lbAddress;
  QLineEdit *m_leAddress;
  QPushButton *m_btnSearch;
  QHBoxLayout *m_lytSearch;

 signals:
  void updateCurrentInfo(FTEDITOR::ContentInfo *contentInfo);
};
}  // namespace FTEDITOR

#endif  // RAMG_H
