/*!
 * @file ConvertUtil.cpp
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "ConvertUtil.h"

#include <stdio.h>

#include <QChar>
#include <QString>
#include <iomanip>
#include <sstream>

#include "content_manager.h"
#include "define/ValueDefine.h"
#include "dl_parser.h"

namespace FTEDITOR {

QString ConvertUtil::asText(uint32_t value) {
  QString line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, value);

  // verify parsing ->
  DlParsed parsed;
  DlParser::parse(FTEDITOR_CURRENT_DEVICE, parsed, line, false);
  uint32_t compiled = DlParser::compile(FTEDITOR_CURRENT_DEVICE, parsed);
  if (compiled != value && line.toLocal8Bit() != "") {
#if _DEBUG
    QByteArray chars = line.toLocal8Bit();
    printf("Parser bug '%s' -> expect %u, compiled %u\n", chars.constData(),
           value, compiled);
#endif
  }
  // <- verify parsing

  return line;
}

QString ConvertUtil::asRaw(uint32_t value) {
  std::stringstream raw;
  raw << "0x" << std::setfill('0') << std::setw(8) << std::hex << value;
  return raw.str().c_str();
}

QString ConvertUtil::asInt(uint32_t value) {
  std::stringstream res;
  res << value;
  return res.str().c_str();
}

QString ConvertUtil::asSignedInt(uint32_t value) {
  std::stringstream res;
  res << (int32_t)value;
  double fl = (double)(int32_t)value / 65536.0;
  res << " (" << fl << ")";
  return res.str().c_str();
}

QString ConvertUtil::asSignedInt16F(uint32_t value) {
  std::stringstream res;
  res << (int32_t)value;
  double fl = (double)(int32_t)value / 65536.0;
  res << " (" << fl << ")";
  return res.str().c_str();
}

QString ConvertUtil::uintToHex(uint32_t value, int fillWith) {
  return "0x" + QString("%1").arg(value, fillWith, 16, QChar('0')).toUpper();
}

QString ConvertUtil::uintToHexNoPrefix(uint32_t value, int fillWith) {
  return QString("%1").arg(value, fillWith, 16, QChar('0')).toUpper();
}

uint32_t ConvertUtil::stringToDec(const QString &value) {
  bool ok;
  uint32_t result = value.toUInt(&ok, DEC);
  if (ok) return result;
  result = value.toUInt(&ok, HEX);
  if (ok) return result;
  return 0;
}

}  // namespace FTEDITOR
