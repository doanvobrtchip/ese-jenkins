/*!
 * @file TabBar.cpp
 * @date 6/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "TabBar.h"

#include <QStyleOptionTab>
#include <QStylePainter>

TabBar::TabBar(QWidget *parent) : QTabBar(parent) {
  setFont(QFont("Segoe UI", 10));
}

void TabBar::paintEvent(QPaintEvent *e) {
  QStylePainter painter(this);
  QStyleOptionTab opt;

  for (int i = 0; i < count(); i++) {
    initStyleOption(&opt, i);
    painter.save();
    painter.drawControl(QStyle::CE_TabBarTabShape, opt);
    painter.drawControl(QStyle::CE_TabBarTabLabel, opt);
    painter.restore();
  }
}
