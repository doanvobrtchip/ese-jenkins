/*!
 * @file ScriptComponent.cpp
 * @date 7/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ScriptComponent.h"

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

ScriptComponent::ScriptComponent(MainWindow *parent)
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
  connect(m_btnLoadExample, &QPushButton::clicked, this,
          &ScriptComponent::loadExample);

  auto hBoxLyt = new QHBoxLayout;
  hBoxLyt->addWidget(m_lblStatus, 1);
  hBoxLyt->addStretch();
  hBoxLyt->addWidget(m_btnLoadExample);
  hBoxLyt->addWidget(m_btnRun);
  vBoxLyt->addLayout(hBoxLyt);

  m_thread = new ScriptThread(this);
  connect(this, &ScriptComponent::buttonCheckedChanged, this,
          [this](bool checked) {
            m_btnRun->setText(checked ? tr("Stop") : tr("Execute"));
          });

  connect(m_thread, &ScriptThread::started, this,
          &ScriptComponent::handleStarted);
  connect(m_thread, &ScriptThread::finished, this,
          &ScriptComponent::handleFinished);
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

  connect(m_mainWindow, &MainWindow::readyToSetup, this,
          &ScriptComponent::setup);

  connect(m_mainWindow, &MainWindow::eventNew, this,
          &ScriptComponent::handleNewEvent);
  setup();
  emit m_mainWindow->readyToSetup(this);
}

void ScriptComponent::initialiseFont() {
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

QString ScriptComponent::exampleFilePath() {
  return m_mainWindow->applicationDataDir() + "/Examples/script/animation.py";
}

void ScriptComponent::focus() {
  setFocus();
  //Liem noted because it does not work
}

void ScriptComponent::loadExample(bool checked) {
  QFile file(exampleFilePath());
  if (!file.open(QIODevice::ReadOnly)) return;
  QTextStream stream(&file);
  QString content = stream.readAll().trimmed();
  m_scriptEditor->setText(content);
}

void ScriptComponent::executeScript() { m_thread->startRuning(); }

void ScriptComponent::stopScript() { m_thread->stopRunning(); }

void ScriptComponent::setup(QObject *obj) {
  auto cmdEditor = m_mainWindow->cmdEditor()->codeEditor();
  if (cmdEditor && (obj == cmdEditor || obj == nullptr)) {
	  connect(cmdEditor, &QPlainTextEdit::textChanged, this,
		  &ScriptComponent::stopScript);
	  connect(cmdEditor, &CodeEditor::cursorChanged, this,
		  &ScriptComponent::stopScript);
  }
  
  auto dlEditor = m_mainWindow->dlEditor()->codeEditor();
  if (dlEditor && (obj == dlEditor || obj == nullptr)) {
	  connect(dlEditor, &QPlainTextEdit::textChanged, this,
		  &ScriptComponent::stopScript);
	  connect(dlEditor, &CodeEditor::cursorChanged, this,
		  &ScriptComponent::stopScript);
  }
}

void ScriptComponent::handleStarted() { emit started(); }

void ScriptComponent::handleFinished() {
  m_cmdEditor->setDisplayListModified(false);
  m_mainWindow->dlEditor()->setDisplayListModified(true);
  m_mainWindow->cmdEditor()->setDisplayListModified(true);
  emit finished();
}

void ScriptComponent::handleNewEvent() { m_scriptEditor->clear(); }

void ScriptComponent::setButtonChecked(bool checked) {
  m_btnRun->setChecked(checked);
  emit buttonCheckedChanged(checked);
}

MainWindow *ScriptComponent::mainWindow() const { return m_mainWindow; }

ScriptEditor *ScriptComponent::editor() const { return m_scriptEditor; }

DlEditor *ScriptComponent::cmdEditor() const { return m_cmdEditor; }

}  // namespace FTEDITOR
