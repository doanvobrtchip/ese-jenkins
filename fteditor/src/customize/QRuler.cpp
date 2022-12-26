/*!
 * @file QRuler.cpp
 * @date 12/12/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "QRuler.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QScrollBar>

#include "constant_mapping.h"
QRuler::QRuler(QWidget *parent, Qt::Orientation orientation)
    : QWidget(parent),
      m_Orientation(orientation),
      m_Scale(16),
      m_ScreenLeft(0),
      m_ScreenTop(0),
      m_ShowIndicator(false),
      m_Indicator(0) {}

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
                   charHeight, Qt::AlignCenter, QString::number(UNTFX(i)));
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

    if (m_ShowIndicator) {
      QPainterPath path;
      // Set pen to this point.
      int indicatorWidth = 10;
      int indicatorHeight = 6;
      int y = height() - indicatorHeight;
      int startPoint = m_Indicator - indicatorWidth / 2;
      path.moveTo(startPoint, y);
      // Draw line from pen point to this point.
      path.lineTo(startPoint + indicatorWidth, y);
      path.lineTo(startPoint + indicatorWidth / 2, y + indicatorHeight);
      path.lineTo(startPoint, y);

      p.setPen(Qt ::NoPen);
      p.fillPath(path, QBrush(QColor("red")));
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

    if (m_ShowIndicator) {
      QPainterPath path;
      int indicatorWidth = 5;
      int indicatorHeight = 10;
      int x = width() - indicatorWidth;
      int startPoint = m_Indicator - indicatorHeight / 2;
      // Set pen to this point.
      path.moveTo(x, startPoint);
      // Draw line from pen point to this point.
      path.lineTo(x, startPoint + indicatorHeight);
      path.lineTo(x + indicatorWidth, startPoint + indicatorHeight / 2);
      path.lineTo(x, startPoint);

      p.setPen(Qt ::NoPen);
      p.fillPath(path, QBrush(QColor("red")));
    }
  }
  p.end();
}

void QRuler::mousePressEvent(QMouseEvent *e) {}

void QRuler::setIndicator(int pos) { m_Indicator = pos; }

void QRuler::setShowIndicator(bool newShowIndicator) {
  m_ShowIndicator = newShowIndicator;
}

void QRuler::setScreenTop(int newScreenTop) { m_ScreenTop = newScreenTop; }

void QRuler::setVisible(bool newVisible) {
  if (QWidget::isVisible() == newVisible) return;
  QWidget::setVisible(newVisible);
  emit visibleChanged(newVisible);
}

void QRuler::setScreenLeft(int newScreenLeft) { m_ScreenLeft = newScreenLeft; }

void QRuler::setScale(int newScale) { m_Scale = newScale; }
