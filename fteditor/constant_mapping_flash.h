/*
EVE Screen Editor
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FTEDITOR_CONSTANT_MAPPING_FLASH_H
#define FTEDITOR_CONSTANT_MAPPING_FLASH_H

// Emulator includes

// STL includes

namespace FTEDITOR {

// ToString functions will return "" on invalid input. Simply check (result[0] == '\0') to find out if the result is invalid

#define FTEDITOR_FLASH_NB 1

extern const char *g_FlashToString[FTEDITOR_FLASH_NB];
extern const int g_FlashSizeBytes[FTEDITOR_FLASH_NB];
extern const wchar_t *g_FlashDeviceType[FTEDITOR_FLASH_NB];

extern int g_CurrentFlash;
#define FTEDITOR_DEFAULT_FLASH 0
#define FTEDITOR_CURRENT_FLASH g_CurrentFlash

inline const char *flashToString(int flashIntf) { return flashIntf < FTEDITOR_FLASH_NB ? g_FlashToString[flashIntf] : ""; }
inline const int flashSizeBytes(int flashIntf) { return flashIntf < FTEDITOR_FLASH_NB ? g_FlashSizeBytes[flashIntf] : 0; }
inline const wchar_t *flashDeviceType(int flashIntf) { return flashIntf < FTEDITOR_FLASH_NB ? g_FlashDeviceType[flashIntf] : L""; }

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_MAPPING_FLASH_H */

/* end of file */
