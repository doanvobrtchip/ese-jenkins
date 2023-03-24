/*!
 * @file RamBase.h
 * @date 3/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMBASE_H
#define RAMBASE_H

#include <QDockWidget>

class QLabel;
class QPushButton;
class QHBoxLayout;

namespace FTEDITOR {
class Inspector;

class RamBase : public QDockWidget {
  Q_OBJECT

 public:
  RamBase(Inspector *parent);
  ~RamBase() = default;
  QWidget *Widget() const;

 public slots:
  virtual void openDialog(bool checked = true);
  virtual void dockBack(bool checked = true);

 protected:
  void closeEvent(QCloseEvent *event) override;

  QWidget *m_widget;
  Inspector *m_insp;
  QLabel *m_lbTitle;
  QPushButton *m_btnOpenDlg;
  QPushButton *m_btnDockBack;
  QHBoxLayout *m_lytTitle;
};
}  // namespace FTEDITOR

#endif  // RAMBASE_H
