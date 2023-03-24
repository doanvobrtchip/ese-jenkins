/*!
 * @file RamCMD.h
 * @date 3/8/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef RAMCMD_H
#define RAMCMD_H

#include "RamBase.h"

class QLineEdit;

namespace FTEDITOR {
class Inspector;
class QHexView;

class RamCMD : public RamBase {
  Q_OBJECT

 public:
  RamCMD(Inspector *parent);
  ~RamCMD() = default;

 public slots:
  void updateView();
  void goToAddress();
  void openDialog(bool checked) override;
  void dockBack(bool checked) override;
  void setLabelUint(QString valueStr, uint value);

 private:
  QLabel *m_lbUint;
  QHexView *m_hexView;
  QLabel *m_lbAddress;
  QLineEdit *m_leAddress;
  QPushButton *m_btnSearch;
  QHBoxLayout *m_lytSearch;
};
}  // namespace FTEDITOR
#endif  // RAMCMD_H
