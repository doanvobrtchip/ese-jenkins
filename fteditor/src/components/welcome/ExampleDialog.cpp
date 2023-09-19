/*!
 * @file ExamplesDialog.cpp
 * @date 6/1/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ExampleDialog.h"

#include <utils/LoggerUtil.h>

#include <QAction>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "TabBar.h"
#include "ThumbnailView.h"
#include "Welcome.h"
#include "main_window.h"
#include "utils/CommonUtil.h"

namespace FTEDITOR {
FTEDITOR::ExampleDialog::ExampleDialog(MainWindow *mainWindow)
    : QDialog(mainWindow),
      m_mainWindow(mainWindow),
      m_tabWidget(NULL),
      m_searchEdit(NULL),
      m_tabBar(NULL),
      m_searchView(NULL) {
  m_tabWidget = new QTabWidget;
  m_tabWidget->tabBar()->setHidden(true);

  setWindowTitle(tr("Examples"));
  setFont(QFont("Segoe UI", 10));

  setupTabs();
  setFocus(Qt::MouseFocusReason);
  resize(1021, 540);
}

void ExampleDialog::setupTabs() {
  QString pathExample = m_mainWindow->applicationDataDir() + "/Examples";
  QDir dirExample(pathExample);
  if (!dirExample.exists()) return;

  m_tabBar = new TabBar;
  auto entryList = dirExample.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (auto &item : entryList) {
    if (!CommonUtil::existProject(dirExample.absoluteFilePath(item))) continue;
    auto view = new ThumbnailView(this, dirExample.absoluteFilePath(item));
    m_tabWidget->addTab(view, item);
    m_tabBar->addTab(item);
    view->updateViewContent(view->search(""));
  }

  // Search tab
  m_searchView = new ThumbnailView(this, "");
  m_tabWidget->addTab(m_searchView, "");
  m_searchTabIndex = m_tabBar->addTab("");
  m_tabBar->setTabVisible(m_searchTabIndex, false);

  connect(m_tabBar, &QTabBar::currentChanged, m_tabWidget,
          &QTabWidget::setCurrentIndex);

  m_searchEdit = new QLineEdit;
  QFont f("Segoe UI", 10);
  m_searchEdit->setFont(f);
  m_searchEdit->setMinimumWidth(60);
  m_searchEdit->addAction(QIcon(":/icons/search.png"),
                          QLineEdit::LeadingPosition);
  auto clearAct = m_searchEdit->addAction(QIcon(":/icons/cross-black.png"),
                                          QLineEdit::TrailingPosition);
  clearAct->setToolTip(tr("Clear"));
  connect(clearAct, &QAction::triggered, m_searchEdit, [this]() {
    m_searchEdit->clear();
    m_tabBar->setTabText(m_searchTabIndex, "");
    m_tabBar->setTabVisible(m_searchTabIndex, false);
    m_tabBar->setCurrentIndex(0);
  });
  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &ExampleDialog::onSearch);

  auto backBtn = new QPushButton;
  backBtn->setIcon(QIcon(":/icons/return.png"));
  backBtn->setIconSize(QSize(24, 24));
  backBtn->setFlat(true);
  backBtn->setToolTip(tr("Back"));
  connect(backBtn, &QPushButton::clicked, this, [this]() {
    close();
    auto welcome = new Welcome(m_mainWindow);
    welcome->open();
  });

  auto headerLyt = new QHBoxLayout;
  headerLyt->addWidget(backBtn);
  headerLyt->addWidget(m_tabBar);
  headerLyt->addStretch();
  headerLyt->addWidget(m_searchEdit, 0, Qt::AlignRight);

  auto mainLyt = new QVBoxLayout;
  mainLyt->addLayout(headerLyt);
  mainLyt->addWidget(m_tabWidget);

  setLayout(mainLyt);
  m_tabBar->setCurrentIndex(0);
}

void ExampleDialog::onSearch(const QString &text) {
  QStringList result;
  for (int i = 0; i < m_tabWidget->count(); ++i) {
    result.append(
        static_cast<ThumbnailView *>(m_tabWidget->widget(i))->search(text));
  }
  m_searchView->updateViewContent(result);

  auto searchTitle = QString(tr("Seach: \"%1\"")).arg(text);
  m_tabBar->setTabText(m_searchTabIndex, searchTitle);
  m_tabBar->setTabVisible(m_searchTabIndex, true);
  m_tabBar->setCurrentIndex(m_searchTabIndex);
}

void ExampleDialog::onItemPressed(const QString &path) {
  m_mainWindow->actOpen(path);
  close();
}
}  // namespace FTEDITOR
