#ifdef _MSC_VER
#ifndef FTEMU_INTTYPES_DEFINED_BASE
#define FTEMU_INTTYPES_DEFINED_BASE
typedef unsigned __int8 uint8_t;
typedef signed __int8 int8_t;
typedef unsigned __int16 uint16_t;
typedef signed __int16 int16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int32 int32_t;
typedef unsigned __int64 uint64_t;
typedef signed __int64 int64_t;
#endif
#else
#include <stdint.h>
#include <stdlib.h>
#endif
#ifndef FTEMU_INTTYPES_DEFINED_FORCEINLINE
#define FTEMU_INTTYPES_DEFINED_FORCEINLINE
#ifdef _MSC_VER
#	define FTEMU_FORCE_INLINE __forceinline
#else
#	define FTEMU_FORCE_INLINE inline __attribute__((always_inline))
#endif
#endif
#ifndef FTEMU_DEBUG_COLORS
#define FTEMU_DEBUG_COLORS
#ifdef WIN32
#	define F9ED ""
#	define F9EW ""
#	define F9EE "\n"
#else
#	define F9ED "\033[0;34m"
#	define F9EW "\033[1;31m"
#	define F9EE "\033[0m\n"
#endif
#endif
