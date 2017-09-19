/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef BT8XXEMU_FLASH_H
#define BT8XXEMU_FLASH_H

#include "bt8xxemu.h"

namespace BT8XXEMU {
	class Flash;
}

typedef struct 
{
	void(*Destroy)(BT8XXEMU::Flash *flash);

	int(*Size)(BT8XXEMU::Flash *flash);
	void(*Cs)(BT8XXEMU::Flash *flash, bool cs);
	uint8_t(*Transfer)(BT8XXEMU::Flash *flash, uint8_t transfer);

} BT8XXEMU_FlashVTable;

namespace BT8XXEMU {

class Flash
{
public:
	const BT8XXEMU_FlashVTable *vTable() const { return m_VTable; }

protected:
	BT8XXEMU_FlashVTable *m_VTable;

};

}

#ifdef BT8XXEMU_FLASH_LIB

#ifdef WIN32
#	define BT8XXEMU_EXPORT __declspec(dllexport)
#else
#	define BT8XXEMU_EXPORT
#endif

extern "C" 
{

BT8XXEMU_EXPORT BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params);

}

#endif

#endif /* #ifndef BT8XXEMU_FLASH_H */

/* end of file */
