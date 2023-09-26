/*!
 * @file DLUtil.cpp
 * @date 1/4/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "DLUtil.h"

#include "constant_common.h"

namespace FTEDITOR {

bool DLUtil::addPalettedSrc(DlEditor *lineEditor, DlParsed &pa, int address,
                            int &line) {
  pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
  pa.Parameter[0].I = address;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  ++line;
  return true;
}

bool DLUtil::addPaletted8Cmds(DlEditor *lineEditor, DlParsed &pa, int address,
                              uint32_t &selection, int &line, int32_t x,
                              int32_t y) {
  DlParsed pav;
  pav.ValidId = true;
  pav.IdLeft = FTEDITOR_DL_VERTEX2F;  // FIXME: Drag-drop outside
                                      // 512px does not work??
  pav.IdRight = 0;
  pav.Parameter[0].I = x;
  pav.Parameter[1].I = y;
  // pav.Parameter[2].I = bitmapHandle;
  // pav.Parameter[3].I = 0;
  pav.ExpectedParameterCount = 2;
  pav.ExpectedStringParameter = false;
  pav.VarArgCount = 0;
  pa.IdRight = FTEDITOR_DL_BEGIN;
  pa.Parameter[0].U = selection;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  ++line;

  int lastVertex;
  pa.IdRight = FTEDITOR_DL_SAVE_CONTEXT;
  pa.ExpectedParameterCount = 0;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
  pa.Parameter[0].U = ONE;
  pa.Parameter[1].U = ZERO;
  pa.ExpectedParameterCount = 2;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdRight = FTEDITOR_DL_COLOR_MASK;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = 0;
  pa.Parameter[2].U = 0;
  pa.Parameter[3].U = 1;
  pa.ExpectedParameterCount = 4;
  lineEditor->insertLine(line, pa);
  ++line;
  addPalettedSrc(lineEditor, pa, address + 3, line);
  lineEditor->insertLine(line, pav);
  ++line;

  pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
  pa.Parameter[0].U = DST_ALPHA;
  pa.Parameter[1].U = ONE_MINUS_DST_ALPHA;
  pa.ExpectedParameterCount = 2;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdRight = FTEDITOR_DL_COLOR_MASK;
  pa.Parameter[0].U = 1;
  pa.Parameter[1].U = 0;
  pa.Parameter[2].U = 0;
  pa.Parameter[3].U = 0;
  pa.ExpectedParameterCount = 4;
  lineEditor->insertLine(line, pa);
  ++line;

  addPalettedSrc(lineEditor, pa, address + 2, line);
  lineEditor->insertLine(line, pav);
  ++line;
  pa.IdRight = FTEDITOR_DL_COLOR_MASK;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = 1;
  pa.Parameter[2].U = 0;
  pa.Parameter[3].U = 0;
  pa.ExpectedParameterCount = 4;
  lineEditor->insertLine(line, pa);
  ++line;

  addPalettedSrc(lineEditor, pa, address + 1, line);
  lineEditor->insertLine(line, pav);
  ++line;

  pa.IdRight = FTEDITOR_DL_COLOR_MASK;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = 0;
  pa.Parameter[2].U = 1;
  pa.Parameter[3].U = 0;
  pa.ExpectedParameterCount = 4;
  lineEditor->insertLine(line, pa);
  ++line;

  addPalettedSrc(lineEditor, pa, address, line);
  lineEditor->insertLine(line, pav);
  ++line;

  pa.IdRight = FTEDITOR_DL_RESTORE_CONTEXT;
  pa.ExpectedParameterCount = 0;
  lineEditor->insertLine(line, pa);
  ++line;

  lastVertex = -2;
  pa.IdLeft = 0;
  pa.IdRight = FTEDITOR_DL_END;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  lineEditor->selectLine(line + lastVertex);
  return true;
}

bool DLUtil::addBitmapHandler(DlEditor *lineEditor, DlParsed &pa,
                              ContentInfo *contentInfo, uint32_t &bitmapHandle,
                              int &line, int &hline) {
  pa.IdRight = FTEDITOR_DL_BITMAP_HANDLE;
  pa.Parameter[0].U = bitmapHandle;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(hline, pa);
  ++hline;
  ++line;

  bool useSetBitmap = FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 &&
                      lineEditor->isCoprocessor() &&
                      contentInfo->bitmapAddress() >= 0;

  if (useSetBitmap) {
    pa.IdLeft = 0xFFFFFF00;
    pa.IdRight = CMD_SETBITMAP & 0xFF;

    pa.Parameter[0].U = contentInfo->bitmapAddress();

    if (contentInfo->Converter == ContentInfo::ImageCoprocessor) {
      pa.Parameter[0].U =
          contentInfo->bitmapAddress() + contentInfo->PalettedAddress;
    }

    pa.Parameter[1].U = contentInfo->ImageFormat;
    pa.Parameter[2].U = contentInfo->CachedImageWidth & 0x7FF;
    pa.Parameter[3].U = contentInfo->CachedImageHeight & 0x7FF;
    pa.ExpectedParameterCount = 4;
    lineEditor->insertLine(hline, pa);
    ++hline;
    ++line;
    pa.IdLeft = 0;
  } else {
    pa.IdRight = FTEDITOR_DL_BITMAP_SOURCE;
    pa.Parameter[0].U = contentInfo->bitmapAddress();
    pa.ExpectedParameterCount = 1;
    lineEditor->insertLine(hline, pa);
    ++hline;
    ++line;
  }

  if (!useSetBitmap) {
    pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT;
    pa.Parameter[0].U = contentInfo->ImageFormat;
    pa.Parameter[1].U = contentInfo->CachedImageStride & 0x3FF;
    pa.Parameter[2].U = contentInfo->CachedImageHeight & 0x1FF;
    pa.ExpectedParameterCount = 3;
    lineEditor->insertLine(hline, pa);
    ++hline;
    ++line;

    if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) {
      // Add _H if necessary
      pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT_H;
      pa.Parameter[0].U = contentInfo->CachedImageStride >> 10;
      pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
      pa.ExpectedParameterCount = 2;
      lineEditor->insertLine(hline, pa);
      ++hline;
      ++line;
    }

    pa.IdRight = FTEDITOR_DL_BITMAP_SIZE;
    pa.Parameter[0].U = 0;  // size filter
    pa.Parameter[1].U = 0;  // wrap x
    pa.Parameter[2].U = 0;  // wrap y
    pa.Parameter[3].U = contentInfo->CachedImageWidth & 0x1FF;
    pa.Parameter[4].U = contentInfo->CachedImageHeight & 0x1FF;
    pa.ExpectedParameterCount = 5;
    lineEditor->insertLine(hline, pa);
    ++hline;
    ++line;

    if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) {
      // Add _H if necessary
      pa.IdRight = FTEDITOR_DL_BITMAP_SIZE_H;
      pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
      pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
      pa.ExpectedParameterCount = 2;
      lineEditor->insertLine(hline, pa);
      ++hline;
      ++line;
    }
  }
  return true;
}

bool DLUtil::addSetFont2Cmd(DlEditor *lineEditor, DlParsed &pa, uint32_t &font,
                            int address, int firstChar, int &line, int &hline) {
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_SETFONT2 & 0xFF;
  pa.Parameter[0].U = font;
  pa.Parameter[1].U = address;
  pa.Parameter[2].U = firstChar;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(hline, pa);
  ++hline;
  ++line;
  pa.IdLeft = 0;
  return true;
}

bool DLUtil::addSetFontCmd(DlEditor *lineEditor, DlParsed &pa, int address,
                           uint32_t &bitmapHandle, int &line, int &hline) {
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_SETFONT & 0xFF;
  pa.Parameter[0].U = bitmapHandle;
  pa.Parameter[1].U = address;
  pa.ExpectedParameterCount = 2;
  lineEditor->insertLine(hline, pa);
  ++hline;
  ++line;
  pa.IdLeft = 0;
  return true;
}

bool DLUtil::addBitmapCmds(DlEditor *lineEditor, DlParsed &pa,
                           ContentInfo *contentInfo, uint32_t &selection,
                           int &line, int32_t x, int32_t y) {
  DlParsed pav;
  pav.ValidId = true;
  pav.IdLeft = FTEDITOR_DL_VERTEX2F;  // FIXME: Drag-drop outside
                                      // 512px does not work??
  pav.IdRight = 0;
  pav.Parameter[0].I = x;
  pav.Parameter[1].I = y;
  // pav.Parameter[2].I = bitmapHandle;
  // pav.Parameter[3].I = 0;
  pav.ExpectedParameterCount = 2;
  pav.ExpectedStringParameter = false;
  pav.VarArgCount = 0;
  pa.IdRight = FTEDITOR_DL_BEGIN;
  pa.Parameter[0].U = selection;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  ++line;
  int lastVertex;

  if (contentInfo && contentInfo->Converter == ContentInfo::ImageCoprocessor) {
    addPalettedSrc(lineEditor, pa, contentInfo->bitmapAddress(), line);
    lineEditor->insertLine(line, pav);
    ++line;
    lastVertex = -1;
  } else if (selection == BITMAPS && contentInfo &&
             contentInfo->Converter == ContentInfo::Image &&
             contentInfo->requirePaletteAddress()) {
    if (contentInfo->ImageFormat == PALETTED8) {
      pa.IdRight = FTEDITOR_DL_SAVE_CONTEXT;
      pa.ExpectedParameterCount = 0;
      lineEditor->insertLine(line, pa);
      ++line;
      pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
      pa.Parameter[0].U = ONE;
      pa.Parameter[1].U = ZERO;
      pa.ExpectedParameterCount = 2;
      lineEditor->insertLine(line, pa);
      ++line;
      pa.IdRight = FTEDITOR_DL_COLOR_MASK;
      pa.Parameter[0].U = 0;
      pa.Parameter[1].U = 0;
      pa.Parameter[2].U = 0;
      pa.Parameter[3].U = 1;
      pa.ExpectedParameterCount = 4;
      lineEditor->insertLine(line, pa);
      ++line;
      addPalettedSrc(lineEditor, pa, contentInfo->MemoryAddress + 3, line);
      lineEditor->insertLine(line, pav);
      ++line;
      pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
      pa.Parameter[0].U = DST_ALPHA;
      pa.Parameter[1].U = ONE_MINUS_DST_ALPHA;
      pa.ExpectedParameterCount = 2;
      lineEditor->insertLine(line, pa);
      ++line;
      pa.IdRight = FTEDITOR_DL_COLOR_MASK;
      pa.Parameter[0].U = 1;
      pa.Parameter[1].U = 0;
      pa.Parameter[2].U = 0;
      pa.Parameter[3].U = 0;
      pa.ExpectedParameterCount = 4;
      lineEditor->insertLine(line, pa);
      ++line;
      addPalettedSrc(lineEditor, pa, contentInfo->MemoryAddress + 2, line);
      lineEditor->insertLine(line, pav);
      ++line;
      pa.IdRight = FTEDITOR_DL_COLOR_MASK;
      pa.Parameter[0].U = 0;
      pa.Parameter[1].U = 1;
      pa.Parameter[2].U = 0;
      pa.Parameter[3].U = 0;
      pa.ExpectedParameterCount = 4;
      lineEditor->insertLine(line, pa);
      ++line;
      addPalettedSrc(lineEditor, pa, contentInfo->MemoryAddress + 1, line);
      lineEditor->insertLine(line, pav);
      ++line;
      pa.IdRight = FTEDITOR_DL_COLOR_MASK;
      pa.Parameter[0].U = 0;
      pa.Parameter[1].U = 0;
      pa.Parameter[2].U = 1;
      pa.Parameter[3].U = 0;
      pa.ExpectedParameterCount = 4;
      lineEditor->insertLine(line, pa);
      ++line;
      addPalettedSrc(lineEditor, pa, contentInfo->MemoryAddress, line);
      lineEditor->insertLine(line, pav);
      ++line;
      pa.IdRight = FTEDITOR_DL_RESTORE_CONTEXT;
      pa.ExpectedParameterCount = 0;
      lineEditor->insertLine(line, pa);
      ++line;
      lastVertex = -2;
    } else {
      addPalettedSrc(lineEditor, pa, contentInfo->MemoryAddress, line);
      lineEditor->insertLine(line, pav);
      ++line;
      lastVertex = -1;
    }
  } else {
    lineEditor->insertLine(line, pav);
    ++line;
    lastVertex = -1;
  }
  pa.IdLeft = 0;
  pa.IdRight = FTEDITOR_DL_END;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  lineEditor->selectLine(line + lastVertex);
  return true;
};

bool DLUtil::addTextCmd(DlEditor *lineEditor, DlParsed &pa,
                        uint32_t &bitmapHandle, QString text, int &line,
                        int32_t x, int32_t y) {
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_TEXT & 0xFF;
  pa.Parameter[0].I = x;
  pa.Parameter[1].I = y;
  pa.Parameter[2].I = bitmapHandle;
  pa.Parameter[3].I = 0;
  pa.StringParameter = text.toStdString();
  pa.ExpectedStringParameter = true;
  pa.ExpectedParameterCount = 5;
  lineEditor->insertLine(line, pa);
  ++line;
  return true;
}

void DLUtil::postProcessEditor(DlEditor *editor) {
  DlParsed pa;
  for (int i = 0; i < editor->getLineCount(); ++i) {
    const DlParsed &parsed = editor->getLine(i);
    if (parsed.ValidId) {
      switch (parsed.IdLeft) {
        case FTEDITOR_CO_COMMAND:
          switch (parsed.IdRight | FTEDITOR_CO_COMMAND) {
            case CMD_BGCOLOR:
            case CMD_FGCOLOR:
            case CMD_GRADCOLOR: {
              DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa,
                              editor->getLineText(i), editor->isCoprocessor(),
                              true);
              if (pa.ExpectedParameterCount == 3)  // Old RGB, upgrade
              {
                uint32_t rgb = pa.Parameter[0].U << 16 |
                               pa.Parameter[1].U << 8 | pa.Parameter[2].U;
                pa.Parameter[0].U = rgb;
                pa.ExpectedParameterCount = 1;
                editor->replaceLine(i, pa);
              }
            } break;
            case CMD_GRADIENT: {
              DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa,
                              editor->getLineText(i), editor->isCoprocessor(),
                              true);
              if (pa.ExpectedParameterCount == 10)  // Old RGB, upgrade
              {
                uint32_t rgb0 = pa.Parameter[2].U << 16 |
                                pa.Parameter[3].U << 8 | pa.Parameter[4].U;
                pa.Parameter[2].U = rgb0;
                pa.Parameter[3].U = pa.Parameter[5].U;
                pa.Parameter[4].U = pa.Parameter[6].U;
                uint32_t rgb1 = pa.Parameter[7].U << 16 |
                                pa.Parameter[8].U << 8 | pa.Parameter[9].U;
                pa.Parameter[5].U = rgb1;
                pa.ExpectedParameterCount = 6;
                editor->replaceLine(i, pa);
              }
            } break;
          }
          break;
      }
    }
  }
}

bool DLUtil::addRunAnimFlash(DlEditor *lineEditor, DlParsed &pa, int &line,
                             int address, int objAdress, int32_t x, int32_t y) {
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_FLASHFAST & 0xFF;
  pa.ExpectedParameterCount = 0;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_FLASHSOURCE & 0xFF;
  pa.Parameter[0].U = address;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_MEMSET & 0xFF;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = 1;
  pa.Parameter[2].U = 0;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_ANIMSTART & 0xFF;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = objAdress;
  pa.Parameter[2].U = ANIM_LOOP;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_ANIMXY & 0xFF;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = x;
  pa.Parameter[2].U = y;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_RUNANIM & 0xFF;
  pa.Parameter[0].U = 0xFFFFFF;
  pa.Parameter[1].U = 0;
  pa.ExpectedParameterCount = 2;
  lineEditor->insertLine(line, pa);
  lineEditor->selectLine(line);
  return true;
}

bool DLUtil::addRunAnimRamG(DlEditor *lineEditor, DlParsed &pa, int &line,
                            int objAdress, int32_t x, int32_t y) {
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_ANIMSTARTRAM & 0xFF;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = objAdress;
  pa.Parameter[2].U = ANIM_LOOP;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_ANIMXY & 0xFF;
  pa.Parameter[0].U = 0;
  pa.Parameter[1].U = x;
  pa.Parameter[2].U = y;
  pa.ExpectedParameterCount = 3;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_ANIMDRAW & 0xFF;
  pa.Parameter[0].U = 0;
  pa.ExpectedParameterCount = 1;
  lineEditor->insertLine(line, pa);
  ++line;

  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_RUNANIM & 0xFF;
  pa.Parameter[0].U = 1;
  pa.Parameter[1].U = -1;
  pa.ExpectedParameterCount = 2;
  lineEditor->insertLine(line, pa);
  lineEditor->selectLine(line);
  return true;
}

}  // namespace FTEDITOR

/* end of file */
