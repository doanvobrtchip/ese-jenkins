/*
BT8XX Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

/*
Header required only for implementing a flash emulation module for the BT8XX Emulator Library. Not documented
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

	uint8_t(*TransferSpi4)(BT8XXEMU::Flash *flash, uint8_t signal);

	uint8_t *(*Data)(BT8XXEMU::Flash *flash);
	size_t(*Size)(BT8XXEMU::Flash *flash);

} BT8XXEMU_FlashVTable;

namespace BT8XXEMU {

class Flash
{
public:
	Flash(BT8XXEMU_FlashVTable *vTable) : m_VTable(vTable) { static_assert(offsetof(Flash, m_VTable) == 0, "Incompatible C++ ABI"); }

	const BT8XXEMU_FlashVTable *vTable() const { return m_VTable; }

protected:
	BT8XXEMU_FlashVTable *m_VTable;

};

}

#ifdef BT8XXEMU_FLASH_LIBRARY

#ifdef WIN32
#	define BT8XXEMU_EXPORT __declspec(dllexport)
#else
#	define BT8XXEMU_EXPORT
#endif

extern "C" 
{

BT8XXEMU_EXPORT extern BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params);

}

#endif

#endif /* #ifndef BT8XXEMU_FLASH_H */

/* end of file */
