/*
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"
#include "constant_mapping_vc1.h"
#include "constant_mapping_vc2.h"
#include "constant_mapping_vc3.h"
#include "constant_mapping_vc4.h"

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
	BT8XXEMU_EmulatorBT880,
	BT8XXEMU_EmulatorBT881,
	BT8XXEMU_EmulatorBT882,
	BT8XXEMU_EmulatorBT883,
	BT8XXEMU_EmulatorBT815,
	BT8XXEMU_EmulatorBT816,
	BT8XXEMU_EmulatorBT817,
	BT8XXEMU_EmulatorBT818,
};

const int g_DeviceToIntf[256] = {
	FTEDITOR_FT800, FTEDITOR_FT801, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	FTEDITOR_FT810, FTEDITOR_FT811, FTEDITOR_FT812, FTEDITOR_FT813, 0, FTEDITOR_BT815, FTEDITOR_BT816, FTEDITOR_BT817, FTEDITOR_BT818, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	FTEDITOR_BT880, FTEDITOR_BT881, FTEDITOR_BT882, FTEDITOR_BT883, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
	"BT880",
	"BT881",
	"BT882",
	"BT883",
	"BT815",
	"BT816",
	"BT817",
	"BT818",
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
	480, // BT880
	480, // BT881
	480, // BT882
	480, // BT883
	800, // BT815
	800, // BT816
	800, // BT817
	800, // BT818
};

const int g_ScreenWidthMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT880
	2048, // BT881
	2048, // BT882
	2048, // BT883
	2048, // BT815
	2048, // BT816
	2048, // BT817
	2048, // BT818
};

const int g_ScreenHeightDefault[FTEDITOR_DEVICE_NB] = {
	272, // FT800
	272, // FT801
	272, // FT810
	272, // FT811
	272, // FT812
	272, // FT813
	272, // BT880
	272, // BT881
	272, // BT882
	272, // BT883
	480, // BT815
	480, // BT816
	480, // BT817
	480, // BT818
};

const int g_ScreenHeightMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT880
	2048, // BT881
	2048, // BT882
	2048, // BT883
	2048, // BT815
	2048, // BT816
	2048, // BT817
	2048, // BT818
};

const int g_DisplayListSize[FTEDITOR_DEVICE_NB] = {
	2048, // FT800
	2048, // FT801
	2048, // FT810
	2048, // FT811
	2048, // FT812
	2048, // FT813
	2048, // BT880
	2048, // BT881
	2048, // BT882
	2048, // BT883
	2048, // BT815
	2048, // BT816
	2048, // BT817
	2048, // BT818
};

// NOTE: This refers to address space in emulator. Used to avoid buffer overruns
const uint32_t g_AddressSpace[FTEDITOR_DEVICE_NB] = {
	4 * 1024 * 1024, // FT800
	4 * 1024 * 1024, // FT801
	4 * 1024 * 1024, // FT810
	4 * 1024 * 1024, // FT811
	4 * 1024 * 1024, // FT812
	4 * 1024 * 1024, // FT813
	4 * 1024 * 1024, // BT880
	4 * 1024 * 1024, // BT881
	4 * 1024 * 1024, // BT882
	4 * 1024 * 1024, // BT883
	4 * 1024 * 1024, // BT815
	4 * 1024 * 1024, // BT816
	4 * 1024 * 1024, // BT817
	4 * 1024 * 1024, // BT818
};

const uint32_t g_AddressMask[FTEDITOR_DEVICE_NB] = {
	0xFFFFF, // FT800
	0xFFFFF, // FT801
	0x3FFFFF, // FT810
	0x3FFFFF, // FT811
	0x3FFFFF, // FT812
	0x3FFFFF, // FT813
	0xFFFFF, // BT880
	0xFFFFF, // BT881
	0xFFFFF, // BT882
	0xFFFFF, // BT883
	0x7FFFFF, // BT815 // Mask for RAM address only
	0x7FFFFF, // BT816
	0x7FFFFF, // BT817
	0x7FFFFF, // BT818
};

///////////////////////////////////////////////////////////////////////

const int32_t *g_Addr[FTEDITOR_DEVICE_NB] = {
	g_AddrVC1, // FT800
	g_AddrVC1, // FT801
	g_AddrVC2, // FT810
	g_AddrVC2, // FT811
	g_AddrVC2, // FT812
	g_AddrVC2, // FT813
	g_AddrVC2_880, // BT880
	g_AddrVC2_880, // BT881
	g_AddrVC2_880, // BT882
	g_AddrVC2_880, // BT883
	g_AddrVC3, // BT815
	g_AddrVC3, // BT816
	g_AddrVC3, // BT817
	g_AddrVC3, // BT818
};

const char **g_AddrToString[FTEDITOR_DEVICE_NB] = {
	g_AddrToStringVC1, // FT800
	g_AddrToStringVC1, // FT801
	g_AddrToStringVC2, // FT810
	g_AddrToStringVC2, // FT811
	g_AddrToStringVC2, // FT812
	g_AddrToStringVC2, // FT813
	g_AddrToStringVC2, // BT880
	g_AddrToStringVC2, // BT881
	g_AddrToStringVC2, // BT882
	g_AddrToStringVC2, // BT883
	g_AddrToStringVC3, // BT815
	g_AddrToStringVC3, // BT816
	g_AddrToStringVC3, // BT817
	g_AddrToStringVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int32_t *g_Reg[FTEDITOR_DEVICE_NB] = {
	g_RegVC1, // FT800
	g_RegVC1, // FT801
	g_RegVC2, // FT810
	g_RegVC2, // FT811
	g_RegVC2, // FT812
	g_RegVC2, // FT813
	g_RegVC2, // BT880
	g_RegVC2, // BT881
	g_RegVC2, // BT882
	g_RegVC2, // BT883
	g_RegVC3, // BT815
	g_RegVC3, // BT816
	g_RegVC4, // BT817
	g_RegVC4, // BT818
};

const char **g_RegToString[FTEDITOR_DEVICE_NB] = {
	g_RegToStringFT800, // FT800
	g_RegToStringFT801, // FT801
	g_RegToStringVC2, // FT810
	g_RegToStringVC2, // FT811
	g_RegToStringVC2, // FT812
	g_RegToStringVC2, // FT813
	g_RegToStringVC2, // BT880
	g_RegToStringVC2, // BT881
	g_RegToStringVC2, // BT882
	g_RegToStringVC2, // BT883
	g_RegToStringVC3, // BT815
	g_RegToStringVC3, // BT816
	g_RegToStringVC4, // BT817
	g_RegToStringVC4, // BT818
};

///////////////////////////////////////////////////////////////////////

const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToStringVC1, // FT800
	g_BitmapFormatToStringVC1, // FT801
	g_BitmapFormatToStringVC2, // FT810
	g_BitmapFormatToStringVC2, // FT811
	g_BitmapFormatToStringVC2, // FT812
	g_BitmapFormatToStringVC2, // FT813
	g_BitmapFormatToStringVC2, // BT880
	g_BitmapFormatToStringVC2, // BT881
	g_BitmapFormatToStringVC2, // BT882
	g_BitmapFormatToStringVC2, // BT883
	g_BitmapFormatToStringVC3, // BT815
	g_BitmapFormatToStringVC3, // BT816
	g_BitmapFormatToStringVC3, // BT817
	g_BitmapFormatToStringVC3, // BT818
};
const int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1, // FT800
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC1,	// FT801
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// FT810
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// FT811
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// FT812
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// FT813
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// BT880
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// BT881
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// BT882
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2,	// BT883
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3, // BT815
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3, // BT816
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3, // BT817
	FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int g_BitmapFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // BT880
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // BT881
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // BT882
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2, // BT883
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3, // BT815
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3, // BT816
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3, // BT817
	FTEDITOR_BITMAP_FORMAT_INTF_NB_VC3, // BT818
};

const int *g_BitmapFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatFromIntfVC1, // FT800
	g_BitmapFormatFromIntfVC1, // FT801
	g_BitmapFormatFromIntfVC2, // FT810
	g_BitmapFormatFromIntfVC2, // FT811
	g_BitmapFormatFromIntfVC2, // FT812
	g_BitmapFormatFromIntfVC2, // FT813
	g_BitmapFormatFromIntfVC2, // BT880
	g_BitmapFormatFromIntfVC2, // BT881
	g_BitmapFormatFromIntfVC2, // BT882
	g_BitmapFormatFromIntfVC2, // BT883
	g_BitmapFormatFromIntfVC3, // BT815
	g_BitmapFormatFromIntfVC3, // BT816
	g_BitmapFormatFromIntfVC3, // BT817
	g_BitmapFormatFromIntfVC3, // BT818
};

const int *g_BitmapFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToIntfVC1, // FT800
	g_BitmapFormatToIntfVC1, // FT801
	g_BitmapFormatToIntfVC2, // FT810
	g_BitmapFormatToIntfVC2, // FT811
	g_BitmapFormatToIntfVC2, // FT812
	g_BitmapFormatToIntfVC2, // FT813
	g_BitmapFormatToIntfVC2, // BT880
	g_BitmapFormatToIntfVC2, // BT881
	g_BitmapFormatToIntfVC2, // BT882
	g_BitmapFormatToIntfVC2, // BT883
	g_BitmapFormatToIntfVC3, // BT815
	g_BitmapFormatToIntfVC3, // BT816
	g_BitmapFormatToIntfVC3, // BT817
	g_BitmapFormatToIntfVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int g_ExtFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	0, // FT800
	0, // FT801
	0, // FT810
	0, // FT811
	0, // FT812
	0, // FT813
	0, // BT880
	0, // BT881
	0, // BT882
	0, // BT883
	FTEDITOR_EXT_FORMAT_INTF_NB_VC3, // BT815
	FTEDITOR_EXT_FORMAT_INTF_NB_VC3, // BT816
	FTEDITOR_EXT_FORMAT_INTF_NB_VC3, // BT817
	FTEDITOR_EXT_FORMAT_INTF_NB_VC3, // BT818
};

const int *g_ExtFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_IntEmpty, // FT810
	g_IntEmpty, // FT811
	g_IntEmpty, // FT812
	g_IntEmpty, // FT813
	g_IntEmpty, // BT880
	g_IntEmpty, // BT881
	g_IntEmpty, // BT882
	g_IntEmpty, // BT883
	g_ExtFormatFromIntfVC3, // BT815
	g_ExtFormatFromIntfVC3, // BT816
	g_ExtFormatFromIntfVC3, // BT817
	g_ExtFormatFromIntfVC3, // BT818
};

const int *g_ExtFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_IntEmpty, // FT810
	g_IntEmpty, // FT811
	g_IntEmpty, // FT812
	g_IntEmpty, // FT813
	g_IntEmpty, // BT880
	g_IntEmpty, // BT881
	g_IntEmpty, // BT882
	g_IntEmpty, // BT883
	g_ExtFormatToIntfVC3, // BT815
	g_ExtFormatToIntfVC3, // BT816
	g_ExtFormatToIntfVC3, // BT817
	g_ExtFormatToIntfVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int g_SnapshotFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	0, // FT800
	0, // FT801
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // BT880
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // BT881
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // BT882
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2, // BT883
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3, // BT815
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3, // BT816
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3, // BT817
	FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC3, // BT818
};

const int *g_SnapshotFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_SnapshotFormatFromIntfVC2, // FT810
	g_SnapshotFormatFromIntfVC2, // FT811
	g_SnapshotFormatFromIntfVC2, // FT812
	g_SnapshotFormatFromIntfVC2, // FT813
	g_SnapshotFormatFromIntfVC2, // BT880
	g_SnapshotFormatFromIntfVC2, // BT881
	g_SnapshotFormatFromIntfVC2, // BT882
	g_SnapshotFormatFromIntfVC2, // BT883
	g_SnapshotFormatFromIntfVC3, // BT815
	g_SnapshotFormatFromIntfVC3, // BT816
	g_SnapshotFormatFromIntfVC3, // BT817
	g_SnapshotFormatFromIntfVC3, // BT818
};

const int *g_SnapshotFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_IntEmpty, // FT800
	g_IntEmpty, // FT801
	g_SnapshotFormatToIntfVC2, // FT810
	g_SnapshotFormatToIntfVC2, // FT811
	g_SnapshotFormatToIntfVC2, // FT812
	g_SnapshotFormatToIntfVC2, // FT813
	g_SnapshotFormatToIntfVC2, // BT880
	g_SnapshotFormatToIntfVC2, // BT881
	g_SnapshotFormatToIntfVC2, // BT882
	g_SnapshotFormatToIntfVC2, // BT883
	g_SnapshotFormatToIntfVC3, // BT815
	g_SnapshotFormatToIntfVC3, // BT816
	g_SnapshotFormatToIntfVC3, // BT817
	g_SnapshotFormatToIntfVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int g_ImageFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // BT880
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // BT881
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // BT882
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2, // BT883
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3, // BT815
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3, // BT816
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3, // BT817
	FTEDITOR_IMAGE_FORMAT_INTF_NB_VC3, // BT818
};

const int *g_ImageFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_ImageFormatFromIntfVC1, // FT800
	g_ImageFormatFromIntfVC1, // FT801
	g_ImageFormatFromIntfVC2, // FT810
	g_ImageFormatFromIntfVC2, // FT811
	g_ImageFormatFromIntfVC2, // FT812
	g_ImageFormatFromIntfVC2, // FT813
	g_ImageFormatFromIntfVC2, // BT880
	g_ImageFormatFromIntfVC2, // BT881
	g_ImageFormatFromIntfVC2, // BT882
	g_ImageFormatFromIntfVC2, // BT883
	g_ImageFormatFromIntfVC3, // BT815
	g_ImageFormatFromIntfVC3, // BT816
	g_ImageFormatFromIntfVC3, // BT817
	g_ImageFormatFromIntfVC3, // BT818
};

const int *g_ImageFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_ImageFormatToIntfVC1, // FT800
	g_ImageFormatToIntfVC1, // FT801
	g_ImageFormatToIntfVC2, // FT810
	g_ImageFormatToIntfVC2, // FT811
	g_ImageFormatToIntfVC2, // FT812
	g_ImageFormatToIntfVC2, // FT813
	g_ImageFormatToIntfVC2, // BT880
	g_ImageFormatToIntfVC2, // BT881
	g_ImageFormatToIntfVC2, // BT882
	g_ImageFormatToIntfVC2, // BT883
	g_ImageFormatToIntfVC3, // BT815
	g_ImageFormatToIntfVC3, // BT816
	g_ImageFormatToIntfVC3, // BT817
	g_ImageFormatToIntfVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

const int g_FontFormatIntfNb[FTEDITOR_DEVICE_NB] = {
	FTEDITOR_FONT_FORMAT_INTF_NB_VC1, // FT800
	FTEDITOR_FONT_FORMAT_INTF_NB_VC1, // FT801
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT810
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT811
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT812
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // FT813
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // BT880
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // BT881
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // BT882
	FTEDITOR_FONT_FORMAT_INTF_NB_VC2, // BT883
	FTEDITOR_FONT_FORMAT_INTF_NB_VC3, // BT815
	FTEDITOR_FONT_FORMAT_INTF_NB_VC3, // BT816
	FTEDITOR_FONT_FORMAT_INTF_NB_VC3, // BT817
	FTEDITOR_FONT_FORMAT_INTF_NB_VC3, // BT818
};

const int *g_FontFormatFromIntf[FTEDITOR_DEVICE_NB] = {
	g_FontFormatFromIntfVC1, // FT800
	g_FontFormatFromIntfVC1, // FT801
	g_FontFormatFromIntfVC2, // FT810
	g_FontFormatFromIntfVC2, // FT811
	g_FontFormatFromIntfVC2, // FT812
	g_FontFormatFromIntfVC2, // FT813
	g_FontFormatFromIntfVC2, // BT880
	g_FontFormatFromIntfVC2, // BT881
	g_FontFormatFromIntfVC2, // BT882
	g_FontFormatFromIntfVC2, // BT883
	g_FontFormatFromIntfVC3, // BT815
	g_FontFormatFromIntfVC3, // BT816
	g_FontFormatFromIntfVC3, // BT817
	g_FontFormatFromIntfVC3, // BT818
};

const int *g_FontFormatToIntf[FTEDITOR_DEVICE_NB] = {
	g_FontFormatToIntfVC1, // FT800
	g_FontFormatToIntfVC1, // FT801
	g_FontFormatToIntfVC2, // FT810
	g_FontFormatToIntfVC2, // FT811
	g_FontFormatToIntfVC2, // FT812
	g_FontFormatToIntfVC2, // FT813
	g_FontFormatToIntfVC2, // BT880
	g_FontFormatToIntfVC2, // BT881
	g_FontFormatToIntfVC2, // BT882
	g_FontFormatToIntfVC2, // BT883
	g_FontFormatToIntfVC3, // BT815
	g_FontFormatToIntfVC3, // BT816
	g_FontFormatToIntfVC3, // BT817
	g_FontFormatToIntfVC3, // BT818
};

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
