/**
 * Defs
 * $Id$
 * \file ft800emu_defs.h
 * \brief Defs
 * \date 2017-08-09 14:12GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_DEFS_H
#define FT800EMU_DEFS_H
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"


//! Display list size
#define FT800EMU_DISPLAY_LIST_SIZE 2048

//! ROM font info address
#ifdef FT810EMU_MODE
#define FT800EMU_ROM_FONTINFO 0x2FFFFC // (RAM_DL - 4)
#else
#define FT800EMU_ROM_FONTINFO 0xFFFFC // (RAM_DL - 4)
#endif

//! RAM address space
#define FT800EMU_RAM_SIZE (4 * 1024 * 1024) // 4 MiB

//! ROM size
#ifdef FT810EMU_MODE
#define FT800EMU_ROM_SIZE (1024 * 1024) // 1024 KiB
#else
#define FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB
#endif

//! ROM index
#define FT800EMU_ROM_INDEX (RAM_DL - FT800EMU_ROM_SIZE) //(RAM_DL - FT800EMU_ROM_SIZE)

//! OTP size
#ifdef FT810EMU_MODE
#define FT800EMU_OTP_SIZE (2048)
#else
#define FT800EMU_OTP_SIZE (2048)
#endif

//! RAM_G addressing mask
#ifdef FT810EMU_MODE
#define FT800EMU_ADDR_MASK (0x3FFFFF)
#else
#define FT800EMU_ADDR_MASK (0xFFFFF)
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


#endif /* #ifndef FT800EMU_DEFS_H */

/* end of file */
