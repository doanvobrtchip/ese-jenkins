/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

/*
Header required only for testing, diagnostic, and development tools. Not documented
*/

#ifndef BT8XXEMU_DIAG_H
#define BT8XXEMU_DIAG_H

#include "bt8xxemu.h"

#ifdef __cplusplus 
extern "C" {
#endif

BT8XXEMU_API extern uint8_t *BT8XXEMU_getRam(BT8XXEMU_Emulator *emulator);
BT8XXEMU_API extern const uint32_t *BT8XXEMU_getDisplayList(BT8XXEMU_Emulator *emulator);

BT8XXEMU_API extern void BT8XXEMU_poke(BT8XXEMU_Emulator *emulator);

BT8XXEMU_API extern int *BT8XXEMU_getDisplayListCoprocessorWrites(BT8XXEMU_Emulator *emulator);
BT8XXEMU_API extern void BT8XXEMU_clearDisplayListCoprocessorWrites(BT8XXEMU_Emulator *emulator);

BT8XXEMU_API extern int BT8XXEMU_getDebugLimiterEffective(BT8XXEMU_Emulator *emulator); // bool
BT8XXEMU_API extern int BT8XXEMU_getDebugLimiterIndex(BT8XXEMU_Emulator *emulator);
BT8XXEMU_API extern void BT8XXEMU_setDebugLimiter(BT8XXEMU_Emulator *emulator, int debugLimiter);

BT8XXEMU_API extern void BT8XXEMU_processTrace(BT8XXEMU_Emulator *emulator, int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

BT8XXEMU_API extern uint8_t *BT8XXEMU_Flash_data(BT8XXEMU_Flash *flash);
BT8XXEMU_API extern size_t BT8XXEMU_Flash_size(BT8XXEMU_Flash *flash);

#ifdef __cplusplus 
} /* extern "C" */
#endif

#endif /* #ifndef BT8XXEMU_DIAG_H */

/* end of file */
