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
	static const uint8_t *getRom();

	static void mcuWrite(size_t address, uint8_t data);
	static uint8_t mcuRead(size_t address);

	/// Return true if the registers were touched since the last call
	static bool untouchResolutionRegisters();

	static __forceinline void rawWriteU32(uint8_t *buffer, size_t address, uint32_t data);
	static __forceinline uint32_t rawReadU32(uint8_t *buffer, size_t address);

private:
	static __forceinline void rawWriteU32(size_t address, uint32_t data);

	MemoryClass(const MemoryClass &);
	MemoryClass &operator=(const MemoryClass &);
	
}; /* class MemoryClass */

__forceinline void MemoryClass::rawWriteU32(uint8_t *buffer, size_t address, uint32_t data)
{
	*static_cast<uint32_t *>(static_cast<void *>(&buffer[address])) = data;
}

__forceinline uint32_t MemoryClass::rawReadU32(uint8_t *buffer, size_t address)
{
	return *static_cast<uint32_t *>(static_cast<void *>(&buffer[address]));
}

extern MemoryClass Memory;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_MEMORY_H */

/* end of file */
