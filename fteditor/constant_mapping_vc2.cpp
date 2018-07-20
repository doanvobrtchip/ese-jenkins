/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "constant_mapping.h"
#include "constant_mapping_vc2.h"

// Emulator includes
#include <vc2.h>

namespace FTEDITOR {

///////////////////////////////////////////////////////////////////////

const int32_t g_AddrVC2[FTEDITOR_RAM_NB] = {
	RAM_G,
	1024 * 1024,
	RAM_DL,
	-1,
	RAM_REG,
	RAM_CMD,
};

const char *g_AddrToStringVC2[FTEDITOR_RAM_NB] = {
	"RAM_G",
	"",
	"RAM_DL",
	"",
	"RAM_REG",
	"RAM_CMD",
};

///////////////////////////////////////////////////////////////////////

const int32_t g_RegVC2[FTEDITOR_REG_NB] = {
	REG_ID,       // 0x302000
	REG_FRAMES,
	REG_CLOCK,
	REG_FREQUENCY,
	REG_RENDERMODE,
	REG_SNAPY,
	REG_SNAPSHOT,
	REG_SNAPFORMAT,
	REG_CPURESET,
	REG_TAP_CRC,
	REG_TAP_MASK,
	REG_HCYCLE,
	REG_HOFFSET,
	REG_HSIZE,
	REG_HSYNC0,
	REG_HSYNC1,
	REG_VCYCLE,   // 0x302040
	REG_VOFFSET,
	REG_VSIZE,
	REG_VSYNC0,
	REG_VSYNC1,
	REG_DLSWAP,
	REG_ROTATE,
	REG_OUTBITS,
	REG_DITHER,
	REG_SWIZZLE,
	REG_CSPREAD,
	REG_PCLK_POL,
	REG_PCLK,
	REG_TAG_X,
	REG_TAG_Y,
	REG_TAG,
	REG_VOL_PB,   // 0x302080
	REG_VOL_SOUND,
	REG_SOUND,
	REG_PLAY,
	REG_GPIO_DIR,
	REG_GPIO,
	REG_GPIOX_DIR,
	REG_GPIOX,
	REG_J1_COLD,
	REG_J1_INT,
	REG_INT_FLAGS,
	REG_INT_EN,
	REG_INT_MASK,
	REG_PLAYBACK_START,
	REG_PLAYBACK_LENGTH,
	REG_PLAYBACK_READPTR,
	REG_PLAYBACK_FREQ, // 0x3020C0
	REG_PLAYBACK_FORMAT,
	REG_PLAYBACK_LOOP,
	REG_PLAYBACK_PLAY,
	REG_PWM_HZ,
	REG_PWM_DUTY,
	REG_MACRO_0,
	REG_MACRO_1,
	REG_CYA0,
	REG_CYA1,
	REG_BUSYBITS,
	-1,
	REG_ROMSUB_SEL,
	REG_RAM_FOLD,
	REG_CMD_READ,
	REG_CMD_WRITE,
	REG_CMD_DL,        // 0x302100
	REG_TOUCH_MODE,
	REG_CTOUCH_EXTENDED,
	REG_TOUCH_CHARGE,
	REG_TOUCH_SETTLE,
	REG_TOUCH_OVERSAMPLE,
	REG_TOUCH_RZTHRESH,
	REG_CTOUCH_TOUCH1_XY,
	REG_CTOUCH_TOUCH4_Y,
	REG_CTOUCH_TOUCH0_XY,
	REG_TOUCH_TAG_XY,
	REG_TOUCH_TAG,
	REG_TOUCH_TAG1_XY,
	REG_TOUCH_TAG1,
	REG_TOUCH_TAG2_XY,
	REG_TOUCH_TAG2,
	REG_TOUCH_TAG3_XY, // 0x302140
	REG_TOUCH_TAG3,
	REG_TOUCH_TAG4_XY,
	REG_TOUCH_TAG4,
	REG_TOUCH_TRANSFORM_A,
	REG_TOUCH_TRANSFORM_B,
	REG_TOUCH_TRANSFORM_C,
	REG_TOUCH_TRANSFORM_D,
	REG_TOUCH_TRANSFORM_E,
	REG_TOUCH_TRANSFORM_F,
	REG_CYA_TOUCH,
	REG_CTOUCH_TOUCH4_X,
	REG_TOUCH_FAULT,
	REG_BIST_EN,
	REG_CRC,
	REG_SPI_EARLY_TX,
	REG_TRIM,          // 0x302180
	REG_ANA_COMP,
	REG_SPI_WIDTH,
	REG_CTOUCH_TOUCH2_XY, 
	REG_CTOUCH_TOUCH3_XY,

	//////////////////////////////
	REG_RASTERY,
	REG_DATESTAMP,
	REG_DATESTAMP + 4,
	REG_DATESTAMP + 8,
	REG_DATESTAMP + 12,
	REG_CMDB_SPACE,
	REG_CMDB_WRITE,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,

	///////////////////////
	REG_TRACKER,
	REG_TRACKER_1,
	REG_TRACKER_2,
	REG_TRACKER_3,
	REG_TRACKER_4,
	REG_MEDIAFIFO_READ,
	REG_MEDIAFIFO_WRITE,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
};

const char *g_RegToStringVC2[FTEDITOR_REG_NB] = {
	"REG_ID",
	"REG_FRAMES",
	"REG_CLOCK",
	"REG_FREQUENCY",
	"REG_RENDERMODE",
	"REG_SNAPY",
	"REG_SNAPSHOT",
	"REG_SNAPFORMAT",
	"REG_CPURESET",
	"REG_TAP_CRC",
	"REG_TAP_MASK",
	"REG_HCYCLE",
	"REG_HOFFSET",
	"REG_HSIZE",
	"REG_HSYNC0",
	"REG_HSYNC1",
	"REG_VCYCLE",
	"REG_VOFFSET",
	"REG_VSIZE",
	"REG_VSYNC0",
	"REG_VSYNC1",
	"REG_DLSWAP",
	"REG_ROTATE",
	"REG_OUTBITS",
	"REG_DITHER",
	"REG_SWIZZLE",
	"REG_CSPREAD",
	"REG_PCLK_POL",
	"REG_PCLK",
	"REG_TAG_X",
	"REG_TAG_Y",
	"REG_TAG",
	"REG_VOL_PB",
	"REG_VOL_SOUND",
	"REG_SOUND",
	"REG_PLAY",
	"REG_GPIO_DIR",
	"REG_GPIO",
	"REG_GPIOX_DIR",
	"REG_GPIOX",
	"REG_J1_COLD",
	"REG_J1_INT",
	"REG_INT_FLAGS",
	"REG_INT_EN",
	"REG_INT_MASK",
	"REG_PLAYBACK_START",
	"REG_PLAYBACK_LENGTH",
	"REG_PLAYBACK_READPTR",
	"REG_PLAYBACK_FREQ",
	"REG_PLAYBACK_FORMAT",
	"REG_PLAYBACK_LOOP",
	"REG_PLAYBACK_PLAY",
	"REG_PWM_HZ",
	"REG_PWM_DUTY",
	"REG_MACRO_0",
	"REG_MACRO_1",
	"REG_CYA0",
	"REG_CYA1",
	"REG_BUSYBITS",
	"",
	"REG_ROMSUB_SEL",
	"REG_RAM_FOLD",
	"REG_CMD_READ",
	"REG_CMD_WRITE",
	"REG_CMD_DL",
	"REG_TOUCH_MODE",
	"REG_CTOUCH_EXTENDED",
	"REG_TOUCH_CHARGE",
	"REG_TOUCH_SETTLE",
	"REG_TOUCH_OVERSAMPLE",
	"REG_TOUCH_RZTHRESH",
	"REG_CTOUCH_TOUCH1_XY",
	"REG_CTOUCH_TOUCH4_Y",
	"REG_CTOUCH_TOUCH0_XY",
	"REG_TOUCH_TAG_XY",
	"REG_TOUCH_TAG",
	"REG_TOUCH_TAG1_XY",
	"REG_TOUCH_TAG1",
	"REG_TOUCH_TAG2_XY",
	"REG_TOUCH_TAG2",
	"REG_TOUCH_TAG3_XY",
	"REG_TOUCH_TAG3",
	"REG_TOUCH_TAG4_XY",
	"REG_TOUCH_TAG4",
	"REG_TOUCH_TRANSFORM_A",
	"REG_TOUCH_TRANSFORM_B",
	"REG_TOUCH_TRANSFORM_C",
	"REG_TOUCH_TRANSFORM_D",
	"REG_TOUCH_TRANSFORM_E",
	"REG_TOUCH_TRANSFORM_F",
	"REG_CYA_TOUCH",
	"REG_CTOUCH_TOUCH4_X",
	"REG_TOUCH_FAULT",
	"REG_BIST_EN",
	"REG_CRC",
	"REG_SPI_EARLY_TX",
	"REG_TRIM",
	"REG_ANA_COMP",
	"REG_SPI_WIDTH",
	"REG_CTOUCH_TOUCH2_XY",
	"REG_CTOUCH_TOUCH3_XY",

	//////////////////////////////
	"REG_RASTERY",
	"REG_DATESTAMP",
	"REG_DATESTAMP + 4",
	"REG_DATESTAMP + 8",
	"REG_DATESTAMP + 12",
	"REG_CMDB_SPACE",
	"REG_CMDB_WRITE",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	
	///////////////////////
	"REG_TRACKER",
	"REG_TRACKER_1",
	"REG_TRACKER_2",
	"REG_TRACKER_3",
	"REG_TRACKER_4",
	"REG_MEDIAFIFO_READ",
	"REG_MEDIAFIFO_WRITE",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};

///////////////////////////////////////////////////////////////////////

const char *g_BitmapFormatToStringVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2] = {
	"ARGB1555", // 0
	"L1",
	"L4",
	"L8",
	"RGB332",
	"ARGB2",
	"ARGB4",
	"RGB565",
	"", // 8
	"TEXT8X8", // 9
	"TEXTVGA", // 10
	"BARGRAPH", // 11
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
	"ARGB8_SNAPSHOT", // 32 // 0x20
};

