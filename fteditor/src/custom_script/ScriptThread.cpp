/*!
 * @file ScriptThread.cpp
 * @date 8/2/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ScriptThread.h"

#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

#include <QDir>
#include <QTimer>
#include <iostream>
#include <sstream>

#include "ScriptEditor.h"
#include "main_window.h"
#include "utils/PyUtil.h"

ScriptThread::ScriptThread(FTEDITOR::ScriptComponent *parent) {
  m_scriptComp = parent;
  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &ScriptThread::handleTimeout);
}

ScriptThread::~ScriptThread() { stopRunning(); }

QString ScriptThread::tempFilePath() {
  return QDir(m_scriptComp->mainWindow()->applicationDataDir())
      .absoluteFilePath(m_tempFileName);
}

QString ScriptThread::outputPath() {
  return QDir::current().absoluteFilePath(m_outputFileName);
}

void ScriptThread::handleTimeout() {
  QFile file(outputPath());
  if (!file.open(QIODevice::ReadOnly)) return;
  QTextStream stream(&file);
  QString content = stream.readAll().trimmed();
  file.close();
  if (!content.isEmpty()) emit contentChanged(content);
}

void ScriptThread::stopRunning() {
  m_timer->stop();
  if (isRunning()) {
    QFile file(tempFilePath());
    file.remove();
    QFile fileOutput(outputPath());
    fileOutput.remove();
	
    emit stateChanged(NotRunning);
    exit();
  }
}

void ScriptThread::startRuning() {
  start();
  m_timer->start(1);
}

void ScriptThread::run() {
  emit stateChanged(Running);
  QFile tempFile(tempFilePath());
  if (!tempFile.open(QIODevice::WriteOnly)) return;
  QTextStream stream(&tempFile);
  stream << m_scriptComp->editor()->text();
  tempFile.close();

  bool error = true;
  auto objName = PyUnicode_FromString(
      QString(m_tempFileName).remove(".py").toUtf8().data());
  auto objOldModule = PyImport_Import(objName);
  Py_DECREF(objName);
  objName = NULL;

  if (objOldModule) {
    auto objModule = PyImport_ReloadModule(objOldModule);

    if (objModule) {
      auto objFunc = PyObject_GetAttrString(objModule, "main");
      if (objFunc && PyCallable_Check(objFunc)) {
        auto objResult = PyObject_CallNoArgs(objFunc);
        error = false;
        Py_XDECREF(objResult);
        objResult = NULL;
        Py_XDECREF(objFunc);
        objFunc = NULL;
      }
      Py_XDECREF(objModule);
      objModule = NULL;
    }
    Py_XDECREF(objOldModule);
    objOldModule = NULL;
  }
  tempFile.remove();

  if (error) {
    emit stateChanged(Error);
    emit textChanged(PyUtil::pyError());
    exit();
    return;
  }
  emit stateChanged(NotRunning);
  exit();
}
