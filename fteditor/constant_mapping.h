/*
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#ifndef FTEDITOR_CONSTANT_MAPPING_H
#define FTEDITOR_CONSTANT_MAPPING_H

// Emulator includes
#include <bt8xxemu.h>

// STL includes

namespace FTEDITOR {

// ToString functions will return "" on invalid input. Simply check (result[0] == '\0') to find out if the result is invalid

// Interface indices for Device
enum DeviceIdx
{
	// FT80x
	FTEDITOR_FT800,
	FTEDITOR_FT801,

	// FT81x (bigger display, hardware JPG)
	FTEDITOR_FT810,
	FTEDITOR_FT811,
	FTEDITOR_FT812,
	FTEDITOR_FT813,

	// BT81x (FT81x with less RAM_G and ROM)
	FTEDITOR_BT880,
	FTEDITOR_BT881,
	FTEDITOR_BT882,
	FTEDITOR_BT883,

	// BT815/BT816 (ASTC and flash)
	FTEDITOR_BT815,
	FTEDITOR_BT816,

	// BT817/BT816
	FTEDITOR_BT817,
	FTEDITOR_BT818,

	FTEDITOR_DEVICE_NB
};
extern const BT8XXEMU_EmulatorMode g_DeviceToEnum[FTEDITOR_DEVICE_NB];
inline BT8XXEMU_EmulatorMode deviceToEnum(int deviceIntf) { return g_DeviceToEnum[deviceIntf % FTEDITOR_DEVICE_NB]; }
extern const int g_DeviceToIntf[256];
inline int deviceToIntf(BT8XXEMU_EmulatorMode deviceEnum) { return g_DeviceToIntf[deviceEnum & 0xFF]; }
extern const char *g_DeviceToString[FTEDITOR_DEVICE_NB];
inline const char *deviceToString(int deviceIntf) { return deviceIntf < FTEDITOR_DEVICE_NB ? g_DeviceToString[deviceIntf] : ""; }

// Tempory for mapping conversion
extern int g_CurrentDevice;
#define FTEDITOR_CURRENT_DEVICE FTEDITOR::g_CurrentDevice
#define FTEDITOR_DEFAULT_DEVICE FTEDITOR_BT817

// Screen specs
extern const int g_ScreenWidthDefault[FTEDITOR_DEVICE_NB];
extern const int g_ScreenWidthMaximum[FTEDITOR_DEVICE_NB];
extern const int g_ScreenHeightDefault[FTEDITOR_DEVICE_NB];
extern const int g_ScreenHeightMaximum[FTEDITOR_DEVICE_NB];
inline int screenWidthDefault(int deviceIntf) { return g_ScreenWidthDefault[deviceIntf]; }
inline int screenWidthMaximum(int deviceIntf) { return g_ScreenWidthMaximum[deviceIntf]; }
inline int screenHeightDefault(int deviceIntf) { return g_ScreenHeightDefault[deviceIntf]; }
inline int screenHeightMaximum(int deviceIntf) { return g_ScreenHeightMaximum[deviceIntf]; }

// Display list specs
extern const int g_DisplayListSize[FTEDITOR_DEVICE_NB];
inline int displayListSize(int deviceIntf) { return g_DisplayListSize[deviceIntf]; }

// Memory specs
extern const uint32_t g_AddressSpace[FTEDITOR_DEVICE_NB];
extern const uint32_t g_AddressMask[FTEDITOR_DEVICE_NB];
inline uint32_t addressSpace(int deviceIntf) { return g_AddressSpace[deviceIntf]; }
inline uint32_t addressMask(int deviceIntf) { return g_AddressMask[deviceIntf]; }
inline int addressSigned(int deviceIntf, int address)
{
	// On BT815 either
	// Flash: 0x800000 to 0xFFFFFF
	// RAM_G: 0x000000 to 0x3FFFFF (0x200000 or 0x400000 is negative address)
	if (deviceIntf >= FTEDITOR_BT815
		&& ((address >> 23) & 0x1))
	{
		// Flash address / 32
		return address & 0xFFFFFF; // & 0x1FFFFFF;
	}
	int mask = g_AddressMask[deviceIntf];
	int masked = (address & mask);
	int negmask = ~(mask >> 1);
	int neg = masked & negmask;
	return neg ? (masked | negmask) : masked;
}

inline bool flashSupport(int deviceIntf) { return deviceIntf >= FTEDITOR_BT815; }

// RAM addresses
#define FTEDITOR_RAM_G 0
#define FTEDITOR_RAM_G_END 1
#define FTEDITOR_RAM_DL 2
#define FTEDITOR_RAM_PAL 3
#define FTEDITOR_RAM_REG 4
#define FTEDITOR_RAM_CMD 5
#define FTEDITOR_RAM_NB 6
extern const int32_t *g_Addr[FTEDITOR_DEVICE_NB];
inline int32_t addr(int deviceIntf, int addrIntf) { return g_Addr[deviceIntf][addrIntf]; }
extern const char **g_AddrToString[FTEDITOR_DEVICE_NB];
inline const char *addrToString(int deviceIntf, int addrIntf) { return g_AddrToString[deviceIntf][addrIntf]; }

// Register addresses
#define FTEDITOR_REG_ID 0
#define FTEDITOR_REG_FRAMES 1
#define FTEDITOR_REG_CLOCK 2
#define FTEDITOR_REG_FREQUENCY 3
#define FTEDITOR_REG_RENDERMODE 4
#define FTEDITOR_REG_SNAPY 5
#define FTEDITOR_REG_SNAPSHOT 6
#define FTEDITOR_REG_SNAPFORMAT 7
#define FTEDITOR_REG_CPURESET 8
#define FTEDITOR_REG_TAP_CRC 9
#define FTEDITOR_REG_TAP_MASK 10
#define FTEDITOR_REG_HCYCLE 11
#define FTEDITOR_REG_HOFFSET 12
#define FTEDITOR_REG_HSIZE 13
#define FTEDITOR_REG_HSYNC0 14
#define FTEDITOR_REG_HSYNC1 15
#define FTEDITOR_REG_VCYCLE 16
#define FTEDITOR_REG_VOFFSET 17
#define FTEDITOR_REG_VSIZE 18
#define FTEDITOR_REG_VSYNC0 19
#define FTEDITOR_REG_VSYNC1 20
#define FTEDITOR_REG_DLSWAP 21
#define FTEDITOR_REG_ROTATE 22
#define FTEDITOR_REG_OUTBITS 23
#define FTEDITOR_REG_DITHER 24
#define FTEDITOR_REG_SWIZZLE 25
#define FTEDITOR_REG_CSPREAD 26
#define FTEDITOR_REG_PCLK_POL 27
#define FTEDITOR_REG_PCLK 28
#define FTEDITOR_REG_TAG_X 29
#define FTEDITOR_REG_TAG_Y 30
#define FTEDITOR_REG_TAG 31
#define FTEDITOR_REG_VOL_PB 32
#define FTEDITOR_REG_VOL_SOUND 33
#define FTEDITOR_REG_SOUND 34
#define FTEDITOR_REG_PLAY 35
#define FTEDITOR_REG_GPIO_DIR 36
#define FTEDITOR_REG_GPIO 37
#define FTEDITOR_REG_GPIOX_DIR 38
#define FTEDITOR_REG_GPIOX 39
#define FTEDITOR_REG_J1_COLD 40
#define FTEDITOR_REG_J1_INT 41
#define FTEDITOR_REG_INT_FLAGS 42
#define FTEDITOR_REG_INT_EN 43
#define FTEDITOR_REG_INT_MASK 44
#define FTEDITOR_REG_PLAYBACK_START 45
#define FTEDITOR_REG_PLAYBACK_LENGTH 46
#define FTEDITOR_REG_PLAYBACK_READPTR 47
#define FTEDITOR_REG_PLAYBACK_FREQ 48
#define FTEDITOR_REG_PLAYBACK_FORMAT 49
#define FTEDITOR_REG_PLAYBACK_LOOP 50
#define FTEDITOR_REG_PLAYBACK_PLAY 51
#define FTEDITOR_REG_PWM_HZ 52
#define FTEDITOR_REG_PWM_DUTY 53
#define FTEDITOR_REG_MACRO_0 54
#define FTEDITOR_REG_MACRO_1 55
#define FTEDITOR_REG_CYA0 56
#define FTEDITOR_REG_CYA1 57
#define FTEDITOR_REG_BUSYBITS 58
#define FTEDITOR_REG_ROMSUB_SEL 60
#define FTEDITOR_REG_RAM_FOLD 61
#define FTEDITOR_REG_CMD_READ 62
#define FTEDITOR_REG_CMD_WRITE 63
#define FTEDITOR_REG_CMD_DL 64
#define FTEDITOR_REG_TOUCH_MODE 65
#define FTEDITOR_REG_CTOUCH_EXTENDED 66
#define FTEDITOR_REG_TOUCH_ADC_MODE 66
#define FTEDITOR_REG_TOUCH_CHARGE 67
#define FTEDITOR_REG_TOUCH_SETTLE 68
#define FTEDITOR_REG_TOUCH_OVERSAMPLE 69
#define FTEDITOR_REG_TOUCH_RZTHRESH 70
#define FTEDITOR_REG_CTOUCH_TOUCH1_XY 71
#define FTEDITOR_REG_TOUCH_RAW_XY 71
#define FTEDITOR_REG_CTOUCH_TOUCH4_Y 72
#define FTEDITOR_REG_TOUCH_RZ 72
#define FTEDITOR_REG_CTOUCH_TOUCH0_XY 73
#define FTEDITOR_REG_TOUCH_SCREEN_XY 73
#define FTEDITOR_REG_TOUCH_TAG_XY 74
#define FTEDITOR_REG_TOUCH_TAG 75
#define FTEDITOR_REG_TOUCH_TAG1_XY 76
#define FTEDITOR_REG_TOUCH_TAG1 77
#define FTEDITOR_REG_TOUCH_TAG2_XY 78
#define FTEDITOR_REG_TOUCH_TAG2 79
#define FTEDITOR_REG_TOUCH_TAG3_XY 80
#define FTEDITOR_REG_TOUCH_TAG3 81
#define FTEDITOR_REG_TOUCH_TAG4_XY 82
#define FTEDITOR_REG_TOUCH_TAG4 83
#define FTEDITOR_REG_TOUCH_TRANSFORM_A 84
#define FTEDITOR_REG_TOUCH_TRANSFORM_B 85
#define FTEDITOR_REG_TOUCH_TRANSFORM_C 86
#define FTEDITOR_REG_TOUCH_TRANSFORM_D 87
#define FTEDITOR_REG_TOUCH_TRANSFORM_E 88
#define FTEDITOR_REG_TOUCH_TRANSFORM_F 89
#define FTEDITOR_REG_CYA_TOUCH 90
#define FTEDITOR_REG_ANALOG 91
#define FTEDITOR_REG_CTOUCH_TOUCH4_X 91
#define FTEDITOR_REG_TOUCH_FAULT 92
#define FTEDITOR_REG_BIST_EN 93
#define FTEDITOR_REG_CRC 94
#define FTEDITOR_REG_302187C 95 // ?
#define FTEDITOR_REG_TRIM 96 // 0x302180
#define FTEDITOR_REG_ANA_COMP 97
#define FTEDITOR_REG_SPI_WIDTH 98

#define FTEDITOR_REG_CTOUCH_TOUCH2_XY 99
#define FTEDITOR_REG_TOUCH_DIRECT_XY 99

#define FTEDITOR_REG_CTOUCH_TOUCH3_XY 100
#define FTEDITOR_REG_TOUCH_DIRECT_Z1Z2 100      // 0x302190

///////////////////////////////////////////////////////////
#define FTEDITOR_REG_RASTERY             101    // 0x302560
#define FTEDITOR_REG_DATESTAMP           102
#define FTEDITOR_REG_DATESTAMP_1         103
#define FTEDITOR_REG_DATESTAMP_2         104
#define FTEDITOR_REG_DATESTAMP_3         105
#define FTEDITOR_REG_CMDB_SPACE          106
#define FTEDITOR_REG_CMDB_WRITE          107
#define FTEDITOR_REG_ADAPTIVE_FRAMERATE  108    
#define FTEDITOR_REG_SPIM_DIR            109    // 0x302580
#define FTEDITOR_REG_SPIM				 110
#define FTEDITOR_REG_ESPIM_READSTART	 111
#define FTEDITOR_REG_ESPIM_SEQ			 112
#define FTEDITOR_REG_ESPIM_SEQ_1		 113
#define FTEDITOR_REG_ESPIM_SEQ_2		 114
#define FTEDITOR_REG_ESPIM_SEQ_3		 115
#define FTEDITOR_REG_ESPIM_ADD			 116
#define FTEDITOR_REG_ESPIM_COUNT		 117    // 0x3025A0
#define FTEDITOR_REG_ESPIM_WINDOW		 118
#define FTEDITOR_REG_ESPIM_WINDOW_1		 119
#define FTEDITOR_REG_ESPIM_WINDOW_2      120
#define FTEDITOR_REG_ESPIM_WINDOW_3		 121
#define FTEDITOR_REG_ESPIM_WINDOW_4		 122
#define FTEDITOR_REG_ESPIM_WINDOW_5		 123
#define FTEDITOR_REG_ESPIM_WINDOW_6		 124
#define FTEDITOR_REG_ESPIM_WINDOW_7      125    // 0x3025C0
#define FTEDITOR_REG_ESPIM_WINDOW_8		 126
#define FTEDITOR_REG_ESPIM_WINDOW_9		 127
#define FTEDITOR_REG_ESPIM_WINDOW_A		 128
#define FTEDITOR_REG_ESPIM_WINDOW_B		 129
#define FTEDITOR_REG_ESPIM_WINDOW_C		 120
#define FTEDITOR_REG_ESPIM_WINDOW_D		 131
#define FTEDITOR_REG_ESPIM_WINDOW_E		 132
#define FTEDITOR_REG_ESPIM_WINDOW_F      133    // 0x3025C0
#define FTEDITOR_REG_ESPIM_DUMMY		 134
#define FTEDITOR_REG_ESPIM_TRIG			 135
#define FTEDITOR_REG_PLAYBACK_PAUSE		 136
#define FTEDITOR_REG_FLASH_STATUS        137    // 0x3025F0
#define FTEDITOR_REG_FULLBUSYBITS		 138
#define FTEDITOR_REG_3025F8				 139 // ?
#define FTEDITOR_REG_SHA1KEY			 140

///////////////////////////////////////////////////////////
#define FTEDITOR_REG_LINECLOCKS    		 141
#define FTEDITOR_REG_UNDERRUN     		 142
#define FTEDITOR_REG_AH_HCYCLE_MAX		 143
#define FTEDITOR_REG_PCLK_FREQ    		 144
#define FTEDITOR_REG_PCLK_2X      		 145
#define FTEDITOR_REG_FLASH_DTR    		 146
#define FTEDITOR_REG_ESPIM_DTR    		 147
#define FTEDITOR_REG_HSF_HSIZE    		 148
#define FTEDITOR_REG_HSF_FT1      		 149
#define FTEDITOR_REG_HSF_FSCALE   		 150
#define FTEDITOR_REG_HSF_F00      		 151
#define FTEDITOR_REG_HSF_F02      		 152
#define FTEDITOR_REG_HSF_F03      		 153
#define FTEDITOR_REG_HSF_F10      		 154
#define FTEDITOR_REG_HSF_F11      		 155
#define FTEDITOR_REG_HSF_F12      		 156
#define FTEDITOR_REG_HSF_F13      		 157

///////////////////////////////////////////////////////////
#define FTEDITOR_REG_TRACKER             158    // 0x309000
#define FTEDITOR_REG_TRACKER_1 			 159
#define FTEDITOR_REG_TRACKER_2 			 160
#define FTEDITOR_REG_TRACKER_3 			 161
#define FTEDITOR_REG_TRACKER_4 			 162
#define FTEDITOR_REG_MEDIAFIFO_READ 	 163
#define FTEDITOR_REG_MEDIAFIFO_WRITE 	 164
#define FTEDITOR_REG_MEDIAFIFO_BASE 	 165
#define FTEDITOR_REG_MEDIAFIFO_SIZE 	 166
#define FTEDITOR_REG_FLASH_SIZE 		 167
#define FTEDITOR_REG_309028 			 168
#define FTEDITOR_REG_ANIM_ACTIVE 		 169
#define FTEDITOR_REG_DF_TUNED 			 170

///////////////////////////////////////////////////////////
#define FTEDITOR_REG_PLAY_CONTROL        171


#define FTEDITOR_REG_NB 172
extern const int32_t *g_Reg[FTEDITOR_DEVICE_NB];
inline int32_t reg(int deviceIntf, int regIntf) { return g_Reg[deviceIntf][regIntf]; }
extern const char **g_RegToString[FTEDITOR_DEVICE_NB];
inline const char *regToString(int deviceIntf, int regIntf) { return g_RegToString[deviceIntf][regIntf]; }

// Display list root types (DlParsed.IdLeft)
#define FTEDITOR_DL_INSTRUCTION 0
#define FTEDITOR_DL_VERTEX2F 1
#define FTEDITOR_DL_VERTEX2II 2
#define FTEDITOR_CO_COMMAND 0xFFFFFF00

// Display list commands (DlParsed.IdRight)
#define FTEDITOR_DL_DISPLAY 0
#define FTEDITOR_DL_BITMAP_SOURCE 1
#define FTEDITOR_DL_CLEAR_COLOR_RGB 2
#define FTEDITOR_DL_TAG 3
#define FTEDITOR_DL_COLOR_RGB 4
#define FTEDITOR_DL_BITMAP_HANDLE 5
#define FTEDITOR_DL_CELL 6
#define FTEDITOR_DL_BITMAP_LAYOUT 7
#define FTEDITOR_DL_BITMAP_SIZE 8
#define FTEDITOR_DL_ALPHA_FUNC 9
#define FTEDITOR_DL_STENCIL_FUNC 10
#define FTEDITOR_DL_BLEND_FUNC 11
#define FTEDITOR_DL_STENCIL_OP 12
#define FTEDITOR_DL_POINT_SIZE 13
#define FTEDITOR_DL_LINE_WIDTH 14
#define FTEDITOR_DL_CLEAR_COLOR_A 15
#define FTEDITOR_DL_COLOR_A 16
#define FTEDITOR_DL_CLEAR_STENCIL 17
#define FTEDITOR_DL_CLEAR_TAG 18
#define FTEDITOR_DL_STENCIL_MASK 19
#define FTEDITOR_DL_TAG_MASK 20
#define FTEDITOR_DL_BITMAP_TRANSFORM_A 21
#define FTEDITOR_DL_BITMAP_TRANSFORM_B 22
#define FTEDITOR_DL_BITMAP_TRANSFORM_C 23
#define FTEDITOR_DL_BITMAP_TRANSFORM_D 24
#define FTEDITOR_DL_BITMAP_TRANSFORM_E 25
#define FTEDITOR_DL_BITMAP_TRANSFORM_F 26
#define FTEDITOR_DL_SCISSOR_XY 27
#define FTEDITOR_DL_SCISSOR_SIZE 28
#define FTEDITOR_DL_CALL 29
#define FTEDITOR_DL_JUMP 30
#define FTEDITOR_DL_BEGIN 31
#define FTEDITOR_DL_COLOR_MASK 32
#define FTEDITOR_DL_END 33
#define FTEDITOR_DL_SAVE_CONTEXT 34
#define FTEDITOR_DL_RESTORE_CONTEXT 35
#define FTEDITOR_DL_RETURN 36
#define FTEDITOR_DL_MACRO 37
#define FTEDITOR_DL_CLEAR 38
#define FTEDITOR_DL_VERTEX_FORMAT 39
#define FTEDITOR_DL_BITMAP_LAYOUT_H 40
#define FTEDITOR_DL_BITMAP_SIZE_H 41
#define FTEDITOR_DL_PALETTE_SOURCE 42
#define FTEDITOR_DL_VERTEX_TRANSLATE_X 43
#define FTEDITOR_DL_VERTEX_TRANSLATE_Y 44
#define FTEDITOR_DL_NOP 45
#define FTEDITOR_DL_BITMAP_EXT_FORMAT 46
#define FTEDITOR_DL_BITMAP_SWIZZLE 47
#define FTEDITOR_DL_INT_FRR 48

// Mappings for bitmap formats
#define ARGB8_SNAPSHOT 0x20
extern const char **g_BitmapFormatToString[FTEDITOR_DEVICE_NB];
extern const int g_BitmapFormatEnumNb[FTEDITOR_DEVICE_NB];
inline const char *bitmapFormatToString(int deviceIntf, int bitmapFormatEnum) { return bitmapFormatEnum < g_BitmapFormatEnumNb[deviceIntf] ? g_BitmapFormatToString[deviceIntf][bitmapFormatEnum] : ""; }

// Valid bitmap formats, using bitmap formats enum
extern const int g_BitmapFormatIntfNb[FTEDITOR_DEVICE_NB];
extern const int *g_BitmapFormatFromIntf[FTEDITOR_DEVICE_NB];
extern const int *g_BitmapFormatToIntf[FTEDITOR_DEVICE_NB];

// Valid extended bitmap formats, using bitmap formats enum
extern const int g_ExtFormatIntfNb[FTEDITOR_DEVICE_NB];
extern const int *g_ExtFormatFromIntf[FTEDITOR_DEVICE_NB];
extern const int *g_ExtFormatToIntf[FTEDITOR_DEVICE_NB];

// Snapshot formats, using bitmap formats enum
extern const int g_SnapshotFormatIntfNb[FTEDITOR_DEVICE_NB];
extern const int *g_SnapshotFormatFromIntf[FTEDITOR_DEVICE_NB];
extern const int *g_SnapshotFormatToIntf[FTEDITOR_DEVICE_NB];

// Image formats, using bitmap formats enum
extern const int g_ImageFormatIntfNb[FTEDITOR_DEVICE_NB];
extern const int *g_ImageFormatFromIntf[FTEDITOR_DEVICE_NB];
extern const int *g_ImageFormatToIntf[FTEDITOR_DEVICE_NB];

// Font formats, using bitmap formats enum
extern const int g_FontFormatIntfNb[FTEDITOR_DEVICE_NB];
extern const int *g_FontFormatFromIntf[FTEDITOR_DEVICE_NB];
extern const int *g_FontFormatToIntf[FTEDITOR_DEVICE_NB];

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_MAPPING_H */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
