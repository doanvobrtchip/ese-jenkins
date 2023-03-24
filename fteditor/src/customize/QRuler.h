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
  void setIndicator(int pos);
  void setScale(int newScale);
  void setScreenTop(int newScreenTop);
  void setScreenLeft(int newScreenLeft);
  void setVisible(bool visible) override;
  void setShowIndicator(bool newShowIndicator);

 protected:
  virtual void paintEvent(QPaintEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *e) override;

 private:
  int m_scale;
  int m_indicator;
  int m_screenTop;
  int m_screenLeft;
  bool m_showIndicator;
  Qt::Orientation m_orientation;

 signals:
  void visibleChanged(bool visible);
};

#endif  // QRULER_H
