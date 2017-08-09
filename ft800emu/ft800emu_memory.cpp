/**
 * Memory
 * $Id$
 * \file ft800emu_memory.cpp
 * \brief Memory
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
#include "ft800emu_touch.h"
#include "ft8xxemu_window_output.h"
#include "ft800emu_graphics_processor.h"
#include "ft800emu_audio_processor.h"
#include "ft800emu_audio_render.h"

// using namespace ...;

#define FT800EMU_COPROCESSOR_MEMLOG 0
#define FT800EMU_MCU_MEMLOG 0
#define FT800EMU_DL_MEMLOG 0

using namespace FT8XXEMU;

namespace FT800EMU {





//static void (*m_Interrupt)());

int Memory::getDirectSwapCount()
{
	return m_DirectSwapCount;
}

int Memory::getWriteOpCount()
{
	return m_WriteOpCount;
}

void Memory::poke()
{
	++m_WriteOpCount;
}

/*void Memory::setInterrupt(void (*interrupt)())
{
	m_Interrupt = interrupt;
}*/

bool Memory::intnLow()
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

bool Memory::intnHigh()
{
	return !intnLow();
}

bool Memory::coprocessorGetReset()
{
	bool result = m_CpuReset || (rawReadU8(REG_CPURESET) & 0x01);
	m_CpuReset = false;
	return result;
}

