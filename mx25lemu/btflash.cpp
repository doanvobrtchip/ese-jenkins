/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include "bt8xxemu.h"
#include "bt8xxemu_flash.h"

#include <stdio.h>

extern BT8XXEMU_FlashVTable g_FlashVTable;

class Flash : public BT8XXEMU::Flash
{
public:
	Flash(const BT8XXEMU_FlashParameters *params) : BT8XXEMU::Flash(&g_FlashVTable)
	{
		static_assert(offsetof(Flash, m_VTable) == 0, "Incompatible C++ ABI");
	}

	~Flash()
	{

	}

	void chipSelect(bool cs)
	{

	}

	void writeProtect(bool wp)
	{

	}

	void hold(bool hold)
	{

	}

	uint8_t transfer(uint8_t data)
	{
		return 0;
	}

	uint8_t *Data;
	uint32_t Size;

private:

};

void Flash_destroy(Flash *flash)
{
	delete flash;
}

void Flash_chipSelect(Flash *flash, bool cs)
{
	flash->chipSelect(cs);
}

void Flash_writeProtect(Flash *flash, bool wp)
{
	flash->writeProtect(wp);
}

void Flash_hold(Flash *flash, bool hold)
{
	flash->hold(hold);
}

uint8_t Flash_transfer(Flash *flash, uint8_t data)
{
	return flash->transfer(data);
}

uint8_t *Flash_data(Flash *flash)
{
	return flash->Data;
}

uint32_t Flash_size(Flash *flash)
{
	return flash->Size;
}

BT8XXEMU_FlashVTable g_FlashVTable = {
	(void(*)(BT8XXEMU::Flash *))Flash_destroy,

	(void(*)(BT8XXEMU::Flash *, bool))Flash_chipSelect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_writeProtect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_hold,
	(uint8_t(*)(BT8XXEMU::Flash *, uint8_t))Flash_transfer,

	(uint8_t *(*)(BT8XXEMU::Flash *))Flash_data,
	(uint32_t(*)(BT8XXEMU::Flash *))Flash_size
};

BT8XXEMU_EXPORT BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return NULL;
	}

	return new Flash(params);
}

/* end of file */
