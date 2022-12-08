/*!
 * @file LoggerUtil.h
 * @date 11/23/2022
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef LOGGERUTIL_H
#define LOGGERUTIL_H

#define debugLog(msg)                                                        \
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ssap MM/dd/yy") + \
                  ": " + msg

#endif  // LOGGERUTIL_H
