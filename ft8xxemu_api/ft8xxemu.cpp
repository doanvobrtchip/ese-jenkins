/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon (jan.boon@kaetemi.be)
 */

#include "ft8xxemu.h"

// Include FT800EMU
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"

// Include FT810EMU
#undef FT800EMU_SPI_I2C_H
#undef FT800EMU_EMULATOR_H
#define FT800EMU FT810EMU
#define FT810EMU_MODE
#include "ft800emu_spi_i2c.h"
#include "ft800emu_emulator.h"

#include <SDL_assert.h>

void (*FT8XXEMU_stop)();
uint8_t (*FT8XXEMU_transfer)(uint8_t data);
void (*FT8XXEMU_csLow)(int low);
void (*FT8XXEMU_csHigh)(int high);

FT8XXEMU_API void FT8XXEMU_run(const FT8XXEMU_EmulatorParameters *params)
{
	switch (params->Mode)
	{
	case FT8XXEMU_EmulatorFT800:
	case FT8XXEMU_EmulatorFT801:
		FT8XXEMU_stop = &FT800EMU::Emulator.stop;
		FT8XXEMU_transfer = &FT800EMU::SPII2C.transfer;
		FT8XXEMU_csLow = &FT800EMU::SPII2C.csLow;
		FT8XXEMU_csHigh = &FT800EMU::SPII2C.csHigh;
		FT800EMU::Emulator.run(*params);
		break;
	case FT8XXEMU_EmulatorFT810:
		FT8XXEMU_stop = &FT810EMU::Emulator.stop;
		FT8XXEMU_transfer = &FT810EMU::SPII2C.transfer;
		FT8XXEMU_csLow = &FT810EMU::SPII2C.csLow;
		FT8XXEMU_csHigh = &FT810EMU::SPII2C.csHigh;
		FT810EMU::Emulator.run(*params);
		break;
	}
}

/* end of file */
