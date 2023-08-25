/*

Copyright (c) Bridgetek Pte Ltd

THIS SOFTWARE IS PROVIDED BY BRIDGETEK PTE LTD "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
BRIDGETEK PTE LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BRIDGETEK DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON BRIDGETEK PARTS.

BRIDGETEK DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Author : BRIDGETEK

Revision History:
0.1 - date 2013.04.24 - Initial Version
0.2 - date 2013.08.19 - added few registers.
1.0 - date 2013.12.10 - Version 1.0.
2.0 - date 2014.11.24 - version 2.0 which includes support for FT81x series.
2.1 - date 2017.03.24 - add DISPLAY_RESOLUTION definitions
3.0 - date 2017.05.31 - Update BT815 definitions wrt VC1 v1.5.1
3.1 - date 2017.05.31 - Update BT815 definitions wrt VC1 v1.5.2
*/

#ifndef _GPU_H_
#define _GPU_H_

/* Definitions used for FT800 co processor command buffer */
#define DL_SIZE                         (8*1024L)  //8KB Display List buffer size
#define CMD_FIFO_SIZE                   (4*1024L)  //4KB coprocessor Fifo size
#define CMD_SIZE                        (4)       //4 byte per coprocessor command of EVE

// New changess

/**************
** Addresses **
**************/

#define RAM_G                   0UL
#define ROM_CHIPID              786432UL
#define RAM_ERR_REPORT          0x309800UL

#if defined(FT_81X_ENABLE) || defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#define RAM_G_SIZE              (1024 * 1024L)
#elif defined(FT_80X_ENABLE) || defined(BT_88X_ENABLE)
#define RAM_G_SIZE              (256 * 1024L)
#endif

#if defined(FT_81X_ENABLE) || defined(BT_88X_ENABLE) || defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#define EVE_HAL_REG_ID          3153920UL
#define EVE_HAL_REG_CPURESET    3153952UL
#define EVE_HAL_REG_J1_INT      3154084UL
#define EVE_HAL_REG_CMD_READ    3154168UL
#define EVE_HAL_REG_TOUCH_TRANSFORM_A   3154256UL
#define EVE_HAL_REG_CRC         3154296UL
#define EVE_HAL_REG_TRIM        3154304UL
#define EVE_HAL_REG_TOUCH_DIRECT_XY     3154316UL
#define EVE_HAL_REG_DATESTAMP   3155300UL
#define EVE_HAL_REG_CMDB_SPACE  3155316UL
#define EVE_HAL_REG_TRACKER     3182592UL
#define RAM_DL                  3145728UL
#define ROMFONT_TABLEADDRESS    3145724UL
#define LOW_FREQ_BOUND          58800000L // 98% of 60Mhz
#define SCISSOR_XY_SHIFT        11
#define SCISSOR_XY_MASK         2047UL
#define SCISSOR_SIZE_SHIFT      12
#define SCISSOR_SIZE_MASK       4095UL
#endif

#if defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#define BITMAP_ADDR_MASK        16777215UL

#elif defined(FT_81X_ENABLE) || defined(BT_88X_ENABLE)
#define BITMAP_ADDR_MASK        4194303UL

#elif defined(FT_80X_ENABLE)
#define EVE_HAL_REG_ID          1057792UL
#define EVE_HAL_REG_CPURESET    1057820UL
#define EVE_HAL_REG_J1_INT      1057940UL
#define EVE_HAL_REG_CMD_READ    1058020UL
#define EVE_HAL_REG_TOUCH_TRANSFORM_A   1058076UL
#define EVE_HAL_REG_CRC         1058152UL
#define EVE_HAL_REG_TRIM        1058156UL
#define EVE_HAL_REG_TOUCH_DIRECT_XY     1058164UL
#define EVE_HAL_REG_DATESTAMP   1058108UL
#define EVE_HAL_REG_CMDB_SPACE  0
#define EVE_HAL_REG_TRACKER     1085440UL
#define RAM_DL                  1048576UL
#define ROMFONT_TABLEADDRESS    1048572UL
#define LOW_FREQ_BOUND          47040000L // 98% of 48Mhz
#define BITMAP_ADDR_MASK        1048575UL
#define SCISSOR_XY_SHIFT        9
#define SCISSOR_XY_MASK         511UL
#define SCISSOR_SIZE_SHIFT      10
#define SCISSOR_SIZE_MASK       1023UL
#endif

#define RAM_PAL                 (RAM_DL + 8192UL)
#define RAM_TOP                 (RAM_DL + 16384UL)
#define RAM_CMD                 (RAM_DL + 32768UL)
#define RAM_J1RAM               (RAM_DL + 36864UL)
#define RAM_ROMSUB              (RAM_DL + 40960UL)
#define RAM_JTBOOT              (RAM_DL + 45056UL)
#define RAM_BIST                (RAM_DL + 524288UL)
#define RAM_COMPOSITE           (RAM_DL + 794624UL)

#define RAM_REG                 (EVE_HAL_REG_ID + 0UL)
#define REG_ID                  (EVE_HAL_REG_ID + 0UL)
#define REG_FRAMES              (REG_ID + 4UL)
#define REG_CLOCK               (REG_ID + 8UL)
#define REG_FREQUENCY           (REG_ID + 12UL)
#define REG_RENDERMODE          (REG_ID + 16UL)
#define REG_SNAPY               (REG_ID + 20UL)
#define REG_SNAPSHOT            (REG_ID + 24UL)
#define REG_SNAPFORMAT          (REG_ID + 28UL)

