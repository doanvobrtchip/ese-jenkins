/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"

// STL includes

namespace FTEDITOR {

///////////////////////////////////////////////////////////////////////

const FT8XXEMU_EmulatorMode g_DeviceToEnum[4] = {
	FT8XXEMU_EmulatorFT800,
	FT8XXEMU_EmulatorFT801,
	FT8XXEMU_EmulatorFT810,
	FT8XXEMU_EmulatorFT811
};
const int g_DeviceToIntf[256] = {
	FTEDITOR_FT800, FTEDITOR_FT801, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	FTEDITOR_FT810, FTEDITOR_FT811, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
const char *g_DeviceToString[4] = {
	"FT800",
	"FT801",
	"FT810",
	"FT811",
};

///////////////////////////////////////////////////////////////////////

const int g_ScreenWidthDefault[FTEDITOR_DEVICE_NB] = {
	480, // FT800
	480, // FT801
	480, // FT810
	480, // FT811
};

const int g_ScreenWidthMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
};

const int g_ScreenHeightDefault[FTEDITOR_DEVICE_NB] = {
	272, // FT800
	272, // FT801
	272, // FT810
	272, // FT811
};

const int g_ScreenHeightMaximum[FTEDITOR_DEVICE_NB] = {
	512, // FT800
	512, // FT801
	2048, // FT810
	2048, // FT811
};

const int g_DisplayListSize[FTEDITOR_DEVICE_NB] = {
	2048, // FT800
	2048, // FT801
	2048, // FT810
	2048, // FT811
};

const uint32_t g_AddressSpace[FTEDITOR_DEVICE_NB] = {
	4 * 1024 * 1024, // FT800
	4 * 1024 * 1024, // FT801
	4 * 1024 * 1024, // FT810
	4 * 1024 * 1024, // FT811
};

const uint32_t g_AddressMask[FTEDITOR_DEVICE_NB] = {
	0xFFFFF, // FT800
	0xFFFFF, // FT801
	0x3FFFFF, // FT810
	0x3FFFFF, // FT811
};

///////////////////////////////////////////////////////////////////////

extern const int32_t g_AddrVC1[FTEDITOR_RAM_NB];
extern const int32_t g_AddrVC2[FTEDITOR_RAM_NB];
const int32_t *g_Addr[FTEDITOR_DEVICE_NB] = {
	g_AddrVC1, // FT800
	g_AddrVC1, // FT801
	g_AddrVC2, // FT810
	g_AddrVC2, // FT811
};

extern const char *g_AddrToStringVC1[FTEDITOR_RAM_NB];
extern const char *g_AddrToStringVC2[FTEDITOR_RAM_NB];
const char **g_AddrToString[FTEDITOR_DEVICE_NB] = {
	g_AddrToStringVC1, // FT800
	g_AddrToStringVC1, // FT801
	g_AddrToStringVC2, // FT810
	g_AddrToStringVC2, // FT811
};

///////////////////////////////////////////////////////////////////////

extern const int32_t g_RegVC1[FTEDITOR_REG_NB];
extern const int32_t g_RegVC2[FTEDITOR_REG_NB];
const int32_t *g_Reg[FTEDITOR_DEVICE_NB] = {
	g_RegVC1, // FT800
	g_RegVC1, // FT801
	g_RegVC2, // FT810
	g_RegVC2, // FT811
};

extern const char *g_RegToStringFT800[FTEDITOR_REG_NB];
extern const char *g_RegToStringFT801[FTEDITOR_REG_NB];
extern const char *g_RegToStringVC2[FTEDITOR_REG_NB];
const char **g_RegToString[FTEDITOR_DEVICE_NB] = {
	g_RegToStringFT800, // FT800
	g_RegToStringFT801, // FT801
	g_RegToStringVC2, // FT810
	g_RegToStringVC2, // FT811
};

///////////////////////////////////////////////////////////////////////

extern const char *g_BitmapFormatToStringVC1[];
extern const char *g_BitmapFormatToStringVC2[];
const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToStringVC1, // FT800
	g_BitmapFormatToStringVC1, // FT801
	g_BitmapFormatToStringVC2, // FT810
	g_BitmapFormatToStringVC2, // FT811
};
int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB] = {
	12,
	12,
	33,
	33,
};

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
