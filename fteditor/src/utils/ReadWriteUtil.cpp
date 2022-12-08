/*!
 * @file WriteFile.cpp
 * @date 11/17/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ReadWriteUtil.h"

#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>

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

QJsonObject ReadWriteUtil::getJsonInfo(QString &filePath)
{
  QJsonObject jo;
  QFileInfo ifp(filePath);
  if (!ifp.exists()) return jo;
  QString suffix = ifp.suffix();

  if (suffix == "json") {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return jo;
    QString data = file.readAll();
    file.close();
    auto jd = QJsonDocument::fromJson(data.toUtf8());
    if (jd.isNull()) return jo;
    return jd.object();
  }

  if (suffix == "readme") {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return jo;
    QTextStream in(&file);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QRegularExpression req("\\s+");
      QStringList listItem = line.split(req);
      if (listItem.at(0) != "name" && !listItem.at(0).isEmpty()) {
        jo.insert(listItem.at(0),
                  QJsonValue({{"offset", listItem.at(1).toInt()},
                              {"length", listItem.at(2).toInt()}}));
      }
    }
    file.close();
  }
  return jo;
}
