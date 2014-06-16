/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_INTRIN_H
#define FT800EMU_INTRIN_H
// #include <...>

#ifdef __x86_64__
#define EFLAGS_TYPE unsigned long long int
#else
#define EFLAGS_TYPE unsigned int
#endif

// Patch for GCC http://patchwork.ozlabs.org/patch/296587/ ->
#ifndef _MSC_VER
#ifdef __x86_64__
/* Read flags register */
static __inline unsigned long long
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
__readeflags (void)
{
	unsigned long long result = 0;
	__asm__ __volatile__ ("pushf\n\t"
			"popq %0\n"
			:"=r"(result)
			:
			:
			);
	return result;
}

/* Write flags register */
static __inline void
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
__writeeflags (unsigned long long X)
{
	__asm__ __volatile__ ("pushq %0\n\t"
			"popf\n"
			:
			:"r"(X)
			:"flags"
			);
}
#endif
#endif
// <-

#ifdef _MSC_VER
#define FT900EMU_DEBUG_BREAK __debugbreak
#else
#define FT900EMU_DEBUG_BREAK __builtin_trap
#endif

#endif /* #ifndef FT900EMU_INTRIN_H */

/* end of file */
