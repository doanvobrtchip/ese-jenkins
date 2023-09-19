/*!
 * @file ExampleDialog.h
 * @date 6/1/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef EXAMPLEDIALOG_H
#define EXAMPLEDIALOG_H

#include <QDialog>

class QWidget;
class QTabWidget;
class QLineEdit;
class TabBar;

namespace FTEDITOR {
class MainWindow;
class ThumbnailView;

class ExampleDialog : public QDialog {
  Q_OBJECT

 public:
  ExampleDialog(MainWindow *mainWindow);
  ~ExampleDialog() = default;
  void setupTabs();

 public slots:
  void onSearch(const QString &text);
  void onItemPressed(const QString &path);

 private:
  MainWindow *m_mainWindow;
  QTabWidget *m_tabWidget;
  QLineEdit *m_searchEdit;
  TabBar *m_tabBar;
  int m_searchTabIndex;
  ThumbnailView *m_searchView; 
};

}  // namespace FTEDITOR
#endif  // EXAMPLEDIALOG_H
