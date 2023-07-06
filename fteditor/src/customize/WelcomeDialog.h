/*!
 * @file WelcomeDialog.h
 * @date 4/25/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

class QWidget;

namespace FTEDITOR {
class MainWindow;

class WelcomeDialog : public QDialog {
 public:
  WelcomeDialog(MainWindow *mainWindow);
  ~WelcomeDialog() = default;

 private:
  MainWindow *m_MainWindow;
};

}  // namespace FTEDITOR
#endif  // WELCOMEDIALOG_H
