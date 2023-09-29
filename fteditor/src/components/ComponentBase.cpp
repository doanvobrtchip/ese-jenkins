/*!
 * @file ComponentBase.cpp
 * @date 8/15/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ComponentBase.h"

#include "main_window.h"
namespace FTEDITOR {

void ComponentBase::finishedSetup(QObject* self, MainWindow* main) {
  setupConnections();
  QObject::connect(main, SIGNAL(readyToSetup(QObject*)), self,
                   SLOT(setupConnections(QObject*)));
  emit main->readyToSetup(self);
}
}  // namespace FTEDITOR