#define REG_CPURESET            (EVE_HAL_REG_CPURESET + 0UL)
#define REG_TAP_CRC             (REG_CPURESET + 4UL)
#define REG_TAP_MASK            (REG_CPURESET + 8UL)
#define REG_HCYCLE              (REG_CPURESET + 12UL)
#define REG_HOFFSET             (REG_CPURESET + 16UL)
#define REG_HSIZE               (REG_CPURESET + 20UL)
#define REG_HSYNC0              (REG_CPURESET + 24UL)
#define REG_HSYNC1              (REG_CPURESET + 28UL)
#define REG_VCYCLE              (REG_CPURESET + 32UL)
#define REG_VOFFSET             (REG_CPURESET + 36UL)
#define REG_VSIZE               (REG_CPURESET + 40UL)
#define REG_VSYNC0              (REG_CPURESET + 44UL)
#define REG_VSYNC1              (REG_CPURESET + 48UL)
#define REG_DLSWAP              (REG_CPURESET + 52UL)
#define REG_ROTATE              (REG_CPURESET + 56UL)
#define REG_OUTBITS             (REG_CPURESET + 60UL)
#define REG_DITHER              (REG_CPURESET + 64UL)
#define REG_SWIZZLE             (REG_CPURESET + 68UL)
#define REG_CSPREAD             (REG_CPURESET + 72UL)
#define REG_PCLK_POL            (REG_CPURESET + 76UL)
#define REG_PCLK                (REG_CPURESET + 80UL)
#define REG_TAG_X               (REG_CPURESET + 84UL)
#define REG_TAG_Y               (REG_CPURESET + 88UL)
#define REG_TAG                 (REG_CPURESET + 92UL)
#define REG_VOL_PB              (REG_CPURESET + 96UL)
#define REG_VOL_SOUND           (REG_CPURESET + 100UL)
#define REG_SOUND               (REG_CPURESET + 104UL)
#define REG_PLAY                (REG_CPURESET + 108UL)
#define REG_GPIO_DIR            (REG_CPURESET + 112UL)
#define REG_GPIO                (REG_CPURESET + 116UL)
#define REG_GPIOX_DIR           (REG_CPURESET + 120UL)
#define REG_GPIOX               (REG_CPURESET + 124UL)
#define REG_J1_COLD             (REG_CPURESET + 128UL)

#define REG_J1_INT              (EVE_HAL_REG_J1_INT + 0UL)
#define REG_INT_FLAGS           (REG_J1_INT + 4UL)
#define REG_INT_EN              (REG_J1_INT + 8UL)
#define REG_INT_MASK            (REG_J1_INT + 12UL)
#define REG_PLAYBACK_START      (REG_J1_INT + 16UL)
#define REG_PLAYBACK_LENGTH     (REG_J1_INT + 20UL)
#define REG_PLAYBACK_READPTR    (REG_J1_INT + 24UL)
#define REG_PLAYBACK_FREQ       (REG_J1_INT + 28UL)
#define REG_PLAYBACK_FORMAT     (REG_J1_INT + 32UL)
#define REG_PLAYBACK_LOOP       (REG_J1_INT + 36UL)
#define REG_PLAYBACK_PLAY       (REG_J1_INT + 40UL)
#define REG_PWM_HZ              (REG_J1_INT + 44UL)
#define REG_PWM_DUTY            (REG_J1_INT + 48UL)
#define REG_MACRO_0             (REG_J1_INT + 52UL)
#define REG_MACRO_1             (REG_J1_INT + 56UL)
#define REG_CYA0                (REG_J1_INT + 60UL)
#define REG_CYA1                (REG_J1_INT + 64UL)
#define REG_BUSYBITS            (REG_J1_INT + 68UL)
#define REG_ROMSUB_SEL          (REG_J1_INT + 76UL)
#define REG_RAM_FOLD            (REG_J1_INT + 80UL)

#define REG_CMD_READ            (EVE_HAL_REG_CMD_READ + 0UL)
#define REG_CMD_WRITE           (REG_CMD_READ + 4UL)
#define REG_CMD_DL              (REG_CMD_READ + 8UL)
#define REG_TOUCH_MODE          (REG_CMD_READ + 12UL)
#define REG_CTOUCH_EXTENDED     (REG_CMD_READ + 16UL)
#define REG_TOUCH_ADC_MODE      (REG_CMD_READ + 16UL)
#define REG_EHOST_TOUCH_X       (REG_CMD_READ + 20UL)
#define REG_TOUCH_CHARGE        (REG_CMD_READ + 20UL)
#define REG_TOUCH_SETTLE        (REG_CMD_READ + 24UL)
#define REG_EHOST_TOUCH_ID      (REG_CMD_READ + 28UL)
#define REG_TOUCH_OVERSAMPLE    (REG_CMD_READ + 28UL)
#define REG_EHOST_TOUCH_Y       (REG_CMD_READ + 32UL)
#define REG_TOUCH_RZTHRESH      (REG_CMD_READ + 32UL)
#define REG_CTOUCH_TOUCH1_XY    (REG_CMD_READ + 36UL)
#define REG_TOUCH_RAW_XY        (REG_CMD_READ + 36UL)
#define REG_CTOUCH_IDS          (REG_CMD_READ + 40UL)
#define REG_CTOUCH_TOUCH4_Y     (REG_CMD_READ + 40UL)
#define REG_TOUCH_RZ            (REG_CMD_READ + 40UL)
#define REG_CTOUCH_TOUCH0_XY    (REG_CMD_READ + 44UL)
#define REG_TOUCH_SCREEN_XY     (REG_CMD_READ + 44UL)
#define REG_TOUCH_TAG_XY        (REG_CMD_READ + 48UL)
#define REG_TOUCH_TAG           (REG_CMD_READ + 52UL)
#define REG_TOUCH_TAG1_XY       (REG_CMD_READ + 56UL)
#define REG_TOUCH_TAG1          (REG_CMD_READ + 60UL)
#define REG_TOUCH_TAG2_XY       (REG_CMD_READ + 64UL)
#define REG_TOUCH_TAG2          (REG_CMD_READ + 68UL)
#define REG_TOUCH_TAG3_XY       (REG_CMD_READ + 72UL)
#define REG_TOUCH_TAG3          (REG_CMD_READ + 76UL)
#define REG_TOUCH_TAG4_XY       (REG_CMD_READ + 80UL)
#define REG_TOUCH_TAG4          (REG_CMD_READ + 84UL)

#define REG_TOUCH_TRANSFORM_A   (EVE_HAL_REG_TOUCH_TRANSFORM_A + 0UL)
#define REG_TOUCH_TRANSFORM_B   (REG_TOUCH_TRANSFORM_A + 4UL)
#define REG_TOUCH_TRANSFORM_C   (REG_TOUCH_TRANSFORM_A + 8UL)
#define REG_TOUCH_TRANSFORM_D   (REG_TOUCH_TRANSFORM_A + 12UL)
#define REG_TOUCH_TRANSFORM_E   (REG_TOUCH_TRANSFORM_A + 16UL)
#define REG_TOUCH_TRANSFORM_F   (REG_TOUCH_TRANSFORM_A + 20UL)
#define REG_TOUCH_CONFIG        (REG_TOUCH_TRANSFORM_A + 24UL)
#define REG_ANALOG              (REG_TOUCH_TRANSFORM_A + 28UL)
#define REG_CTOUCH_GESTURE      (REG_TOUCH_TRANSFORM_A + 28UL)
#define REG_CTOUCH_TOUCH4_X     (REG_TOUCH_TRANSFORM_A + 28UL)
#define REG_PATCHED_TOUCH_FAULT (REG_TOUCH_TRANSFORM_A + 28UL)
#define REG_EHOST_TOUCH_ACK     (REG_TOUCH_TRANSFORM_A + 32UL)
#define REG_PATCHED_ANALOG      (REG_TOUCH_TRANSFORM_A + 32UL)
#define REG_TOUCH_FAULT         (REG_TOUCH_TRANSFORM_A + 32UL)
#define REG_BIST_EN             (REG_TOUCH_TRANSFORM_A + 36UL)

