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

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONSTANT_COMMON_H */

/* end of file */
