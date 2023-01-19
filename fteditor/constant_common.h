/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#pragma warning(disable : 4005) // macro redefinition

#ifndef FTEDITOR_CONSTANT_COMMON_H
#define FTEDITOR_CONSTANT_COMMON_H

namespace FTEDITOR {

#define START_RAM_G          0UL
#define START_RAM_DL         3145728UL
// REVIEW 2023-01-19: This is not a common constant between EVE revisions, does not belong here.
// The proper value can be obtained from `FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_DL)`.
// The dl_parser_impl.h file is compiled for each device, so it will map the proper value if you use that call to populate the map of constants.
// In EVE HAL, this constant/macro is exposed as `RAM_DL`.

#define NEAREST              0UL
#define BILINEAR             1UL

#define BITMAPS              1UL
#define POINTS               2UL
#define LINES                3UL
#define LINE_STRIP           4UL
#define EDGE_STRIP_R         5UL
#define EDGE_STRIP_L         6UL
#define EDGE_STRIP_A         7UL
#define EDGE_STRIP_B         8UL
#define RECTS                9UL

#define LINEAR_SAMPLES       0UL
#define ULAW_SAMPLES         1UL
#define ADPCM_SAMPLES        2UL

#define ARGB1555             0UL
#define L1                   1UL
#define L4                   2UL
#define L8                   3UL
#define RGB332               4UL
#define ARGB2                5UL
#define ARGB4                6UL
#define RGB565               7UL
#define PALETTED             8UL
#define TEXT8X8              9UL
#define TEXTVGA              10UL
#define BARGRAPH             11UL
#define PALETTED4444         15UL
#define PALETTED565          14UL
#define PALETTED8            16UL
#define L2                   17UL

#define GLFORMAT             31UL

#define COMPRESSED_RGBA_ASTC_10x10_KHR 37819UL
#define COMPRESSED_RGBA_ASTC_10x5_KHR 37816UL
#define COMPRESSED_RGBA_ASTC_10x6_KHR 37817UL
#define COMPRESSED_RGBA_ASTC_10x8_KHR 37818UL
#define COMPRESSED_RGBA_ASTC_12x10_KHR 37820UL
#define COMPRESSED_RGBA_ASTC_12x12_KHR 37821UL
#define COMPRESSED_RGBA_ASTC_4x4_KHR 37808UL
#define COMPRESSED_RGBA_ASTC_5x4_KHR 37809UL
#define COMPRESSED_RGBA_ASTC_5x5_KHR 37810UL
#define COMPRESSED_RGBA_ASTC_6x5_KHR 37811UL
#define COMPRESSED_RGBA_ASTC_6x6_KHR 37812UL
#define COMPRESSED_RGBA_ASTC_8x5_KHR 37813UL
#define COMPRESSED_RGBA_ASTC_8x6_KHR 37814UL
#define COMPRESSED_RGBA_ASTC_8x8_KHR 37815UL

#define BORDER               0UL
#define REPEAT               1UL

#define NEVER                0UL
#define LESS                 1UL
#define LEQUAL               2UL
#define GREATER              3UL
#define GEQUAL               4UL
#define EQUAL                5UL
#define NOTEQUAL             6UL
#define ALWAYS               7UL

#define ZERO                 0UL
#define KEEP                 1UL
#define REPLACE              2UL
#define INCR                 3UL
#define DECR                 4UL
#define INVERT               5UL

// #define ZERO                 0UL
#define ONE                  1UL
#define SRC_ALPHA            2UL
#define DST_ALPHA            3UL
#define ONE_MINUS_SRC_ALPHA  4UL
#define ONE_MINUS_DST_ALPHA  5UL

// #define ZERO                 0UL
// #define ONE                  1UL
#define RED                  2UL
#define GREEN                3UL
#define BLUE                 4UL
#define ALPHA                5UL

#define ANIM_ONCE            0UL
#define ANIM_LOOP            1UL
#define ANIM_HOLD            2UL

#define FLASH_STATUS_INIT    0UL
#define FLASH_STATUS_DETACHED 1UL
#define FLASH_STATUS_BASIC   2UL
#define FLASH_STATUS_FULL    3UL

#define CTOUCH_MODE_EXTENDED 0UL
#define CTOUCH_MODE_COMPATIBILITY 1UL

#define TOUCHMODE_OFF        0UL
#define TOUCHMODE_ONESHOT    1UL
#define TOUCHMODE_FRAME      2UL
#define TOUCHMODE_CONTINUOUS 3UL

#define CMDBUF_SIZE          4096UL

#define DLSWAP_DONE          0UL
#define DLSWAP_LINE          1UL
#define DLSWAP_FRAME         2UL

#define OPT_MONO             1UL
#define OPT_NODL             2UL
#define OPT_NOTEAR           4UL
#define OPT_FULLSCREEN       8UL
#define OPT_MEDIAFIFO        16UL
#define OPT_SOUND            32UL
#define OPT_FLAT             256UL
#define OPT_SIGNED           256UL
#define OPT_DITHER           256UL // BT817
#define OPT_CENTERX          512UL
#define OPT_CENTERY          1024UL
#define OPT_CENTER           1536UL
#define OPT_RIGHTX           2048UL
#define OPT_NOBACK           4096UL
#define OPT_NOHANDS          49152UL
#define OPT_NOTICKS          8192UL
#define OPT_NOHM             16384UL
#define OPT_NOPOINTER        16384UL
#define OPT_NOSECS           32768UL

// BT815
#define OPT_FLASH            64UL
#define OPT_OVERLAY          128UL
#define OPT_FORMAT           4096UL
#define OPT_FILL             8192UL

// FT800
#define CMD_DLSTART          4294967040UL
#define CMD_SWAP             4294967041UL
#define CMD_INTERRUPT        4294967042UL
#define CMD_CRC              4294967043UL
#define CMD_HAMMERAUX        4294967044UL
#define CMD_MARCH            4294967045UL
#define CMD_IDCT_DELETED     4294967046UL
#define CMD_EXECUTE          4294967047UL
#define CMD_GETPOINT         4294967048UL
#define CMD_BGCOLOR          4294967049UL
#define CMD_FGCOLOR          4294967050UL
#define CMD_GRADIENT         4294967051UL
#define CMD_TEXT             4294967052UL
#define CMD_BUTTON           4294967053UL
#define CMD_KEYS             4294967054UL
#define CMD_PROGRESS         4294967055UL
#define CMD_SLIDER           4294967056UL
#define CMD_SCROLLBAR        4294967057UL
#define CMD_TOGGLE           4294967058UL
#define CMD_GAUGE            4294967059UL
#define CMD_CLOCK            4294967060UL
#define CMD_CALIBRATE        4294967061UL
#define CMD_SPINNER          4294967062UL
#define CMD_STOP             4294967063UL
#define CMD_MEMCRC           4294967064UL
#define CMD_REGREAD          4294967065UL
#define CMD_MEMWRITE         4294967066UL
#define CMD_MEMSET           4294967067UL
#define CMD_MEMZERO          4294967068UL
#define CMD_MEMCPY           4294967069UL
#define CMD_APPEND           4294967070UL
#define CMD_SNAPSHOT         4294967071UL
#define CMD_TOUCH_TRANSFORM  4294967072UL
#define CMD_BITMAP_TRANSFORM 4294967073UL
#define CMD_INFLATE          4294967074UL
#define CMD_GETPTR           4294967075UL
#define CMD_LOADIMAGE        4294967076UL
#define CMD_GETPROPS         4294967077UL
#define CMD_LOADIDENTITY     4294967078UL
#define CMD_TRANSLATE        4294967079UL
#define CMD_SCALE            4294967080UL
#define CMD_ROTATE           4294967081UL
#define CMD_SETMATRIX        4294967082UL
#define CMD_SETFONT          4294967083UL
#define CMD_TRACK            4294967084UL
#define CMD_DIAL             4294967085UL
#define CMD_NUMBER           4294967086UL
#define CMD_SCREENSAVER      4294967087UL
#define CMD_SKETCH           4294967088UL
#define CMD_LOGO             4294967089UL
#define CMD_COLDSTART        4294967090UL
#define CMD_GETMATRIX        4294967091UL
#define CMD_GRADCOLOR        4294967092UL

// FT801
#define CMD_CSKETCH          4294967093UL
// #define CMD_DEPRECATED_CSKETCH 4294967093UL

// FT810
#define CMD_SETROTATE        4294967094UL
#define CMD_SNAPSHOT2        4294967095UL
#define CMD_SETBASE          4294967096UL
#define CMD_MEDIAFIFO        4294967097UL
#define CMD_PLAYVIDEO        4294967098UL
#define CMD_SETFONT2         4294967099UL
#define CMD_SETSCRATCH       4294967100UL
#define CMD_INT_RAMSHARED    4294967101UL
#define CMD_INT_SWLOADIMAGE  4294967102UL
#define CMD_ROMFONT          4294967103UL
#define CMD_VIDEOSTART       4294967104UL
#define CMD_VIDEOFRAME       4294967105UL
#define CMD_SYNC             4294967106UL
#define CMD_SETBITMAP        4294967107UL

// BT815
#define CMD_FLASHERASE       4294967108UL
#define CMD_FLASHWRITE       4294967109UL
#define CMD_FLASHREAD        4294967110UL
#define CMD_FLASHUPDATE      4294967111UL
#define CMD_FLASHDETACH      4294967112UL
#define CMD_FLASHATTACH      4294967113UL
#define CMD_FLASHFAST        4294967114UL
#define CMD_FLASHSPIDESEL    4294967115UL
#define CMD_FLASHSPITX       4294967116UL
#define CMD_FLASHSPIRX       4294967117UL
#define CMD_FLASHSOURCE      4294967118UL
#define CMD_CLEARCACHE       4294967119UL
#define CMD_INFLATE2         4294967120UL
#define CMD_ROTATEAROUND     4294967121UL
#define CMD_RESETFONTS       4294967122UL
#define CMD_ANIMSTART        4294967123UL
#define CMD_ANIMSTOP         4294967124UL
#define CMD_ANIMXY           4294967125UL
#define CMD_ANIMDRAW         4294967126UL
#define CMD_GRADIENTA        4294967127UL
#define CMD_FILLWIDTH        4294967128UL
#define CMD_APPENDF          4294967129UL
#define CMD_ANIMFRAME        4294967130UL
#define CMD_NOP              4294967131UL
#define CMD_SHA1             4294967132UL
#define CMD_HMAC             4294967133UL
#define CMD_VIDEOSTARTF      4294967135UL

// BT817
#define CMD_LINETIME         4294967134UL
#define CMD_CALIBRATESUB     4294967136UL
#define CMD_TESTCARD         4294967137UL
#define CMD_HSF              4294967138UL
#define CMD_APILEVEL         4294967139UL
#define CMD_GETIMAGE         4294967140UL
#define CMD_WAIT             4294967141UL
#define CMD_RETURN           4294967142UL
#define CMD_CALLLIST         4294967143UL
#define CMD_NEWLIST          4294967144UL
#define CMD_ENDLIST          4294967145UL
#define CMD_PCLKFREQ         4294967146UL
#define CMD_FONTCACHE        4294967147UL
#define CMD_FONTCACHEQUERY   4294967148UL
#define CMD_ANIMFRAMERAM     4294967149UL
#define CMD_ANIMSTARTRAM     4294967150UL
#define CMD_RUNANIM          4294967151UL
#define CMD_FLASHPROGRAM     4294967152UL /* Patched in from flash firmware */

#define DISPLAY() ((0UL<<24))
#define JUMP(dest) ((30UL<<24)|(((dest)&65535UL)<<0))
#define CLEAR(c,s,t) ((38UL<<24)|(((c)&1UL)<<2)|(((s)&1UL)<<1)|(((t)&1UL)<<0))
#define CLEAR_COLOR_RGB(red,green,blue) ((2UL<<24)|(((red)&255UL)<<16)|(((green)&255UL)<<8)|(((blue)&255UL)<<0))

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONSTANT_COMMON_H */

/* end of file */
