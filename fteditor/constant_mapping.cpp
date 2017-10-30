/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"
#include "constant_mapping_vc1.h"
#include "constant_mapping_vc2.h"
#include "constant_mapping_vc3.h"

// STL includes

namespace FTEDITOR {

///////////////////////////////////////////////////////////////////////

const int g_IntEmpty[1] = { 0 };

///////////////////////////////////////////////////////////////////////

const BT8XXEMU_EmulatorMode g_DeviceToEnum[FTEDITOR_DEVICE_NB] = {
	BT8XXEMU_EmulatorFT800,
	BT8XXEMU_EmulatorFT801,
	BT8XXEMU_EmulatorFT810,
	BT8XXEMU_EmulatorFT811,
	BT8XXEMU_EmulatorFT812,
	BT8XXEMU_EmulatorFT813,
	BT8XXEMU_EmulatorBT815
};

const int g_DeviceToIntf[256] = {
	FTEDITOR_FT800, FTEDITOR_FT801, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	FTEDITOR_FT810, FTEDITOR_FT811, FTEDITOR_FT812, FTEDITOR_FT813, 0, FTEDITOR_BT815, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char *g_DeviceToString[FTEDITOR_DEVICE_NB] = {
	"FT800",
	"FT801",
	"FT810",
	"FT811",
	"FT812",
	"FT813",
	"BT815"
};

///////////////////////////////////////////////////////////////////////

int g_CurrentDevice = FTEDITOR_DEFAULT_DEVICE;

///////////////////////////////////////////////////////////////////////

const int g_ScreenWidthDefault[FTEDITOR_DEVICE_NB] = {
	480, // FT800
	480, // FT801
	480, // FT810
	480, // FT811
	480, // FT812
	480, // FT813
	480, // BT815
};

const int g_ScreenWidthMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT815
};

const int g_ScreenHeightDefault[FTEDITOR_DEVICE_NB] = {
	272, // FT800
	272, // FT801
	272, // FT810
	272, // FT811
	272, // FT812
	272, // FT813
	272, // BT815
};

const int g_ScreenHeightMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT815
};

const int g_DisplayListSize[FTEDITOR_DEVICE_NB] = {
	2048, // FT800
	2048, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT815
};

// NOTE: This refers to address space in emulator. Used to avoid buffer overruns
const uint32_t g_AddressSpace[FTEDITOR_DEVICE_NB] = {
	4 * 1024 * 1024, // FT800
	4 * 1024 * 1024, // FT801
	4 * 1024 * 1024, // FT810
	4 * 1024 * 1024, // FT811
	4 * 1024 * 1024, // FT812
	4 * 1024 * 1024, // FT813
	4 * 1024 * 1024, // BT815
};

const uint32_t g_AddressMask[FTEDITOR_DEVICE_NB] = {
	0xFFFFF, // FT800
	0xFFFFF, // FT801
	0x3FFFFF, // FT810
	0x3FFFFF, // FT811
	0x3FFFFF, // FT812
	0x3FFFFF, // FT813
	0x3FFFFF, // BT815 // Mask for RAM address only
};

///////////////////////////////////////////////////////////////////////

const int32_t *g_Addr[FTEDITOR_DEVICE_NB] = {
	g_AddrVC1, // FT800
	g_AddrVC1, // FT801
	g_AddrVC2, // FT810
	g_AddrVC2, // FT811
	g_AddrVC2, // FT812
	g_AddrVC2, // FT813
	g_AddrVC3, // BT815
};

const char **g_AddrToString[FTEDITOR_DEVICE_NB] = {
	g_AddrToStringVC1, // FT800
	g_AddrToStringVC1, // FT801
	g_AddrToStringVC2, // FT810
	g_AddrToStringVC2, // FT811
	g_AddrToStringVC2, // FT812
	g_AddrToStringVC2, // FT813
	g_AddrToStringVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const int32_t *g_Reg[FTEDITOR_DEVICE_NB] = {
	g_RegVC1, // FT800
	g_RegVC1, // FT801
	g_RegVC2, // FT810
	g_RegVC2, // FT811
	g_RegVC2, // FT812
	g_RegVC2, // FT813
	g_RegVC3, // BT815
};

const char **g_RegToString[FTEDITOR_DEVICE_NB] = {
	g_RegToStringFT800, // FT800
	g_RegToStringFT801, // FT801
	g_RegToStringVC2, // FT810
	g_RegToStringVC2, // FT811
	g_RegToStringVC2, // FT812
	g_RegToStringVC2, // FT813
	g_RegToStringVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToStringVC1, // FT800
	g_BitmapFormatToStringVC1, // FT801
	g_BitmapFormatToStringVC2, // FT810
	g_BitmapFormatToStringVC2, // FT811
	g_BitmapFormatToStringVC2, // FT812
	g_BitmapFormatToStringVC2, // FT813
	g_BitmapFormatToStringVC3, // BT815
};
const int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3,
};

