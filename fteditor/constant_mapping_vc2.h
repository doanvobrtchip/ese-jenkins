/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_CONSTANT_MAPPING_VC2_H
#define FTEDITOR_CONSTANT_MAPPING_VC2_H

namespace FTEDITOR {

extern const int32_t g_AddrVC2[FTEDITOR_RAM_NB];
extern const char *g_AddrToStringVC2[FTEDITOR_RAM_NB];

extern const int32_t g_RegVC2[FTEDITOR_REG_NB];
extern const char *g_RegToStringVC2[FTEDITOR_REG_NB];

#define FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2 33
#define FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2 15
#define FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2 3
#define FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2 12
#define FTEDITOR_FONT_FORMAT_INTF_NB_VC2 4
extern const char *g_BitmapFormatToStringVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2];
extern const int g_BitmapFormatFromIntfVC2[FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2];
extern const int g_BitmapFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2];
extern const int g_SnapshotFormatFromIntfVC2[FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2];
extern const int g_SnapshotFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2];
extern const int g_ImageFormatFromIntfVC2[FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2];
extern const int g_ImageFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2];
extern const int g_FontFormatFromIntfVC2[FTEDITOR_FONT_FORMAT_INTF_NB_VC2];
extern const int g_FontFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2];

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_MAPPING_VC2_H */

/* end of file */
