/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FT800EMU_MEMORY_H
#define FT800EMU_MEMORY_H
// #include <...>

// System includes
#include <mutex>
#include "ft8xxemu_thread_state.h"

// Project includes
#include "ft800emu_defs.h"
#include "bt8xxemu_flash.h"

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Emulator;
	class Touch;
	class GraphicsProcessor;
	class AudioProcessor;
	class AudioRender;

typedef int32_t ramaddr;
// typedef ramaddr flashaddr;

/**
 * Memory
 * \brief Memory
 * \date 2013-06-21 21:53GMT
 * \author Jan Boon (Kaetemi)
 */
class Memory
{
public:
	Memory(FT8XXEMU::System *system, BT8XXEMU_EmulatorMode emulatorMode, std::mutex &swapDLMutex,
		FT8XXEMU::ThreadState &threadMCU, FT8XXEMU::ThreadState &threadCoprocessor,
#ifdef BT815EMU_MODE
		BT8XXEMU_Flash *flash,
#endif
		const wchar_t *romFilePath = 0, const wchar_t *otpFilePath = 0);
	~Memory();

	void done();

	inline void setTouch(Touch *touch) { m_Touch = touch; }
	inline void setGraphicsProcessor(GraphicsProcessor *graphicsProcessor) { m_GraphicsProcessor = graphicsProcessor; }
	inline void setAudioProcessor(AudioProcessor *audioProcessor) { m_AudioProcessor = audioProcessor; }
	inline void setAudioRender(AudioRender *audioRender) { m_AudioRender = audioRender; }

	void enableReadDelay(bool enabled = true);

	inline uint8_t *getRam() { return m_Ram; }
	inline const uint32_t *getDisplayList() { return m_DisplayListActive; }

#ifdef BT815EMU_MODE
	inline uint8_t *getFlash() { return m_Flash ? m_Flash->vTable()->Data(m_Flash) : NULL; }
	// Voodoo magic...
	// static inline uint8_t *getFlashFromRamPtr(uint8_t *ram) { return (reinterpret_cast<Memory *>(ram) - offsetof(Memory, m_Ram))->getFlash(); }
#endif

#ifdef BT817EMU_MODE
	inline uint8_t spimi() { return m_Spimi; }
	inline uint8_t spimiL() { return m_SpimiL; }
#endif

	//static void setInterrupt(void (*interrupt)());
	bool hasInterrupt();

	// Use separate functions for microcontroller access in case we need to put a hook on certain adresses for performance reasons.
	void mcuWriteU32(ramaddr address, uint32_t data);
	uint32_t mcuReadU32(ramaddr address);

	// Use separate functions for coprocessor access in case we need to put a hook on certain adresses for performance reasons.
	void coprocessorWriteU32(ramaddr address, uint32_t data);
	uint32_t coprocessorReadU32(ramaddr address);
	void coprocessorWriteU16(ramaddr address, uint16_t data);
	uint16_t coprocessorReadU16(ramaddr address);
	void coprocessorWriteU8(ramaddr address, uint8_t data);
	uint8_t coprocessorReadU8(ramaddr address);
	bool coprocessorGetReset();

	static BT8XXEMU_FORCE_INLINE void rawWriteU32(uint8_t *buffer, ramaddr address, uint32_t data);
	static BT8XXEMU_FORCE_INLINE uint32_t rawReadU32(uint8_t *buffer, ramaddr address);
	static BT8XXEMU_FORCE_INLINE void rawWriteU16(uint8_t *buffer, ramaddr address, uint16_t data);
	static BT8XXEMU_FORCE_INLINE uint16_t rawReadU16(uint8_t *buffer, ramaddr address);
	static BT8XXEMU_FORCE_INLINE void rawWriteU8(uint8_t *buffer, ramaddr address, uint8_t data);
	static BT8XXEMU_FORCE_INLINE uint8_t rawReadU8(uint8_t *buffer, ramaddr address);

	void swapDisplayList();

	// Get nb of frames swapped without waiting for begin of frame render (when REG_PCLK == 0).
	int getDirectSwapCount();

	// Tracking of coprocessor writes to display list
	int *getDisplayListCoprocessorWrites();
	void clearDisplayListCoprocessorWrites();

	// Gets the real swap count
	int getRealSwapCount();
	// Gets a total count of write operations
	int getWriteOpCount();
	// Increases the write op count
	void poke();
	// Mark REG_DLSWAP as written
	void flagDLSwap();

private:
	// RAM
	union
	{
		uint32_t m_RamU32[FT800EMU_RAM_SIZE / sizeof(uint32_t)] = { 0 };
		uint8_t m_Ram[FT800EMU_RAM_SIZE];
	};
	uint32_t m_DisplayListA[FT800EMU_DISPLAY_LIST_SIZE];
	uint32_t m_DisplayListB[FT800EMU_DISPLAY_LIST_SIZE];
	uint32_t *m_DisplayListActive = m_DisplayListA;
	uint32_t *m_DisplayListFree = m_DisplayListB;

