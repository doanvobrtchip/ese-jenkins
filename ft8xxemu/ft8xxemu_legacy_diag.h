/*
 * FT8XX Emulator Library: Diagnostics
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

/* Shim for backwards compatibility. Deprecated. Do not redistribute this header. */

#ifndef FT8XXEMU_DIAG_H
#define FT8XXEMU_DIAG_H

#include "ft8xxemu_legacy.h"

#ifdef __cplusplus 
extern "C" {
#endif

FT8XXEMU_API extern uint8_t *(*FT8XXEMU_getRam)();
FT8XXEMU_API extern const uint32_t *(*FT8XXEMU_getDisplayList)();

FT8XXEMU_API extern void(*FT8XXEMU_poke)();

FT8XXEMU_API extern int *(*FT8XXEMU_getDisplayListCoprocessorWrites)();
FT8XXEMU_API extern void(*FT8XXEMU_clearDisplayListCoprocessorWrites)();

FT8XXEMU_API extern bool(*FT8XXEMU_getDebugLimiterEffective)();
FT8XXEMU_API extern int(*FT8XXEMU_getDebugLimiterIndex)();
FT8XXEMU_API extern void(*FT8XXEMU_setDebugLimiter)(int debugLimiter);

FT8XXEMU_API extern void(*FT8XXEMU_processTrace)(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

#ifdef __cplusplus 
} /* extern "C" */
#endif

#endif /* #ifndef FT8XXEMU_DIAG_H */

/* end of file */
