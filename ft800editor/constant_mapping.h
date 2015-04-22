/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FT800EMUQT_CONSTANT_MAPPING_H
#define FT800EMUQT_CONSTANT_MAPPING_H

// Emulator includes
#include <ft8xxemu.h>

// STL includes

namespace FT800EMUQT {

// ToString functions will return "" on invalid input. Simply check (result[0] == '\0') to find out if the result is invalid

// Interface indices for Device
#define FTEDITOR_FT800 0
#define FTEDITOR_FT801 1
#define FTEDITOR_FT810 2
#define FTEDITOR_FT811 3
#define FTEDITOR_DEVICE_NB 4
extern FT8XXEMU_EmulatorMode g_DeviceToEnum[FTEDITOR_DEVICE_NB];
inline FT8XXEMU_EmulatorMode deviceToEnum(int deviceIntf) { return g_DeviceToEnum[deviceIntf % FTEDITOR_DEVICE_NB]; }
extern int g_DeviceToIntf[256];
inline int deviceToIntf(FT8XXEMU_EmulatorMode deviceEnum) { return g_DeviceToIntf[deviceEnum & 0xFF]; }
extern const char *g_DeviceToString[FTEDITOR_DEVICE_NB];
inline const char *deviceToString(int deviceIntf) { return deviceIntf < FTEDITOR_DEVICE_NB ? g_DeviceToString[deviceIntf] : ""; }

// Tempory for mapping conversion
#ifdef FT810EMU_MODE
#define FTEDITOR_CURRENT_DEVICE FTEDITOR_FT811
#else
#define FTEDITOR_CURRENT_DEVICE FTEDITOR_FT801
#endif

// Mappings for bitmap formats
#define FTEDITOR_ARGB8 0x20 // Additional bitmap format
extern const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB];
extern int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB];
inline const char *bitmapFormatToString(int deviceIntf, int bitmapFormatEnum) { return bitmapFormatEnum < g_BitmapFormatEnumNb[deviceIntf] ? g_BitmapFormatToString[deviceIntf][bitmapFormatEnum] : ""; }

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONSTANT_MAPPING_H */

/* end of file */
