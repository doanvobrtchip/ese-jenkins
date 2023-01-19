/*!
 * @file ReadWriteUtil.h
 * @date 11/17/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef READWRITEUTIL_H
#define READWRITEUTIL_H

class QByteArray;
class QString;
class QJsonObject;

class ReadWriteUtil {
 public:
  static bool writeBinary(QString fileName, QByteArray &data);
  static bool writeText(QString fileName, QString &data);
  static QJsonObject getJsonInfo(QString &filePath);
  static QString readConvertedCharsFile(QString &file);
  static void readConvertedCharsIndexFile(QString &file, int &first, int &last);
};

#endif  // READWRITEUTIL_H
