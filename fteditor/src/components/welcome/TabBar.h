/*!
 * @file TabBar.h
 * @date 6/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef TABBAR_H
#define TABBAR_H

#include <QGridLayout>
#include <QTabBar>

class TabBar : public QTabBar {
 public:
  TabBar(QWidget *parent = 0);

 protected:
  void paintEvent(QPaintEvent *e);
};

#endif  // TABBAR_H
