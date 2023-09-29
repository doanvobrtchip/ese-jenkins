/*!
 * @file Script.h
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef SCRIPT_H
#define SCRIPT_H

#include <QWidget>

#include "ComponentBase.h"

class QPushButton;
class QsciLexerPython;
class ScriptEditor;
class ScriptThread;
class QLabel;

namespace FTEDITOR {
class DlEditor;
class MainWindow;

class Script : public QWidget, public ComponentBase {
  Q_OBJECT

 public:
  Script(MainWindow *parent = 0);
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
  void setupConnections(QObject *obj) override;
  void loadExample(bool checked);
  void stopScript();
  void handleStarted();
  void handleFinished();
  void onClearEvent();

 signals:
  void buttonCheckedChanged(bool checked);
  void started();
  void finished();

 private:
  MainWindow *m_mainWindow;
  ScriptEditor *m_scriptEditor;
  ScriptThread *m_thread;
  QPushButton *m_btnRun;
  QPushButton *m_btnLoadExample;
  QLabel *m_lblStatus;
  DlEditor *m_cmdEditor;
  QsciLexerPython *m_LexerPy;
};

}  // namespace FTEDITOR
#endif  // SCRIPT_H
