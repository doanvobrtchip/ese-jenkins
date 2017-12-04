/*
BT8XX Emulator Samples
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
*/

#define BT815EMU_MODE

#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>
#include <ft800emu_vc.h>

#include <stdio.h>
#include <assert.h>
#define BTFLASH_CMD_WREN (0x06) /* Write Enable */
#define BTFLASH_CMD_WRDI (0x04) /* Write Disable */
#define BTFLASH_CMD_RDID (0x9F) /* Read Identification */
#define BTFLASH_CMD_RDSR (0x05) /* Read Status Register */
#define BTFLASH_CMD_WRSR (0x01) /* Write Status Register */
#define BTFLASH_CMD_FASTDTRD (0x0D) /* Fast DT Read */
#define BTFLASH_CMD_2DTRD (0xBD) /* Dual I/O DT Read */
#define BTFLASH_CMD_4DTRD (0xED) /* Quad I/O DT Read */
#define BTFLASH_CMD_READ (0x03) /* Read Data */
#define BTFLASH_CMD_FAST_READ (0x0B) /* Fast Read Data */
#define BTFLASH_CMD_RDSFDP (0x5A) /* Read SFDP */
#define BTFLASH_CMD_2READ (0xBB) /* 2x IO Read */
#define BTFLASH_CMD_4READ (0xEB) /* 4x IO Read */
#define BTFLASH_CMD_4PP (0x38) /* Quad Page Program */
#define BTFLASH_CMD_SE (0x20) /* Sector Erase */
#define BTFLASH_CMD_BE (0xD8) /* Block Erase */
#define BTFLASH_CMD_BE32K (0x52) /* Block Erase 32kB */
#define BTFLASH_CMD_CE_60 (0x60) /* Chip Erase */
#define BTFLASH_CMD_CE_C7 (0xC7) /* Chip Erase */
#define BTFLASH_CMD_PP (0x02) /* Page Program */
#define BTFLASH_CMD_CP (0xAD) /* Continuously Program mode */
#define BTFLASH_CMD_DP (0xB9) /* Deep Power Down */
#define BTFLASH_CMD_RDP (0xAB) /* Release from Deep Power Down */
#define BTFLASH_CMD_RES (0xAB) /* Read Electronic ID */
#define BTFLASH_CMD_REMS (0x90) /* Read Electronic Manufacturer and Device ID */
#define BTFLASH_CMD_REMS2 (0xEF) /* Read ID for 2x IO Mode */
#define BTFLASH_CMD_REMS4 (0xDF) /* Read ID for 4x IO Mode */
#define BTFLASH_CMD_REMS4D (0xCF) /* Read ID for 4x IO DT Mode */
#define BTFLASH_CMD_ENSO (0xB1) /* Enter Secured OTP */
#define BTFLASH_CMD_EXSO (0xC1) /* Exit Secured OTP */
#define BTFLASH_CMD_RDSCUR (0x2B) /* Read Security Register */
#define BTFLASH_CMD_WRSCUR (0x2F) /* Write Security Register */
#define BTFLASH_CMD_ESRY (0x70) /* Enable SO to output RY/BY# */
#define BTFLASH_CMD_DSRY (0x80) /* Disable SO to output RY/BY# */
#define BTFLASH_CMD_CLSR (0x30) /* Clear SR Fail Flags */
#define BTFLASH_CMD_HPM (0xA3) /* High Performance Enable Mode */
#define BTFLASH_CMD_WPSEL (0x68) /* Write Protection Selection */
#define BTFLASH_CMD_SBLK (0x36) /* Single Block Lock */
#define BTFLASH_CMD_SBULK (0x39) /* Single Block Unlock */
#define BTFLASH_CMD_RDBLOCK (0x3C) /* Block Protect Read */
#define BTFLASH_CMD_GBLK (0x7E) /* Gang Block Lock */
#define BTFLASH_CMD_GBULK (0x98) /* Gang Block Unlock */

#define BTFLASH_CMD_EN4B (0xB7) /* Enter 4-byte mode */
#define BTFLASH_CMD_EX4B (0xE9) /* Exit 4-byte mode */

#define BTFLASH_STATUS_QE_FLAG (1 << 6) /* Quad Enable */

