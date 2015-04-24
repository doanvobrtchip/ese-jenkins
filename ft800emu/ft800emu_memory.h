/**
 * MemoryClass
 * $Id$
 * \file ft800emu_memory.h
 * \brief MemoryClass
 * \date 2013-06-21 21:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_MEMORY_H
#define FT800EMU_MEMORY_H
// #include <...>

// System includes
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"

// Project includes

#ifndef FT8XXEMU_NODEFS
#define FT800EMU_DISPLAY_LIST_SIZE 2048
#ifdef FT810EMU_MODE
#define FT800EMU_ROM_FONTINFO 0x2FFFFC // (RAM_DL - 4)
#else
#define FT800EMU_ROM_FONTINFO 0xFFFFC // (RAM_DL - 4)
#endif
#define FT800EMU_RAM_SIZE (4 * 1024 * 1024) // 4 MiB

#ifdef FT810EMU_MODE
#define FT800EMU_ADDR_MASK (0x3FFFFF)
#define FT800EMU_REG_ROTATE_M(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#define FT800EMU_REG_ROTATE_S(ram) ((ram[REG_ROTATE] & 0x2) != 0)
#define FT800EMU_REG_ROTATE_X(ram) ((ram[REG_ROTATE] & 0x4) != 0) // Swap and mirror vertical
#define FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram) (FT800EMU_REG_ROTATE_M(ram) ^ FT800EMU_REG_ROTATE_X(ram))
#define FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram) (FT800EMU_REG_ROTATE_M(ram) ^ FT800EMU_REG_ROTATE_S(ram))
#define FT800EMU_REG_ROTATE_SWAP_XY(ram) FT800EMU_REG_ROTATE_S(ram)
#else
#define FT800EMU_ADDR_MASK (0xFFFFF)
#define FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#define FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram) ((ram[REG_ROTATE] & 0x1) != 0)
#endif
#endif

namespace FT800EMU {

typedef int32_t ramaddr;

/**
 * MemoryClass
 * \brief MemoryClass
 * \date 2013-06-21 21:53GMT
 * \author Jan Boon (Kaetemi)
 */
class MemoryClass
{
public:
	MemoryClass() { }

	static void begin(FT8XXEMU_EmulatorMode emulatorMode, const char *romFilePath = 0, const char *otpFilePath = 0);
	static void end();

	static void enableReadDelay(bool enabled = true);

	static uint8_t *getRam();
	static const uint32_t *getDisplayList();

	//static void setInterrupt(void (*interrupt)());
	static bool intnLow();
	static bool intnHigh();

	// Use separate functions for microcontroller access in case we need to put a hook on certain adresses for performance reasons.
	static void mcuWriteU32(ramaddr address, uint32_t data);
	static uint32_t mcuReadU32(ramaddr address);
	// static void mcuWrite(size_t address, uint8_t data);
	// static uint8_t mcuRead(size_t address);

	// Use separate functions for coprocessor access in case we need to put a hook on certain adresses for performance reasons.
	static void coprocessorWriteU32(ramaddr address, uint32_t data);
	static uint32_t coprocessorReadU32(ramaddr address);
	static void coprocessorWriteU16(ramaddr address, uint16_t data);
	static uint16_t coprocessorReadU16(ramaddr address);
	static void coprocessorWriteU8(ramaddr address, uint8_t data);
	static uint8_t coprocessorReadU8(ramaddr address);
	static bool coprocessorGetReset();

	static FT8XXEMU_FORCE_INLINE void rawWriteU32(uint8_t *buffer, ramaddr address, uint32_t data);
	static FT8XXEMU_FORCE_INLINE uint32_t rawReadU32(uint8_t *buffer, ramaddr address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU16(uint8_t *buffer, ramaddr address, uint16_t data);
	static FT8XXEMU_FORCE_INLINE uint16_t rawReadU16(uint8_t *buffer, ramaddr address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU8(uint8_t *buffer, ramaddr address, uint8_t data);
	static FT8XXEMU_FORCE_INLINE uint8_t rawReadU8(uint8_t *buffer, ramaddr address);

	static void swapDisplayList();

	// Get nb of frames swapped without waiting for begin of frame render (when REG_PCLK == 0).
	static int getDirectSwapCount();

	// Tracking of coprocessor writes to display list
	static int *getDisplayListCoprocessorWrites();
	static void clearDisplayListCoprocessorWrites();

	// Gets the real swap count
	static int getRealSwapCount();
	// Gets a total count of write operations
	static int getWriteOpCount();
	// Increases the write op count
	static void poke();
	// Mark REG_DLSWAP as written
	static void flagDLSwap();

private:
	static FT8XXEMU_FORCE_INLINE void rawWriteU32(ramaddr address, uint32_t data);
	static FT8XXEMU_FORCE_INLINE uint32_t rawReadU32(ramaddr address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU16(ramaddr address, uint16_t data);
	static FT8XXEMU_FORCE_INLINE uint16_t rawReadU16(ramaddr address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU8(ramaddr address, uint8_t data);
	static FT8XXEMU_FORCE_INLINE uint8_t rawReadU8(ramaddr address);

	template<typename T>
	static FT8XXEMU_FORCE_INLINE void actionWrite(const ramaddr address, T &data);
	template<typename T>
	static FT8XXEMU_FORCE_INLINE void postWrite(const ramaddr address, T data);

	MemoryClass(const MemoryClass &);
	MemoryClass &operator=(const MemoryClass &);

}; /* class MemoryClass */

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU32(uint8_t *buffer, ramaddr address, uint32_t data)
{
	*static_cast<uint32_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT8XXEMU_FORCE_INLINE uint32_t MemoryClass::rawReadU32(uint8_t *buffer, ramaddr address)
{
	return *static_cast<uint32_t *>(static_cast<void *>(&buffer[address]));
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU16(uint8_t *buffer, ramaddr address, uint16_t data)
{
	*static_cast<uint16_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT8XXEMU_FORCE_INLINE uint16_t MemoryClass::rawReadU16(uint8_t *buffer, ramaddr address)
{
	return *static_cast<uint16_t *>(static_cast<void *>(&buffer[address]));
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU8(uint8_t *buffer, ramaddr address, uint8_t data)
{
	buffer[address] = data;
}

FT8XXEMU_FORCE_INLINE uint8_t MemoryClass::rawReadU8(uint8_t *buffer, ramaddr address)
{
	return buffer[address];
}

extern MemoryClass Memory;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_MEMORY_H */

/* end of file */
