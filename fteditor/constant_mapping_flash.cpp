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
	"MX25L16 (2 MiB)",
	"MX25L32 (4 MiB)",
	"MX25L64 (8 MiB)",
	"MX25L128 (16 MiB)",
	"MX25L256 (32 MiB)",
	"MX25L512 (64 MiB)",
	"MX25L1024 (128 MiB)",
	"MX25L2048 (256 MiB)",
};

const int g_FlashSizeBytes[FTEDITOR_FLASH_NB] = { // Flash sizes
	2 * 1024 * 1024,
	4 * 1024 * 1024,
	8 * 1024 * 1024,
	16 * 1024 * 1024,
	32 * 1024 * 1024,
	64 * 1024 * 1024,
	128 * 1024 * 1024,
	256 * 1024 * 1024,
};

const wchar_t *g_FlashDeviceType[FTEDITOR_FLASH_NB] = { // Library for flash device
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
	L"mx25lemu",
};

const wchar_t *g_FlashFirmware[FTEDITOR_FLASH_NB] = { // Library for flash device
	L"unified.blob",
	L"unified.blob",
	L"unified.blob",
	L"unified.blob",
	L"mx25l.blob",
	L"mx25l.blob",
	L"mx25l.blob",
	L"mx25l.blob",
};

int g_CurrentFlash = FTEDITOR_DEFAULT_FLASH;

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
