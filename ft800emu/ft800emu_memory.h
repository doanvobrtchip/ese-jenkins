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
#include "ft800emu_inttypes.h"

// Project includes

#define FT800EMU_DISPLAY_LIST_SIZE 2048
#define FT800EMU_ROM_FONTINFO 0xFFFFC // (RAM_DL - 4)

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

	static void begin();
	static void end();

	static uint8_t *getRam();
	static const uint32_t *getDisplayList();

	// Use separate functions for microcontroller access in case we need to put a hook on certain adresses for performance reasons.
	static void mcuWrite(size_t address, uint8_t data);
	static uint8_t mcuRead(size_t address);

	// Use separate functions for coprocessor access in case we need to put a hook on certain adresses for performance reasons.
	static void coprocessorWrite(size_t address, uint8_t data);
	static uint8_t coprocessorRead(size_t address);
	static void coprocessorWriteU32(size_t address, uint32_t data);
	static uint32_t coprocessorReadU32(size_t address);
	static void coprocessorWriteU16(size_t address, uint16_t data);
	static uint16_t coprocessorReadU16(size_t address);
	static void coprocessorWriteU8(size_t address, uint8_t data);
	static uint8_t coprocessorReadU8(size_t address);

	static FT800EMU_FORCE_INLINE void rawWriteU32(uint8_t *buffer, size_t address, uint32_t data);
	static FT800EMU_FORCE_INLINE uint32_t rawReadU32(uint8_t *buffer, size_t address);
	static FT800EMU_FORCE_INLINE void rawWriteU16(uint8_t *buffer, size_t address, uint16_t data);
	static FT800EMU_FORCE_INLINE uint16_t rawReadU16(uint8_t *buffer, size_t address);
	static FT800EMU_FORCE_INLINE void rawWriteU8(uint8_t *buffer, size_t address, uint8_t data);
	static FT800EMU_FORCE_INLINE uint8_t rawReadU8(uint8_t *buffer, size_t address);

	static void swapDisplayList();

	// Get nb of frames swapped without waiting for begin of frame render (when REG_PCLK == 0).
	static int getDirectSwapCount();

private:
	static FT800EMU_FORCE_INLINE void rawWriteU32(size_t address, uint32_t data);
	static FT800EMU_FORCE_INLINE uint32_t rawReadU32(size_t address);
	static FT800EMU_FORCE_INLINE void rawWriteU16(size_t address, uint16_t data);
	static FT800EMU_FORCE_INLINE uint16_t rawReadU16(size_t address);
	static FT800EMU_FORCE_INLINE void rawWriteU8(size_t address, uint8_t data);
	static FT800EMU_FORCE_INLINE uint8_t rawReadU8(size_t address);

	template<typename T>
    static FT800EMU_FORCE_INLINE void actionWrite(const size_t address, T &data);

	MemoryClass(const MemoryClass &);
	MemoryClass &operator=(const MemoryClass &);
	
}; /* class MemoryClass */

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU32(uint8_t *buffer, size_t address, uint32_t data)
{
	*static_cast<uint32_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT800EMU_FORCE_INLINE uint32_t MemoryClass::rawReadU32(uint8_t *buffer, size_t address)
{
	return *static_cast<uint32_t *>(static_cast<void *>(&buffer[address]));
}

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU16(uint8_t *buffer, size_t address, uint16_t data)
{
	*static_cast<uint16_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT800EMU_FORCE_INLINE uint16_t MemoryClass::rawReadU16(uint8_t *buffer, size_t address)
{
	return *static_cast<uint16_t *>(static_cast<void *>(&buffer[address]));
}

FT800EMU_FORCE_INLINE void MemoryClass::rawWriteU8(uint8_t *buffer, size_t address, uint8_t data)
{
	*static_cast<uint8_t *>(static_cast<void *>(&buffer[address])) = data;
}

FT800EMU_FORCE_INLINE uint8_t MemoryClass::rawReadU8(uint8_t *buffer, size_t address)
{
	return *static_cast<uint8_t *>(static_cast<void *>(&buffer[address]));
}

extern MemoryClass Memory;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_MEMORY_H */

/* end of file */
