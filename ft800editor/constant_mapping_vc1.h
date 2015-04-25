/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_CONSTANT_MAPPING_VC1_H
#define FTEDITOR_CONSTANT_MAPPING_VC1_H

namespace FTEDITOR {

extern const int32_t g_AddrVC1[FTEDITOR_RAM_NB];
extern const char *g_AddrToStringVC1[FTEDITOR_RAM_NB];

extern const int32_t g_RegVC1[FTEDITOR_REG_NB];
extern const char *g_RegToStringFT800[FTEDITOR_REG_NB];
extern const char *g_RegToStringFT801[FTEDITOR_REG_NB];

#define FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1 12
#define FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1 12
#define FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1 9
#define FTEDITOR_FONT_FORMAT_INTF_NB_VC1 3
extern const char *g_BitmapFormatToStringVC1[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1];
extern const int g_BitmapFormatFromIntfVC1[FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1];
extern const int g_BitmapFormatToIntfVC1[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1];
extern const int g_ImageFormatFromIntfVC1[FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1];
extern const int g_ImageFormatToIntfVC1[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1];
extern const int g_FontFormatFromIntfVC1[FTEDITOR_FONT_FORMAT_INTF_NB_VC1];
extern const int g_FontFormatToIntfVC1[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1];

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_MAPPING_VC1_H */

/* end of file */
