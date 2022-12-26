/*!
 * @file QRuler.h
 * @date 12/12/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef QRULER_H
#define QRULER_H

#include <QAbstractScrollArea>
#include <QWidget>

class QRuler : public QWidget {
  Q_OBJECT

 public:
  QRuler(QWidget *parent = nullptr, Qt::Orientation orientation = Qt::Vertical);
  ~QRuler() = default;
  void setScale(int newScale);
  void setScreenLeft(int newScreenLeft);
  void setScreenTop(int newScreenTop);
  void setVisible(bool visible) override;

  void setShowIndicator(bool newShowIndicator);
  void setIndicator(int pos);

 protected:
  virtual void paintEvent(QPaintEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *e) override;

 private:
  Qt::Orientation m_Orientation;
  int m_Scale;
  int m_ScreenLeft;
  int m_ScreenTop;
  bool m_ShowIndicator;
  int m_Indicator;

 signals:
  void visibleChanged(bool visible);
};

#endif  // QRULER_H
