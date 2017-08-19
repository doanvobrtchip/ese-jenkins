/*
 * FT8XX Emulator Library: Diagnostics
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#ifndef BT8XXEMU_DIAG_H
#define BT8XXEMU_DIAG_H

#include "ft8xxemu.h"

#ifdef __cplusplus 
extern "C" {
#endif

BT8XXEMU_API extern uint8_t *BT8XXEMU_getRam();
BT8XXEMU_API extern const uint32_t *BT8XXEMU_getDisplayList();

BT8XXEMU_API extern void BT8XXEMU_poke();

BT8XXEMU_API extern int *BT8XXEMU_getDisplayListCoprocessorWrites();
BT8XXEMU_API extern void BT8XXEMU_clearDisplayListCoprocessorWrites();

BT8XXEMU_API extern int BT8XXEMU_getDebugLimiterEffective(); // bool
BT8XXEMU_API extern int BT8XXEMU_getDebugLimiterIndex();
BT8XXEMU_API extern void BT8XXEMU_setDebugLimiter(int debugLimiter);

BT8XXEMU_API extern void BT8XXEMU_processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

#ifdef __cplusplus 
} /* extern "C" */
#endif

#endif /* #ifndef BT8XXEMU_DIAG_H */

/* end of file */