#define BTFLASH_DEVICE_TYPE L"mx25lemu"
#define BTFLASH_SIZE (8 * 1024 * 1024)
#define BTFLASH_SIZE_EXTENDED (4 * BTFLASH_SIZE)
//#define BTFLASH_DATA_FILE L"C:/source/ft800emu/reference/vc3roms/stdflash.bin"
#define BTFLASH_DATA_FILE L"C:/Users/paul.jiao/Source/Repos/FT8XXEMU_FTDISG/FT8XXEMU/reference/vc3roms/stdflash.bin"
#define BTFLASH_ELECTRONIC_ID 0x16
#define BTFLASH_ELECTRONIC_ID_EXTENDED (BTFLASH_ELECTRONIC_ID + 2)
#define BTFLASH_MANUFACTURER_ID 0xC2
#define BTFLASH_DEVICE_ID 0x16
#define BTFLASH_MEMORY_TYPE 0x20
#define BTFLASH_MEMORY_DENSITY 0x17

static uint8_t lastValue = 0xFF;

static const uint8_t outMask1 = 0x31;
static const uint8_t outMaskRead4 = 0x30;

static const uint8_t csMask = 0x10;
static const uint8_t clkMask = 0x20;

void cableSelect(BT8XXEMU_Flash *flash, bool cs)
{
	uint8_t signal = (lastValue & 0xF) | (cs ? 0 : csMask);
	BT8XXEMU_Flash_transferSpi4(flash, signal);
}

