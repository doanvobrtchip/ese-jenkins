/*!
 * @file RamBase.cpp
 * @date 3/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamBase.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "inspector.h"
#include "main_window.h"

namespace FTEDITOR {

RamBase::RamBase(Inspector *parent)
    : QDockWidget(parent->mainWindow()), m_insp(parent) {
  setFloating(true);
  setFocusPolicy(Qt::StrongFocus);
  setFont(QFont("Segoe UI", 10));

  m_btnOpenDlg = new QPushButton;
  m_btnOpenDlg->setContentsMargins(0, 0, 0, 0);
  m_btnOpenDlg->setIconSize(QSize(10, 10));
  m_btnOpenDlg->setIcon(QIcon(":/icons/external-link.png"));

  m_btnDockBack = new QPushButton;
  m_btnDockBack->setContentsMargins(0, 0, 0, 0);
  m_btnDockBack->setIconSize(QSize(10, 10));
  m_btnDockBack->setIcon(QIcon(":/icons/dock-back.png"));
  m_btnDockBack->setVisible(false);

  m_lbTitle = new QLabel;
  m_widget = new QWidget(this);

  m_lytTitle = new QHBoxLayout;
  m_lytTitle->setAlignment(Qt::AlignLeft);
  m_lytTitle->addWidget(m_lbTitle);
  m_lytTitle->addWidget(m_btnOpenDlg);
  m_lytTitle->addWidget(m_btnDockBack);

  connect(this, &QDockWidget::visibilityChanged, m_lbTitle,
          [this](bool visible) {
            m_btnOpenDlg->setVisible(!visible);
            m_btnDockBack->setVisible(visible);
            m_lbTitle->setVisible(!visible);
          });
  connect(m_btnOpenDlg, &QPushButton::clicked, this, &RamBase::openDialog);
  connect(m_btnDockBack, &QPushButton::clicked, this, &RamBase::dockBack);
}

QWidget *RamBase::Widget() const { return m_widget; }

void RamBase::openDialog(bool checked) {
  setWidget(m_widget);
  show();
  raise();
  activateWindow();
  if (m_focusWhenOpen) m_focusWhenOpen->setFocus(Qt::MouseFocusReason);
}

void RamBase::dockBack(bool checked) {
  m_insp->addSplitter(m_widget);
  close();
  if (m_focusWhenOpen) m_focusWhenOpen->setFocus(Qt::MouseFocusReason);
}

void RamBase::closeEvent(QCloseEvent *event) { dockBack(); }

void RamBase::setFocusWhenOpen(QWidget *newFocusWhenOpen) {
  m_focusWhenOpen = newFocusWhenOpen;
}
}  // namespace FTEDITOR
