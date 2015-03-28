/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "ft8xxemu.h"

#include <string.h>

// Include FT8XXEMU_PLATFORM
#include "ft8xxemu_graphics_driver.h"

// Include FT800EMU
#ifdef FTEMU_HAVE_FT800EMU
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"
#endif

// Include FT810EMU
#ifdef FTEMU_HAVE_FT810EMU
#undef FT800EMU_SPI_I2C_H
#undef FT800EMU_EMULATOR_H
#define FT800EMU FT810EMU
#define FT810EMU_MODE
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"
#undef FT800EMU
#undef FT810EMU_MODE
#endif

void (*FT8XXEMU_stop)() = NULL;
uint8_t (*FT8XXEMU_transfer)(uint8_t data) = NULL;
void (*FT8XXEMU_cs)(int cs) = NULL;
int(*FT8XXEMU_int)() = NULL;

static const char *s_Version =
	"FT8XX Emulator Library\n"
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
		FT800EMU::Emulator.run(*params);
		break;
#endif
#ifdef FTEMU_HAVE_FT810EMU
	case FT8XXEMU_EmulatorFT810:
		FT8XXEMU_stop = &FT810EMU::Emulator.stop;
		FT8XXEMU_transfer = &FT810EMU::SPII2C.transfer;
		FT8XXEMU_cs = &FT810EMU::SPII2C.csLow;
		FT8XXEMU_int = &FT810EMU::SPII2C.intnLow;
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