	// Diagnostics
	int m_DisplayListCoprocessorWrites[FT800EMU_DISPLAY_LIST_SIZE];
	ramaddr m_LastCoprocessorCommandRead = -1;

	// Optimization counters
	int m_DirectSwapCount;
	int m_RealSwapCount;
	int m_WriteOpCount;
	// bool m_CoprocessorWritesDL;

	// Threading
	std::mutex &m_SwapDLMutex;
	FT8XXEMU::ThreadState &m_ThreadMCU;
	FT8XXEMU::ThreadState &m_ThreadCoprocessor;

	// Dependencies
	FT8XXEMU::System *m_System;
	Touch *m_Touch = NULL;
	GraphicsProcessor *m_GraphicsProcessor = NULL;
	AudioProcessor *m_AudioProcessor = NULL;
	AudioRender *m_AudioRender = NULL;

	// Avoid getting hammered in wait loops
	ramaddr m_LastCoprocessorRead = -1;
	int m_IdenticalCoprocessorReadCounter = 0;
	int m_SwapCoprocessorReadCounter = 0;
	int m_WaitCoprocessorReadCounter = 0;
#ifdef FT810EMU_MODE
	int m_FifoCoprocessorReadCounter = 0;
#endif

	ramaddr m_LastMCURead = -1;
	int m_IdenticalMCUReadCounter = 0;
	int m_WaitMCUReadCounter = 0;
	int m_SwapMCUReadCounter = 0;
#ifdef FT810EMU_MODE
	int m_FifoMCUReadCounter = 0;
#endif

	uint32_t m_OverrideRasterY = 0;
#ifndef FT810EMU_MODE
	int m_HasCachedTouchRawXY = 0;
	uint32_t m_CachedTouchRawXY = 0xFFFFFFFF;
#endif

#ifdef BT817EMU_MODE
	bool m_SpimRising;
	uint8_t m_Spimi;
	uint8_t m_SpimiL;
#endif

	bool m_ReadDelay = false;

	bool m_CpuReset = false;

	BT8XXEMU_EmulatorMode m_EmulatorMode;

#ifdef BT815EMU_MODE
	BT8XXEMU_Flash *m_Flash;
#endif

private:
	BT8XXEMU_FORCE_INLINE void rawWriteU32(ramaddr address, uint32_t data);
	BT8XXEMU_FORCE_INLINE uint32_t rawReadU32(ramaddr address);
	BT8XXEMU_FORCE_INLINE void rawWriteU16(ramaddr address, uint16_t data);
	BT8XXEMU_FORCE_INLINE uint16_t rawReadU16(ramaddr address);
	BT8XXEMU_FORCE_INLINE void rawWriteU8(ramaddr address, uint8_t data);
	BT8XXEMU_FORCE_INLINE uint8_t rawReadU8(ramaddr address);

	template<typename T>
	BT8XXEMU_FORCE_INLINE void actionWrite(const ramaddr address, T &data);
	template<typename T>
	BT8XXEMU_FORCE_INLINE void postWrite(const ramaddr address, T data);

	Memory(const Memory &);
	Memory &operator=(const Memory &);

}; /* class Memory */

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU32(uint8_t *buffer, ramaddr address, uint32_t data)
{
	*static_cast<uint32_t *>(static_cast<void *>(&buffer[address])) = data;
}

BT8XXEMU_FORCE_INLINE uint32_t Memory::rawReadU32(uint8_t *buffer, ramaddr address)
{
	return *static_cast<uint32_t *>(static_cast<void *>(&buffer[address]));
}

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU16(uint8_t *buffer, ramaddr address, uint16_t data)
{
	*static_cast<uint16_t *>(static_cast<void *>(&buffer[address])) = data;
}

BT8XXEMU_FORCE_INLINE uint16_t Memory::rawReadU16(uint8_t *buffer, ramaddr address)
{
	return *static_cast<uint16_t *>(static_cast<void *>(&buffer[address]));
}

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU8(uint8_t *buffer, ramaddr address, uint8_t data)
{
	buffer[address] = data;
}

BT8XXEMU_FORCE_INLINE uint8_t Memory::rawReadU8(uint8_t *buffer, ramaddr address)
{
	return buffer[address];
}

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_MEMORY_H */

/* end of file */
