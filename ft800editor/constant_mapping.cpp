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
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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

extern const char *g_BitmapFormatToStringVC1[];
extern const char *g_BitmapFormatToStringVC2[];
const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB] = {
	g_BitmapFormatToStringVC1,
	g_BitmapFormatToStringVC1,
	g_BitmapFormatToStringVC2,
	g_BitmapFormatToStringVC2,
};
int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB] {
	8,
	8,
	33,
	33,
};

///////////////////////////////////////////////////////////////////////

} /* namespace FT800EMUQT */

/* end of file */
