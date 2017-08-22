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
#include "ft8xxemu_window_output.h"

// Include FT800EMU
#define BT8XXEMU_NODEFS
#ifdef FTEMU_HAVE_FT800EMU
//#include "ft800emu_bus_slave.h"
#include "ft800emu_emulator.h"
//#include "ft800emu_memory.h"
//#include "ft800emu_graphics_processor.h"
#endif

// Include FT810EMU
#ifdef FTEMU_HAVE_FT810EMU
#undef FT800EMU_BUS_SLAVE_H
#undef FT800EMU_EMULATOR_H
#undef FT800EMU_MEMORY_H
#undef FT800EMU_GRAPHICS_PROCESSOR_H
#define FT800EMU FT810EMU
#define FT810EMU_MODE
//#include "ft800emu_bus_slave.h"
#include "ft800emu_emulator.h"
//#include "ft800emu_memory.h"
//#include "ft800emu_graphics_processor.h"
#undef FT800EMU
#undef FT810EMU_MODE
#endif

static BT8XXEMU::IEmulator *s_Emulator = 0;

static const char *c_Version =
	"FT8XX Emulator Library v" BT8XXEMU_VERSION_STRING "\n"
	"Copyright(C) 2013 - 2015  Future Technology Devices International Ltd\n"
	"Author: Jan Boon <jan.boon@kaetemi.be>";
BT8XXEMU_API const char *BT8XXEMU_version()
{
	return c_Version;
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
		| BT8XXEMU_EmulatorEnableTouchTransformation
		| BT8XXEMU_EmulatorEnableMainPerformance;

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
		static_cast<FT800EMU::Emulator *>(s_Emulator)->run(*params);
		break;
#endif
#ifdef FTEMU_HAVE_FT810EMU
	case BT8XXEMU_EmulatorFT810:
	case BT8XXEMU_EmulatorFT811:
	case BT8XXEMU_EmulatorFT812:
	case BT8XXEMU_EmulatorFT813:
		s_Emulator = new FT810EMU::Emulator();
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

BT8XXEMU_API uint8_t BT8XXEMU_transfer(uint8_t data)
{
	return s_Emulator->transfer(data);
}

BT8XXEMU_API void BT8XXEMU_cs(int cs)
{
	s_Emulator->cs(cs != 0);
}

BT8XXEMU_API int BT8XXEMU_hasInterrupt()
{
	return s_Emulator->hasInterrupt();
}

BT8XXEMU_API void BT8XXEMU_touchSetXY(int idx, int x, int y, int pressure)
{
	s_Emulator->touchSetXY(idx, x, y, pressure);
}

BT8XXEMU_API void BT8XXEMU_touchResetXY(int idx)
{
	s_Emulator->touchResetXY(idx);
}

BT8XXEMU_API uint8_t *BT8XXEMU_getRam()
{
	return s_Emulator->getRam();
}

BT8XXEMU_API const uint32_t *BT8XXEMU_getDisplayList()
{
	return s_Emulator->getDisplayList();
}

BT8XXEMU_API void BT8XXEMU_poke()
{
	s_Emulator->poke();
}

BT8XXEMU_API int *BT8XXEMU_getDisplayListCoprocessorWrites()
{
	return s_Emulator->getDisplayListCoprocessorWrites();
}

BT8XXEMU_API void BT8XXEMU_clearDisplayListCoprocessorWrites()
{
	return s_Emulator->clearDisplayListCoprocessorWrites();
}

BT8XXEMU_API int BT8XXEMU_getDebugLimiterEffective()
{
	return s_Emulator->getDebugLimiterEffective();
}

BT8XXEMU_API int BT8XXEMU_getDebugLimiterIndex()
{
	return s_Emulator->getDebugLimiterIndex();
}

BT8XXEMU_API void BT8XXEMU_setDebugLimiter(int debugLimiter)
{
	s_Emulator->setDebugLimiter(debugLimiter);
}

BT8XXEMU_API void BT8XXEMU_processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize)
{
	s_Emulator->processTrace(result, size, x, y, hsize);
}

/* end of file */
