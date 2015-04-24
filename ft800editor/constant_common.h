/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FT800EMUQT_CONSTANT_COMMON_H
#define FT800EMUQT_CONSTANT_COMMON_H

namespace FT800EMUQT {

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

#define CMD_APPEND           4294967070UL
#define CMD_BGCOLOR          4294967049UL
#define CMD_BITMAP_TRANSFORM 4294967073UL
#define CMD_BUTTON           4294967053UL
#define CMD_CALIBRATE        4294967061UL
#define CMD_CLOCK            4294967060UL
#define CMD_COLDSTART        4294967090UL
#define CMD_CRC              4294967043UL
#define CMD_CSKETCH          4294967093UL
#define CMD_DEPRECATED_CSKETCH 4294967093UL
#define CMD_DIAL             4294967085UL
#define CMD_DLSTART          4294967040UL
#define CMD_EXECUTE          4294967047UL
#define CMD_FGCOLOR          4294967050UL
#define CMD_GAUGE            4294967059UL
#define CMD_GETMATRIX        4294967091UL
#define CMD_GETPOINT         4294967048UL
#define CMD_GETPROPS         4294967077UL
#define CMD_GETPTR           4294967075UL
#define CMD_GRADCOLOR        4294967092UL
#define CMD_GRADIENT         4294967051UL
#define CMD_HAMMERAUX        4294967044UL
#define CMD_IDCT_DELETED     4294967046UL
#define CMD_INFLATE          4294967074UL
#define CMD_INTERRUPT        4294967042UL
#define CMD_INT_RAMSHARED    4294967101UL
#define CMD_INT_SWLOADIMAGE  4294967102UL
#define CMD_KEYS             4294967054UL
#define CMD_LOADIDENTITY     4294967078UL
#define CMD_LOADIMAGE        4294967076UL
#define CMD_LOGO             4294967089UL
#define CMD_MARCH            4294967045UL
#define CMD_MEDIAFIFO        4294967097UL
#define CMD_MEMCPY           4294967069UL
#define CMD_MEMCRC           4294967064UL
#define CMD_MEMSET           4294967067UL
#define CMD_MEMWRITE         4294967066UL
#define CMD_MEMZERO          4294967068UL
#define CMD_NUMBER           4294967086UL
#define CMD_PLAYVIDEO        4294967098UL
#define CMD_PROGRESS         4294967055UL
#define CMD_REGREAD          4294967065UL
#define CMD_ROMFONT          4294967103UL
#define CMD_ROTATE           4294967081UL
#define CMD_SCALE            4294967080UL
#define CMD_SCREENSAVER      4294967087UL
#define CMD_SCROLLBAR        4294967057UL
#define CMD_SETBASE          4294967096UL
#define CMD_SETBITMAP        4294967107UL
#define CMD_SETFONT          4294967083UL
#define CMD_SETFONT2         4294967099UL
#define CMD_SETMATRIX        4294967082UL
#define CMD_SETROTATE        4294967094UL
#define CMD_SETSCRATCH       4294967100UL
#define CMD_SKETCH           4294967088UL
#define CMD_SLIDER           4294967056UL
#define CMD_SNAPSHOT         4294967071UL
#define CMD_SNAPSHOT2        4294967095UL
#define CMD_SPINNER          4294967062UL
#define CMD_STOP             4294967063UL
#define CMD_SWAP             4294967041UL
#define CMD_SYNC             4294967106UL
#define CMD_TEXT             4294967052UL
#define CMD_TOGGLE           4294967058UL
#define CMD_TOUCH_TRANSFORM  4294967072UL
#define CMD_TRACK            4294967084UL
#define CMD_TRANSLATE        4294967079UL
#define CMD_VIDEOFRAME       4294967105UL
#define CMD_VIDEOSTART       4294967104UL

#define DISPLAY() ((0UL<<24))
#define JUMP(dest) ((30UL<<24)|(((dest)&65535UL)<<0))
#define CLEAR(c,s,t) ((38UL<<24)|(((c)&1UL)<<2)|(((s)&1UL)<<1)|(((t)&1UL)<<0))

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONSTANT_COMMON_H */

/* end of file */