#define REG_CRC                 (EVE_HAL_REG_CRC + 0UL)
#define REG_SPI_EARLY_TX        (REG_CRC + 4UL)

#define REG_TRIM                (EVE_HAL_REG_TRIM + 0UL)
#define REG_ANA_COMP            (REG_TRIM + 4UL)
#define REG_SPI_WIDTH           (REG_TRIM + 8UL)

#define REG_CTOUCH_TOUCH2_XY    (EVE_HAL_REG_TOUCH_DIRECT_XY + 0UL)
#define REG_TOUCH_DIRECT_XY     (REG_CTOUCH_TOUCH2_XY + 0UL)
#define REG_CTOUCH_TOUCH3_XY    (REG_CTOUCH_TOUCH2_XY + 4UL)
#define REG_TOUCH_DIRECT_Z1Z2   (REG_CTOUCH_TOUCH2_XY + 4UL)
#define REG_EJPG_READY          (REG_CTOUCH_TOUCH2_XY + 8UL)
#define REG_EJPG_BUSY           (REG_CTOUCH_TOUCH2_XY + 12UL)
#define REG_EJPG_DAT            (REG_CTOUCH_TOUCH2_XY + 16UL)
#define REG_EJPG_OPTIONS        (REG_CTOUCH_TOUCH2_XY + 20UL)
#define REG_EJPG_DST            (REG_CTOUCH_TOUCH2_XY + 24UL)
#define REG_EJPG_W              (REG_CTOUCH_TOUCH2_XY + 28UL)
#define REG_EJPG_H              (REG_CTOUCH_TOUCH2_XY + 32UL)
#define REG_EJPG_FORMAT         (REG_CTOUCH_TOUCH2_XY + 36UL)
#define REG_EJPG_RI             (REG_CTOUCH_TOUCH2_XY + 40UL)
#define REG_EJPG_TQ             (REG_CTOUCH_TOUCH2_XY + 44UL)
#define REG_EJPG_TDA            (REG_CTOUCH_TOUCH2_XY + 48UL)
#define REG_EJPG_Q              (REG_CTOUCH_TOUCH2_XY + 52UL)
#define REG_EJPG_HT             (REG_CTOUCH_TOUCH2_XY + 180UL)
#define REG_EJPG_DCC            (REG_CTOUCH_TOUCH2_XY + 436UL)
#define REG_EJPG_ACC            (REG_CTOUCH_TOUCH2_XY + 460UL)
#define REG_EJPG_SCALE          (REG_CTOUCH_TOUCH2_XY + 972UL)
#define REG_EJPG_DEBUG          (REG_CTOUCH_TOUCH2_XY + 976UL)
#define REG_RASTERY             (REG_CTOUCH_TOUCH2_XY + 980UL)
#define REG_DATESTAMP           (EVE_HAL_REG_DATESTAMP + 0UL)
#define REG_CMDB_SPACE          (EVE_HAL_REG_CMDB_SPACE + 0UL)
#define REG_CMDB_WRITE          (REG_CMDB_SPACE + 4UL)
#define REG_ADAPTIVE_FRAMERATE  (REG_CMDB_SPACE + 8UL)
#define REG_SPIM_DIR            (REG_CMDB_SPACE + 12UL)
#define REG_SPIM                (REG_CMDB_SPACE + 16UL)
#define REG_ESPIM_READSTART     (REG_CMDB_SPACE + 20UL)
#define REG_ESPIM_SEQ           (REG_CMDB_SPACE + 24UL)
#define REG_ESPIM_ADD           (REG_CMDB_SPACE + 40UL)
#define REG_ESPIM_COUNT         (REG_CMDB_SPACE + 44UL)
#define REG_ESPIM_WINDOW        (REG_CMDB_SPACE + 48UL)
#define REG_ESPIM_DUMMY         (REG_CMDB_SPACE + 112UL)
#define REG_ESPIM_TRIG          (REG_CMDB_SPACE + 116UL)
#define REG_PLAYBACK_PAUSE      (REG_CMDB_SPACE + 120UL)
#define REG_FLASH_STATUS        (REG_CMDB_SPACE + 124UL)
#define REG_FULLBUSYBITS        (REG_CMDB_SPACE + 128UL)
#define REG_SHA1KEY             (REG_CMDB_SPACE + 144UL)

#if defined(BT_81XA_ENABLE)
#define REG_UNDERRUN            (REG_CMDB_SPACE + 152UL)
#define REG_AH_HCYCLE_MAX       (REG_CMDB_SPACE + 156UL)
#define REG_PCLK_FREQ           (REG_CMDB_SPACE + 160UL)
#define REG_PCLK_2X             (REG_CMDB_SPACE + 164UL)
#define REG_FLASH_DTR           (REG_CMDB_SPACE + 168UL)
#define REG_ESPIM_DTR           (REG_CMDB_SPACE + 172UL)
#define REG_HSF_HSIZE           (REG_CMDB_SPACE + 176UL)
#define REG_HSF_FT1             (REG_CMDB_SPACE + 180UL)
#define REG_HSF_FSCALE          (REG_CMDB_SPACE + 184UL)
#define REG_HSF_F00             (REG_CMDB_SPACE + 188UL)
#define REG_HSF_F02             (REG_CMDB_SPACE + 192UL)
#define REG_HSF_F03             (REG_CMDB_SPACE + 196UL)
#define REG_HSF_F10             (REG_CMDB_SPACE + 200UL)
#define REG_HSF_F11             (REG_CMDB_SPACE + 204UL)
#define REG_HSF_F12             (REG_CMDB_SPACE + 208UL)
#define REG_HSF_F13             (REG_CMDB_SPACE + 212UL)
#endif