uint8_t transferU8(BT8XXEMU_Flash *flash, uint8_t u8)
{
	uint8_t res = 0xFF;
	uint8_t signal;
	for (int i = 0; i < 8; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | ((u8 >> 7) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | clkMask | ((u8 >> 7) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | ((u8 >> 7) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u8 <<= 1;
	}
	return res;
}

uint32_t transferU24(BT8XXEMU_Flash *flash, uint32_t u24)
{
	uint32_t res = 0xFFFFFF00;
	uint8_t signal;
	for (int i = 0; i < 24; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | ((u24 >> 23) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | clkMask | ((u24 >> 23) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | ((u24 >> 23) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u24 <<= 1;
	}
	return res;
}

uint32_t transferU32(BT8XXEMU_Flash *flash, uint32_t u32)
{
	uint32_t res = 0xFFFFFF00;
	uint8_t signal;
	for (int i = 0; i < 32; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | ((u32 >> 31) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | clkMask | ((u32 >> 31) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | ((u32 >> 31) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u32 <<= 1;
	}
	return res;
}

uint8_t transferDTU8(BT8XXEMU_Flash *flash, uint8_t u8)
{
	uint8_t res = 0xFF;
	uint8_t signal;
	for (int i = 0; i < 8; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u8 >> 7) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & ~lastValue) | ((u8 >> 7) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u8 >> 7) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u8 <<= 1;
	}
	return res;
}

uint32_t transferDTU24(BT8XXEMU_Flash *flash, uint32_t u24)
{
	uint32_t res = 0xFFFFFF00;
	uint8_t signal;
	for (int i = 0; i < 24; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u24 >> 23) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & ~lastValue) | ((u24 >> 23) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u24 >> 23) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u24 <<= 1;
	}
	return res;
}

uint32_t transferDTU32(BT8XXEMU_Flash *flash, uint32_t u32)
{
	uint32_t res = 0xFFFFFF00;
	uint8_t signal;
	for (int i = 0; i < 32; ++i)
	{
		res <<= 1;
		res |= (lastValue & 0x2) >> 1;
		// printf("%i\n", lastValue & 0x2);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u32 >> 31) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & ~lastValue) | ((u32 >> 31) & 0x1);
		// printf("%i\n", signal & 1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMask1 & 0xF) | (clkMask & lastValue) | ((u32 >> 31) & 0x1);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u32 <<= 1;
	}
	return res;
}

void write4U8(BT8XXEMU_Flash *flash, uint8_t u8)
{
	uint8_t signal;
	for (int i = 0; i < 2; ++i)
	{
		signal = (u8 >> 4);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = clkMask | (u8 >> 4);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (u8 >> 4);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u8 <<= 4;
	}
}

void write4U24(BT8XXEMU_Flash *flash, uint32_t u24)
{
	uint32_t signal;
	for (int i = 0; i < 6; ++i)
	{
		signal = ((u24 >> 20) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = clkMask | ((u24 >> 20) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = ((u24 >> 20) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u24 <<= 4;
	}
}

void write4U32(BT8XXEMU_Flash *flash, uint32_t u32)
{
	uint32_t signal;
	for (int i = 0; i < 8; ++i)
	{
		signal = ((u32 >> 28) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = clkMask | ((u32 >> 28) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = ((u32 >> 28) & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		u32 <<= 4;
	}
}

uint8_t read4U8(BT8XXEMU_Flash *flash)
{
	uint8_t res = 0xFF;
	uint8_t signal;
	for (int i = 0; i < 2; ++i)
	{
		res <<= 4;
		res |= (lastValue & 0xF);
		signal = (lastValue & ~outMaskRead4 & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMaskRead4 & 0xF) | clkMask;
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
		signal = (lastValue & ~outMaskRead4 & 0xF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, signal);
	}
	return res;
}

///////////////////////////////////////////////////////////////////////
// Emulator utility
///////////////////////////////////////////////////////////////////////

void swrbegin(BT8XXEMU_Emulator *emulator, size_t address)
{
	BT8XXEMU_chipSelect(emulator, 1);

	BT8XXEMU_transfer(emulator, (2 << 6) | ((address >> 16) & 0x3F));
	BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, address & 0xFF);
	// BT8XXEMU_transfer(0x00);
}

void swr8(BT8XXEMU_Emulator *emulator, uint8_t value)
{
	BT8XXEMU_transfer(emulator, value);
}

void swr16(BT8XXEMU_Emulator *emulator, uint16_t value)
{
	BT8XXEMU_transfer(emulator, value & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
}

void swr32(BT8XXEMU_Emulator *emulator, uint32_t value)
{
	BT8XXEMU_transfer(emulator, value & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(emulator, (value >> 24) & 0xFF);
}

void swrend(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMU_chipSelect(emulator, 0);
}

void wr32(BT8XXEMU_Emulator *emulator, size_t address, uint32_t value)
{
	swrbegin(emulator, address);
	swr32(emulator, value);
	swrend(emulator);
}

uint32_t rd32(BT8XXEMU_Emulator *emulator, size_t address)
{
	BT8XXEMU_chipSelect(emulator, 1);

	BT8XXEMU_transfer(emulator, (address >> 16) & 0x3F);
	BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(emulator, address & 0xFF);
	BT8XXEMU_transfer(emulator, 0x00);

	uint32_t value;
	value = BT8XXEMU_transfer(emulator, rand() & 0xFF);
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 8;
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 16;
	value |= BT8XXEMU_transfer(emulator, rand() & 0xFF) << 24;

	BT8XXEMU_chipSelect(emulator, 0);
	return value;
}

void flush(BT8XXEMU_Emulator *emulator)
{
	while (rd32(emulator, REG_CMD_READ) != rd32(emulator, REG_CMD_WRITE));
}

///////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////

int main(int, char*[])
{
	printf("%s\n\n", BT8XXEMU_version());

	BT8XXEMU_FlashParameters flashParams;
	BT8XXEMU_Flash_defaults(BT8XXEMU_VERSION_API, &flashParams);
	flashParams.DeviceType = BTFLASH_DEVICE_TYPE;
	flashParams.SizeBytes = BTFLASH_SIZE;
	flashParams.DataFilePath = BTFLASH_DATA_FILE;
	flashParams.StdOut = true;
	BT8XXEMU_Flash *flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);

	uint8_t *data = BT8XXEMU_Flash_data(flash);
	assert(data[0] == 0x70);
	size_t size = BT8XXEMU_Flash_size(flash);
	assert(size == BTFLASH_SIZE);

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Read Identification Data
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_RES);
		uint8_t electronicID = transferU8(flash, rand() & 0xFF);
		printf("RES: %x\n", (int)electronicID);
		assert(electronicID == BTFLASH_ELECTRONIC_ID);

		// Can be read continuously
		electronicID = transferU8(flash, rand() & 0xFF);
		assert(electronicID == BTFLASH_ELECTRONIC_ID);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_REMS);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 0);
		uint8_t manufacturerID = transferU8(flash, rand() & 0xFF);
		uint8_t deviceID = transferU8(flash, rand() & 0xFF);
		printf("REMS: %x %x\n", (int)manufacturerID, (int)deviceID);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
		assert(deviceID == BTFLASH_DEVICE_ID);

		// Can be read continuously
		manufacturerID = transferU8(flash, rand() & 0xFF);
		deviceID = transferU8(flash, rand() & 0xFF);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
		assert(deviceID == BTFLASH_DEVICE_ID);
		manufacturerID = transferU8(flash, rand() & 0xFF);
		deviceID = transferU8(flash, rand() & 0xFF);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
		assert(deviceID == BTFLASH_DEVICE_ID);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_REMS);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 1);
		uint8_t deviceID = transferU8(flash, rand() & 0xFF);
		uint8_t manufacturerID = transferU8(flash, rand() & 0xFF);
		printf("REMS (01h): %x %x\n", (int)deviceID, (int)manufacturerID);
		assert(deviceID == BTFLASH_DEVICE_ID);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);

		// Can be read continuously
		deviceID = transferU8(flash, rand() & 0xFF);
		manufacturerID = transferU8(flash, rand() & 0xFF);
		assert(deviceID == BTFLASH_DEVICE_ID);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
		deviceID = transferU8(flash, rand() & 0xFF);
		manufacturerID = transferU8(flash, rand() & 0xFF);
		assert(deviceID == BTFLASH_DEVICE_ID);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_RDID);
		uint8_t manufacturerID = transferU8(flash, rand() & 0xFF);
		uint8_t memoryType = transferU8(flash, rand() & 0xFF);
		uint8_t memoryDensity = transferU8(flash, rand() & 0xFF);
		printf("RDID: %x %x %x\n", (int)manufacturerID, (int)memoryType, (int)memoryDensity);
		assert(manufacturerID == BTFLASH_MANUFACTURER_ID);
		assert(memoryType == BTFLASH_MEMORY_TYPE);
		assert(memoryDensity == BTFLASH_MEMORY_DENSITY);

		// Can not be read continuously
		printf("Start of expected errors, reading beyond RDID\n");
		manufacturerID = transferU8(flash, rand() & 0xFF);
		// memoryType = transferU8(flash, rand() & 0xFF);
		// memoryDensity = transferU8(flash, rand() & 0xFF);
		assert(manufacturerID != BTFLASH_MANUFACTURER_ID);
		// assert(memoryType != BTFLASH_MEMORY_TYPE);
		// assert(memoryDensity != BTFLASH_MEMORY_DENSITY);
		printf("End of expected errors\n");
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_RDSFDP);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 0);
		uint8_t sfdp0 = transferU8(flash, rand() & 0xFF);
		uint8_t sfdp1 = transferU8(flash, rand() & 0xFF);
		uint8_t sfdp2 = transferU8(flash, rand() & 0xFF);
		printf("SFDP: %x-%x-%x-...\n", (int)sfdp0, (int)sfdp1, (int)sfdp2);
		assert(sfdp0 == 83);
		assert(sfdp1 == 70);
		assert(sfdp2 == 68);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_RDSFDP);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 3);
		transferU8(flash, 0);
		uint8_t sfdp3 = transferU8(flash, rand() & 0xFF);
		uint8_t sfdp4 = transferU8(flash, rand() & 0xFF);
		uint8_t sfdp5 = transferU8(flash, rand() & 0xFF);
		printf("SFDP (03h): %x-%x-%x-...\n", (int)sfdp3, (int)sfdp4, (int)sfdp5);
		assert(sfdp3 == 80);
		assert(sfdp4 == 0);
		assert(sfdp5 == 1);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Read
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 0);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("READ: %x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU8(flash, 0);
		transferU8(flash, 0);
		transferU8(flash, 3);
		uint8_t reqd3 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd4 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd5 = transferU8(flash, rand() & 0xFF);
		printf("READ (03h): ...-%x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x8E);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU24(flash, (uint32_t)size - 1);
		uint8_t reqdF = transferU8(flash, rand() & 0xFF);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("READ (-1): ...-%x %x-%x-%x-...\n", (int)reqdF, (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqdF == 0xFF);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Fast Read
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_FAST_READ);
		transferU24(flash, 0);
		transferU8(flash, rand() & 0xFF);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("FAST_READ: %x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FAST_READ);
		transferU24(flash, 3);
		transferU8(flash, rand() & 0xFF);
		uint8_t reqd3 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd4 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd5 = transferU8(flash, rand() & 0xFF);
		printf("FAST_READ (03h): ...-%x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x8E);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FAST_READ);
		transferU24(flash, (uint32_t)size - 1);
		transferU8(flash, rand() & 0xFF);
		uint8_t reqdF = transferU8(flash, rand() & 0xFF);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("FAST_READ (-1): ...-%x %x-%x-%x-...\n", (int)reqdF, (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqdF == 0xFF);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Fast DT Read
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_FASTDTRD);
		transferDTU24(flash, 0);
		for (int i = 0; i < 6; ++i)
		{
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF) | clkMask);
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF));
		}
		uint8_t reqd0 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferDTU8(flash, rand() & 0xFF);
		printf("FASTDTRD: %x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FASTDTRD);
		transferDTU24(flash, 3);
		for (int i = 0; i < 6; ++i)
		{
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF) | clkMask);
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF));
		}
		uint8_t reqd3 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd4 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd5 = transferDTU8(flash, rand() & 0xFF);
		printf("FASTDTRD (03h): ...-%x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x8E);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FASTDTRD);
		transferDTU24(flash, (uint32_t)size - 1);
		for (int i = 0; i < 6; ++i)
		{
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF) | clkMask);
			lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF));
		}
		uint8_t reqdF = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd0 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferDTU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferDTU8(flash, rand() & 0xFF);
		printf("FASTDTRD (-1): ...-%x %x-%x-%x-...\n", (int)reqdF, (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqdF == 0xFF);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// 4x IO Read
	/////////////////////////////////////////////////////////////////

	; {
		// Enable QE
		printf("Enable QE\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_RDSR);
		uint8_t sr = transferU8(flash, rand() & 0xFF);
		sr |= BTFLASH_STATUS_QE_FLAG;
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_WRSR);
		transferU8(flash, sr);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_4READ);
		write4U24(flash, 0);
		write4U8(flash, 0x0F); // Enable PE
		write4U8(flash, rand() & 0xFF);
		write4U8(flash, rand() & 0xFF);
		uint8_t reqd0 = read4U8(flash);
		uint8_t reqd1 = read4U8(flash);
		uint8_t reqd2 = read4U8(flash);
		printf("4READ: %x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		// Skip 4READ instruction under P?
		write4U24(flash, 3); // 03h
		write4U8(flash, 0x12); // Disable PE
		write4U8(flash, rand() & 0xFF);
		write4U8(flash, rand() & 0xFF);
		uint8_t reqd3 = read4U8(flash);
		uint8_t reqd4 = read4U8(flash);
		uint8_t reqd5 = read4U8(flash);
		printf("4READ (03h): ...-%x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x8E);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		// Verify that PE is off now
		transferU8(flash, BTFLASH_CMD_RES);
		uint8_t electronicID = transferU8(flash, rand() & 0xFF);
		printf("RES: %x\n", (int)electronicID);
		assert(electronicID == BTFLASH_ELECTRONIC_ID);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		// Disable QE
		printf("Disable QE\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_RDSR);
		uint8_t sr = transferU8(flash, rand() & 0xFF);
		sr &= ~BTFLASH_STATUS_QE_FLAG;
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_WRSR);
		transferU8(flash, sr);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("Start of expected errors, sending invalid 4x IO Read\n");
		transferU8(flash, BTFLASH_CMD_4READ);
		write4U24(flash, 0);
		write4U8(flash, 0x0F); // Enable PE
		write4U8(flash, rand() & 0xFF);
		write4U8(flash, rand() & 0xFF);
		uint8_t reqd0 = read4U8(flash);
		printf("4READ: %x--...\n", (int)reqd0);
		printf("End of expected errors\n");
		assert(reqd0 != 0x70);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Chip Erase
	/////////////////////////////////////////////////////////////////

	; {
		printf("Start of expected errors, sending invalid Chip Erase 60h\n");
		transferU8(flash, BTFLASH_CMD_CE_60);
		printf("End of expected errors\n");
		assert(data[0] == 0x70);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("Start of expected errors, sending invalid Chip Erase C7h\n");
		transferU8(flash, BTFLASH_CMD_CE_C7);
		printf("End of expected errors\n");
		assert(data[0] == 0x70);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);
	transferU8(flash, BTFLASH_CMD_WREN);
	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("Start of expected errors, transfering excess data to Chip Erase\n");
		transferU8(flash, BTFLASH_CMD_CE_C7);
		transferU8(flash, rand() & 0xFF);
		printf("End of expected errors\n");
		assert(data[0] == 0x70);
		cableSelect(flash, false);
		assert(data[0] == 0x70);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("Chip Erase\n");
		transferU8(flash, BTFLASH_CMD_CE_C7);
		assert(data[0] == 0x70); // Chip Erase only when CS goes high
		cableSelect(flash, false);
		for (int i = 0; i < size; ++i)
			assert(data[i] == 0xFF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("Start of expected errors, sending invalid Chip Erase C7h\n");
		transferU8(flash, BTFLASH_CMD_CE_C7);
		printf("End of expected errors\n");
		assert(data[0] == 0xFF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Page Program
	/////////////////////////////////////////////////////////////////

	; {
		printf("PP (3)\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_PP);
		transferU24(flash, 3);
		transferU8(flash, 0xAB);
		transferU8(flash, 0xCD);
		transferU8(flash, 0xEF);
		assert(data[3] != 0xAB);
		assert(data[4] != 0xCD);
		assert(data[5] != 0xEF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	assert(data[3] == 0xAB);
	assert(data[4] == 0xCD);
	assert(data[5] == 0xEF);

	; {
		printf("PP (31)\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_PP);
		transferU24(flash, 31);
		transferU8(flash, 0xAB);
		transferU8(flash, 0xCD);
		transferU8(flash, 0xEF);
		assert(data[31] != 0xAB);
		assert(data[32] != 0xCD);
		assert(data[33] != 0xEF);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF) | clkMask);
		lastValue = BT8XXEMU_Flash_transferSpi4(flash, (lastValue & ~outMask1 & 0xF));
		printf("Start of expected errors, not deselecting chip at byte boundary\n");
		cableSelect(flash, false);
		cableSelect(flash, true);
		printf("End of expected errors\n");
		assert(data[31] != 0xAB);
		assert(data[32] != 0xCD);
		assert(data[33] != 0xEF);
		// transferU8(flash, BTFLASH_CMD_WRDI);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		printf("PP (256 + 31)\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_PP);
		transferU24(flash, 256 + 31);
		for (int i = 0; i < 256 + 128; ++i)
			transferU8(flash, i / 2);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	assert(data[256 + 31] == 128);
	assert(data[256 + 31 + 128] == 64);

	/////////////////////////////////////////////////////////////////

	BT8XXEMU_Flash_destroy(flash);
	flash = NULL;
	data = NULL;

	/////////////////////////////////////////////////////////////////
	//// 256
	/////////////////////////////////////////////////////////////////

	flashParams.SizeBytes = BTFLASH_SIZE_EXTENDED;
	flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);

	data = BT8XXEMU_Flash_data(flash);
	assert(data[0] == 0x70);
	size = BT8XXEMU_Flash_size(flash);
	assert(size == BTFLASH_SIZE_EXTENDED);

	/////////////////////////////////////////////////////////////////
	//// Read Identification Data
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_RES);
		uint8_t electronicID = transferU8(flash, rand() & 0xFF);
		printf("RES: %x\n", (int)electronicID);
		assert(electronicID == BTFLASH_ELECTRONIC_ID_EXTENDED);

		// Can be read continuously
		electronicID = transferU8(flash, rand() & 0xFF);
		assert(electronicID == BTFLASH_ELECTRONIC_ID_EXTENDED);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Enter 4-byte read
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_EN4B);
		printf("EN4B\n");
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Read
	/////////////////////////////////////////////////////////////////

	data[BTFLASH_SIZE_EXTENDED / 2] = 0xAB;
	data[BTFLASH_SIZE_EXTENDED / 2 + 1] = 0xCD;
	data[BTFLASH_SIZE_EXTENDED / 2 + 2] = 0xEF;

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU32(flash, BTFLASH_SIZE_EXTENDED / 2);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("READ: ...-%x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0xAB);
		assert(reqd1 == 0xCD);
		assert(reqd2 == 0xEF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FAST_READ);
		transferU32(flash, BTFLASH_SIZE_EXTENDED / 2);
		transferU8(flash, rand() & 0xFF);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("FAST_READ: ...-%x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0xAB);
		assert(reqd1 == 0xCD);
		assert(reqd2 == 0xEF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		// Enable QE
		printf("Enable QE\n");
		transferU8(flash, BTFLASH_CMD_WREN);
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_RDSR);
		uint8_t sr = transferU8(flash, rand() & 0xFF);
		sr |= BTFLASH_STATUS_QE_FLAG;
		cableSelect(flash, false);
		cableSelect(flash, true);
		transferU8(flash, BTFLASH_CMD_WRSR);
		transferU8(flash, sr);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_4READ);
		write4U32(flash, BTFLASH_SIZE_EXTENDED / 2);
		write4U8(flash, 0x00); // No PE
		write4U8(flash, rand() & 0xFF);
		write4U8(flash, rand() & 0xFF);
		uint8_t reqd0 = read4U8(flash);
		uint8_t reqd1 = read4U8(flash);
		uint8_t reqd2 = read4U8(flash);
		printf("4READ: ...-%x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0xAB);
		assert(reqd1 == 0xCD);
		assert(reqd2 == 0xEF);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////
	//// Exit 4-byte read
	/////////////////////////////////////////////////////////////////

	; {
		transferU8(flash, BTFLASH_CMD_EX4B);
		printf("EX4B\n");
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU24(flash, 0);
		uint8_t reqd0 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd1 = transferU8(flash, rand() & 0xFF);
		uint8_t reqd2 = transferU8(flash, rand() & 0xFF);
		printf("READ: %x-%x-%x-...\n", (int)reqd0, (int)reqd1, (int)reqd2);
		assert(reqd0 == 0x70);
		assert(reqd1 == 0xDF);
		assert(reqd2 == 0xFB);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	/////////////////////////////////////////////////////////////////

	BT8XXEMU_Flash_destroy(flash);
	flash = NULL;
	data = NULL;

	/////////////////////////////////////////////////////////////////
	//// Emulator Coprocessor
	/////////////////////////////////////////////////////////////////

	flashParams.SizeBytes = BTFLASH_SIZE; // BTFLASH_SIZE_EXTENDED;
	flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);

	data = BT8XXEMU_Flash_data(flash);
	assert(data[0] == 0x70);
	size = BT8XXEMU_Flash_size(flash);
	assert(size == BTFLASH_SIZE);

	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, BT8XXEMU_EmulatorBT815);
	params.Flags |= BT8XXEMU_EmulatorEnableStdOut;

	params.Flash = flash;

	BT8XXEMU_Emulator *emulator = NULL;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &emulator, &params);
	uint8_t *ram = BT8XXEMU_getRam(emulator);

	wr32(emulator, REG_HSIZE, 480);
	wr32(emulator, REG_VSIZE, 272);
	wr32(emulator, REG_PCLK, 5);

	flush(emulator);
	while (!rd32(emulator, REG_FLASH_STATUS));
	assert(rd32(emulator, REG_FLASH_STATUS) == FLASH_STATUS_BASIC);

	/////////////////////////////////////////////////////////////////
	//// Enter full speed mode
	/////////////////////////////////////////////////////////////////

	; {
		printf("CMD_FLASHFAST\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHFAST);
		uint32_t resAddr = rd32(emulator, REG_CMD_WRITE);
		wr32(emulator, REG_CMD_WRITE, resAddr + 4);
		flush(emulator);
		assert(rd32(emulator, resAddr) == 0);
		assert(rd32(emulator, REG_FLASH_STATUS) == FLASH_STATUS_FULL);
	}

	for (int i = 0; i < 4096; ++i)
	{
		ram[i] = 0x55;
	}

	; {
		printf("CMD_FLASHREAD (FLASH_STATUS_FULL)\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHREAD);
		wr32(emulator, REG_CMDB_WRITE, 128); // dest
		wr32(emulator, REG_CMDB_WRITE, 128); // src
		wr32(emulator, REG_CMDB_WRITE, 512); // num
		flush(emulator);
		for (int i = 128; i < 128 + 512; ++i)
			assert(ram[i] == data[i]);
	}

	for (int i = 0; i < 4096; ++i)
	{
		ram[i] = data[i];
	}

	; {
		assert(data[0] == 0x70);
		printf("CMD_FLASHERASE (FLASH_STATUS_FULL)\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHERASE);
		flush(emulator);
		for (int i = 0; i < 32 * 4096; ++i)
			assert(data[i] == 0xFF);
	}

	; {
		assert(ram[0] == 0x70);
		assert(data[0] != 0x70);
		printf("CMD_FLASHWRITE (FLASH_STATUS_FULL\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHWRITE);
		wr32(emulator, REG_CMDB_WRITE, 0);
		wr32(emulator, REG_CMDB_WRITE, 2048);
		for (int i = 0; i < 512; ++i)
			wr32(emulator, REG_CMDB_WRITE, ((uint32_t *)ram)[i]);
		flush(emulator);
		for (int i = 0; i < 2048; ++i)
			assert(data[i] == ram[i]);
		assert(data[0] == 0x70);
	}

	for (int i = 0; i < 4096; ++i)
	{
		ram[i] = 0x55;
	}

	; {
		assert(ram[128] != data[128]);
		printf("CMD_FLASHREAD (FLASH_STATUS_FULL)\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHREAD);
		wr32(emulator, REG_CMDB_WRITE, 128); // dest
		wr32(emulator, REG_CMDB_WRITE, 128); // src
		wr32(emulator, REG_CMDB_WRITE, 512); // num
		flush(emulator);
		for (int i = 128; i < 128 + 512; ++i)
			assert(ram[i] == data[i]);
	}

	/////////////////////////////////////////////////////////////////
	//// Detach, attach, direct access
	/////////////////////////////////////////////////////////////////

	; {
		printf("CMD_FLASHDETACH\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHDETACH);
		flush(emulator);
		assert(rd32(emulator, REG_FLASH_STATUS) == FLASH_STATUS_DETACHED);
	}

	; {
		printf("CMD_FLASHSPIDESEL\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHSPIDESEL);
		flush(emulator);
		printf("CMD_FLASHSPITX\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHSPITX);
		wr32(emulator, REG_CMDB_WRITE, 1);
		wr32(emulator, REG_CMDB_WRITE, BTFLASH_CMD_RES);
		flush(emulator);
		printf("CMD_FLASHSPIRX\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHSPIRX);
		wr32(emulator, REG_CMDB_WRITE, 0);
		wr32(emulator, REG_CMDB_WRITE, 4);
		flush(emulator);
		printf("CMD_FLASHSPIDESEL\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHSPIDESEL);
		flush(emulator);
		assert(ram[0] == BTFLASH_ELECTRONIC_ID);
		assert(ram[1] == BTFLASH_ELECTRONIC_ID);
		assert(ram[2] == BTFLASH_ELECTRONIC_ID);
		assert(ram[3] == BTFLASH_ELECTRONIC_ID);
	}

	; {
		printf("CMD_FLASHATTACH\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHATTACH);
		flush(emulator);
		assert(rd32(emulator, REG_FLASH_STATUS) == FLASH_STATUS_BASIC);
	}

	/////////////////////////////////////////////////////////////////
	//// Regular read
	/////////////////////////////////////////////////////////////////

	; {
		printf("CMD_FLASHREAD\n");
		for (int i = 0; i < 40; ++i)
			ram[i] = 0x55;
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHREAD);
		wr32(emulator, REG_CMDB_WRITE, 0); // dest
		wr32(emulator, REG_CMDB_WRITE, 0); // src
		wr32(emulator, REG_CMDB_WRITE, 40); // num
		flush(emulator);
		assert(ram[0] == 0x70);
		for (int i = 0; i < 40; ++i)
			assert(ram[i] == data[i]);
	}

	/////////////////////////////////////////////////////////////////
	//// Update
	/////////////////////////////////////////////////////////////////

	for (int i = 0; i < 4096; ++i)
	{
		ram[i] = data[i];
		data[i + 4096] = 0x55;
	}

	; {
		assert(data[4096] != 0x70);
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHUPDATE);
		wr32(emulator, REG_CMDB_WRITE, 4096); // dest
		wr32(emulator, REG_CMDB_WRITE, 0); // src
		wr32(emulator, REG_CMDB_WRITE, 4096); // num
		flush(emulator);
		assert(data[4096] == 0x70);
		for (int i = 0; i < 4096; ++i)
		{
			assert(ram[i] == data[i]);
			assert(ram[i] == data[i + 4096]);
		}
	}

	/////////////////////////////////////////////////////////////////
	//// Erase, write
	/////////////////////////////////////////////////////////////////

	for (int i = 0; i < 4096; ++i)
	{
		ram[i] = data[i];
	}

	; {
		assert(data[0] == 0x70);
		printf("CMD_FLASHERASE\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHERASE);
		flush(emulator);
		for (int i = 0; i < 32 * 4096; ++i)
			assert(data[i] == 0xFF);
	}

	; {
		assert(ram[0] == 0x70);
		assert(data[0] != 0x70);
		printf("CMD_FLASHWRITE\n");
		wr32(emulator, REG_CMDB_WRITE, CMD_FLASHWRITE);
		wr32(emulator, REG_CMDB_WRITE, 0);
		wr32(emulator, REG_CMDB_WRITE, 2048);
		for (int i = 0; i < 512; ++i)
			wr32(emulator, REG_CMDB_WRITE, ((uint32_t *)ram)[i]);
		flush(emulator);
		for (int i = 0; i < 2048; ++i)
			assert(data[i] == ram[i]);
		assert(data[0] == 0x70);
	}

	/////////////////////////////////////////////////////////////////

	/*
	                                          Detached Basic Full
	#define CMD_FLASHATTACH      4294967113UL Ok       -     -
	#define CMD_FLASHDETACH      4294967112UL -        Ok    ?
	#define CMD_FLASHFAST        4294967114UL -        Ok-ish-    // Seeing only one test read on 0xFC0 and two on 0x00
	#define CMD_FLASHREAD        4294967110UL -        Ok    Ok
	#define CMD_FLASHERASE       4294967108UL -        Ok    Fail // In fast mode it's resetting and not setting the QE flag back
	#define CMD_FLASHWRITE       4294967109UL -        Ok    ???
	#define CMD_FLASHSPIDESEL    4294967115UL Ok       -     -
	#define CMD_FLASHSPIRX       4294967117UL Ok       -     -
	#define CMD_FLASHSPITX       4294967116UL Ok       -     -
	#define CMD_FLASHUPDATE      4294967111UL -        Ok    /
	#define CMD_FLASHSOURCE      4294967118UL /        /     /
	
	*/

	BT8XXEMU_stop(emulator);
	BT8XXEMU_destroy(emulator);
	emulator = NULL;
	BT8XXEMU_Flash_destroy(flash);
	flash = NULL;
	data = NULL;

	/////////////////////////////////////////////////////////////////
	//// Read Identification Data
	/////////////////////////////////////////////////////////////////

	char c;
	printf("Press key to exit");
	scanf("%c", &c);
	return EXIT_SUCCESS;
}
