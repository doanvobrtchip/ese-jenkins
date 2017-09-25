/*
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FTEDITOR_CONSTANT_MAPPING_VC3_H
#define FTEDITOR_CONSTANT_MAPPING_VC3_H

namespace FTEDITOR {

extern const int32_t g_AddrVC3[FTEDITOR_RAM_NB];
extern const char *g_AddrToStringVC3[FTEDITOR_RAM_NB];

extern const int32_t g_RegVC3[FTEDITOR_REG_NB];
extern const char *g_RegToStringVC3[FTEDITOR_REG_NB];

#define FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3 33
#define FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3 15
#define FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3 3
#define FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3 12
#define FTEDITOR_FONT_FORMAT_INTF_NB_VC3 4
extern const char *g_BitmapFormatToStringVC3[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3];
extern const int g_BitmapFormatFromIntfVC3[FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3];
extern const int g_BitmapFormatToIntfVC3[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3];
extern const int g_SnapshotFormatFromIntfVC3[FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3];
extern const int g_SnapshotFormatToIntfVC3[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3];
extern const int g_ImageFormatFromIntfVC3[FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3];
extern const int g_ImageFormatToIntfVC3[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3];
extern const int g_FontFormatFromIntfVC3[FTEDITOR_FONT_FORMAT_INTF_NB_VC3];
extern const int g_FontFormatToIntfVC3[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3];

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_MAPPING_VC3_H */

/* end of file */