#define REG_TRACKER             (EVE_HAL_REG_TRACKER + 0UL)
#define REG_TRACKER_1           (REG_TRACKER + 4UL)
#define REG_TRACKER_2           (REG_TRACKER + 8UL)
#define REG_TRACKER_3           (REG_TRACKER + 12UL)
#define REG_TRACKER_4           (REG_TRACKER + 16UL)
#define REG_MEDIAFIFO_READ      (REG_TRACKER + 20UL)
#define REG_MEDIAFIFO_WRITE     (REG_TRACKER + 24UL)
#define REG_MEDIAFIFO_BASE      (REG_TRACKER + 28UL)
#define REG_MEDIAFIFO_SIZE      (REG_TRACKER + 32UL)
#define REG_FLASH_SIZE          (REG_TRACKER + 36UL)
#define REG_ANIM_ACTIVE         (REG_TRACKER + 44UL)
#define REG_DF_TUNED            (REG_TRACKER + 48UL)
#define REG_PLAY_CONTROL        (REG_TRACKER + 334UL)

#define REG_CTOUCH_TOUCHA_XY    REG_CTOUCH_TOUCH1_XY
#define REG_CTOUCH_TOUCHB_XY    REG_CTOUCH_TOUCH2_XY
#define REG_CTOUCH_TOUCHC_XY    REG_CTOUCH_TOUCH3_XY
#define REG_CTOUCH_TOUCHD_X     REG_CTOUCH_TOUCH4_X
#define REG_CTOUCH_TOUCHD_Y     REG_CTOUCH_TOUCH4_Y

#define EVE_GPUDEFS_IMPLEMENT   REG_ID,                \
                                REG_CPURESET,          \
                                REG_J1_INT,            \
                                REG_CMD_READ,          \
                                REG_TOUCH_TRANSFORM_A, \
                                REG_CRC,               \
                                REG_TRIM,              \
                                REG_TOUCH_DIRECT_XY,   \
                                REG_DATESTAMP,         \
                                REG_CMDB_SPACE,        \
                                REG_TRACKER,           \
                                RAM_DL,                \
                                ROMFONT_TABLEADDRESS,  \
                                RAM_G_SIZE,            \
                                LOW_FREQ_BOUND,        \
                                BITMAP_ADDR_MASK,      \
                                SCISSOR_SIZE_SHIFT

/*************
** Commands **
*************/

// FT800
#define CMD_DLSTART             4294967040UL
#define CMD_SWAP                4294967041UL
#define CMD_INTERRUPT           4294967042UL
#define CMD_CRC                 4294967043UL
#define CMD_HAMMERAUX           4294967044UL
#define CMD_MARCH               4294967045UL
#define CMD_EXECUTE             4294967047UL
#define CMD_GETPOINT            4294967048UL
#define CMD_BGCOLOR             4294967049UL
#define CMD_FGCOLOR             4294967050UL
#define CMD_GRADIENT            4294967051UL
#define CMD_TEXT                4294967052UL
#define CMD_BUTTON              4294967053UL
#define CMD_KEYS                4294967054UL
#define CMD_PROGRESS            4294967055UL
#define CMD_SLIDER              4294967056UL
#define CMD_SCROLLBAR           4294967057UL
#define CMD_TOGGLE              4294967058UL
#define CMD_GAUGE               4294967059UL
#define CMD_CLOCK               4294967060UL
#define CMD_CALIBRATE           4294967061UL
#define CMD_SPINNER             4294967062UL
#define CMD_STOP                4294967063UL
#define CMD_MEMCRC              4294967064UL
#define CMD_REGREAD             4294967065UL
#define CMD_MEMWRITE            4294967066UL
#define CMD_MEMSET              4294967067UL
#define CMD_MEMZERO             4294967068UL
#define CMD_MEMCPY              4294967069UL
#define CMD_APPEND              4294967070UL
#define CMD_SNAPSHOT            4294967071UL
#define CMD_BITMAP_TRANSFORM    4294967073UL
#define CMD_TOUCH_TRANSFORM     4294967072UL
#define CMD_INFLATE             4294967074UL
#define CMD_GETPTR              4294967075UL
#define CMD_LOADIMAGE           4294967076UL
#define CMD_GETPROPS            4294967077UL
#define CMD_LOADIDENTITY        4294967078UL
#define CMD_TRANSLATE           4294967079UL
#define CMD_SCALE               4294967080UL
#define CMD_ROTATE              4294967081UL
#define CMD_SETMATRIX           4294967082UL
#define CMD_SETFONT             4294967083UL
#define CMD_TRACK               4294967084UL
#define CMD_DIAL                4294967085UL
#define CMD_NUMBER              4294967086UL
#define CMD_SCREENSAVER         4294967087UL
#define CMD_SKETCH              4294967088UL
#define CMD_LOGO                4294967089UL
#define CMD_COLDSTART           4294967090UL
#define CMD_GETMATRIX           4294967091UL
#define CMD_GRADCOLOR           4294967092UL

// FT801
#if defined(FT_80X_ENABLE)
#define CMD_CSKETCH             4294967093UL
#endif

// FT810
#if defined(FT_81X_ENABLE) || defined(BT_88X_ENABLE) || defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#define CMD_SETROTATE           4294967094UL
#define CMD_SNAPSHOT2           4294967095UL
#define CMD_SETBASE             4294967096UL
#define CMD_MEDIAFIFO           4294967097UL
#define CMD_PLAYVIDEO           4294967098UL
#define CMD_SETFONT2            4294967099UL
#define CMD_SETSCRATCH          4294967100UL
#define CMD_ROMFONT             4294967103UL
#define CMD_VIDEOSTART          4294967104UL
#define CMD_VIDEOFRAME          4294967105UL
#define CMD_SYNC                4294967106UL
#define CMD_SETBITMAP           4294967107UL
#endif

// BT815
#if defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#define CMD_FLASHERASE          4294967108UL
#define CMD_FLASHWRITE          4294967109UL
#define CMD_FLASHREAD           4294967110UL
#define CMD_FLASHUPDATE         4294967111UL
#define CMD_FLASHDETACH         4294967112UL
#define CMD_FLASHATTACH         4294967113UL
#define CMD_FLASHFAST           4294967114UL
#define CMD_FLASHSPIDESEL       4294967115UL
#define CMD_FLASHSPITX          4294967116UL
#define CMD_FLASHSPIRX          4294967117UL
#define CMD_FLASHSOURCE         4294967118UL
#define CMD_CLEARCACHE          4294967119UL
#define CMD_INFLATE2            4294967120UL
#define CMD_ROTATEAROUND        4294967121UL
#define CMD_RESETFONTS          4294967122UL
#define CMD_ANIMSTART           4294967123UL
#define CMD_ANIMSTOP            4294967124UL
#define CMD_ANIMXY              4294967125UL
#define CMD_ANIMDRAW            4294967126UL
#define CMD_GRADIENTA           4294967127UL
#define CMD_FILLWIDTH           4294967128UL
#define CMD_APPENDF             4294967129UL
#define CMD_ANIMFRAME           4294967130UL
#define CMD_NOP                 4294967131UL
#define CMD_VIDEOSTARTF         4294967135UL
#endif

