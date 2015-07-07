/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "ft8xxemu.h"
#include "ft8xxemu_diag.h"

#include <string.h>
#include <stdio.h>

// Version
#define FT8XXEMU_VERSION_STRING "2.0.6"

// Include FT8XXEMU_PLATFORM
#include "ft8xxemu_graphics_driver.h"

// Include FT800EMU
#define FT8XXEMU_NODEFS
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

void(*FT8XXEMU_stop)() = NULL;
uint8_t(*FT8XXEMU_transfer)(uint8_t data) = NULL;
void(*FT8XXEMU_cs)(int cs) = NULL;
int(*FT8XXEMU_int)() = NULL;

uint8_t *(*FT8XXEMU_getRam)() = NULL;
const uint32_t *(*FT8XXEMU_getDisplayList)() = NULL;
void(*FT8XXEMU_poke)() = NULL;
int *(*FT8XXEMU_getDisplayListCoprocessorWrites)() = NULL;
void(*FT8XXEMU_clearDisplayListCoprocessorWrites)() = NULL;
bool(*FT8XXEMU_getDebugLimiterEffective)() = NULL;
int(*FT8XXEMU_getDebugLimiterIndex)() = NULL;
void(*FT8XXEMU_setDebugLimiter)(int debugLimiter) = NULL;
void(*FT8XXEMU_processTrace)(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = NULL;

static const char *s_Version =
	"FT8XX Emulator Library v" FT8XXEMU_VERSION_STRING "\n"
	"Copyright(C) 2013 - 2015  Future Technology Devices International Ltd\n"
	"Author: Jan Boon <jan.boon@kaetemi.be>";
FT8XXEMU_API const char *FT8XXEMU_version()
{
	return s_Version;
}

FT8XXEMU_API void FT8XXEMU_defaults(uint32_t versionApi, FT8XXEMU_EmulatorParameters *params, FT8XXEMU_EmulatorMode mode)
{
	if (versionApi != FT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return;
	}

	memset(params, 0, sizeof(FT8XXEMU_EmulatorParameters));

	params->Flags =
		FT8XXEMU_EmulatorEnableKeyboard
		| FT8XXEMU_EmulatorEnableMouse
		| FT8XXEMU_EmulatorEnableAudio
		| FT8XXEMU_EmulatorEnableDebugShortkeys
		| FT8XXEMU_EmulatorEnableCoprocessor
		| FT8XXEMU_EmulatorEnableGraphicsMultithread
		| FT8XXEMU_EmulatorEnableDynamicDegrade
		| FT8XXEMU_EmulatorEnableTouchTransformation;

	params->Mode = mode;
}

FT8XXEMU_API void FT8XXEMU_run(uint32_t versionApi, const FT8XXEMU_EmulatorParameters *params)
{
	if (versionApi != FT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return;
	}

	switch (params->Mode)
	{
	case 0:
#ifdef FTEMU_HAVE_FT800EMU
	case FT8XXEMU_EmulatorFT800:
	case FT8XXEMU_EmulatorFT801:
		FT8XXEMU_stop = &FT800EMU::Emulator.stop;
		FT8XXEMU_transfer = &FT800EMU::SPII2C.transfer;
		FT8XXEMU_cs = &FT800EMU::SPII2C.csLow;
		FT8XXEMU_int = &FT800EMU::SPII2C.intnLow;
		FT8XXEMU_getRam = &FT800EMU::Memory.getRam;
		FT8XXEMU_getDisplayList = &FT800EMU::Memory.getDisplayList;
		FT8XXEMU_poke = &FT800EMU::Memory.poke;
		FT8XXEMU_getDisplayListCoprocessorWrites = &FT800EMU::Memory.getDisplayListCoprocessorWrites;
		FT8XXEMU_clearDisplayListCoprocessorWrites = &FT800EMU::Memory.clearDisplayListCoprocessorWrites;
		FT8XXEMU_getDebugLimiterEffective = &FT800EMU::GraphicsProcessor.getDebugLimiterEffective;
		FT8XXEMU_getDebugLimiterIndex = &FT800EMU::GraphicsProcessor.getDebugLimiterIndex;
		FT8XXEMU_setDebugLimiter = &FT800EMU::GraphicsProcessor.setDebugLimiter;
		FT8XXEMU_processTrace = &FT800EMU::GraphicsProcessor.processTrace;
		FT800EMU::Emulator.run(*params);
		break;
#endif
#ifdef FTEMU_HAVE_FT810EMU
	case FT8XXEMU_EmulatorFT810:
	case FT8XXEMU_EmulatorFT811:
		FT8XXEMU_stop = &FT810EMU::Emulator.stop;
		FT8XXEMU_transfer = &FT810EMU::SPII2C.transfer;
		FT8XXEMU_cs = &FT810EMU::SPII2C.csLow;
		FT8XXEMU_int = &FT810EMU::SPII2C.intnLow;
		FT8XXEMU_getRam = &FT810EMU::Memory.getRam;
		FT8XXEMU_getDisplayList = &FT810EMU::Memory.getDisplayList;
		FT8XXEMU_poke = &FT810EMU::Memory.poke;
		FT8XXEMU_getDisplayListCoprocessorWrites = &FT810EMU::Memory.getDisplayListCoprocessorWrites;
		FT8XXEMU_clearDisplayListCoprocessorWrites = &FT810EMU::Memory.clearDisplayListCoprocessorWrites;
		FT8XXEMU_getDebugLimiterEffective = &FT810EMU::GraphicsProcessor.getDebugLimiterEffective;
		FT8XXEMU_getDebugLimiterIndex = &FT810EMU::GraphicsProcessor.getDebugLimiterIndex;
		FT8XXEMU_setDebugLimiter = &FT810EMU::GraphicsProcessor.setDebugLimiter;
		FT8XXEMU_processTrace = &FT810EMU::GraphicsProcessor.processTrace;
		FT810EMU::Emulator.run(*params);
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

// Set touch XY. Call on every frame when using custom graphics output
FT8XXEMU_API void FT8XXEMU_touchSetXY(int idx, int x, int y, int pressure)
{
	FT8XXEMU::g_SetTouchScreenXY(idx, x, y, pressure);
}

// Reset touch XY. Call once no longer touching when using custom graphics output
FT8XXEMU_API void FT8XXEMU_touchResetXY(int idx)
{
	FT8XXEMU::g_ResetTouchScreenXY(idx);
}

/* end of file */
