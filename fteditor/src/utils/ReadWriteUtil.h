/*!
 * @file ReadWriteUtil.h
 * @date 11/17/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef READWRITEUTIL_H
#define READWRITEUTIL_H

class QByteArray;
class QString;

class ReadWriteUtil {
 public:
  ReadWriteUtil();

  bool writeBinary(QString fileName, QByteArray &data);
  bool writeText(QString fileName, QString &data);
};

#endif  // READWRITEUTIL_H
