/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "ft8xxemu.h"
#include "ft8xxemu_diag.h"

#include <string.h>
#include <stdio.h>

// Version
#define BT8XXEMU_VERSION_STRING "3.0.0"

// Include BT8XXEMU_PLATFORM
#include "ft8xxemu_graphics_driver.h"

// Include FT800EMU
#define BT8XXEMU_NODEFS
#ifdef FTEMU_HAVE_FT800EMU
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"
#include "ft800emu_memory.h"
#include "ft800emu_graphics_processor.h"
#endif

// Include FT810EMU
#ifdef FTEMU_HAVE_FT810EMU
#undef FT800EMU_SPI_I2C_H
#undef FT800EMU_EMULATOR_H
#undef FT800EMU_MEMORY_H
#undef FT800EMU_GRAPHICS_PROCESSOR_H
#define FT800EMU FT810EMU
#define FT810EMU_MODE
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"
#include "ft800emu_memory.h"
#include "ft800emu_graphics_processor.h"
#undef FT800EMU
#undef FT810EMU_MODE
#endif

uint8_t(*BT8XXEMU_transfer)(uint8_t data) = NULL;
void(*BT8XXEMU_cs)(int cs) = NULL;
int(*BT8XXEMU_int)() = NULL;

uint8_t *(*BT8XXEMU_getRam)() = NULL;
const uint32_t *(*BT8XXEMU_getDisplayList)() = NULL;
void(*BT8XXEMU_poke)() = NULL;
int *(*BT8XXEMU_getDisplayListCoprocessorWrites)() = NULL;
void(*BT8XXEMU_clearDisplayListCoprocessorWrites)() = NULL;
bool(*BT8XXEMU_getDebugLimiterEffective)() = NULL;
int(*BT8XXEMU_getDebugLimiterIndex)() = NULL;
void(*BT8XXEMU_setDebugLimiter)(int debugLimiter) = NULL;
void(*BT8XXEMU_processTrace)(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = NULL;

static BT8XXEMU::IEmulator *s_Emulator = 0;

static const char *s_Version =
	"FT8XX Emulator Library v" BT8XXEMU_VERSION_STRING "\n"
	"Copyright(C) 2013 - 2015  Future Technology Devices International Ltd\n"
	"Author: Jan Boon <jan.boon@kaetemi.be>";
BT8XXEMU_API const char *BT8XXEMU_version()
{
	return s_Version;
}

BT8XXEMU_API void BT8XXEMU_defaults(uint32_t versionApi, BT8XXEMU_EmulatorParameters *params, BT8XXEMU_EmulatorMode mode)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return;
	}

	memset(params, 0, sizeof(BT8XXEMU_EmulatorParameters));

	params->Flags =
		BT8XXEMU_EmulatorEnableKeyboard
		| BT8XXEMU_EmulatorEnableMouse
		| BT8XXEMU_EmulatorEnableAudio
		| BT8XXEMU_EmulatorEnableDebugShortkeys
		| BT8XXEMU_EmulatorEnableCoprocessor
		| BT8XXEMU_EmulatorEnableGraphicsMultithread
		| BT8XXEMU_EmulatorEnableDynamicDegrade
		| BT8XXEMU_EmulatorEnableTouchTransformation;

	params->Mode = mode;
}

BT8XXEMU_API void BT8XXEMU_run(uint32_t versionApi, const BT8XXEMU_EmulatorParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return;
	}

	switch (params->Mode)
	{
	case 0:
#ifdef FTEMU_HAVE_FT800EMU
	case BT8XXEMU_EmulatorFT800:
	case BT8XXEMU_EmulatorFT801:
		s_Emulator = new FT800EMU::Emulator();
		BT8XXEMU_transfer = &FT800EMU::SPII2C.transfer;
		BT8XXEMU_cs = &FT800EMU::SPII2C.csLow;
		BT8XXEMU_int = &FT800EMU::SPII2C.intnLow;
		BT8XXEMU_getRam = &FT800EMU::Memory.getRam;
		BT8XXEMU_getDisplayList = &FT800EMU::Memory.getDisplayList;
		BT8XXEMU_poke = &FT800EMU::Memory.poke;
		BT8XXEMU_getDisplayListCoprocessorWrites = &FT800EMU::Memory.getDisplayListCoprocessorWrites;
		BT8XXEMU_clearDisplayListCoprocessorWrites = &FT800EMU::Memory.clearDisplayListCoprocessorWrites;
		BT8XXEMU_getDebugLimiterEffective = &FT800EMU::GraphicsProcessor.getDebugLimiterEffective;
		BT8XXEMU_getDebugLimiterIndex = &FT800EMU::GraphicsProcessor.getDebugLimiterIndex;
		BT8XXEMU_setDebugLimiter = &FT800EMU::GraphicsProcessor.setDebugLimiter;
		BT8XXEMU_processTrace = &FT800EMU::GraphicsProcessor.processTrace;
		static_cast<FT800EMU::Emulator *>(s_Emulator)->run(*params);
		break;
#endif
#ifdef FTEMU_HAVE_FT810EMU
	case BT8XXEMU_EmulatorFT810:
	case BT8XXEMU_EmulatorFT811:
	case BT8XXEMU_EmulatorFT812:
	case BT8XXEMU_EmulatorFT813:
		s_Emulator = new FT810EMU::Emulator();
		BT8XXEMU_transfer = &FT810EMU::SPII2C.transfer;
		BT8XXEMU_cs = &FT810EMU::SPII2C.csLow;
		BT8XXEMU_int = &FT810EMU::SPII2C.intnLow;
		BT8XXEMU_getRam = &FT810EMU::Memory.getRam;
		BT8XXEMU_getDisplayList = &FT810EMU::Memory.getDisplayList;
		BT8XXEMU_poke = &FT810EMU::Memory.poke;
		BT8XXEMU_getDisplayListCoprocessorWrites = &FT810EMU::Memory.getDisplayListCoprocessorWrites;
		BT8XXEMU_clearDisplayListCoprocessorWrites = &FT810EMU::Memory.clearDisplayListCoprocessorWrites;
		BT8XXEMU_getDebugLimiterEffective = &FT810EMU::GraphicsProcessor.getDebugLimiterEffective;
		BT8XXEMU_getDebugLimiterIndex = &FT810EMU::GraphicsProcessor.getDebugLimiterIndex;
		BT8XXEMU_setDebugLimiter = &FT810EMU::GraphicsProcessor.setDebugLimiter;
		BT8XXEMU_processTrace = &FT810EMU::GraphicsProcessor.processTrace;
		static_cast<FT810EMU::Emulator *>(s_Emulator)->run(*params);
		break;
#endif
	default:
		{
			fprintf(stderr, "Invalid ft8xxemu emulator mode selected\n");
			return;
		}
		break;
	}
}

BT8XXEMU_API void BT8XXEMU_stop()
{
	s_Emulator->stop();
}

// Set touch XY. Call on every frame when using custom graphics output
BT8XXEMU_API void BT8XXEMU_touchSetXY(int idx, int x, int y, int pressure)
{
	FT8XXEMU::g_SetTouchScreenXY(idx, x, y, pressure);
}

// Reset touch XY. Call once no longer touching when using custom graphics output
BT8XXEMU_API void BT8XXEMU_touchResetXY(int idx)
{
	FT8XXEMU::g_ResetTouchScreenXY(idx);
}

/* end of file */
