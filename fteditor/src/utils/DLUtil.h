/*!
 * @file DLUtils.h
 * @date 1/4/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef DLUTIL_H
#define DLUTIL_H

#include "content_manager.h"
#include "dl_editor.h"

using namespace FTEDITOR;
class DLUtil {
 public:
  static bool addPalettedSrc(DlEditor *lineEditor, DlParsed &pa, int address, int &line);
  static bool addPaletted8Cmds(DlEditor *lineEditor, DlParsed &pa, int address, uint32_t &selection,
                            int &line, int32_t x, int32_t y);
  static bool addBitmapHandler(DlEditor *lineEditor, DlParsed &pa, ContentInfo *contentInfo,
                        uint32_t &bitmapHandle, int &line, int &hline);
  static bool addSetFont2Cmd(DlEditor *lineEditor, DlParsed &pa, uint32_t &font, int address, int firstChar,
                           int &line, int &hline);
  static bool addSetFontCmd(DlEditor *lineEditor, DlParsed &pa, int address, uint32_t &bitmapHandle,
                         int &line, int &hline);
  static bool addBitmapCmds(DlEditor *lineEditor, DlParsed &pa, ContentInfo *contentInfo,
                         uint32_t &selection, int &line, int32_t x, int32_t y);
  static bool addTextCmd(DlEditor *lineEditor, DlParsed &pa, uint32_t &bitmapHandle, QString text,
                      int &line, int32_t x, int32_t y);
};

#endif  // DLUTIL_H
