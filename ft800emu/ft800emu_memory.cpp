/**
 * MemoryClass
 * $Id$
 * \file ft800emu_memory.cpp
 * \brief MemoryClass
 * \date 2013-06-21 21:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_memory.h"
#include "ft8xxemu_system_windows.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft8xxemu.h"
#include "ft800emu_vc.h"
#include "ft8xxemu_system.h"
#include "ft8xxemu_graphics_driver.h"
#include "ft800emu_graphics_processor.h"
#include "ft800emu_audio_processor.h"
#include "ft800emu_audio_render.h"

// using namespace ...;

#ifdef FT810EMU_MODE
#define FT800EMU_ROM_SIZE (1024 * 1024) // 1024 KiB
#else
#define FT800EMU_ROM_SIZE (256 * 1024) // 256 KiB
#endif
#define FT800EMU_ROM_INDEX (RAM_DL - FT800EMU_ROM_SIZE) //(RAM_DL - FT800EMU_ROM_SIZE)

#define FT800EMU_COPROCESSOR_MEMLOG 0

namespace FT800EMU {

MemoryClass Memory;

// RAM
static uint32_t s_RamU32[FT800EMU_RAM_SIZE / sizeof(uint32_t)];
static uint8_t *s_Ram = static_cast<uint8_t *>(static_cast<void *>(s_RamU32));
static uint32_t s_DisplayListA[FT800EMU_DISPLAY_LIST_SIZE];
static uint32_t s_DisplayListB[FT800EMU_DISPLAY_LIST_SIZE];
static uint32_t *s_DisplayListActive = s_DisplayListA;
static uint32_t *s_DisplayListFree = s_DisplayListB;

static int s_DisplayListCoprocessorWrites[FT800EMU_DISPLAY_LIST_SIZE];
static size_t s_LastCoprocessorCommandRead = -1;

static int s_DirectSwapCount;
static int s_RealSwapCount;
static int s_WriteOpCount;

// Avoid getting hammered in wait loops
static size_t s_LastCoprocessorRead = -1;
static int s_IdenticalCoprocessorReadCounter = 0;
static int s_SwapCoprocessorReadCounter = 0;
static int s_WaitCoprocessorReadCounter = 0;

static size_t s_LastMCURead = -1;
static int s_IdenticalMCUReadCounter = 0;
static int s_WaitMCUReadCounter = 0;
static int s_SwapMCUReadCounter = 0;

static bool s_ReadDelay;

static long s_TouchScreenSet = 0;
static int s_TouchScreenX1;
static int s_TouchScreenX2;
static int s_TouchScreenY1;
static int s_TouchScreenY2;
static long s_TouchScreenFrameTime;
static bool s_TouchScreenJitter = true;

static bool s_CpuReset = false;

static FT8XXEMU_EmulatorMode s_EmulatorMode;
static bool s_EnableTouchMatrix = false;

//static void (*s_Interrupt)());

long s_LastJitteredTime;
long s_LastDeltas[8] = { 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000 };
long s_LastDeltaI = 0;
inline long jitteredTime(long micros)
{
	long delta = micros - s_LastJitteredTime;
	if (delta > 0)
	{
		const long od = delta;
		long avgLastDelta = 0;
		for (int i = 0; i < 8; ++i)
			avgLastDelta += s_LastDeltas[i];
		avgLastDelta /= 8;
		if (avgLastDelta < delta)
		{
			delta = avgLastDelta;
		}
		static const long deltadiv = 3;
		if (delta >= deltadiv)
		{
			delta /= deltadiv;
			delta = (long)rand() % delta;
			s_LastJitteredTime += delta;
		}
		if (od > 0)
		{
			s_LastDeltas[s_LastDeltaI] = od;
			++s_LastDeltaI;
			s_LastDeltaI %= 8;
		}
	}
	return s_LastJitteredTime;
}

inline void transformTouchXY(int &x, int &y)
{
	uint8_t *ram = Memory.getRam();
	// Transform (currently just depending on REG_ROTATE, ignoring TRANSFORM)
	if (s_EnableTouchMatrix) // s15.16 matrix
	{
		const int64_t xe = x;
		const int64_t ye = y;
		const int64_t transformA = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_A / sizeof(uint32_t)]);
		const int64_t transformB = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_B / sizeof(uint32_t)]);
		const int64_t transformC = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_C / sizeof(uint32_t)]);
		const int64_t transformD = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_D / sizeof(uint32_t)]);
		const int64_t transformE = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_E / sizeof(uint32_t)]);
		const int64_t transformF = reinterpret_cast<const int32_t &>(s_RamU32[REG_TOUCH_TRANSFORM_F / sizeof(uint32_t)]);
		int64_t xtf = (transformA * xe) + (transformB * ye) + transformC;
		int64_t ytf = (transformD * xe) + (transformE * ye) + transformF;
		x = (xtf >> 16);
		y = (ytf >> 16);
		// printf("x: %i ,y: %i\n", x, y);
	}

	/*
	if (Memory.rawReadU32(ram, REG_ROTATE) & 0x01)
	{
		// printf("rotated\n");
		x = Memory.rawReadU32(ram, REG_HSIZE) - x - 1;
		y = Memory.rawReadU32(ram, REG_VSIZE) - y - 1;
	}
	*/
}