///////////////////////////////////////////////////////////////////////

const int g_BitmapFormatFromIntfVC2[FTEDITOR_BITMAP_FORMAT_INTF_NB_VC2] = {
	ARGB1555, // 0
	L1, // 1
	L2, // 2
	L4, // 3
	L8, // 4
	RGB332, // 5
	ARGB2, // 6
	ARGB4, // 7
	RGB565, // 8
	PALETTED565, // 9
	PALETTED4444, // 10
	PALETTED8, // 11
	TEXT8X8, // 12
	TEXTVGA, // 13
	BARGRAPH, // 14
};

const int g_BitmapFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2] = {
	0, 1, 3, 4, 5, 6, 7, 8,
	11, 12, 13, 14, 0, 0, 9, 10,
	11, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0
};

///////////////////////////////////////////////////////////////////////

const int g_SnapshotFormatFromIntfVC2[FTEDITOR_SNAPSHOT_FORMAT_INTF_NB_VC2] = {
	RGB565,
	ARGB4,
	ARGB8_SNAPSHOT,
};

const int g_SnapshotFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2] = {
	0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	2
};

///////////////////////////////////////////////////////////////////////

const int g_ImageFormatFromIntfVC2[FTEDITOR_IMAGE_FORMAT_INTF_NB_VC2] = {
	ARGB1555,
	L1,
	L2,
	L4,
	L8,
	RGB332,
	ARGB2,
	ARGB4,
	RGB565,
	PALETTED565,
	PALETTED4444,
	PALETTED8,
};

const int g_ImageFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2] = {
	0, 1, 3, 4, 5, 6, 7, 8,
	11, 0, 0, 0, 0, 0, 9, 10,
	11, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0
};

///////////////////////////////////////////////////////////////////////

const int g_FontFormatFromIntfVC2[FTEDITOR_FONT_FORMAT_INTF_NB_VC2] = {
	L1,
	L2,
	L4,
	L8,
};

const int g_FontFormatToIntfVC2[FTEDITOR_BITMAP_FORMAT_ENUM_NB_VC2] = {
	1, 0, 2, 3, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 
};

///////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
