/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_INTRIN_H
#define FT800EMU_INTRIN_H
// #include <...>

#ifdef _MSC_VER
#define FT900EMU_DEBUG_BREAK __debugbreak
#else
#define FT900EMU_DEBUG_BREAK __builtin_trap
#endif

#endif /* #ifndef FT900EMU_INTRIN_H */

/* end of file */
