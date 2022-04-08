/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
BT880 Emulator Library
BT815 Emulator Library
BT817 Emulator Library
Copyright (C) 2016-2022  Bridgetek Pte Lte
*/

#ifndef FT800EMU_DEFS_H
#define FT800EMU_DEFS_H
#include "bt8xxemu.h"
#include "bt8xxemu_inttypes.h"


//! Display list size
#define FT800EMU_DISPLAY_LIST_SIZE 2048

//! RAM address space (total, not RAM_G)
#define FT800EMU_RAM_SIZE (4 * 1024 * 1024) // 4 MiB

//! ROM size
#if defined(BT817EMU_MODE)
#define FT800EMU_ROM_SIZE ((512 + 128) * 1024) // 640 KiB
#elif defined(FT810EMU_MODE) && !defined(BT880EMU_MODE)
#define FT800EMU_ROM_SIZE (1024 * 1024) // 1024 KiB
#else
#define FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB
#endif

//! ROM index
#if defined(FT810EMU_MODE)
#define FT800EMU_ROM_INDEX (RAM_DL - (1024 * 1024))
#else
#define FT800EMU_ROM_INDEX (RAM_DL - (256 * 1024))
#endif

//! ROM font info address
#define FT800EMU_ROM_FONTINFO (RAM_DL - 4)

//! OTP size
#define FT800EMU_OTP_SIZE (2048)

//! RAM_G addressing mask
#if defined(BT880EMU_MODE)
#define FT800EMU_ADDR_MASK (0x33FFFF)
#elif defined(FT810EMU_MODE)
#define FT800EMU_ADDR_MASK (0x3FFFFF)
#else
#define FT800EMU_ADDR_MASK (0xFFFFF)
#endif

//! Global addressing mask
#if defined(BT880EMU_MODE)
#define FT800EMU_MASK_ADDR(addr) (addr < RAM_DL ? (addr & FT800EMU_ADDR_MASK) : addr)
#else
#define FT800EMU_MASK_ADDR(addr) (addr)
#endif

//! Rotate macros
#ifdef FT810EMU_MODE
#define FT800EMU_REG_ROTATE_M(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#define FT800EMU_REG_ROTATE_S(ram) ((ram[REG_ROTATE] & 0x2) != 0)
#define FT800EMU_REG_ROTATE_X(ram) ((ram[REG_ROTATE] & 0x4) != 0) // Swap and mirror vertical
#define FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram) (FT800EMU_REG_ROTATE_M(ram) ^ FT800EMU_REG_ROTATE_X(ram))
#define FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram) (FT800EMU_REG_ROTATE_M(ram) ^ FT800EMU_REG_ROTATE_S(ram))
#define FT800EMU_REG_ROTATE_SWAP_XY(ram) FT800EMU_REG_ROTATE_S(ram)
#else
#define FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#define FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#endif

//! Debug break
#if defined(_MSC_VER) && !defined(NDEBUG)
#define FT800EMU_DEBUG_BREAK() __debugbreak()
#else
#define FT800EMU_DEBUG_BREAK() do { } while (false)
#endif


#endif /* #ifndef FT800EMU_DEFS_H */

/* end of file */
