/*!
 * @file Welcome.h
 * @date 4/25/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef WELCOME_H
#define WELCOME_H

#include <QDialog>

#include "ComponentBase.h"

namespace FTEDITOR {
class MainWindow;

class Welcome : public QDialog, public ComponentBase {
  Q_OBJECT
 public:
  Welcome(MainWindow *mainWindow);
  ~Welcome() = default;

 public slots:
  void setupConnections(QObject *obj) override{};

 private:
  MainWindow *m_mainWindow;
};

}  // namespace FTEDITOR
#endif  // WELCOME_H