inline void getTouchScreenXY(long micros, int &x, int &y, bool jitter)
{
	long delta;//
	if (jitter)
	{
		delta = jitteredTime(micros) - s_TouchScreenSet;
	}
	else
	{
		delta = micros - s_TouchScreenSet;
	}
	/*if (s_TouchScreenJitter)
	{
		delta += ((rand() % 2000) - 1000) * s_TouchScreenFrameTime / 1000; // add some time jitter
	}*/
	if (delta < 0)
	{
		x = s_TouchScreenX1;
		y = s_TouchScreenY1;
	}
	else if (delta >= s_TouchScreenFrameTime)
	{
		x = s_TouchScreenX2;
		y = s_TouchScreenY2;
	}
	else
	{
		long xdelta = s_TouchScreenX2 - s_TouchScreenX1;
		long ydelta = s_TouchScreenY2 - s_TouchScreenY1;
		x = s_TouchScreenX1 + (int)(xdelta * delta / s_TouchScreenFrameTime);
		y = s_TouchScreenY1 + (int)(ydelta * delta / s_TouchScreenFrameTime);
	}
	if (jitter)
	{
		x += ((rand() % 2000) - 1000) / 1000;
		y += ((rand() % 2000) - 1000) / 1000;
	}
	// transformTouchXY(x, y);
}

inline void getTouchScreenXY(long micros, int &x, int &y)
{
	getTouchScreenXY(micros, x, y, s_TouchScreenJitter);
}

inline uint32_t getTouchScreenXY(int x, int y)
{
	uint16_t const touch_screen_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_screen_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_screen_xy = (uint32_t)touch_screen_x << 16 | touch_screen_y;
	return touch_screen_xy;
}

inline uint32_t getTouchScreenXY(long micros)
{
	if (s_TouchScreenSet)
	{
		int x, y;
		getTouchScreenXY(micros, x, y);
		transformTouchXY(x, y);
		return getTouchScreenXY(x, y);
	}
	else
	{
		return 0x80008000;
	}
}

inline uint32_t getTouchScreenXY()
{
	long micros = FT8XXEMU::System.getMicros();
	return getTouchScreenXY(micros);
}

void MemoryClass::setTouchScreenXY(int x, int y, int pressure)
{
	++s_WriteOpCount;

	uint16_t const touch_raw_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_raw_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_raw_xy = (uint32_t)touch_raw_x << 16 | touch_raw_y;
	if (multiTouch())
	{
		// no-op
	}
	else
	{
		rawWriteU32(REG_TOUCH_RAW_XY, touch_raw_xy);
	}
	if (s_TouchScreenSet)
	{
		if (s_TouchScreenJitter)
		{
			long micros = jitteredTime(FT8XXEMU::System.getMicros());
			getTouchScreenXY(micros, s_TouchScreenX1, s_TouchScreenY1, false);
			//s_TouchScreenX1 = s_TouchScreenX2;
			//s_TouchScreenY1 = s_TouchScreenY2;
			s_TouchScreenSet = micros;
		}
		else
		{
			long micros = FT8XXEMU::System.getMicros();
			s_TouchScreenX1 = s_TouchScreenX2;
			s_TouchScreenY1 = s_TouchScreenY2;
			s_TouchScreenSet = micros;
		}
	}
	else
	{
		long micros = FT8XXEMU::System.getMicros();
		s_LastJitteredTime = micros;
		s_TouchScreenX1 = x;
		s_TouchScreenY1 = y;
		s_TouchScreenSet = micros;
	}
	s_TouchScreenX2 = x;
	s_TouchScreenY2 = y;

	transformTouchXY(x, y);
	if (multiTouch())
	{
		rawWriteU32(REG_CTOUCH_TOUCH0_XY, getTouchScreenXY(x, y));
	}
	else
	{
		rawWriteU32(REG_TOUCH_SCREEN_XY, getTouchScreenXY(x, y));
		rawWriteU32(REG_TOUCH_RZ, pressure);
	}
}

void MemoryClass::setTouchScreenXYFrameTime(long micros)
{
	s_TouchScreenFrameTime = micros; // * 3 / 2;
}

