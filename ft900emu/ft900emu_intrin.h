/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_INTRIN_H
#define FT800EMU_INTRIN_H
// #include <...>
#include <xmmintrin.h>

#define FT900EMU_DEBUG_BREAK_ENABLED 1

#if FT900EMU_DEBUG_BREAK_ENABLED
#ifdef _MSC_VER
#define FT900EMU_DEBUG_BREAK __debugbreak
#else
#define FT900EMU_DEBUG_BREAK __builtin_trap
// #define FT900EMU_DEBUG_BREAK abort
#endif
#else
#define FT900EMU_DEBUG_BREAK()
#endif

#endif /* #ifndef FT900EMU_INTRIN_H */

/* end of file */