template<typename T>
BT8XXEMU_FORCE_INLINE void Memory::actionWrite(const ramaddr address, T &data)
{
	// switches for 1 byte regs
	// least significant byte
	if (address % 4 == 0)
	{
		switch (address)
		{
		case REG_PCLK:
			// FTEMU_printf("Write REG_PCLK %u\n", (uint32_t)data);
			// if (data == 0 && m_Ram[REG_DLSWAP] == DLSWAP_FRAME)
			if (data == 0 && (m_Ram[REG_DLSWAP] == DLSWAP_FRAME || m_Ram[REG_DLSWAP] == DLSWAP_LINE))
			{
				// FTEMU_printf("Direct swap from REG_PCLK\n");
				// Direct swap
				FT8XXEMU::System.enterSwapDL();
				// if (data == 0 && m_Ram[REG_DLSWAP] == DLSWAP_FRAME)
				if (data == 0 && (m_Ram[REG_DLSWAP] == DLSWAP_FRAME || m_Ram[REG_DLSWAP] == DLSWAP_LINE))
				{
					swapDisplayList();
					m_Ram[REG_DLSWAP] = 0;
					flagDLSwap();
					GraphicsProcessor.processBlank();
					++m_DirectSwapCount;
				}
				FT8XXEMU::System.leaveSwapDL();
			}
			break;
		case REG_DLSWAP:
#if FT800EMU_DL_MEMLOG
			FTEMU_printf("Write REG_DLSWAP %u\n", data);
#endif
			// if (data == DLSWAP_FRAME && m_Ram[REG_PCLK] == 0)
			if ((data == DLSWAP_FRAME || data == DLSWAP_LINE) && m_Ram[REG_PCLK] == 0)
			{
				// FTEMU_printf("Direct swap from DLSWAP_FRAME\n");
				// Direct swap
				FT8XXEMU::System.enterSwapDL();
				// if (data == DLSWAP_FRAME && m_Ram[REG_PCLK] == 0)
				if ((data == DLSWAP_FRAME || data == DLSWAP_LINE) && m_Ram[REG_PCLK] == 0)
				{
					// FTEMU_printf("Go\n");
					swapDisplayList();
					data = 0;
					GraphicsProcessor.processBlank();
					++m_DirectSwapCount;
				}
				// else
				// {
					// FTEMU_printf("No go\n");
				// }
				FT8XXEMU::System.leaveSwapDL();
			}
			break;
		// case REG_VOL_SOUND:
		// 	FTEMU_printf("REG_VOL_SOUND %i\n", data);
		// 	break;
		// case REG_SOUND:
		// 	FTEMU_printf("REG_SOUND %i\n", data);
		// 	break;
		case REG_PLAY:
			if (data & 0x01)
			{
				// FTEMU_printf("REG_PLAY\n");
				AudioProcessor.play();
			}
			break;
		case REG_PLAYBACK_PLAY:
			if (data & 0x01)
			{
				// FTEMU_printf("REG_PLAYBACK_PLAY\n");
				AudioRender.playbackPlay();
			}
			break;
		case REG_CPURESET:
			if (data & 0x01)
			{
				// TODO: Perhaps this should lock until the cpu reset is actually pushed through to ensure following commands go through...
				m_CpuReset = true;
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
			// FTEMU_printf("REG_CMD_READ %u\n", data);
			break;
		case REG_CMD_WRITE:
			data &= 0xFFF;
			// FTEMU_printf("REG_CMD_WRITE %u\n", data);
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
		case REG_ROMSUB_SEL:
			data &= 0x3;
			break;
#endif
		}
	}
}

template<typename T>
BT8XXEMU_FORCE_INLINE void Memory::postWrite(const ramaddr address, const T data)
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
			if (m_EmulatorMode >= BT8XXEMU_EmulatorFT801)
#endif
			{
				if (!TouchClass::multiTouch())
				{
					rawWriteU32(REG_TOUCH_DIRECT_XY, 0); // REG_CTOUCH_TOUCHB_XY
					rawWriteU32(REG_TOUCH_DIRECT_Z1Z2, 0); // REG_CTOUCH_TOUCHC_XY
					rawWriteU32(REG_ANALOG, 0); // REG_CTOUCH_TOUCH4_X
				}
				Touch[0].resetXY();
				Touch[1].resetXY();
				Touch[2].resetXY();
				Touch[3].resetXY();
				Touch[4].resetXY();
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

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU32(ramaddr address, uint32_t data)
{
	rawWriteU32(m_Ram, address, data);
}

BT8XXEMU_FORCE_INLINE uint32_t Memory::rawReadU32(ramaddr address)
{
	return rawReadU32(m_Ram, address);
}

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU16(ramaddr address, uint16_t data)
{
	rawWriteU16(m_Ram, address, data);
}

BT8XXEMU_FORCE_INLINE uint16_t Memory::rawReadU16(ramaddr address)
{
	return rawReadU16(m_Ram, address);
}

BT8XXEMU_FORCE_INLINE void Memory::rawWriteU8(ramaddr address, uint8_t data)
{
	rawWriteU8(m_Ram, address, data);
}

BT8XXEMU_FORCE_INLINE uint8_t Memory::rawReadU8(ramaddr address)
{
	return rawReadU8(m_Ram, address);
}

#ifdef FT810EMU_MODE
static const uint8_t m_RomFT810[FT800EMU_ROM_SIZE] = {
#include "resources/rom_ft810.h"
};
#else
static const uint8_t m_RomFT800[FT800EMU_ROM_SIZE] = {
#include "resources/rom_ft800.h"
};
static const uint8_t m_RomFT801[FT800EMU_ROM_SIZE] = {
#include "resources/rom_ft801.h"
};
#endif

#ifdef FT800EMU_OTP_SIZE
static const uint8_t m_OTP810[FT800EMU_OTP_SIZE] = {
#include "resources/otp_810.h"
};
static const uint8_t m_OTP811[FT800EMU_OTP_SIZE] = {
#include "resources/otp_811.h"
};
static const uint8_t m_OTP812[FT800EMU_OTP_SIZE] = {
#include "resources/otp_812.h"
};
static const uint8_t m_OTP813[FT800EMU_OTP_SIZE] = {
#include "resources/otp_813.h"
};
#endif

Memory::Memory(BT8XXEMU_EmulatorMode emulatorMode, std::mutex &swapDLMutex,
	FT8XXEMU::ThreadState &threadMCU, FT8XXEMU::ThreadState &threadCoprocessor,
	const char *romFilePath, const char *otpFilePath)
	: m_SwapDLMutex(swapDLMutex), m_ThreadMCU(threadMCU), m_ThreadCoprocessor(threadCoprocessor)
{
	// memset(m_Ram, 0, FT800EMU_RAM_SIZE);
	// memset(m_DisplayListA, 0, sizeof(uint32_t) * FT800EMU_DISPLAY_LIST_SIZE);
	// memset(m_DisplayListB, 0, sizeof(uint32_t) * FT800EMU_DISPLAY_LIST_SIZE);
	
	if (romFilePath)
	{
		FILE *f;
		f = fopen(romFilePath, "rb");
		if (!f) FTEMU_printf("Failed to open ROM file\n");
		else
		{
			size_t s = fread(&m_Ram[FT800EMU_ROM_INDEX], 1, FT800EMU_ROM_SIZE, f);
			if (s != FT800EMU_ROM_SIZE) FTEMU_printf("Incomplete ROM file\n");
			else FTEMU_printf("Loaded ROM file\n");
			if (fclose(f)) FTEMU_printf("Error closing ROM file\n");
		}
	}
	else
	{
#ifdef FT810EMU_MODE
		memcpy(&m_Ram[FT800EMU_ROM_INDEX], m_RomFT810, sizeof(m_RomFT810));
#else
		if (emulatorMode >= BT8XXEMU_EmulatorFT801) memcpy(&m_Ram[FT800EMU_ROM_INDEX], m_RomFT801, sizeof(m_RomFT801));
		else memcpy(&m_Ram[FT800EMU_ROM_INDEX], m_RomFT800, sizeof(m_RomFT800));
#endif
	}

	if (otpFilePath)
	{
		FILE *f;
		f = fopen(otpFilePath, "rb");
		if (!f) FTEMU_printf("Failed to open OTP file\n");
		else
		{
			size_t s = fread(&m_Ram[FT800EMU_OTP_SIZE], 1, FT800EMU_OTP_SIZE, f);
			if (s != FT800EMU_OTP_SIZE) FTEMU_printf("Incomplete OTP file\n");
			else FTEMU_printf("Loaded OTP file\n");
			if (fclose(f)) FTEMU_printf("Error closing OTP file\n");
		}
	}
	else
	{
#ifdef FT810EMU_MODE
		if (emulatorMode >= BT8XXEMU_EmulatorFT813) memcpy(&m_Ram[RAM_JTBOOT], m_OTP813, sizeof(m_OTP813));
		else if (emulatorMode >= BT8XXEMU_EmulatorFT812) memcpy(&m_Ram[RAM_JTBOOT], m_OTP812, sizeof(m_OTP812));
		else if (emulatorMode >= BT8XXEMU_EmulatorFT811) memcpy(&m_Ram[RAM_JTBOOT], m_OTP811, sizeof(m_OTP811));
		else memcpy(&m_Ram[RAM_JTBOOT], m_OTP810, sizeof(m_OTP810));
#endif
	}

	m_DirectSwapCount = 0;
	m_RealSwapCount = 0;
	m_WriteOpCount = 0;
	//m_CoprocessorWritesDL = false;

	m_ReadDelay = false;

	m_LastCoprocessorRead = -1;
	m_IdenticalCoprocessorReadCounter = 0;
	m_SwapCoprocessorReadCounter = 0;
	m_WaitCoprocessorReadCounter = 0;
#ifdef FT810EMU_MODE
	m_FifoCoprocessorReadCounter = 0;
#endif

	m_LastMCURead = -1;
	m_IdenticalMCUReadCounter = 0;
	m_WaitMCUReadCounter = 0;
	m_SwapMCUReadCounter = 0;
#ifdef FT810EMU_MODE
	m_FifoMCUReadCounter = 0;
#endif

	m_EmulatorMode = emulatorMode;

#ifdef FT810EMU_MODE
	rawWriteU32(ROM_CHIPID, rawReadU32(RAM_JTBOOT + FT800EMU_OTP_SIZE - 4));
#else
	rawWriteU16(ROM_CHIPID, (((uint16_t)emulatorMode & 0xFF) << 8) | ((uint16_t)emulatorMode >> 8)); // endianness??
	rawWriteU16(ROM_CHIPID + 2, 0x0001);
#endif

	FTEMU_printf("CHIPID: 0x%x 0x%x 0x%x 0x%x\n", (int)rawReadU8(ROM_CHIPID), (int)rawReadU8(ROM_CHIPID + 1), (int)rawReadU8(ROM_CHIPID + 2), (int)rawReadU8(ROM_CHIPID + 3));
	FTEMU_printf("CHIPID: 0x%x\n", (int)rawReadU32(ROM_CHIPID));

	rawWriteU32(REG_ID, 0x7C);
	rawWriteU32(REG_FRAMES, 0); // Frame counter - is this updated before or after frame render?
	rawWriteU32(REG_CLOCK, 0);
#ifdef FT810EMU_MODE
	rawWriteU32(REG_FREQUENCY, 60000000);
#else
	rawWriteU32(REG_FREQUENCY, 48000000);
#endif
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
	
	rawWriteU32(REG_PLAYBACK_PLAY, 0);
	rawWriteU32(REG_PLAYBACK_LOOP, 0);
	rawWriteU32(REG_PLAYBACK_FORMAT, 0);
	rawWriteU32(REG_PLAYBACK_FREQ, 8000);
	rawWriteU32(REG_PLAYBACK_READPTR, 0);
	rawWriteU32(REG_PLAYBACK_LENGTH, 0);
	rawWriteU32(REG_PLAYBACK_START, 0);

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

	rawWriteU32(REG_CMD_WRITE, 0);
	rawWriteU32(REG_CMD_READ, 0);
	rawWriteU32(REG_CMD_DL, 0);

#ifdef FT810EMU_MODE
	rawWriteU32(REG_J1_COLD, 1);
#endif

	m_CpuReset = false;
}

Memory::~Memory()
{
	// ...
}

void Memory::enableReadDelay(bool enabled)
{
	m_ReadDelay = enabled;
}

void Memory::mcuWriteU32(ramaddr address, uint32_t data)
{
#if FT800EMU_MCU_MEMLOG
	FTEMU_printf("MCU write U32 %i, %i\n", (int)address, (int)data);
#endif

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("MCU U32 write address %i exceeds RAM size\n", (int)address);
		if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Write address exceeds RAM size");
		return;
	}

	// m_WaitMCUReadCounter = 0;
	// m_SwapMCUReadCounter = 0;

    actionWrite(address, data);
	rawWriteU32(address, data);

	if (address != REG_CMD_WRITE
#ifdef FT810EMU_MODE
		&& address != REG_CMDB_WRITE
#endif
		&& address < RAM_CMD)
	{
		++m_WriteOpCount;
	}

	switch (address)
	{
#ifdef FT810EMU_MODE
	case REG_MEDIAFIFO_WRITE:
		m_FifoCoprocessorReadCounter = 0;
		break;
	case REG_CMDB_WRITE:
#endif
	case REG_CMD_WRITE:
		m_WaitCoprocessorReadCounter = 0;
		break;
	}

	postWrite(address, data);
}

/* void Memory::mcuWrite(ramaddr address, uint8_t data)
{
	m_SwapMCUReadCounter = 0;
	if (address == REG_CMD_WRITE + 3)
	{
		m_WaitCoprocessorReadCounter = 0;
	}
    actionWrite(address, data);
	rawWriteU8(address, data);
} */

void Memory::swapDisplayList()
{
	memcpy(static_cast<void *>(m_DisplayListFree), static_cast<void *>(&m_Ram[RAM_DL]), sizeof(m_DisplayListA));
	memcpy(static_cast<void *>(&m_Ram[RAM_DL]), static_cast<void *>(m_DisplayListActive), sizeof(m_DisplayListA));
	uint32_t *active = m_DisplayListFree;
	m_DisplayListFree = m_DisplayListActive;
	m_DisplayListActive = active;
	++m_RealSwapCount;
	//if (m_CoprocessorWritesDL)
	//{
		for (int c = 0; c < FT800EMU_DISPLAY_LIST_SIZE; ++c)
		{
			uint32_t v = m_DisplayListActive[c];
			if (v != m_DisplayListFree[c])
			{
				++m_WriteOpCount; // Display list changed
				goto BreakLoop;
			}
			switch (v >> 24)
			{
			case FT800EMU_DL_DISPLAY:
				goto BreakLoop;
			case FT800EMU_DL_JUMP:
			case FT800EMU_DL_CALL:
			case FT800EMU_DL_RETURN:
				++m_WriteOpCount; // Not optimized
				goto BreakLoop;
			}
		}
	BreakLoop:;
	//	m_CoprocessorWritesDL = false;
	//}
}

int Memory::getRealSwapCount()
{
	return m_RealSwapCount;
}

void Memory::flagDLSwap()
{
	m_SwapMCUReadCounter = 0;
	m_SwapCoprocessorReadCounter = 0;
}

uint32_t Memory::mcuReadU32(ramaddr address)
{
#if FT800EMU_MCU_MEMLOG
	// if (address != 3182612 && address != 3182616)
	FTEMU_printf("MCU read U32 %i\n", (int)address);
#endif

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("MCU U32 read address %i exceeds RAM size\n", (int)address);
		if (FT8XXEMU::g_Exception) FT8XXEMU::g_Exception("Read address exceeds RAM size");
		return 0;
	}

	if (m_ReadDelay)
	{
		switch (address)
		{
#ifdef FT810EMU_MODE
		case REG_MEDIAFIFO_READ:
			++m_FifoMCUReadCounter;
			if (m_FifoMCUReadCounter > 8)
			{
				// FTEMU_printf(" Delay MCU \n");
				m_ThreadCoprocessor.prioritize();
				FT8XXEMU::System.delayForMCU(1);
				m_ThreadCoprocessor.unprioritize();
			}
			break;
		case REG_CMDB_SPACE:
#endif
		case REG_CMD_READ: // wait for read advance from coprocessor thread
			++m_WaitMCUReadCounter;
			if (m_WaitMCUReadCounter > 8)
			{
				// FTEMU_printf(" Delay MCU \n");
				m_ThreadCoprocessor.prioritize();
				FT8XXEMU::System.delayForMCU(1);
				m_ThreadCoprocessor.unprioritize();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++m_SwapMCUReadCounter;
			if (m_SwapMCUReadCounter > 8)
			{
				// FTEMU_printf(" Delay MCU ");
				m_ThreadCoprocessor.prioritize();
				FT8XXEMU::System.delayForMCU(1);
				m_ThreadCoprocessor.unprioritize();
			}
			break;
		default:
			if (m_LastMCURead == address)
			{
				++m_IdenticalMCUReadCounter;
				if (m_IdenticalMCUReadCounter > 8)
				{
					// FTEMU_printf(" Switch ");
					m_ThreadCoprocessor.prioritize();
					FT8XXEMU::System.switchThread();
					m_ThreadCoprocessor.unprioritize();
				}
			}
			else
			{
				// FTEMU_printf("Reset %i\n", address);
				m_LastMCURead = address;
				m_IdenticalMCUReadCounter = 0;
			}
			break;
		}
	}

	switch (address)
	{
		case REG_TOUCH_SCREEN_XY:
		{
			return Touch[0].getXY();
			// TODO MULTITOUCH
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

/* uint8_t Memory::mcuRead(ramaddr address)
{
	if (m_ReadDelay && address % 4 == 0)
	{
		switch (address)
		{
		case REG_CMD_READ: // wait for read advance from coprocessor thread
			++m_SwapMCUReadCounter;
			if (m_SwapMCUReadCounter > 8)
			{
				// FTEMU_printf(" Delay MCU ");
				System.prioritizeCoprocessorThread();
				System.delayForMCU(1);
				System.unprioritizeCoprocessorThread();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++m_WaitMCUReadCounter;
			if (m_WaitMCUReadCounter > 8)
			{
				// FTEMU_printf(" Delay MCU ");
				System.prioritizeCoprocessorThread();
				System.delayForMCU(1);
				System.unprioritizeCoprocessorThread();
			}
			break;
		default:
			if (m_LastMCURead == address)
			{
				++m_IdenticalMCUReadCounter;
				if (m_IdenticalMCUReadCounter > 8)
				{
					// FTEMU_printf(" Switch ");
					System.prioritizeCoprocessorThread();
					System.switchThread();
					System.unprioritizeCoprocessorThread();
				}
			}
			else
			{
				// FTEMU_printf("Reset %i\n", address);
				m_LastMCURead = address;
				m_IdenticalMCUReadCounter = 0;
			}
			break;
		}
	}

	return rawReadU8(address);
} */

void Memory::coprocessorWriteU32(ramaddr address, uint32_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	FTEMU_printf("Coprocessor write U32 %i, %i\n", (int)address, (int)data);
#endif

	if (address == REG_CMD_READ)
	{
		if (data & 0x1)
		{
			FTEMU_printf("WARNING: Coprocessor has flagged an error\n");
		}
	}
	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U32 write address %i exceeds RAM size\n", (int)address);
		return;
	}

	if (address >= RAM_DL && address < RAM_DL + 8192)
	{
		//m_CoprocessorWritesDL = true;
		int dlAddr = (int)((address - RAM_DL) >> 2);
		m_DisplayListCoprocessorWrites[dlAddr] = m_LastCoprocessorCommandRead;
#if FT800EMU_DL_MEMLOG
		FTEMU_printf("Coprocessor command at %i writes value 0x%x to display list at %i\n", (int)m_LastCoprocessorCommandRead, (unsigned int)data, (int)dlAddr);
#endif
	}
	else if (address != REG_DLSWAP
		&& address != REG_CMD_READ && address != REG_CMD_DL 
		&& address != REG_J1_INT && address != REG_INT_FLAGS
		&& address < RAM_J1RAM)
	{
		++m_WriteOpCount;
	}

    actionWrite(address, data);
	rawWriteU32(address, data);
	postWrite(address, data);

	switch (address)
	{
#ifdef FT810EMU_MODE
	case REG_MEDIAFIFO_READ:
		m_FifoMCUReadCounter = 0;
		break;
	case REG_CMDB_SPACE:
#endif
	case REG_CMD_READ:
		m_WaitMCUReadCounter = 0;
		break;
	}
}

static uint32_t m_OverrideRasterY = 0;
#ifndef FT810EMU_MODE
static int m_HasCachedTouchRawXY = 0;
static uint32_t m_CachedTouchRawXY = 0xFFFFFFFF;
#endif

uint32_t Memory::coprocessorReadU32(ramaddr address)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	// if (address != 3182612 && address != 3182616)
	FTEMU_printf("Coprocessor read U32 %i (cmd %i)\n", (int)address, (int)((address - RAM_CMD) / 4));
#endif

	/*if (address >= RAM_COMPOSITE)
	{
		FTEMU_printf("Coprocessor read U32 %u %u\n", (unsigned int)address, (unsigned int)rawReadU32(address));
	}*/

	/*static bool showOneMore = true;
	if (address == REG_TOUCH_RAW_XY && (rawReadU32(address) != 0xFFFFFFFF || showOneMore))
	{
		FTEMU_printf("Coprocessor read REG_TOUCH_RAW_XY %x\n", (unsigned int)rawReadU32(address));
		showOneMore = rawReadU32(address) != 0xFFFFFFFF;
	}

	if (address == REG_TOUCH_RZ)
	{
		FTEMU_printf("Coprocessor read REG_TOUCH_RZ %x\n", (unsigned int)rawReadU32(address));
	}*/

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U32 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	if (m_ReadDelay && address < RAM_J1RAM)
	{
		switch (address)
		{
#ifdef FT810EMU_MODE
		case REG_MEDIAFIFO_WRITE:
			++m_FifoCoprocessorReadCounter;
			if (m_FifoCoprocessorReadCounter > 8)
			{
				m_ThreadMCU.prioritize();
				FT8XXEMU::System.delay(1);
				m_ThreadMCU.unprioritize();
			}
			break;
#endif
		case REG_CMD_WRITE: // wait for writes from mcu thread
			++m_WaitCoprocessorReadCounter;
			if (m_WaitCoprocessorReadCounter > 8)
			{
				m_ThreadMCU.prioritize();
				FT8XXEMU::System.delay(1);
				m_ThreadMCU.unprioritize();
			}
			break;
		case REG_DLSWAP: // wait for frame swap from main thread
			++m_SwapCoprocessorReadCounter;
			if (m_SwapCoprocessorReadCounter > 8)
			{
				// FTEMU_printf("Waiting for frame swap, currently %i\n", rawReadU32(REG_DLSWAP));
				m_ThreadMCU.prioritize();
				FT8XXEMU::System.delay(1);
				m_ThreadMCU.unprioritize();
			}
			break;
		default:
			if (address >= RAM_CMD && address < RAM_CMD + 4096)
			{
				m_WaitCoprocessorReadCounter = 0;
				m_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
			}
			if (m_LastCoprocessorRead == address || (address == REG_TOUCH_RAW_XY && m_LastCoprocessorRead == REG_TOUCH_RZ))
			{
				++m_IdenticalCoprocessorReadCounter;
				if (m_IdenticalCoprocessorReadCounter > 8)
				{
					// FTEMU_printf(" Switch ");
					m_ThreadMCU.prioritize();
					FT8XXEMU::System.switchThread();
					m_ThreadMCU.unprioritize();
				}
			}
			else
			{
				// FTEMU_printf("Reset %i\n", address);
				m_LastCoprocessorRead = address;
				m_IdenticalCoprocessorReadCounter = 0;
			}
			break;
		}
	}
	else
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			m_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
#ifdef FT810EMU_MODE
		else if (address < RAM_DL)
		{
			m_FifoCoprocessorReadCounter = 0;
		}
#endif
	}

	switch (address)
	{
	case REG_TOUCH_SCREEN_XY:
		return Touch[0].getXY();
		// TODO: MULTITOUCH 1,2,3,4
#ifndef FT810EMU_MODE
	case REG_TOUCH_RZ:
		if (m_EmulatorMode == BT8XXEMU_EmulatorFT800)
		{
			m_CachedTouchRawXY = rawReadU32(REG_TOUCH_RAW_XY);
			if (m_CachedTouchRawXY != 0xFFFFFFFF)
			{
				m_HasCachedTouchRawXY = 2;
			}
			else
			{
				m_HasCachedTouchRawXY = 0;
				return 32767;
			}
		}
		break;
	case REG_TOUCH_RAW_XY:
		if (m_EmulatorMode == BT8XXEMU_EmulatorFT800)
		{
			if (m_HasCachedTouchRawXY)
			{
				--m_HasCachedTouchRawXY;
				return m_CachedTouchRawXY;
			}
		}
		break;
#endif
#ifdef FT810EMU_MODE
	case REG_RASTERY:
		++m_OverrideRasterY;
		return ((m_OverrideRasterY & 1) << 11); // Override REG_RASTERY
#endif
	}

	return rawReadU32(address);
}

void Memory::coprocessorWriteU16(ramaddr address, uint16_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	FTEMU_printf("Coprocessor write U16 %i, %i\n", (int)address, (int)data);
#endif

	if (address < RAM_J1RAM)
	{
		++m_WriteOpCount;
	}

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U16 write address %i exceeds RAM size\n", (int)address);
		return;
	}

    actionWrite(address, data);
	rawWriteU16(address, data);
	postWrite(address, data);
}

