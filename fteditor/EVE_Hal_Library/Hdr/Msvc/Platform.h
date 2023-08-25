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

Abstract:

This file contains is functions for all UI fields.

Author : BRIDGETEK

Revision History:
0.1 - date 2013.04.24 - initial version
0.2 - date 2014.04.28 - Split in individual files according to platform
0.3 - date 2020.09.22 - Support the new board ME817EV
*/

#pragma warning(disable: 4005)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

/* platform specific macros */
#define MSVC_PLATFORM                           (1)    // enable by default for MSVC platform

//#define VM816C50A_MPSSE                       (1)
//#define VM816C50A_LIBFT4222                   (1)
//#define ME817EV_MPSSE                         (1)
//#define ME817EV_LIBFT4222                     (1)

#ifdef VM816C50A_MPSSE 
/* Define all the macros specific to VM815 module */
#define BT816_ENABLE                            (1)
#define ENABLE_SPI_SINGLE                       (1)
#define DISPLAY_RESOLUTION_WVGA                 (1)
#define RESISTANCE_THRESHOLD                    (1800)
#define MSVC_PLATFORM_SPI_LIBMPSSE              (1)
#endif

#ifdef VM816C50A_LIBFT4222
/* Define all the macros specific to VM816 module */
#define BT816_ENABLE                            (1)
#define ENABLE_SPI_QUAD                         (1)
#define DISPLAY_RESOLUTION_WVGA                 (1)
#define RESISTANCE_THRESHOLD                    (1800)
#define MSVC_PLATFORM_SPI_LIBFT4222             (1)
#endif
    
#ifdef ME817EV_MPSSE 
/* Define all the macros specific to VM815 module */
#define BT817_ENABLE
#define ENABLE_SPI_SINGLE                       (1)
//#define DISPLAY_RESOLUTION_WSVGA                (1)
//#define DISPLAY_RESOLUTION_WXGA                 (1)
#define RESISTANCE_THRESHOLD                    (1800)
#define MSVC_PLATFORM_SPI_LIBMPSSE              (1)
#endif

#ifdef ME817EV_LIBFT4222
/* Define all the macros specific to VM816 module */
#define BT817_ENABLE
#define ENABLE_SPI_QUAD                         (1)
//#define DISPLAY_RESOLUTION_WSVGA                (1)
//#define DISPLAY_RESOLUTION_WXGA                 (1)
#define RESISTANCE_THRESHOLD                    (1800)
#define MSVC_PLATFORM_SPI_LIBFT4222             (1)
#endif

/// Re-Mapping FT800 Series to FT80X
#if defined(FT800_ENABLE) || defined(FT801_ENABLE)
#define FT80X_ENABLE
#endif

/// Re-Mapping FT810 Series to FT81X
#if defined(FT810_ENABLE) || defined(FT811_ENABLE) || defined(FT812_ENABLE) || defined(FT813_ENABLE)
#define FT81X_ENABLE
#endif

/// Re-Mapping BT880 Series to BT88X
#if defined(BT880_ENABLE) || defined(BT881_ENABLE) || defined(BT882_ENABLE) || defined(BT883_ENABLE)
#define BT88X_ENABLE
#endif

/// Re-Mapping BT815 Series to BT81X
#if defined(BT815_ENABLE) || defined(BT816_ENABLE)
#define BT81X_ENABLE
#endif

/// Re-Mapping BT817 Series to BT81XA
#if defined(BT817_ENABLE) || defined(BT818_ENABLE)
#define BT81XA_ENABLE
#endif

#if defined(FT80X_ENABLE)
#define FT_80X_ENABLE
#elif defined(FT81X_ENABLE)
#define FT_81X_ENABLE
#elif defined(BT88X_ENABLE)
#define BT_88X_ENABLE
#elif defined(BT81X_ENABLE)
#define BT_81X_ENABLE
#elif defined(BT81XA_ENABLE)
#define BT_81XA_ENABLE
#endif

/* Type of file to load from SDCard or Windows file system */
#define LOADIMAGE 1  //loadimage command takes destination address and options before the actual bitmap data
#define INFLATE 2    //inflate command takes destination address before the actual bitmap
#define LOAD 3       //load bitmaps directly
#define INFLATE2 4

/* C library inclusions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include <direct.h>
#include <time.h>
#include <io.h>
#include <stdint.h>

/*================ Type definition ================*/
typedef char             bool_t;
typedef char             char8_t;
typedef unsigned char    uchar8_t;
typedef signed char        schar8_t;
typedef float            float_t;

#define BYTE_SIZE  (1)
#define SHORT_SIZE (2)
#define WORD_SIZE  (4)
#define DWORD_SIZE (8)

#define NUMBITS_IN_BYTE  (1*8)
#define NUMBITS_IN_SHORT (2*8)
#define NUMBITS_IN_WORD  (4*8)
#define NUMBITS_IN_DWORD (8*8)

#define prog_uchar8_t  uchar8_t
#define prog_char8_t   char8_t
#define prog_uint16_t  uint16_t

#define strcpy_P     strcpy
#define strlen_P     strlen

#define PROGMEM



/* D2xx and SPI from BRIDGETEK inclusions */
#include "ftd2xx.h"

#ifdef MSVC_PLATFORM_SPI_LIBMPSSE
#include "LibMPSSE_spi.h"
#endif

#ifdef MSVC_PLATFORM_SPI_LIBFT4222
#include "LibFT4222.h"
#endif

/* HAL inclusions */
#include "Gpu_Hal.h"
#include "Gpu.h"
#include "CoPro_Cmds.h"
#include "Hal_Utils.h"

/* Macros specific to optimization */
#define BUFFER_OPTIMIZATION          (1)
#define BUFFER_OPTIMIZATION_DLRAM    (1)
#define BUFFER_OPTIMIZATION_CMDRAM   (1)
#define MSVC_PLATFORM_SPI            (1)

#ifdef MSVC_PLATFORM
#ifdef MSVC_PLATFORM_SPI_LIBMPSSE
#define FT800_SEL_PIN   0
#define FT800_PD_N      7
#endif
#ifdef MSVC_PLATFORM_SPI_LIBFT4222
#define FT800_SEL_PIN   1    /* GPIO is not utilized in Lib4222 as it is directly managed by firmware */
#define FT800_PD_N      GPIO_PORT0
#endif
#endif

#endif /*_PLATFORM_H_*/
/* Nothing beyond this*/

