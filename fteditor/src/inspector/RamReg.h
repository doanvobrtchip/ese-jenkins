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

class RamReg : public RamBase {
  Q_OBJECT

 public:
  RamReg(Inspector *widget);
  ~RamReg() = default;
  void initDisplayReg();
  void releaseDisplayReg();
  bool wantRegister(int regEnum);
  bool eventFilter(QObject *watched, QEvent *event) override;

 public slots:
  void onCopy();
  void updateView(int dlCMDCount);
  void onPrepareContextMenu(const QPoint &pos);
  void openDialog(bool checked) override;
  void dockBack(bool checked) override;

 private:
  QAction *m_actCopy;
  QMenu *m_menuContext;
  QTreeWidget *m_registers;
  std::vector<uint32_t> m_registerCopy;
  std::vector<QTreeWidgetItem *> m_registerItems;
};
}  // namespace FTEDITOR
#endif  // RAMREG_H
