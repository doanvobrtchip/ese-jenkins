/*!
 * @file LoggerUtil.h
 * @date 11/23/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef LOGGERUTIL_H
#define LOGGERUTIL_H
#include <QDateTime>

#define debugLog(msg)                                                        \
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ssap MM/dd/yy") + \
                  ": " + msg

#endif  // LOGGERUTIL_H
