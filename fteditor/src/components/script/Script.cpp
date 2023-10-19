/*!
 * @file Script.cpp
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "Script.h"

#include <Qsci/qscilexerpython.h>

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "ScriptEditor.h"
#include "ScriptThread.h"
#include "code_editor.h"
#include "dl_editor.h"
#include "main_window.h"

namespace FTEDITOR {

Script::Script(MainWindow *parent)
    : QWidget(parent), m_mainWindow(parent), m_thread(NULL) {
  m_cmdEditor = new DlEditor(parent, true);
  m_cmdEditor->setVisible(false);
  m_cmdEditor->setDisplayListModified(false);

  initialiseFont();
  m_scriptEditor = new ScriptEditor(m_LexerPy, this);

  auto vBoxLyt = new QVBoxLayout(this);
  vBoxLyt->addWidget(m_scriptEditor);

  m_lblStatus = new QLabel(tr("Not running!"));
  m_lblStatus->setWordWrap(true);
  m_lblStatus->setMaximumHeight(95);

  m_btnRun = new QPushButton(tr("Execute"));
  m_btnRun->setCheckable(true);
  connect(m_btnRun, &QPushButton::clicked, this, [this](bool checked) {
    return checked ? this->executeScript() : this->stopScript();
  });

  m_btnLoadExample = new QPushButton(tr("Load Example"));
  connect(m_btnLoadExample, &QPushButton::clicked, this, &Script::loadExample);

  auto hBoxLyt = new QHBoxLayout;
  hBoxLyt->addWidget(m_lblStatus, 1);
  hBoxLyt->addStretch();
  hBoxLyt->addWidget(m_btnLoadExample);
  hBoxLyt->addWidget(m_btnRun);
  vBoxLyt->addLayout(hBoxLyt);

  m_thread = new ScriptThread(this);
  connect(this, &Script::buttonCheckedChanged, this, [this](bool checked) {
    m_btnRun->setText(checked ? tr("Stop") : tr("Execute"));
  });

  connect(m_thread, &ScriptThread::started, this, &Script::handleStarted);
  connect(m_thread, &ScriptThread::finished, this, &Script::handleFinished);
  connect(m_thread, &ScriptThread::textChanged, m_lblStatus, &QLabel::setText);
  connect(m_thread, &ScriptThread::stateChanged, this,
          [this](ScriptThread::State state) {
            m_lblStatus->setStyleSheet(
                QString("QLabel { color : %1; }")
                    .arg(state == ScriptThread::Error ? "red" : "black"));
            if (state == ScriptThread::NotRunning) {
              m_lblStatus->setText(tr("Not running!"));
            } else if (state == ScriptThread::Running) {
              m_lblStatus->setText(tr("Running..."));
            }
            setButtonChecked(state == ScriptThread::Running);
          });
  connect(m_thread, &ScriptThread::contentChanged, this, [this](QString text) {
    m_cmdEditor->codeEditor()->setPlainText(text);
  });

  connect(m_mainWindow, &MainWindow::clearEvent, this, &Script::onClearEvent);
  ComponentBase::finishedSetup(this, m_mainWindow);
}

void Script::initialiseFont() {
  m_LexerPy = new QsciLexerPython(this);
  m_LexerPy->setDefaultFont(QFont("Segoe UI", 10));
  m_LexerPy->setFoldComments(true);
  m_LexerPy->setColor((palette().color(QPalette::Text)),
                      QsciLexerPython::Default);
  m_LexerPy->setColor(QColor::fromRgb(255, 0, 0), QsciLexerPython::Number);

  m_LexerPy->setColor(QColor::fromRgb(160, 160, 160),
                      QsciLexerPython::UnclosedString);
  m_LexerPy->setColor(QColor::fromRgb(160, 160, 160),
                      QsciLexerPython::DoubleQuotedString);
  m_LexerPy->setColor(QColor::fromRgb(160, 160, 160),
                      QsciLexerPython::SingleQuotedString);
  m_LexerPy->setColor(QColor::fromRgb(250, 125, 0),
                      QsciLexerPython::TripleSingleQuotedString);
  m_LexerPy->setColor(QColor::fromRgb(250, 125, 0),
                      QsciLexerPython::TripleDoubleQuotedString);

  m_LexerPy->setColor(QColor::fromRgb(0, 128, 58), QsciLexerPython::Comment);
  m_LexerPy->setColor(QColor::fromRgb(0, 128, 58),
                      QsciLexerPython::CommentBlock);

  m_LexerPy->setColor(QColor::fromRgb(0, 0, 255), QsciLexerPython::Keyword);
  m_LexerPy->setColor(QColor(Qt::black), QsciLexerPython::ClassName);
  m_LexerPy->setColor(QColor::fromRgb(255, 102, 255),
                      QsciLexerPython::FunctionMethodName);
  m_LexerPy->setColor(QColor::fromRgb(0, 0, 128), QsciLexerPython::Operator);
  m_LexerPy->setColor(QColor::fromRgb(117, 0, 153),
                      QsciLexerPython::HighlightedIdentifier);
}

QString Script::exampleFilePath() {
  QString defaultFilePath =
      m_mainWindow->applicationDataDir() + "/Examples/scripts/animation.py";
  if (m_mainWindow->CurrentFileName().isEmpty()) return defaultFilePath;
  QFileInfo fi(m_mainWindow->CurrentFileName());
  QString scriptFilePath =
      fi.absolutePath() + "/scripts/" + (fi.completeBaseName() + ".py");
  QFileInfo scriptFI(scriptFilePath);
  if (scriptFI.exists()) return scriptFilePath;
  return defaultFilePath;
}

void Script::focus() {
  setFocus();
  // Liem noted because it does not work
}

void Script::clear() { m_scriptEditor->clear(); }

void Script::ensureStopPython() {
  stopScript();
  while (m_thread->isRunning()) {
  }
}

void Script::loadExample(bool checked) {
  QFile file(exampleFilePath());
  if (!file.open(QIODevice::ReadOnly)) return;
  QTextStream stream(&file);
  QString content = stream.readAll().trimmed();
  m_scriptEditor->setText(content);
}

void Script::executeScript() { m_thread->startRuning(); }

void Script::stopScript() { m_thread->stopRunning(); }

void Script::setupConnections(QObject *obj) {
  if (auto cmd = m_mainWindow->cmdEditor(); cmd != nullptr) {
    if (auto cmdEditor = cmd->codeEditor();
        cmdEditor && (obj == cmdEditor || obj == nullptr)) {
      connect(cmdEditor, &QPlainTextEdit::textChanged, this,
              &Script::stopScript);
      connect(cmdEditor, &CodeEditor::cursorChanged, this, &Script::stopScript);
    }
  }

  if (auto dl = m_mainWindow->dlEditor(); dl != nullptr) {
    if (auto dlEditor = dl->codeEditor();
        dlEditor && (obj == dlEditor || obj == nullptr)) {
      connect(dlEditor, &QPlainTextEdit::textChanged, this,
              &Script::stopScript);
      connect(dlEditor, &CodeEditor::cursorChanged, this, &Script::stopScript);
    }
  }
}

void Script::handleStarted() { emit started(); }

void Script::handleFinished() {
  m_cmdEditor->setDisplayListModified(false);
  m_mainWindow->dlEditor()->setDisplayListModified(true);
  m_mainWindow->cmdEditor()->setDisplayListModified(true);
  emit finished();
}

void Script::onClearEvent() { m_scriptEditor->clear(); }

void Script::setButtonChecked(bool checked) {
  m_btnRun->setChecked(checked);
  emit buttonCheckedChanged(checked);
}

MainWindow *Script::mainWindow() const { return m_mainWindow; }

ScriptEditor *Script::editor() const { return m_scriptEditor; }

DlEditor *Script::cmdEditor() const { return m_cmdEditor; }

}  // namespace FTEDITOR
