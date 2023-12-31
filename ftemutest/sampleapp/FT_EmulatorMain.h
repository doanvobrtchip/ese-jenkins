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
0.2 - date 2014.04.28 - made changes

*/#ifndef FT800EMU_MAIN_H
#define FT800EMU_MAIN_H

#include "FT_Platform.h"
#include <ft8xxemu_inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSVC_FT800EMU
#define BUFFER_OPTIMIZATION
#endif

void Ft_GpuEmu_SPII2C_begin();
void Ft_GpuEmu_SPII2C_end();
void Ft_GpuEmu_SPII2C_csLow();
void Ft_GpuEmu_SPII2C_csHigh();

void  Ft_GpuEmu_SPII2C_StartRead(uint32_t addr);
uint8_t Ft_GpuEmu_SPII2C_transfer(uint8_t data);
void  Ft_GpuEmu_SPII2C_StartWrite(uint32_t addr);

#ifdef __cplusplus
}
#endif
#endif