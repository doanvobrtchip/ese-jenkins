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

class RamDL : public RamBase {
  Q_OBJECT

 public:
  RamDL(Inspector *parent);
  void updateData();
  int dlCMDCount() const;
  bool *handleUsage() const;
  int firstDisplayCMD() const;
  QTreeWidget *DisplayList() const;
  QString getDLContent(bool isBigEndian);
  QByteArray getDLBinary(bool isBigEndian);
  bool eventFilter(QObject *watched, QEvent *event) override;

 public slots:
  void onCopy();
  void updateView();
  void onPrepareContextMenu(const QPoint &pos);
  void openDialog(bool checked) override;
  void dockBack(bool checked) override;

 private:
  bool *m_hUsage;
  int m_dlCMDCount;
  QAction *m_actCopy;
  QMenu *m_menuContext;
  int m_firstDisplayCMD;
  QTreeWidget *m_displayList;
  bool m_displayListUpdate[FTEDITOR_DL_SIZE];
  uint32_t m_displayListCopy[FTEDITOR_DL_SIZE];
  QTreeWidgetItem *m_displayListItems[FTEDITOR_DL_SIZE];
};
}  // namespace FTEDITOR
#endif  // RAMDL_H