void MemoryClass::resetTouchScreenXY()
{
	s_TouchScreenSet = 0;
	Memory.rawWriteU32(REG_TOUCH_TAG, 0);
	if (multiTouch())
	{
		// No touch detected
		rawWriteU32(REG_CTOUCH_TOUCH0_XY, 0x80008000);
		rawWriteU32(REG_CTOUCH_TOUCH1_XY, 0x80008000);
		rawWriteU32(REG_CTOUCH_TOUCH2_XY, 0x80008000);
		rawWriteU32(REG_CTOUCH_TOUCH3_XY, 0x80008000);
		rawWriteU32(REG_CTOUCH_TOUCH4_X, 0x8000);
		rawWriteU32(REG_CTOUCH_TOUCH4_Y, 0x8000);
	}
	else
	{
		rawWriteU32(REG_TOUCH_RZ, 32767);
		rawWriteU32(REG_TOUCH_RAW_XY, 0xFFFFFFFF);
		rawWriteU32(REG_TOUCH_SCREEN_XY, 0x80008000);
	}
	s_LastJitteredTime = FT8XXEMU::System.getMicros();
}

int MemoryClass::getDirectSwapCount()
{
	return s_DirectSwapCount;
}

int MemoryClass::getWriteOpCount()
{
	return s_WriteOpCount;
}

void MemoryClass::poke()
{
	++s_WriteOpCount;
}

/*void MemoryClass::setInterrupt(void (*interrupt)())
{
	s_Interrupt = interrupt;
}*/

bool MemoryClass::intnLow()
{
	uint8_t en = rawReadU8(REG_INT_EN);
	if (en & 0x01)
	{
		uint32_t mask = rawReadU32(REG_INT_MASK);
		uint32_t flags = rawReadU32(REG_INT_FLAGS);
		return (mask & flags) != 0;
	}
	else
	{
		return false;
	}
}

bool MemoryClass::intnHigh()
{
	return !intnLow();
}

bool MemoryClass::coprocessorGetReset()
{
	bool result = s_CpuReset || (rawReadU8(REG_CPURESET) & 0x01);
	s_CpuReset = false;
	return result;
}

void MemoryClass::enableTouchMatrix(bool enabled)
{
	s_EnableTouchMatrix = enabled;
}

template<typename T>
FT8XXEMU_FORCE_INLINE void MemoryClass::actionWrite(const size_t address, T &data)
{
	// switches for 1 byte regs
	// least significant byte
	if (address % 4 == 0)
	{
		switch (address)
		{
		case REG_PCLK:
			// printf("Write REG_PCLK %u\n", (uint32_t)data);
			// if (data == 0 && s_Ram[REG_DLSWAP] == DLSWAP_FRAME)
			if (data == 0 && (s_Ram[REG_DLSWAP] == DLSWAP_FRAME || s_Ram[REG_DLSWAP] == DLSWAP_LINE))
			{
				// printf("Direct swap from REG_PCLK\n");
				// Direct swap
				FT8XXEMU::System.enterSwapDL();
				// if (data == 0 && s_Ram[REG_DLSWAP] == DLSWAP_FRAME)
				if (data == 0 && (s_Ram[REG_DLSWAP] == DLSWAP_FRAME || s_Ram[REG_DLSWAP] == DLSWAP_LINE))
				{
					swapDisplayList();
					s_Ram[REG_DLSWAP] = 0;
					flagDLSwap();
					GraphicsProcessor.processBlank();
					++s_DirectSwapCount;
				}
				FT8XXEMU::System.leaveSwapDL();
			}
			break;
		case REG_DLSWAP:
			// printf("Write REG_DLSWAP %u\n", data);
			// if (data == DLSWAP_FRAME && s_Ram[REG_PCLK] == 0)
			if ((data == DLSWAP_FRAME || data == DLSWAP_LINE) && s_Ram[REG_PCLK] == 0)
			{
				// printf("Direct swap from DLSWAP_FRAME\n");
				// Direct swap
				FT8XXEMU::System.enterSwapDL();
				// if (data == DLSWAP_FRAME && s_Ram[REG_PCLK] == 0)
				if ((data == DLSWAP_FRAME || data == DLSWAP_LINE) && s_Ram[REG_PCLK] == 0)
				{
					// printf("Go\n");
					swapDisplayList();
					data = 0;
					GraphicsProcessor.processBlank();
					++s_DirectSwapCount;
				}
				// else
				// {
					// printf("No go\n");
				// }
				FT8XXEMU::System.leaveSwapDL();
			}
			break;
		// case REG_VOL_SOUND:
		// 	printf("REG_VOL_SOUND %i\n", data);
		// 	break;
		// case REG_SOUND:
		// 	printf("REG_SOUND %i\n", data);
		// 	break;
		case REG_PLAY:
			if (data & 0x01)
			{
				// printf("REG_PLAY\n");
				AudioProcessor.play();
			}
			break;
		case REG_PLAYBACK_PLAY:
			if (data & 0x01)
			{
				// printf("REG_PLAYBACK_PLAY\n");
				AudioRender.playbackPlay();
			}
			break;
		case REG_CPURESET:
			if (data & 0x01)
			{
				// TODO: Perhaps this should lock until the cpu reset is actually pushed through to ensure following commands go through...
				s_CpuReset = true;
			}
			break;
		case REG_SNAPSHOT:
			if (data & 1)
			{
				rawWriteU32(REG_BUSYBITS, 0xFFFFFFFF); // ?
			}
			break;
#ifdef FT810EMU_MODE
		case REG_CMD_READ:
			data &= 0xFFF;
			break;
		case REG_CMD_WRITE:
			data &= 0xFFF;
			break;
		case REG_CMD_DL:
			data &= 0x1FFF;
			break;
		case REG_RAM_FOLD:
			data &= 0x3;
			break;
		case REG_CMDB_WRITE:
		{
			uint32_t wp = rawReadU32(REG_CMD_WRITE) & 0xFFF;
			rawWriteU32(RAM_CMD + wp, data);
			wp += 4;
			wp &= 0xFFF;
			rawWriteU32(REG_CMD_WRITE, wp);
			break;
		}
#endif
		}
	}
}

