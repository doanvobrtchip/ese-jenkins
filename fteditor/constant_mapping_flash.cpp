/*
EVE Screen Editor
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include "constant_mapping_flash.h"

// STL includes

namespace FTEDITOR {

///////////////////////////////////////////////////////////////////////

// TODO: Load from a simple CSV file

const char *g_FlashToString[FTEDITOR_FLASH_NB] = { // Flash device names
	"MX25L64 (8 MiB)",
};

const int g_FlashSizeBytes[FTEDITOR_FLASH_NB] = { // Flash sizes
	8 * 1024 * 1024,
};

const wchar_t *g_FlashDeviceType[FTEDITOR_FLASH_NB] = { // Library for flash device
	L"mx25lemu",
};

const wchar_t *g_FlashFirmware[FTEDITOR_FLASH_NB] = { // Library for flash device
	L"unified.blob",
};

int g_CurrentFlash = FTEDITOR_DEFAULT_FLASH;

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