// BT817
#if defined(BT_81XA_ENABLE)
#define CMD_CALIBRATESUB        4294967136UL
#define CMD_TESTCARD            4294967137UL
#define CMD_HSF                 4294967138UL
#define CMD_APILEVEL            4294967139UL
#define CMD_GETIMAGE            4294967140UL
#define CMD_WAIT                4294967141UL
#define CMD_RETURN              4294967142UL
#define CMD_CALLLIST            4294967143UL
#define CMD_NEWLIST             4294967144UL
#define CMD_ENDLIST             4294967145UL
#define CMD_PCLKFREQ            4294967146UL
#define CMD_FONTCACHE           4294967147UL
#define CMD_FONTCACHEQUERY      4294967148UL
#define CMD_ANIMFRAMERAM        4294967149UL
#define CMD_ANIMSTARTRAM        4294967150UL
#define CMD_RUNANIM             4294967151UL
#define CMD_FLASHPROGRAM        4294967152UL
#endif

/*****************
** Display List **
*****************/

#define VERTEX2F(x, y) ((1UL << 30) | (((x)&32767UL) << 15) | (((y)&32767UL) << 0))
#define VERTEX2II(x, y, handle, cell) ((2UL << 30) | (((x)&511UL) << 21) | (((y)&511UL) << 12) | (((handle)&31UL) << 7) | (((cell)&127UL) << 0))
#define BITMAP_SOURCE(addr) ((1UL << 24) | ((addr) < 0 ? (((addr) & (BITMAP_ADDR_MASK >> 1))) : ((addr)&BITMAP_ADDR_MASK)))
#define BITMAP_SOURCE2(flash_or_ram, addr) ((1UL << 24) | ((flash_or_ram) << 23) | (((addr) & (BITMAP_ADDR_MASK >> 1)) << 0))
#define CLEAR_COLOR_RGB(red, green, blue) ((2UL << 24) | (((red)&255UL) << 16) | (((green)&255UL) << 8) | (((blue)&255UL) << 0))
#define TAG(s) ((3UL << 24) | (((s)&255UL) << 0))
#define COLOR_RGB(red, green, blue) ((4UL << 24) | (((red)&255UL) << 16) | (((green)&255UL) << 8) | (((blue)&255UL) << 0))
#define BITMAP_HANDLE(handle) ((5UL << 24) | (((handle)&31UL) << 0))
#define CELL(cell) ((6UL << 24) | (((cell)&127UL) << 0))
#define BITMAP_LAYOUT(format, linestride, height) ((7UL << 24) | (((format)&31UL) << 19) | (((linestride)&1023UL) << 9) | (((height)&511UL) << 0))
#define BITMAP_SIZE(filter, wrapx, wrapy, width, height) ((8UL << 24) | (((filter)&1UL) << 20) | (((wrapx)&1UL) << 19) | (((wrapy)&1UL) << 18) | (((width)&511UL) << 9) | (((height)&511UL) << 0))
#define ALPHA_FUNC(func, ref) ((9UL << 24) | (((func)&7UL) << 8) | (((ref)&255UL) << 0))
#define STENCIL_FUNC(func, ref, mask) ((10UL << 24) | (((func)&7UL) << 16) | (((ref)&255UL) << 8) | (((mask)&255UL) << 0))
#define BLEND_FUNC(src, dst) ((11UL << 24) | (((src)&7UL) << 3) | (((dst)&7UL) << 0))
#define STENCIL_OP(sfail, spass) ((12UL << 24) | (((sfail)&7UL) << 3) | (((spass)&7UL) << 0))
#define POINT_SIZE(size) ((13UL << 24) | (((size)&8191UL) << 0))
#define LINE_WIDTH(width) ((14UL << 24) | (((width)&4095UL) << 0))
#define CLEAR_COLOR_A(alpha) ((15UL << 24) | (((alpha)&255UL) << 0))
#define COLOR_A(alpha) ((16UL << 24) | (((alpha)&255UL) << 0))
#define CLEAR_STENCIL(s) ((17UL << 24) | (((s)&255UL) << 0))
#define CLEAR_TAG(s) ((18UL << 24) | (((s)&255UL) << 0))
#define STENCIL_MASK(mask) ((19UL << 24) | (((mask)&255UL) << 0))
#define TAG_MASK(mask) ((20UL << 24) | (((mask)&1UL) << 0))
#define BITMAP_TRANSFORM_C(c) ((23UL << 24) | (((c)&16777215UL) << 0))
#define BITMAP_TRANSFORM_F(f) ((26UL << 24) | (((f)&16777215UL) << 0))
#define SCISSOR_XY(x, y) ((27UL << 24) | (((x)&SCISSOR_XY_MASK) << SCISSOR_XY_SHIFT) | (((y)&SCISSOR_XY_MASK) << 0))
#define SCISSOR_SIZE(width, height) ((28UL << 24) | (((width)&SCISSOR_SIZE_MASK) << SCISSOR_SIZE_SHIFT) | (((height)&SCISSOR_SIZE_MASK) << 0))
#define CALL(dest) ((29UL << 24) | (((dest)&65535UL) << 0))
#define JUMP(dest) ((30UL << 24) | (((dest)&65535UL) << 0))
#define BEGIN(prim) ((31UL << 24) | (((prim)&15UL) << 0))
#define COLOR_MASK(r, g, b, a) ((32UL << 24) | (((r)&1UL) << 3) | (((g)&1UL) << 2) | (((b)&1UL) << 1) | (((a)&1UL) << 0))
#define CLEAR(c, s, t) ((38UL << 24) | (((c)&1UL) << 2) | (((s)&1UL) << 1) | (((t)&1UL) << 0))
#define VERTEX_FORMAT(frac) ((39UL << 24) | (((frac)&7UL) << 0))
#define BITMAP_LAYOUT_H(linestride, height) ((40UL << 24) | (((linestride)&3UL) << 2) | (((height)&3UL) << 0))
#define BITMAP_SIZE_H(width, height) ((41UL << 24) | (((width)&3UL) << 2) | (((height)&3UL) << 0))
#define PALETTE_SOURCE(addr) ((42UL << 24) | (((addr)&4194303UL) << 0))
#define VERTEX_TRANSLATE_X(x) ((43UL << 24) | (((x)&131071UL) << 0))
#define NOP() ((45UL << 24))
#define BITMAP_EXT_FORMAT(format) ((46UL << 24) | (((format)&65535UL) << 0))
#define BITMAP_SWIZZLE(r, g, b, a) ((47UL << 24) | (((r)&7UL) << 9) | (((g)&7UL) << 6) | (((b)&7UL) << 3) | (((a)&7UL) << 0))
#define INT_FRR() ((48UL << 24))
#define END() ((33UL << 24))
#define SAVE_CONTEXT() ((34UL << 24))
#define RESTORE_CONTEXT() ((35UL << 24))
#define RETURN() ((36UL << 24))
#define MACRO(m) ((37UL << 24) | (((m)&1UL) << 0))
#define DISPLAY() ((0UL << 24))

