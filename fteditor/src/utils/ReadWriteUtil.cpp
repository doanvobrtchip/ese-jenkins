/*!
 * @file WriteFile.cpp
 * @date 11/17/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ReadWriteUtil.h"

#include <QByteArray>
#include <QFile>
#include <QTextStream>

ReadWriteUtil::ReadWriteUtil() {}

bool ReadWriteUtil::writeBinary(QString fileName, QByteArray &data) {
  QFile f(fileName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

  f.write(data);
  f.close();
  return true;
}

bool ReadWriteUtil::writeText(QString fileName, QString &data) {
  QFile f(fileName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

  QTextStream ts(&f);
  ts << data;
  ts.flush();
  f.close();
  return true;
}
