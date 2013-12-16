#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef signed __int8 int8_t;
typedef unsigned __int16 uint16_t;
typedef signed __int16 int16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int32 int32_t;
typedef unsigned __int64 uint64_t;
typedef signed __int64 int64_t;
#else
#include <stdint.h>
#include <stdlib.h>
#endif
typedef uint32_t argb8888;
#ifdef WIN32
#	define FT800EMU_FORCE_INLINE __forceinline
#else
#	define FT800EMU_FORCE_INLINE inline __attribute__((always_inline))
#endif