template<typename T>
FT8XXEMU_FORCE_INLINE void MemoryClass::postWrite(const size_t address, const T data)
{
	// switches for 1 byte regs
	// least significant byte
	if (address % 4 == 0)
	{
		switch (address)
		{
		case REG_DLSWAP:
			flagDLSwap();
			break;
		case REG_CTOUCH_EXTENDED:
#ifndef FT810EMU_MODE
			if (s_EmulatorMode >= FT8XXEMU_EmulatorFT801)
#endif
			{
				if (!multiTouch())
				{
					rawWriteU32(REG_TOUCH_DIRECT_XY, 0); // REG_CTOUCH_TOUCHB_XY
					rawWriteU32(REG_TOUCH_DIRECT_Z1Z2, 0); // REG_CTOUCH_TOUCHC_XY
					rawWriteU32(REG_ANALOG, 0); // REG_CTOUCH_TOUCH4_X
				}
				resetTouchScreenXY();
			}
			break;
		case REG_SNAPSHOT:
			if (data & 1)
			{
				FT8XXEMU::System.renderWake();
			}
			break;
		}
	}
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU32(size_t address, uint32_t data)
{
	rawWriteU32(s_Ram, address, data);
}

FT8XXEMU_FORCE_INLINE uint32_t MemoryClass::rawReadU32(size_t address)
{
	return rawReadU32(s_Ram, address);
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU16(size_t address, uint16_t data)
{
	rawWriteU16(s_Ram, address, data);
}

FT8XXEMU_FORCE_INLINE uint16_t MemoryClass::rawReadU16(size_t address)
{
	return rawReadU16(s_Ram, address);
}

FT8XXEMU_FORCE_INLINE void MemoryClass::rawWriteU8(size_t address, uint8_t data)
{
	rawWriteU8(s_Ram, address, data);
}

FT8XXEMU_FORCE_INLINE uint8_t MemoryClass::rawReadU8(size_t address)
{
	return rawReadU8(s_Ram, address);
}

#ifdef FT810EMU_MODE
static const uint8_t s_RomFT810[FT800EMU_ROM_SIZE] = { // TODO_FT810EMU: Correct ROM
#include "resources/rom_ft810.h"
};
#else
static const uint8_t s_RomFT800[FT800EMU_ROM_SIZE] = {
#include "resources/rom_ft800.h"
};
static const uint8_t s_RomFT801[FT800EMU_ROM_SIZE] = {
#include "resources/rom_ft801.h"
};
#endif

void MemoryClass::begin(FT8XXEMU_EmulatorMode emulatorMode, const char *romFilePath)
{
	FT8XXEMU::g_SetTouchScreenXY = &setTouchScreenXY;
	FT8XXEMU::g_ResetTouchScreenXY = &resetTouchScreenXY;
	if (romFilePath)
	{
		FILE *f;
		f = fopen(romFilePath, "rb");
		if (!f) printf("Failed to open ROM file\n");
		else
		{
			size_t s = fread(&s_Ram[FT800EMU_ROM_INDEX], 1, FT800EMU_ROM_SIZE, f);
			if (s != FT800EMU_ROM_SIZE) printf("Incomplete ROM file\n");
			else printf("Loaded ROM file\n");
			if (fclose(f)) printf("Error closing ROM file\n");
		}
	}
	else
	{
#ifdef FT810EMU_MODE
		memcpy(&s_Ram[FT800EMU_ROM_INDEX], s_RomFT810, sizeof(s_RomFT810));
#else
		if (emulatorMode >= FT8XXEMU_EmulatorFT801) memcpy(&s_Ram[FT800EMU_ROM_INDEX], s_RomFT801, sizeof(s_RomFT801));
		else memcpy(&s_Ram[FT800EMU_ROM_INDEX], s_RomFT800, sizeof(s_RomFT800));
#endif
	}

	s_DirectSwapCount = 0;
	s_RealSwapCount = 0;
	s_WriteOpCount = 0;

	s_ReadDelay = false;

	s_LastCoprocessorRead = -1;
	s_IdenticalCoprocessorReadCounter = 0;
	s_SwapCoprocessorReadCounter = 0;
	s_WaitCoprocessorReadCounter = 0;

	s_LastMCURead = -1;
	s_IdenticalMCUReadCounter = 0;
	s_WaitMCUReadCounter = 0;
	s_SwapMCUReadCounter = 0;
	s_TouchScreenSet = 0;
	s_LastJitteredTime = FT8XXEMU::System.getMicros();

	s_EmulatorMode = emulatorMode;

	rawWriteU32(REG_ID, 0x7C);
	rawWriteU32(REG_FRAMES, 0); // Frame counter - is this updated before or after frame render?
	rawWriteU32(REG_CLOCK, 0);
	rawWriteU32(REG_FREQUENCY, 48000000);
	rawWriteU32(REG_RENDERMODE, 0);
	rawWriteU32(REG_SNAPY, 0);
	rawWriteU32(REG_SNAPSHOT, 0);
	rawWriteU32(REG_CPURESET, 0);
	rawWriteU32(REG_TAP_CRC, 0); // Not used by emulator yet // TODO: CRC value of RGB signals output
	rawWriteU32(REG_TAP_MASK, ~0); // Not used by emulator yet // TODO: CRC value of RGB signals output
	rawWriteU32(REG_HCYCLE, 548);
	rawWriteU32(REG_HOFFSET, 43);
	rawWriteU32(REG_HSIZE, 480);
	rawWriteU32(REG_HSYNC0, 0);
	rawWriteU32(REG_HSYNC1, 41);
	rawWriteU32(REG_VCYCLE, 292);
	rawWriteU32(REG_VOFFSET, 12);
	rawWriteU32(REG_VSIZE, 272);
	rawWriteU32(REG_VSYNC0, 0);
	rawWriteU32(REG_VSYNC1, 10);
	rawWriteU32(REG_DLSWAP, 0);
	rawWriteU32(REG_ROTATE, 0);
	rawWriteU32(REG_OUTBITS, 0x1B6); // Not used by emulator
	rawWriteU32(REG_DITHER, 1); // Not used by emulator
	rawWriteU32(REG_SWIZZLE, 0); // Not used by emulator
	rawWriteU32(REG_CSPREAD, 1); // Not used by emulator
	rawWriteU32(REG_PCLK_POL, 0); // Not used by emulator
	rawWriteU32(REG_PCLK, 0);
	rawWriteU32(REG_TAG, 0);
	rawWriteU32(REG_TAG_X, 0);
	rawWriteU32(REG_TAG_Y, 0);
	rawWriteU32(REG_VOL_PB, 0xFF);
	rawWriteU32(REG_VOL_SOUND, 0xFF);
	rawWriteU32(REG_SOUND, 0);

	rawWriteU32(REG_TOUCH_TRANSFORM_A, 0x10000);
	rawWriteU32(REG_TOUCH_TRANSFORM_B, 0x00);
	rawWriteU32(REG_TOUCH_TRANSFORM_C, 0x00);
	rawWriteU32(REG_TOUCH_TRANSFORM_D, 0x00);
	rawWriteU32(REG_TOUCH_TRANSFORM_E, 0x10000);
	rawWriteU32(REG_TOUCH_TRANSFORM_F, 0x00);
	rawWriteU32(REG_TOUCH_TAG, 0);
	rawWriteU32(REG_TOUCH_TAG_XY, 0);

	/*
	MAPPING:
	REG_TOUCH_ADC_MODE		REG_CTOUCH_EXTENDED
	REG_TOUCH_SCREEN_XY		REG_CTOUCH_TOUCH0_XY
	REG_TOUCH_RAW_XY		REG_CTOUCH_TOUCHA_XY
	REG_TOUCH_DIRECT_XY		REG_CTOUCH_TOUCHB_XY
	REG_TOUCH_DIRECT_Z1Z2	REG_CTOUCH_TOUCHC_XY
	REG_ANALOG				REG_CTOUCH_TOUCH4_X
	REG_TOUCH_RZ			REG_CTOUCH_TOUCH4_Y
	*/
	rawWriteU32(REG_TOUCH_ADC_MODE, 0x01); // REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY
	rawWriteU32(REG_TOUCH_RZ, 0x7FFF); // REG_CTOUCH_TOUCH4_X
	rawWriteU32(REG_TOUCH_SCREEN_XY, 0x80008000); // REG_CTOUCH_TOUCH0_XY
	rawWriteU32(REG_TOUCH_RAW_XY, 0xFFFFFFFF); // REG_CTOUCH_TOUCHA_XY
	rawWriteU32(REG_TOUCH_DIRECT_XY, 0); // REG_CTOUCH_TOUCHB_XY
	rawWriteU32(REG_TOUCH_DIRECT_Z1Z2, 0); // REG_CTOUCH_TOUCHC_XY // Not used by emulator
	rawWriteU32(REG_ANALOG, 0); // REG_CTOUCH_TOUCH4_X

	rawWriteU32(REG_TOUCH_RZTHRESH, 0xFFFF);
	rawWriteU32(REG_TOUCH_OVERSAMPLE, 7); // Not used by emulator
	rawWriteU32(REG_TOUCH_SETTLE, 3); // Not used by emulator
	rawWriteU32(REG_TOUCH_CHARGE, 0x1770); // Not used by emulator
	rawWriteU32(REG_TOUCH_MODE, 3); // Not used by emulator yet // NOTE: Currently emulator always emulates continuous mode // TODO
	
	rawWriteU32(REG_PWM_HZ, 250);
	rawWriteU32(REG_PWM_DUTY, 128);
	rawWriteU32(REG_INT_MASK, 0xFF);
	rawWriteU32(REG_INT_EN, 0);
	rawWriteU32(REG_INT_FLAGS, 0);

	s_CpuReset = false;
}

void MemoryClass::end()
{

}

bool MemoryClass::multiTouch()
{
#ifdef FT810EMU_MODE
	return (Memory.rawReadU32(REG_CTOUCH_EXTENDED) & 0x01) == CTOUCH_MODE_EXTENDED;
#else
	return s_EmulatorMode >= FT8XXEMU_EmulatorFT801
		&& ((Memory.rawReadU32(REG_CTOUCH_EXTENDED) & 0x01) == CTOUCH_MODE_EXTENDED);
#endif
}

void MemoryClass::enableReadDelay(bool enabled)
{
	s_ReadDelay = enabled;
}

uint8_t *MemoryClass::getRam()
{
	return s_Ram;
}

const uint32_t *MemoryClass::getDisplayList()
{
	return s_DisplayListActive;
}

void MemoryClass::mcuWriteU32(size_t address, uint32_t data)
{
	if ((address & ~0x3) >= FT800EMU_RAM_SIZE)
	{
		printf("MCU U32 write address %i exceeds RAM size\n", (int)address);
		if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Write address exceeds RAM size");
		return;
	}

	// s_WaitMCUReadCounter = 0;
	// s_SwapMCUReadCounter = 0;

    actionWrite(address, data);
	rawWriteU32(address, data);

	++s_WriteOpCount;

	switch (address)
	{
#ifdef FT810EMU_MODE
	case REG_CMDB_WRITE:
#endif
	case REG_CMD_WRITE:
		s_WaitCoprocessorReadCounter = 0;
		break;
	}

	postWrite(address, data);
}

/* void MemoryClass::mcuWrite(size_t address, uint8_t data)
{
	s_SwapMCUReadCounter = 0;
	if (address == REG_CMD_WRITE + 3)
	{
		s_WaitCoprocessorReadCounter = 0;
	}
    actionWrite(address, data);
	rawWriteU8(address, data);
} */

void MemoryClass::swapDisplayList()
{
	memcpy(static_cast<void *>(s_DisplayListFree), static_cast<void *>(&s_Ram[RAM_DL]), sizeof(s_DisplayListA));
	memcpy(static_cast<void *>(&s_Ram[RAM_DL]), static_cast<void *>(s_DisplayListActive), sizeof(s_DisplayListA));
	uint32_t *active = s_DisplayListFree;
	s_DisplayListFree = s_DisplayListActive;
	s_DisplayListActive = active;
	++s_RealSwapCount;
}

int MemoryClass::getRealSwapCount()
{
	return s_RealSwapCount;
}

void MemoryClass::flagDLSwap()
{
	s_SwapMCUReadCounter = 0;
	s_SwapCoprocessorReadCounter = 0;
}

uint32_t MemoryClass::mcuReadU32(size_t address)
{
	if (s_ReadDelay)
	{
		switch (address)
		{
#ifdef FT810EMU_MODE
		case REG_CMDB_SPACE:
#endif
		case REG_CMD_READ: // wait for read advance from coprocessor thread
			++s_WaitMCUReadCounter;
			if (s_WaitMCUReadCounter > 8)
			{
				// printf(" Delay MCU \n");
				FT8XXEMU::System.prioritizeCoprocessorThread();
				FT8XXEMU::System.delayForMCU(1);
				FT8XXEMU::System.unprioritizeCoprocessorThread();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++s_SwapMCUReadCounter;
			if (s_SwapMCUReadCounter > 8)
			{
				// printf(" Delay MCU ");
				FT8XXEMU::System.prioritizeCoprocessorThread();
				FT8XXEMU::System.delayForMCU(1);
				FT8XXEMU::System.unprioritizeCoprocessorThread();
			}
			break;
		default:
			if (s_LastMCURead == address)
			{
				++s_IdenticalMCUReadCounter;
				if (s_IdenticalMCUReadCounter > 8)
				{
					// printf(" Switch ");
					FT8XXEMU::System.prioritizeCoprocessorThread();
					FT8XXEMU::System.switchThread();
					FT8XXEMU::System.unprioritizeCoprocessorThread();
				}
			}
			else
			{
				// printf("Reset %i\n", address);
				s_LastMCURead = address;
				s_IdenticalMCUReadCounter = 0;
			}
			break;
		}
	}

	switch (address)
	{
		case REG_TOUCH_SCREEN_XY:
		{
			return getTouchScreenXY();
		}
		case REG_INT_FLAGS:
		{
			uint32_t result = rawReadU32(address);
			rawWriteU32(REG_INT_FLAGS, 0);
			return result;
		}
#ifdef FT810EMU_MODE
		case REG_CMDB_SPACE:
		{
			uint32_t wp = rawReadU32(REG_CMD_WRITE);
			uint32_t rp = rawReadU32(REG_CMD_READ);
			return 4092 - ((wp - rp) & 0xFFF);
		}
#endif
	}

	return rawReadU32(address);
}

/* uint8_t MemoryClass::mcuRead(size_t address)
{
	if (s_ReadDelay && address % 4 == 0)
	{
		switch (address)
		{
		case REG_CMD_READ: // wait for read advance from coprocessor thread
			++s_SwapMCUReadCounter;
			if (s_SwapMCUReadCounter > 8)
			{
				// printf(" Delay MCU ");
				System.prioritizeCoprocessorThread();
				System.delayForMCU(1);
				System.unprioritizeCoprocessorThread();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++s_WaitMCUReadCounter;
			if (s_WaitMCUReadCounter > 8)
			{
				// printf(" Delay MCU ");
				System.prioritizeCoprocessorThread();
				System.delayForMCU(1);
				System.unprioritizeCoprocessorThread();
			}
			break;
		default:
			if (s_LastMCURead == address)
			{
				++s_IdenticalMCUReadCounter;
				if (s_IdenticalMCUReadCounter > 8)
				{
					// printf(" Switch ");
					System.prioritizeCoprocessorThread();
					System.switchThread();
					System.unprioritizeCoprocessorThread();
				}
			}
			else
			{
				// printf("Reset %i\n", address);
				s_LastMCURead = address;
				s_IdenticalMCUReadCounter = 0;
			}
			break;
		}
	}

	return rawReadU8(address);
} */

void MemoryClass::coprocessorWriteU32(size_t address, uint32_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	printf("Coprocessor write U32 %i, %i\n", (int)address, (int)data);
#endif

	++s_WriteOpCount;

	if ((address & ~0x3) >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U32 write address %i exceeds RAM size\n", (int)address);
		return;
	}

	if (address >= RAM_DL && address < RAM_DL + 8192)
	{
		int dlAddr = (address - RAM_DL) >> 2;
		s_DisplayListCoprocessorWrites[dlAddr] = s_LastCoprocessorCommandRead;
		// printf("Coprocessor command at %i writes to display list at %i\n", s_LastCoprocessorCommandRead, dlAddr);
	}

    actionWrite(address, data);
	rawWriteU32(address, data);
	postWrite(address, data);

	switch (address)
	{
#ifdef FT810EMU_MODE
	case REG_CMDB_SPACE:
#endif
	case REG_CMD_READ:
		s_WaitMCUReadCounter = 0;
		break;
	}
}

static uint32_t s_OverrideRasterY = 0;

uint32_t MemoryClass::coprocessorReadU32(size_t address)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	printf("Coprocessor read U32 %i\n", (int)address);
#endif

	/*if (address >= RAM_COMPOSITE)
	{
		printf("Coprocessor read U32 %u %u\n", (unsigned int)address, (unsigned int)rawReadU32(address));
	}*/

	if ((address & ~0x3) >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U32 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	if (s_ReadDelay && address < RAM_J1RAM)
	{
		switch (address)
		{
		case REG_CMD_WRITE: // wait for writes from mcu thread
			++s_WaitCoprocessorReadCounter;
			if (s_WaitCoprocessorReadCounter > 8)
			{
				FT8XXEMU::System.prioritizeMCUThread();
				FT8XXEMU::System.delay(1);
				FT8XXEMU::System.unprioritizeMCUThread();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++s_SwapCoprocessorReadCounter;
			if (s_SwapCoprocessorReadCounter > 8)
			{
				// printf("Waiting for frame swap, currently %i\n", rawReadU32(REG_DLSWAP));
				FT8XXEMU::System.prioritizeMCUThread();
				FT8XXEMU::System.delay(1);
				FT8XXEMU::System.unprioritizeMCUThread();
			}
			break;
		default:
			if (address >= RAM_CMD && address < RAM_CMD + 4096)
			{
				s_WaitCoprocessorReadCounter = 0;
				s_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
			}
			if (s_LastCoprocessorRead == address || (address == REG_TOUCH_RAW_XY && s_LastCoprocessorRead == REG_TOUCH_RZ))
			{
				++s_IdenticalCoprocessorReadCounter;
				if (s_IdenticalCoprocessorReadCounter > 8)
				{
					// printf(" Switch ");
					FT8XXEMU::System.prioritizeMCUThread();
					FT8XXEMU::System.switchThread();
					FT8XXEMU::System.unprioritizeMCUThread();
				}
			}
			else
			{
				// printf("Reset %i\n", address);
				s_LastCoprocessorRead = address;
				s_IdenticalCoprocessorReadCounter = 0;
			}
			break;
		}
	}
	else
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			s_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
	}

	switch (address)
	{
	case REG_TOUCH_SCREEN_XY:
		return getTouchScreenXY();
#ifdef FT810EMU_MODE
	case REG_RASTERY:
		++s_OverrideRasterY;
		return ((s_OverrideRasterY & 1) << 11); // Override REG_RASTERY
#endif
	}

	return rawReadU32(address);
}

void MemoryClass::coprocessorWriteU16(size_t address, uint16_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	printf("Coprocessor write U16 %i, %i\n", (int)address, (int)data);
#endif

	++s_WriteOpCount;

	if ((address & ~0x3) >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U16 write address %i exceeds RAM size\n", (int)address);
		return;
	}

    actionWrite(address, data);
	rawWriteU16(address, data);
	postWrite(address, data);
}

uint16_t MemoryClass::coprocessorReadU16(size_t address)
{
	// printf("Coprocessor read U16 %i\n", (int)address);

	if (address % 4 == 0)
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			s_WaitCoprocessorReadCounter = 0;
			s_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
	}

	if ((address & ~0x1) >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U16 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	return rawReadU16(address);
}

void MemoryClass::coprocessorWriteU8(size_t address, uint8_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	printf("Coprocessor write U8 %i, %i\n", (int)address, (int)data);
#endif

	++s_WriteOpCount;

	if (address >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U8 write address %i exceeds RAM size\n", (int)address);
		return;
	}

	if (address == REG_J1_INT && data)
	{
		// TODO: MUTEX!!!
		rawWriteU8(REG_INT_FLAGS, rawReadU8(REG_INT_FLAGS) | INT_CMDEMPTY);
		return;
	}

    actionWrite(address, data);
	rawWriteU8(address, data);
	postWrite(address, data);
}

uint8_t MemoryClass::coprocessorReadU8(size_t address)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	printf("Coprocessor read U8 %i\n", (int)address);
#endif

	if (address >= FT800EMU_RAM_SIZE)
	{
		printf("Coprocessor U8 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	if (address % 4 == 0)
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			s_WaitCoprocessorReadCounter = 0;
			s_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
	}

	return rawReadU8(address);
}

int *MemoryClass::getDisplayListCoprocessorWrites()
{
	return s_DisplayListCoprocessorWrites;
}

void MemoryClass::clearDisplayListCoprocessorWrites()
{
	for (int i = 0; i < FT800EMU_DISPLAY_LIST_SIZE; ++i)
	{
		s_DisplayListCoprocessorWrites[i] = -1;
	}
}

} /* namespace FT800EMU */

/* end of file */
