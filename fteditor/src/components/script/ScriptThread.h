/*!
 * @file ScriptThread.h
 * @date 8/2/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include <QThread>

#include "Script.h"

class ScriptThread : public QThread {
  Q_OBJECT

 public:
  enum State { NotRunning, Starting, Running, Error };
  ScriptThread(FTEDITOR::Script *parent = 0);
  ~ScriptThread();
  QString tempFilePath();
  QString outputPath();

 public slots:
  void handleTimeout();
  void stopRunning();
  void startRuning();

 protected:
  void run();

 private:
  const QString m_tempFileName = "temp.py";
  const QString m_outputFileName = "output";

  FTEDITOR::Script *m_scriptComp;
  QTimer *m_timer;

 signals:
  void textChanged(QString text);
  void stateChanged(ScriptThread::State state);
  void contentChanged(QString text);
};

#endif  // SCRIPTTHREAD_H
