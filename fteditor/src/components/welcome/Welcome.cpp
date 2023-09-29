/*!
 * @file Welcome.cpp
 * @date 4/25/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "Welcome.h"

#include <utils/LoggerUtil.h>

#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

#include "ExampleDialog.h"
#include "main_window.h"

namespace FTEDITOR {
Welcome::Welcome(MainWindow *mainWindow)
    : QDialog(mainWindow), m_mainWindow(mainWindow) {
  setWindowTitle(tr("Welcome"));
  setFont(QFont("Segoe UI", 10));
  setAttribute(Qt::WA_DeleteOnClose);

  auto vBoxLyt = new QVBoxLayout;

  QPixmap pixmapLogo(":/icons/logo.png");
  auto lbLogo = new QLabel;
  lbLogo->setPixmap(pixmapLogo);
  vBoxLyt->addWidget(lbLogo);

  auto btnCreate = new QPushButton(tr("Create"));
  auto btnExamples = new QPushButton(tr("Examples"));
  auto btnOpen = new QPushButton(tr("Open"));
  auto hLytButton = new QHBoxLayout;
  hLytButton->addWidget(btnCreate);
  hLytButton->addWidget(btnExamples);
  hLytButton->addWidget(btnOpen);
  hLytButton->addStretch();
  vBoxLyt->addLayout(hLytButton);

  auto lbRecentPrj = new QLabel(tr("Recent Projects"));
  vBoxLyt->addWidget(lbRecentPrj);

  auto lRecentPrj = mainWindow->RecentActionList();
  auto listWidget = new QListWidget;
  for (auto &prj : lRecentPrj) {
    auto item = new QListWidgetItem(listWidget);
    item->setSizeHint(QSize(0, 24));
    item->setText(prj->text());
    item->setData(Qt::UserRole, prj->data());
  }
  vBoxLyt->addWidget(listWidget);
  setLayout(vBoxLyt);

  connect(btnCreate, &QPushButton::clicked, this, [this]() {
    if (m_mainWindow->actSaveAs()) close();
  });
  connect(btnExamples, &QPushButton::clicked, this, [this]() {
    close();
    auto exampleDialog = new ExampleDialog(m_mainWindow);
    exampleDialog->open();
  });
  connect(btnOpen, &QPushButton::clicked, this, [this]() {
    if (m_mainWindow->actOpen()) close();
  });
  connect(listWidget, &QListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
            m_mainWindow->openProject(item->data(Qt::UserRole).toString());
            close();
          });

  ComponentBase::finishedSetup(this, m_mainWindow);
}
}  // namespace FTEDITOR
