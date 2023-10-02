/*!
 * @file Registers.h
 * @date 8/15/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef REGISTERS_H
#define REGISTERS_H

#include <QWidget>

#include "ComponentBase.h"

class QSpinBox;
class QUndoStack;
class QLabel;

namespace FTEDITOR {
class DlEditor;
class MainWindow;

class Registers : public QWidget, public ComponentBase {
  Q_OBJECT
 public:
  explicit Registers(MainWindow *parent);

  QSpinBox *hSize() const;
  QSpinBox *vSize() const;
  QSpinBox *rotate() const;
  DlEditor *macro() const;
  QJsonObject toJson(bool exportScript);
  void fromJson(QJsonObject obj);
  
  int latestHSF() const;
  
  public slots:
  void setupConnections(QObject *obj) override;
  void setHSize(int newValue);
  void setVSize(int newValue);
  void setRotate(int newValue);
  void setHSF(int newValue);
  void setPlayCtrl(int newPlayCtrl);

  void onHSizeChanged(int newValue);
  void onVSizeChanged(int newValue);
  void onRotateChanged(int newValue);
  void onHSFChanged(int newValue);
  void onClearEvent();
  void onDisplaySizeChanged(int hSize, int vSize);

 private:
  MainWindow *m_mainWindow;
  QUndoStack *m_undoStack;
  QSpinBox *m_hSize;
  QSpinBox *m_vSize;
  QSpinBox *m_rotate;
  DlEditor *m_macro;
  QSpinBox *m_hsf;
  int m_playCtrl;
  QLabel *m_lbCurrPlayCtrl;
  bool m_undoRedoWorking;

  int m_latestHSize;
  int m_latestVSize;
  int m_latestRotate;
  int m_latestHSF;

 signals:
  void hSizeChanged(int newValue);
  void vSizeChanged(int newValue);
  void rotateChanged(int newValue);
  void playCtrlChanged(int newValue);
};
}  // namespace FTEDITOR
#endif  // REGISTERS_H
