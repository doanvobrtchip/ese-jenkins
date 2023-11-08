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
0.1 - date 2013.04.24 - Initial version
0.2 - date 2013.08.19 - few minor edits
0.3 - date 2015.04.29 - New version for multi-emulation mode

*/

#pragma warning(disable: 4996)

#include "Platform.h"

#ifdef MSVC_FT800EMU
#include <Emulator.h>


BT8XXEMU_EmulatorMode GpuEmu_Mode();
BT8XXEMU_Emulator *g_Emulator = NULL;

extern "C" void setup();
extern "C" void loop();

void mcu(BT8XXEMU_Emulator *sender, void *context)
{
	setup();
}

int32_t main(int32_t argc,char8_t *argv[])
{
	printf("%s\n\n", BT8XXEMU_version());
	BT8XXEMU_Flash *flash = NULL;
#if FLASH_ENABLE
#define BTFLASH_DATA_FILE   L"..//..//Test//Flash.bin" //path to Flash file
#define BTFLASH_DEVICE_TYPE L"mx25lemu"
#define BTFLASH_SIZE (16 * 1024 * 1024)
	BT8XXEMU_FlashParameters flashParams;
	BT8XXEMU_Flash_defaults(BT8XXEMU_VERSION_API, &flashParams);
	wcscpy(flashParams.DeviceType, BTFLASH_DEVICE_TYPE);
	wcscpy(flashParams.DataFilePath, BTFLASH_DATA_FILE);
	flashParams.SizeBytes = BTFLASH_SIZE;
	flashParams.StdOut = true;
	flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);
#endif

	BT8XXEMU_EmulatorParameters params;

	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, GpuEmu_Mode());
	params.Main = mcu;
	params.Flash = flash;
	params.Flags |= BT8XXEMU_EmulatorEnableStdOut;

	BT8XXEMU_run(BT8XXEMU_VERSION_API, &g_Emulator, &params);

	BT8XXEMU_stop(g_Emulator);
	BT8XXEMU_destroy(g_Emulator);

	BT8XXEMU_Flash_destroy(flash);
	return 0;
}

void GpuEmu_SPII2C_csLow()
{
	BT8XXEMU_chipSelect(g_Emulator, 1);
}

void GpuEmu_SPII2C_csHigh()
{
	BT8XXEMU_chipSelect(g_Emulator, 0);
}

void GpuEmu_SPII2C_begin()
{

}

void GpuEmu_SPII2C_end()
{

}


uint8_t GpuEmu_SPII2C_transfer(uint8_t data)
{
	return BT8XXEMU_transfer(g_Emulator, data);
}


void  GpuEmu_SPII2C_StartRead(uint32_t addr)
{
	GpuEmu_SPII2C_csLow();
	GpuEmu_SPII2C_transfer((addr >> 16) & 0xFF);
	GpuEmu_SPII2C_transfer((addr >> 8) & 0xFF);
	GpuEmu_SPII2C_transfer(addr & 0xFF);

	GpuEmu_SPII2C_transfer(0); //Dummy Read Byte
}

void  GpuEmu_SPII2C_StartWrite(uint32_t addr)
{
	GpuEmu_SPII2C_csLow();
	GpuEmu_SPII2C_transfer(((addr >> 16) & 0xFF) | 0x80);
	GpuEmu_SPII2C_transfer((addr >> 8) & 0xFF);
	GpuEmu_SPII2C_transfer(addr & 0xFF);
	
}


BT8XXEMU_EmulatorMode GpuEmu_Mode(){
#if defined(FT800_ENABLE)
		return BT8XXEMU_EmulatorFT800;
#elif defined(FT801_ENABLE)
		return BT8XXEMU_EmulatorFT801;
#elif defined(FT810_ENABLE)
		return BT8XXEMU_EmulatorFT810;
#elif defined(FT811_ENABLE)
		return BT8XXEMU_EmulatorFT811;
#elif defined(FT812_ENABLE)
		return BT8XXEMU_EmulatorFT812;
#elif defined(FT813_ENABLE)
		return BT8XXEMU_EmulatorFT813;
#elif defined(BT81X_ENABLE)
		return BT8XXEMU_EmulatorBT815;
#elif defined(BT81XA_ENABLE)
		return BT8XXEMU_EmulatorBT815;
#endif
		return BT8XXEMU_EmulatorFT800;
}


#endif