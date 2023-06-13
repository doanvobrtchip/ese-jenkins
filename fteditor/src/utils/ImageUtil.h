/*!
 * @file ImageUtil.h
 * @date 6/9/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

class QGraphicsEffect;
class QImage;

class ImageUtil
{
public:
  static QImage applyEffectToImage(QImage src, QGraphicsEffect *effect,
                                   int extent = 0);
};

#endif // IMAGEUTIL_H
