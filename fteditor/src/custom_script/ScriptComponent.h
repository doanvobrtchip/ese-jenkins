/*!
 * @file ScriptComponent.h
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef SCRIPTCOMPONENT_H
#define SCRIPTCOMPONENT_H

#include <QWidget>

class QPushButton;
class QsciLexerPython;
class ScriptEditor;
class ScriptThread;
class QLabel;

namespace FTEDITOR {
class DlEditor;
class MainWindow;

class ScriptComponent : public QWidget {
  Q_OBJECT

 public:
  ScriptComponent(MainWindow *parent = 0);
  void executeScript();
  void setButtonChecked(bool checked);
  MainWindow *mainWindow() const;
  ScriptEditor *editor() const;
  DlEditor *cmdEditor() const;
  void initialiseFont();
  QString exampleFilePath();
  void focus();
  void clear();
  void ensureStopPython();

 public slots:
  void loadExample(bool checked);
  void stopScript();
  void setup(QObject *obj = nullptr);
  void handleStarted();
  void handleFinished();
  void handleNewEvent();

 signals:
  void buttonCheckedChanged(bool checked);
  void started();
  void finished();

 private:
  ScriptEditor *m_scriptEditor;
  MainWindow *m_mainWindow;
  ScriptThread *m_thread;
  QPushButton *m_btnRun;
  QPushButton *m_btnLoadExample;
  QLabel *m_lblStatus;
  DlEditor *m_cmdEditor;
  QsciLexerPython *m_LexerPy;
};

}  // namespace FTEDITOR
#endif  // SCRIPTCOMPONENT_H
