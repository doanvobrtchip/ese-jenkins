/*!
 * @file ImageUtil.cpp
 * @date 6/9/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ImageUtil.h"

#include <QGraphicsEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>

QImage ImageUtil::applyEffectToImage(QImage src, QGraphicsEffect *effect,
                                     int extent) {
  if (src.isNull()) return QImage();  
  if (!effect) return src;           
  QGraphicsScene scene;
  QGraphicsPixmapItem item;
  item.setPixmap(QPixmap::fromImage(src));
  item.setGraphicsEffect(effect);
  scene.addItem(&item);
  QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
  res.fill(Qt::transparent);
  QPainter ptr(&res);
  scene.render(&ptr, QRectF(),
               QRectF(-extent, -extent, src.width() + extent * 2,
                      src.height() + extent * 2));
  return res;
}
