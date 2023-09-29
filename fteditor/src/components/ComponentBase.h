/*!
 * @file ComponentBase.h
 * @date 8/15/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef COMPONENTBASE_H
#define COMPONENTBASE_H

#include <QObject>

namespace FTEDITOR {
class MainWindow;

class ComponentBase {
 public:
  void finishedSetup(QObject *self, FTEDITOR::MainWindow *obj);

 public slots:
  virtual void setupConnections(QObject *obj = nullptr) = 0;
};
}  // namespace FTEDITOR
#endif  // COMPONENTBASE_H
