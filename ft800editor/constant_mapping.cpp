/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"

// STL includes

namespace FT800EMUQT {

///////////////////////////////////////////////////////////////////////

FT8XXEMU_EmulatorMode g_DeviceToEnum[4] = {
	FT8XXEMU_EmulatorFT800,
	FT8XXEMU_EmulatorFT801,
	FT8XXEMU_EmulatorFT810,
	FT8XXEMU_EmulatorFT811
};
int g_DeviceToIntf[256] = {
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

extern const int32_t g_AddrVC1[FTEDITOR_RAM_NB];
extern const int32_t g_AddrVC2[FTEDITOR_RAM_NB];
const int32_t *g_Addr[FTEDITOR_RAM_NB] = {
	g_AddrVC1, // FT800
	g_AddrVC1, // FT801
	g_AddrVC2, // FT810
	g_AddrVC2, // FT811
};

const char *g_AddrToStringA[FTEDITOR_RAM_NB] = {
	"RAM_G",
	"RAM_DL",
	"RAM_CMD",
	"RAM_REG"
};

const char **g_AddrToString[FTEDITOR_DEVICE_NB] = {
	g_AddrToStringA, // FT800
	g_AddrToStringA, // FT801
	g_AddrToStringA, // FT810
	g_AddrToStringA, // FT811
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

} /* namespace FT800EMUQT */

/* end of file */
