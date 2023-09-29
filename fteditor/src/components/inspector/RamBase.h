/*!
 * @file RamBase.h
 * @date 3/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMBASE_H
#define RAMBASE_H

#include <QDockWidget>

#include "ComponentBase.h"

class QLabel;
class QPushButton;
class QHBoxLayout;

namespace FTEDITOR {
class Inspector;

class RamBase : public QDockWidget, public ComponentBase {
  Q_OBJECT

 public:
  RamBase(Inspector *parent);
  ~RamBase() = default;
  QWidget *Widget() const;
  void setFocusWhenOpen(QWidget *newFocusWhenOpen);

 public slots:
  void openDialog(bool checked = true);
  void dockBack(bool checked = true);

 protected:
  void closeEvent(QCloseEvent *event) override;

  QWidget *m_widget;
  Inspector *m_insp;
  QLabel *m_lbTitle;
  QHBoxLayout *m_lytTitle;
  QWidget *m_focusWhenOpen;
  QPushButton *m_btnOpenDlg;
  QPushButton *m_btnDockBack;
};
}  // namespace FTEDITOR

#endif  // RAMBASE_H
