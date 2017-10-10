/*
BT8XX Emulator Samples
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
*/

#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>

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
#define BTFLASH_CMD_RDP (0xAB) /* Release from Deep Power Down ¨*/
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

#define BTFLASH_DEVICE_TYPE L"mx25lemu"
#define BTFLASH_DATA_SIZE (8 * 1024 * 1024)
#define BTFLASH_DATA_FILE L"C:/source/ft800emu/reference/vc3roms/stdflash.bin"
#define BTFLASH_ELECTRONIC_ID 0x16
#define BTFLASH_MANUFACTURER_ID 0xC2
#define BTFLASH_DEVICE_ID 0x16
#define BTFLASH_MEMORY_TYPE 0x20
#define BTFLASH_MEMORY_DENSITY 0x17

static uint8_t lastValue = 0xFF;

static const uint8_t outMask1 = 0x31;

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

int main(int, char* [])
{
	printf("%s\n\n", BT8XXEMU_version());

	BT8XXEMU_FlashParameters flashParams;
	BT8XXEMU_Flash_defaults(BT8XXEMU_VERSION_API, &flashParams);
	flashParams.DeviceType = BTFLASH_DEVICE_TYPE;
	flashParams.DataSizeBytes = BTFLASH_DATA_SIZE;
	flashParams.DataFilePath = BTFLASH_DATA_FILE;
	BT8XXEMU_Flash *flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);

	uint8_t *data = BT8XXEMU_Flash_data(flash);
	assert(data[0] == 0x70);
	int size = BT8XXEMU_Flash_size(flash);
	assert(size == BTFLASH_DATA_SIZE);

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
		printf("READ (03h): %x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x6C);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_READ);
		transferU24(flash, size - 1);
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
		printf("FAST_READ (03h): %x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x6C);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FAST_READ);
		transferU24(flash, size - 1);
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
		printf("FASTDTRD (03h): %x-%x-%x-...\n", (int)reqd3, (int)reqd4, (int)reqd5);
		assert(reqd3 == 0x92);
		assert(reqd4 == 0x6C);
		assert(reqd5 == 0x00);
	}

	cableSelect(flash, false);
	cableSelect(flash, true);

	; {
		transferU8(flash, BTFLASH_CMD_FASTDTRD);
		transferDTU24(flash, size - 1);
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
		printf("Chip Erase\n");
		transferU8(flash, BTFLASH_CMD_CE_C7);
		// assert(data[0] == 0x70); // FIXME: Chip Erase only when CS goes high
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

	BT8XXEMU_Flash_destroy(flash);
	flash = NULL;
	char c;
	printf("Press key to exit");
	scanf("%c", &c);
	return EXIT_SUCCESS;
}