/************
** Options **
************/

#ifdef POINTS
#undef POINTS
#endif

#define CMDBUF_SIZE             4096UL

#define DLSWAP_DONE             0UL
#define DLSWAP_LINE             1UL
#define DLSWAP_FRAME            2UL

#define OPT_MONO                1UL
#define OPT_NODL                2UL
#define OPT_FLAT                256UL
#define OPT_SIGNED              256UL
#define OPT_CENTERX             512UL
#define OPT_CENTERY             1024UL
#define OPT_CENTER              1536UL
#define OPT_NOBACK              4096UL
#define OPT_NOHM                16384UL
#define OPT_NOPOINTER           16384UL
#define OPT_NOSECS              32768UL
#define OPT_NOHANDS             49152UL
#define OPT_NOTICKS             8192UL
#define OPT_RIGHTX              2048UL

#define KEEP                    1UL
#define REPLACE                 2UL
#define INCR                    3UL
#define DECR                    4UL
#define INVERT                  5UL

#define ZERO                    0UL
#define ONE                     1UL
#define SRC_ALPHA               2UL
#define DST_ALPHA               3UL
#define ONE_MINUS_SRC_ALPHA     4UL
#define ONE_MINUS_DST_ALPHA     5UL

#define NEVER                   0UL
#define LESS                    1UL
#define LEQUAL                  2UL
#define GREATER                 3UL
#define GEQUAL                  4UL
#define EQUAL                   5UL
#define NOTEQUAL                6UL
#define ALWAYS                  7UL

#define ARGB1555                0UL
#define L1                      1UL
#define L4                      2UL
#define L8                      3UL
#define RGB332                  4UL
#define ARGB2                   5UL
#define ARGB4                   6UL
#define RGB565                  7UL
#define PALETTED                8UL
#define TEXT8X8                 9UL
#define TEXTVGA                 10UL
#define BARGRAPH                11UL

#define NEAREST                 0UL
#define BILINEAR                1UL

#define LINEAR_SAMPLES          0UL
#define ULAW_SAMPLES            1UL
#define ADPCM_SAMPLES           2UL
        
#define BITMAPS                 1UL
#define POINTS                  2UL
#define LINES                   3UL
#define LINE_STRIP              4UL
#define EDGE_STRIP_R            5UL
#define EDGE_STRIP_L            6UL
#define EDGE_STRIP_A            7UL
#define EDGE_STRIP_B            8UL
#define RECTS                   9UL

#define FTPOINTS                POINTS

#define TOUCHMODE_OFF           0UL
#define TOUCHMODE_ONESHOT       1UL
#define TOUCHMODE_FRAME         2UL
#define TOUCHMODE_CONTINUOUS    3UL

#define BORDER                  0UL
#define REPEAT                  1UL

#define ADC_SINGLE_ENDED        0UL
#define ADC_DIFFERENTIAL        1UL

#define CTOUCH_MODE_COMPATIBILITY   1UL
#define CTOUCH_MODE_EXTENDED        0UL

#define INT_SWAP                1UL
#define INT_TOUCH               2UL
#define INT_TAG                 4UL
#define INT_SOUND               8UL
#define INT_PLAYBACK            16UL
#define INT_CMDEMPTY            32UL
#define INT_CMDFLAG             64UL
#define INT_CONVCOMPLETE        128UL
// #define INT_UNDERRUN        256UL

// End new changes

#if defined(FT80X_ENABLE)
#define FT800_VERSION           "1.9.0".
#define CMD_IDCT                4294967046UL
#define DECR_WRAP               7UL
#define INCR_WRAP               6UL
#define REG_CYA_TOUCH           1058100UL
#define BITMAP_TRANSFORM_A(a)   ((21UL<<24)|(((a)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(b)   ((22UL<<24)|(((b)&131071UL)<<0))
#define BITMAP_TRANSFORM_D(d)   ((24UL<<24)|(((d)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(e)   ((25UL<<24)|(((e)&131071UL)<<0))

#elif defined(FT81X_ENABLE)
#define FT81X_VERSION           "1.0.4"
#define CMD_IDCT_DELETED        4294967046UL
#define CMD_INT_RAMSHARED       4294967101UL
#define CMD_INT_SWLOADIMAGE     4294967102UL
#define INT_G8                  18UL
#define INT_L8C                 12UL
#define INT_VGA                 13UL
#define L2                      17UL
#define OPT_NOTEAR              4UL
#define OPT_FULLSCREEN          8UL
#define OPT_MEDIAFIFO           16UL
#define OPT_SOUND               32UL
#define PALETTED565             14UL
#define PALETTED4444            15UL
#define PALETTED8               16UL
//modified below to reflect the same as datasheet
#define REG_CYA_TOUCH           3154280UL
#define BITMAP_TRANSFORM_A(a)   ((21UL<<24)|(((a)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(b)   ((22UL<<24)|(((b)&131071UL)<<0))
#define BITMAP_TRANSFORM_D(d)   ((24UL<<24)|(((d)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(e)   ((25UL<<24)|(((e)&131071UL)<<0))

