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

#define countof(arr) sizeof(arr) / sizeof(arr[0])

// Interface indices for Device
#define FTEDITOR_FT800 0
#define FTEDITOR_FT801 1
#define FTEDITOR_FT810 2
#define FTEDITOR_FT811 3
extern FT8XXEMU_EmulatorMode g_DeviceToEnum[4];
inline FT8XXEMU_EmulatorMode deviceToEnum(int deviceIntf) { return g_DeviceToEnum[deviceIntf % countof(g_DeviceToEnum)]; }
extern int g_DeviceToIntf[256];
inline int deviceToIntf(FT8XXEMU_EmulatorMode deviceEnum) { return g_DeviceToIntf[deviceEnum & 0xFF]; }
extern const char *g_DeviceToString[4];
inline const char *deviceToString(FT8XXEMU_EmulatorMode deviceEnum) { return g_DeviceToString[deviceToIntf(deviceEnum)]; }

#undef countof

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONSTANT_MAPPING_H */

/* end of file */
