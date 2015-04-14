/*

Copyright (c) Future Technology Devices International 2014

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Abstract:

This file contains is functions for all UI fields.

Author : FTDI 

Revision History: 
0.1 - date 2013.04.24 - initial version
0.2 - date 2014.04.28 - Split in individual files according to platform

*/

#ifndef _FT_PLATFORM_H_
#define _FT_PLATFORM_H_

/* platform specific macros */
#define MSVC_FT800EMU 							(1)// enable by default for emulator platform

/* Display configuration specific macros */
//#define SAMAPP_DISPLAY_QVGA					(1)
//#define ORIENTATION_PORTRAIT 					(1)
#define ORIENTATION_LANDSCAPE 					(1)
//#define SAMAPP_DISPLAY_WQVGA					(1)
#define SAMAPP_DISPLAY_WVGA						(1)
//#define SAMAPP_DISPLAY_HVGA_PORTRAIT			(1)

/* Chip configuration specific macros */
//#define FT_800_ENABLE							(1)
//#define FT_801_ENABLE							(1)
//#define FT_810_ENABLE							(1)
#define FT_811_ENABLE							(1)

#if defined(FT_800_ENABLE) || defined(FT_801_ENABLE)
#define FT_80X_ENABLE							(1)
#endif

#if defined(FT_810_ENABLE) || defined(FT_811_ENABLE)
#define FT_81X_ENABLE							(1)
#endif

/* SPI specific macros - compile time switches for SPI single, dial and quad use cases */
#define ENABLE_SPI_SINGLE						(1)
//#define ENABLE_SPI_DUAL						(1)
//#define ENABLE_SPI_QUAD						(1)



/* Standard C libraries */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include <direct.h>
#include <time.h>
#include <io.h>
/* HAL inclusions */
#include "FT_DataTypes.h"


#include "FT_EmulatorMain.h"






#include "FT_Gpu_Hal.h"
#include "FT_Gpu.h"
#include "FT_CoPro_Cmds.h"
#include "FT_Hal_Utils.h"




#define BUFFER_OPTIMIZATION


#endif /*_FT_PLATFORM_H_*/
/* Nothing beyond this*/