#elif defined(BT81X_ENABLE) || defined(BT81XA_ENABLE)
#define VC1_VERSION             "1.5.8"
#define FLASH_DEFAULT_SHA1KEY   0xf589cf07
#define PPC                     16
#define BANKS                   16
#define BANKW                   16
#define ANIM_ONCE               0UL
#define ANIM_LOOP               1UL
#define ANIM_HOLD               2UL
#define CMD_DEPRECATED_CSKETCH  4294967093UL
#define CMD_HMAC                4294967133UL
#define CMD_IDCT_DELETED        4294967046UL
#define CMD_INT_RAMSHARED       4294967101UL
#define CMD_INT_SWLOADIMAGE     4294967102UL
#define CMD_LAST_               4294967134UL
#define CMD_SHA1                4294967132UL
#define COMPRESSED_RGBA_ASTC_4x4_KHR    37808UL
#define COMPRESSED_RGBA_ASTC_5x4_KHR    37809UL
#define COMPRESSED_RGBA_ASTC_5x5_KHR    37810UL
#define COMPRESSED_RGBA_ASTC_6x5_KHR    37811UL
#define COMPRESSED_RGBA_ASTC_6x6_KHR    37812UL
#define COMPRESSED_RGBA_ASTC_8x5_KHR    37813UL
#define COMPRESSED_RGBA_ASTC_8x6_KHR    37814UL
#define COMPRESSED_RGBA_ASTC_8x8_KHR    37815UL
#define COMPRESSED_RGBA_ASTC_10x5_KHR   37816UL
#define COMPRESSED_RGBA_ASTC_10x6_KHR   37817UL
#define COMPRESSED_RGBA_ASTC_10x8_KHR   37818UL
#define COMPRESSED_RGBA_ASTC_10x10_KHR  37819UL
#define COMPRESSED_RGBA_ASTC_12x10_KHR  37820UL
#define COMPRESSED_RGBA_ASTC_12x12_KHR  37821UL
#define FLASH_STATUS_INIT       0UL
#define FLASH_STATUS_DETACHED   1UL
#define FLASH_STATUS_BASIC      2UL
#define FLASH_STATUS_FULL       3UL
#define GLFORMAT                31UL
#define INT_G8                  18UL
#define INT_L8C                 12UL
#define INT_VGA                 13UL
#define JT_CYA                  9UL
#define JT_DRIVES               4UL
#define JT_FRAMES               6UL
#define JT_ONESHOT              7UL
#define JT_PEN                  5UL
#define JT_PWM                  8UL
#define JT_RI_ADC_MODE          14UL
#define JT_RI_CHARGE            13UL
#define JT_RI_CTOUCH_EXTENDED   14UL
#define JT_RI_EHOST_ID          16UL
#define JT_RI_EHOST_X           13UL
#define JT_RI_EHOST_Y           17UL
#define JT_RI_INT               45UL
#define JT_RI_MODE              12UL
#define JT_RI_OVERSAMPLE        16UL
#define JT_RI_RZTHRESH          17UL
#define JT_RI_SETTLE            15UL
#define JT_RI_SHORTS            31UL
#define JT_RI_UART_BUSY         30UL
#define JT_RI_XF_AH             19UL
#define JT_RI_XF_AL             18UL
#define JT_RI_XF_BH             21UL
#define JT_RI_XF_BL             20UL
#define JT_RI_XF_CH             23UL
#define JT_RI_XF_CL             22UL
#define JT_RI_XF_DH             25UL
#define JT_RI_XF_DL             24UL
#define JT_RI_XF_EH             27UL
#define JT_RI_XF_EL             26UL
#define JT_RI_XF_FH             29UL
#define JT_RI_XF_FL             28UL
#define JT_RO_ANALOG            37UL
#define JT_RO_CTOUCH_TOUCH0_XY  34UL
#define JT_RO_CTOUCH_TOUCH4X    37UL
#define JT_RO_CTOUCH_TOUCH4Y    33UL
#define JT_RO_CTOUCH_TOUCHA_XY  32UL
#define JT_RO_CTOUCH_TOUCHB_XY  35UL
#define JT_RO_CTOUCH_TOUCHC_XY  36UL
#define JT_RO_DIRECT_XY         35UL
#define JT_RO_DIRECT_Z1Z2       36UL
#define JT_RO_EHOST_ACK         41UL
#define JT_RO_FAULT             41UL
#define JT_RO_INTERRUPT         39UL
#define JT_RO_RAW_XY            32UL
#define JT_RO_RZ                33UL
#define JT_RO_SCREEN_XY         34UL
#define JT_RO_SHORT_DETECT      40UL
#define JT_RO_UART              38UL
#define JT_RO_WAKE              44UL
#define JT_R_SCRATCH            11UL
#define JT_SCL                  42UL
#define JT_SDA                  43UL
#define JT_TICK                 10UL
#define JT_XD                   3UL
#define JT_XEOC                 2UL
#define JT_XMCLK                0UL
#define JT_XSAMP_CLK            1UL
#define L2                      17UL
#define OPT_NOTEAR              4UL
#define OPT_FULLSCREEN          8UL
#define OPT_MEDIAFIFO           16UL
#define OPT_SOUND               32UL
#define OPT_FLASH               64UL
#define OPT_OVERLAY             128UL
#define OPT_DITHER              256UL
#define OPT_FORMAT              4096UL
#define OPT_FILL                8192UL
#define PALETTED565             14UL
#define PALETTED4444            15UL
#define PALETTED8               16UL
#define RED                     2UL
#define GREEN                   3UL
#define BLUE                    4UL
#define ALPHA                   5UL
#define REG_CYA_TOUCH           3154280UL
#define REG_PLAY_CONTROL        3182926UL
#define SS_A0                   19UL
#define SS_A1                   20UL
#define SS_A2                   21UL
#define SS_A3                   22UL
#define SS_A4                   23UL
#define SS_A5                   24UL
#define SS_A6                   25UL
#define SS_A7                   26UL
#define SS_PAUSE                18UL
#define SS_Q0                   0UL
#define SS_Q1                   1UL
#define SS_Q2                   2UL
#define SS_Q3                   3UL
#define SS_Q4                   4UL
#define SS_Q5                   5UL
#define SS_Q6                   6UL
#define SS_Q7                   7UL
#define SS_Q8                   8UL
#define SS_Q9                   9UL
#define SS_QA                   10UL
#define SS_QB                   11UL
#define SS_QC                   12UL
#define SS_QD                   13UL
#define SS_QE                   14UL
#define SS_QF                   15UL
#define SS_QI                   31UL
#define SS_S0                   16UL
#define SS_S1                   17UL
#define ARGB8_SNAPSHOT          0x20
#define BITMAP_TRANSFORM_A(p,v) ((21UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(p,v) ((22UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_D(p,v) ((24UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(p,v) ((25UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#undef BITMAP_TRANSFORM_A //New BT815 file do not compatible with the legacy code
#undef BITMAP_TRANSFORM_B //New BT815 file do not compatible with the legacy code
#undef BITMAP_TRANSFORM_D //New BT815 file do not compatible with the legacy code
#undef BITMAP_TRANSFORM_E //New BT815 file do not compatible with the legacy code
#define BITMAP_TRANSFORM_A_EXT(p,v) ((21UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_B_EXT(p,v) ((22UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_D_EXT(p,v) ((24UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
#define BITMAP_TRANSFORM_E_EXT(p,v) ((25UL<<24)|(((p)&1UL)<<17)|(((v)&131071UL)<<0))
/* Added to ensure previous macros are fine */
#define BITMAP_TRANSFORM_A(a)       BITMAP_TRANSFORM_A_EXT(0,a)
#define BITMAP_TRANSFORM_B(b)       BITMAP_TRANSFORM_B_EXT(0,b)
#define BITMAP_TRANSFORM_D(d)       BITMAP_TRANSFORM_D_EXT(0,d)
#define BITMAP_TRANSFORM_E(e)       BITMAP_TRANSFORM_E_EXT(0,e)