///////////////////////////////////////////////////////////////////////

const int g_BitmapFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3, // BT815
};

const int *g_BitmapFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatFromIntfVC1, // FT800
	g_BitmapFormatFromIntfVC1, // FT801
	g_BitmapFormatFromIntfVC2, // FT810
	g_BitmapFormatFromIntfVC2, // FT811
	g_BitmapFormatFromIntfVC2, // FT812
	g_BitmapFormatFromIntfVC2, // FT813
	g_BitmapFormatFromIntfVC3, // BT815
};

const int *g_BitmapFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToIntfVC1, // FT800
	g_BitmapFormatToIntfVC1, // FT801
	g_BitmapFormatToIntfVC2, // FT810
	g_BitmapFormatToIntfVC2, // FT811
	g_BitmapFormatToIntfVC2, // FT812
	g_BitmapFormatToIntfVC2, // FT813
	g_BitmapFormatToIntfVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const int g_ExtFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	0, // FT800
	0, // FT801
	0, // FT810
	0, // FT811
	0, // FT812
	0, // FT813
	FTEDITOR_EXT_FORMAT_INTF_NB_VC3, // BT815
};

const int *g_ExtFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	NULL, // FT800
	NULL, // FT801
	NULL, // FT810
	NULL, // FT811
	NULL, // FT812
	NULL, // FT813
	g_ExtFormatFromIntfVC3, // BT815
};

const int *g_ExtFormatToIntf[FTEDITOR_DEVICE_NB] = {
	NULL, // FT800
	NULL, // FT801
	NULL, // FT810
	NULL, // FT811
	NULL, // FT812
	NULL, // FT813
	g_ExtFormatToIntfVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const int g_SnapshotFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	0,
	0,
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2,
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2,
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2,
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2,
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3, // BT815
};

const int *g_SnapshotFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_SnapshotFormatFromIntfVC2, // FT810
	g_SnapshotFormatFromIntfVC2, // FT811
	g_SnapshotFormatFromIntfVC2, // FT812
	g_SnapshotFormatFromIntfVC2, // FT813
	g_SnapshotFormatFromIntfVC3, // BT815
};

const int *g_SnapshotFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_SnapshotFormatToIntfVC2, // FT810
	g_SnapshotFormatToIntfVC2, // FT811
	g_SnapshotFormatToIntfVC2, // FT812
	g_SnapshotFormatToIntfVC2, // FT813
	g_SnapshotFormatToIntfVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const int g_ImageFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3, // BT815
};

const int *g_ImageFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_ImageFormatFromIntfVC1, // FT800
	g_ImageFormatFromIntfVC1, // FT801
	g_ImageFormatFromIntfVC2, // FT810
	g_ImageFormatFromIntfVC2, // FT811
	g_ImageFormatFromIntfVC2, // FT812
	g_ImageFormatFromIntfVC2, // FT813
	g_ImageFormatFromIntfVC3, // BT815
};

const int *g_ImageFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_ImageFormatToIntfVC1, // FT800
	g_ImageFormatToIntfVC1, // FT801
	g_ImageFormatToIntfVC2, // FT810
	g_ImageFormatToIntfVC2, // FT811
	g_ImageFormatToIntfVC2, // FT812
	g_ImageFormatToIntfVC2, // FT813
	g_ImageFormatToIntfVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

const int g_FontFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_FONT_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_FONT_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_FONT_FORMAT_INTF_NB_VC3, // BT815
};

const int *g_FontFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_FontFormatFromIntfVC1, // FT800
	g_FontFormatFromIntfVC1, // FT801
	g_FontFormatFromIntfVC2, // FT810
	g_FontFormatFromIntfVC2, // FT811
	g_FontFormatFromIntfVC2, // FT812
	g_FontFormatFromIntfVC2, // FT813
	g_FontFormatFromIntfVC3, // BT815
};

const int *g_FontFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_FontFormatToIntfVC1, // FT800
	g_FontFormatToIntfVC1, // FT801
	g_FontFormatToIntfVC2, // FT810
	g_FontFormatToIntfVC2, // FT811
	g_FontFormatToIntfVC2, // FT812
	g_FontFormatToIntfVC2, // FT813
	g_FontFormatToIntfVC3, // BT815
};

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
