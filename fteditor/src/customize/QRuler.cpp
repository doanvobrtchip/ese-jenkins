/*!
 * @file QRuler.cpp
 * @date 12/12/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "QRuler.h"

#include <QPainter>
#include <QPen>
#include <QScrollBar>

#include "constant_mapping.h"
QRuler::QRuler(QWidget *parent, Qt::Orientation orientation)
    : QWidget(parent),
      m_Orientation(orientation),
      m_Scale(16),
      m_ScreenLeft(0),
      m_ScreenTop(0) {
}

void QRuler::paintEvent(QPaintEvent *e) {
  QPainter p(this);

#define UNTFX(x) ((((x - m_ScreenLeft) * 16) / m_Scale))
#define UNTFY(y) ((((y - m_ScreenTop) * 16) / m_Scale))

  QPen pen(Qt::blue);
  p.setPen(pen);
  int distance = (m_Scale / 16 <= 3) ? 3 : m_Scale / 16;

  int charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
  int charHeight = fontMetrics().height();
  if (m_Orientation == Qt::Horizontal) {
    // Draw positive value
    for (int i = m_ScreenLeft, y = 0; i < width(); i += distance, ++y) {
      if (y % 10 == 0) {
        p.drawLine(QLineF(i, height() - 10, i, height()));
        p.drawText(i - (charWidth * 4 + 6) / 2, 3, charWidth * 4 + 6,
                   charHeight, Qt::AlignCenter,
                   QString::number((float)UNTFX(i)));
      } else if (y % 10 == 5) {
        p.drawLine(QLineF(i, height() - 7, i, height()));
      } else {
        p.drawLine(QLineF(i, height() - 4, i, height()));
      }
    }
    // Draw negative value
    for (int i = m_ScreenLeft - distance, y = 1; i > 0; i -= distance, ++y) {
      if (y % 10 == 0) {
        p.drawLine(QLineF(i, height() - 10, i, height()));
        p.drawText(i - (charWidth * 4 + 6) / 2, 3, charWidth * 4 + 6,
                   charHeight, Qt::AlignCenter, QString::number(UNTFX(i)));
      } else if (y % 10 == 5) {
        p.drawLine(QLineF(i, height() - 7, i, height()));
      } else {
        p.drawLine(QLineF(i, height() - 4, i, height()));
      }
    }
  } else {
    // Draw positive value
    for (int i = m_ScreenTop, y = 0; i < height(); i += distance, y++) {
      if (y % 10 == 0) {
        p.drawLine(QLineF(width() - 10, i, width(), i));
        p.drawText(0, i - charHeight + 2, width() - 4, charHeight,
                   Qt::AlignRight | Qt::AlignBottom, QString::number(UNTFY(i)));
      } else if (y % 10 == 5) {
        p.drawLine(QLineF(width() - 7, i, width(), i));
      } else {
        p.drawLine(QLineF(width() - 4, i, width(), i));
      }
    }
    // Draw negative value
    for (int i = m_ScreenTop - distance, y = 1; i > 0; i -= distance, y++) {
      if (y % 10 == 0) {
        p.drawLine(QLineF(width() - 10, i, width(), i));
        p.drawText(0, i - charHeight + 2, width() - 4, charHeight,
                   Qt::AlignRight | Qt::AlignBottom, QString::number(UNTFY(i)));
      } else if (y % 10 == 5) {
        p.drawLine(QLineF(width() - 7, i, width(), i));
      } else {
        p.drawLine(QLineF(width() - 4, i, width(), i));
      }
    }
  }
  p.end();
}

void QRuler::mousePressEvent(QMouseEvent *e) {}

void QRuler::setScreenTop(int newScreenTop) { m_ScreenTop = newScreenTop; }

void QRuler::setVisible(bool newVisible)
{
 if (QWidget::isVisible() == newVisible) return;
 QWidget::setVisible(newVisible);
 emit visibleChanged(newVisible);
}

void QRuler::setScreenLeft(int newScreenLeft) { m_ScreenLeft = newScreenLeft; }

void QRuler::setScale(int newScale) { m_Scale = newScale; }