#else
#error "Please select a proper chip in Platform.h"
#endif 

/* ==================================================================================== */

#define GPU_NUMCHAR_PERFONT (128)
#define GPU_FONT_TABLE_SIZE (148)

/* FT81x and FT80x font table structure */
/* Font table address in ROM can be found by reading the address from 0xFFFFC location. */
/* 16 font tables are present at the address read from location 0xFFFFC */
typedef struct Gpu_Fonts
{
	/* All the values are in bytes */
	/* Width of each character font from 0 to 127 */
	uint8_t    FontWidth[GPU_NUMCHAR_PERFONT];
	/* Bitmap format of font wrt bitmap formats supported by FT800 - L1, L4, L8 */
	uint32_t    FontBitmapFormat;
	/* Font line stride in FT800 ROM */
	uint32_t    FontLineStride;
	/* Font width in pixels */
	uint32_t    FontWidthInPixels;
	/* Font height in pixels */
	uint32_t    FontHeightInPixels;
	/* Pointer to font graphics raw data */
	uint32_t    PointerToFontGraphicsData;
}Gpu_Fonts_t;

/* BT815 inclusion - extended font table for unicode support */
typedef struct Gpu_FontsExt_t
{
	uint32_t Signature;
	uint32_t Size;
	uint32_t Format;
	uint32_t Swizzle;
	uint32_t LayoutWidth;
	uint32_t LayoutHeight;
	uint32_t PixelWidth;
	uint32_t pixelHeight;
	uint32_t StartOfGraphicData;
	uint32_t OffsetGlyphData;
	//uint32_t OffsetWidthData;//to be OffsetWidthData=Starting adress+StartOfGraphicData*4.
}Gpu_FontsExt_t;

#if defined(DISPLAY_RESOLUTION_WXGA)
/* Values specific to QVGA LCD display */
#define DispWidth      1280L
#define DispHeight     800L
#define DispHCycle     1411L
#define DispHOffset    120L
#define DispHSync0     0L
#define DispHSync1     100L
#define DispVCycle     815L
#define DispVOffset    14L
#define DispVSync0     0L
#define DispVSync1     10L
#define DispPCLK       1
#define DispSwizzle    0
#define DispPCLKPol    0
#define DispCSpread    0
#define DispDither     0

#define DispOutbit     0
#define DispAdaptiveFrameRate     0
#define DispPCLK2X     0
#define DispPCLKFreq   0x8B1

#elif defined(DISPLAY_RESOLUTION_WSVGA)
/* Values specific to QVGA LCD display */
#define DispWidth      1024L
#define DispHeight     600L
#define DispHCycle     1344L
#define DispHOffset    160L
#define DispHSync0     0L
#define DispHSync1     100L
#define DispVCycle     635L
#define DispVOffset    23L
#define DispVSync0     0L
#define DispVSync1     10L
#define DispPCLK       1
#define DispSwizzle    0
#define DispPCLKPol    1
#define DispCSpread    0
#define DispDither     1

#define DispOutbit     0
#define DispAdaptiveFrameRate     0
#define DispPCLK2X     0
#define DispPCLKFreq   0xD12
    
#elif defined(DISPLAY_RESOLUTION_QVGA)
/* Values specific to QVGA LCD display */
#define DispWidth      320L
#define DispHeight     240L
#define DispHCycle     408L
#define DispHOffset    70L
#define DispHSync0     0L
#define DispHSync1     10L
#define DispVCycle     263L
#define DispVOffset    13L
#define DispVSync0     0L
#define DispVSync1     2L
#define DispPCLK       8
#define DispSwizzle    2
#define DispPCLKPol    0
#define DispCSpread    1
#define DispDither     1

#elif defined(DISPLAY_RESOLUTION_WVGA)
/* Values specific to QVGA LCD display */
#define DispWidth      800L
#define DispHeight     480L
#define DispHCycle     928L
#define DispHOffset    88L
#define DispHSync0     0L
#define DispHSync1     48L
#define DispVCycle     525L
#define DispVOffset    32L
#define DispVSync0     0L
#define DispVSync1     3L
#define DispPCLK       2
#define DispSwizzle    0
#define DispPCLKPol    1
#define DispCSpread    0
#define DispDither     1
#elif defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
/* Values specific to HVGA LCD display */
#define DispWidth      320L
#define DispHeight     480L
#define DispHCycle     400L
#define DispHOffset    40L
#define DispHSync0     0L
#define DispHSync1     10L
#define DispVCycle     500L
#define DispVOffset    10L
#define DispVSync0     0L
#define DispVSync1     5L
#define DispPCLK       4
#define DispSwizzle    2
#define DispPCLKPol    1
#define DispCSpread    1
#define DispDither     1
#else
/* Default is WQVGA - 480x272 */
#define DispWidth     480L
#define DispHeight    272L
#define DispHCycle    548L
#define DispHOffset   43L
#define DispHSync0    0L
#define DispHSync1    41L
#define DispVCycle    292L
#define DispVOffset   12L
#define DispVSync0    0L
#define DispVSync1    10L
#define DispPCLK      5
#define DispSwizzle   0
#define DispPCLKPol   1
#define DispCSpread   1
#define DispDither    1
#endif
#ifdef ME810A_HV35R
#define DispPCLK     5
#endif



#endif /* #ifndef _GPU_H_ */

/* Nothing beyond this */