uint16_t Memory::coprocessorReadU16(ramaddr address)
{
	// FTEMU_printf("Coprocessor read U16 %i\n", (int)address);

	if (address % 4 == 0)
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			m_WaitCoprocessorReadCounter = 0;
			m_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
#ifdef FT810EMU_MODE
		else if (address < RAM_DL)
		{
			m_FifoCoprocessorReadCounter = 0;
		}
#endif
	}

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U16 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	return rawReadU16(address);
}

void Memory::coprocessorWriteU8(ramaddr address, uint8_t data)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	FTEMU_printf("Coprocessor write U8 %i, %i\n", (int)address, (int)data);
#endif

	if (address < RAM_J1RAM)
	{
		++m_WriteOpCount;
	}

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U8 write address %i exceeds RAM size\n", (int)address);
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

uint8_t Memory::coprocessorReadU8(ramaddr address)
{
#if FT800EMU_COPROCESSOR_MEMLOG
	FTEMU_printf("Coprocessor read U8 %i\n", (int)address);
#endif

	if (address < 0 || address >= FT800EMU_RAM_SIZE)
	{
		FTEMU_printf("Coprocessor U8 read address %i exceeds RAM size\n", (int)address);
		return 0;
	}

	if (address % 4 == 0)
	{
		if (address >= RAM_CMD && address < RAM_CMD + 4096)
		{
			m_WaitCoprocessorReadCounter = 0;
			m_LastCoprocessorCommandRead = (address - RAM_CMD) >> 2;
		}
#ifdef FT810EMU_MODE
		else if (address < RAM_DL)
		{
			m_FifoCoprocessorReadCounter = 0;
		}
#endif
	}

	return rawReadU8(address);
}

int *Memory::getDisplayListCoprocessorWrites()
{
	return m_DisplayListCoprocessorWrites;
}

void Memory::clearDisplayListCoprocessorWrites()
{
	for (int i = 0; i < FT800EMU_DISPLAY_LIST_SIZE; ++i)
	{
		m_DisplayListCoprocessorWrites[i] = -1;
	}
}

} /* namespace FT800EMU */

/* end of file */
