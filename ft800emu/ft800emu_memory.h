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

#define FT800EMU_DISPLAY_LIST_SIZE 2048
#define FT800EMU_ROM_FONTINFO 0xFFFFC // (RAM_DL - 4)
#define FT800EMU_RAM_SIZE (4 * 1024 * 1024) // 4 MiB

#ifdef FT810EMU_MODE
#define FT800EMU_ADDR_MASK (0x3FFFFF)
#else
#define FT800EMU_ADDR_MASK (0xFFFFF)
#endif

namespace FT800EMU {

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

	static void begin(FT8XXEMU_EmulatorMode emulatorMode, const char *romFilePath = 0);
	static void end();

	static void enableReadDelay(bool enabled = true);

	static uint8_t *getRam();
	static const uint32_t *getDisplayList();

	static bool multiTouch();

	//static void setInterrupt(void (*interrupt)());
	static bool intnLow();
	static bool intnHigh();

	// Use separate functions for microcontroller access in case we need to put a hook on certain adresses for performance reasons.
	static void mcuWriteU32(size_t address, uint32_t data);
	static uint32_t mcuReadU32(size_t address);
	// static void mcuWrite(size_t address, uint8_t data);
	// static uint8_t mcuRead(size_t address);

	// Use separate functions for coprocessor access in case we need to put a hook on certain adresses for performance reasons.
	static void coprocessorWriteU32(size_t address, uint32_t data);
	static uint32_t coprocessorReadU32(size_t address);
	static void coprocessorWriteU16(size_t address, uint16_t data);
	static uint16_t coprocessorReadU16(size_t address);
	static void coprocessorWriteU8(size_t address, uint8_t data);
	static uint8_t coprocessorReadU8(size_t address);
	static bool coprocessorGetReset();

	static FT8XXEMU_FORCE_INLINE void rawWriteU32(uint8_t *buffer, size_t address, uint32_t data);
	static FT8XXEMU_FORCE_INLINE uint32_t rawReadU32(uint8_t *buffer, size_t address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU16(uint8_t *buffer, size_t address, uint16_t data);
	static FT8XXEMU_FORCE_INLINE uint16_t rawReadU16(uint8_t *buffer, size_t address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU8(uint8_t *buffer, size_t address, uint8_t data);
	static FT8XXEMU_FORCE_INLINE uint8_t rawReadU8(uint8_t *buffer, size_t address);

	static void swapDisplayList();

	// Get nb of frames swapped without waiting for begin of frame render (when REG_PCLK == 0).
	static int getDirectSwapCount();

	// Set touch screen xy for interpolation in time when touch is pressed
	static void setTouchScreenXY(int x, int y, int pressure);
	// Touch is not pressed
	static void resetTouchScreenXY();
	// Internal call used for touch interpolation
	static void setTouchScreenXYFrameTime(long micros);

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
	static FT8XXEMU_FORCE_INLINE void rawWriteU32(size_t address, uint32_t data);
	static FT8XXEMU_FORCE_INLINE uint32_t rawReadU32(size_t address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU16(size_t address, uint16_t data);
	static FT8XXEMU_FORCE_INLINE uint16_t rawReadU16(size_t address);
	static FT8XXEMU_FORCE_INLINE void rawWriteU8(size_t address, uint8_t data);
	static FT8XXEMU_FORCE_INLINE uint8_t rawReadU8(size_t address);

	template<typename T>
    static FT8XXEMU_FORCE_INLINE void actionWrite(const size_t address, T &data);
    static FT8XXEMU_FORCE_INLINE void postWrite(const size_t address);

	MemoryClass(const MemoryClass &);
	MemoryClass &operator=(const MemoryClass &);

}; /* class MemoryClass */

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU32(uint8_t *buffer, size_t address, uint32_t data)
{
	*static_cast<uint32_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT8XXEMU_FORCE_INLINE uint32_t MemoryClass::rawReadU32(uint8_t *buffer, size_t address)
{
	return *static_cast<uint32_t *>(static_cast<void *>(&buffer[address]));
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU16(uint8_t *buffer, size_t address, uint16_t data)
{
	*static_cast<uint16_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT8XXEMU_FORCE_INLINE uint16_t MemoryClass::rawReadU16(uint8_t *buffer, size_t address)
{
	return *static_cast<uint16_t *>(static_cast<void *>(&buffer[address]));
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU8(uint8_t *buffer, size_t address, uint8_t data)
{
	buffer[address] = data;
}

FT8XXEMU_FORCE_INLINE uint8_t MemoryClass::rawReadU8(uint8_t *buffer, size_t address)
{
	return buffer[address];
}

extern MemoryClass Memory;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_MEMORY_H */

/* end of file */
