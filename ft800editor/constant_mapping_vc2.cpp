/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"

// Emulator includes
#include <vc2.h>

namespace FT800EMUQT {

///////////////////////////////////////////////////////////////////////

const char *g_BitmapFormatToStringVC2[] = {
	"ARGB1555",
	"L1",
	"L4",
	"L8",
	"RGB332",
	"ARGB2",
	"ARGB4",
	"RGB565",
	"", // 8
	"",
	"", // 10
	"",
	"",
	"",
	"PALETTED565",
	"PALETTED4444",
	"PALETTED8",
	"L2", // 17
	"",
	"",
	"", // 20
	"", // 21
	"", // 22
	"", // 23
	"", // 24
	"", // 25
	"", // 26
	"", // 27
	"", // 28
	"", // 29
	"", // 30
	"", // 31
	"ARGB8", // 32 // 0x20
};

///////////////////////////////////////////////////////////////////////

} /* namespace FT800EMUQT */

/* end of file */
