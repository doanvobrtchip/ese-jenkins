/*
FT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include "ft8xxemu.h"
#include "ft8xxemu_diag.h"

#include "bt8xxemu.h"
#include "bt8xxemu_diag.h"

#include <string.h>
#include <stdio.h>

static void(*s_Setup)();
static void(*s_Loop)();
static BT8XXEMU_Emulator *s_Emulator;
static void(*s_MCUSleep)(int ms);
static int(*s_Graphics)(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags);
static void(*s_Exception)(const char *message);
static void(*s_Close)();
static bool s_Closed;

static void mainLoop(BT8XXEMU_Emulator *sender, void *context)
{
	if (s_Setup) 
	{
		s_Setup();
	}
	if (s_Loop) 
	{
		while (!s_Closed && BT8XXEMU_isRunning(s_Emulator))
		{
			s_Loop();
		}
	}
}

static void mcuSleep(BT8XXEMU_Emulator *sender, void *context, int ms)
{
	s_MCUSleep(ms);
}

static int graphics(BT8XXEMU_Emulator *sender, void *context, int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags)
{
	return s_Graphics(output, buffer, hsize, vsize, (FT8XXEMU_FrameFlags)flags);
}

static void log(BT8XXEMU_Emulator *sender, void *context, BT8XXEMU_LogType type, const char *message)
{
	s_Exception(message);
}

static void close(BT8XXEMU_Emulator *sender, void *context)
{
	s_Closed = true;
	s_Close();
}

FT8XXEMU_API const char *FT8XXEMU_version()
{
	return BT8XXEMU_version();
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
		// | FT8XXEMU_EmulatorEnableDynamicDegrade
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

	if (s_Emulator)
	{
		BT8XXEMU_destroy(s_Emulator);
		s_Emulator = NULL;
	}

	s_Closed = false;
	BT8XXEMU_EmulatorParameters params_;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params_, (BT8XXEMU_EmulatorMode)params->Mode);

	s_Setup = params->Setup;
	s_Loop = params->Loop;
	params_.Main = mainLoop;
	params_.MousePressure = params->MousePressure;
	params_.ExternalFrequency = params->ExternalFrequency;
	params_.ReduceGraphicsThreads = params->ReduceGraphicsThreads;
	if (params->MCUSleep)
	{
		s_MCUSleep = params->MCUSleep;
		params_.MCUSleep = mcuSleep;
	}
	if (params->RomFilePath)
	{
		size_t a;
		mbstowcs_s(&a, params_.RomFilePath, params->RomFilePath, 259);
	}
	if (params->OtpFilePath)
	{
		size_t a;
		mbstowcs_s(&a, params_.OtpFilePath, params->OtpFilePath, 259);
	}
	if (params->CoprocessorRomFilePath)
	{
		size_t a;
		mbstowcs_s(&a, params_.CoprocessorRomFilePath, params->CoprocessorRomFilePath, 259);
	}
	if (params->Graphics)
	{
		s_Graphics = params->Graphics;
		params_.Graphics = graphics;
	}
	if (params->Exception)
	{
		s_Exception = params->Exception;
		params_.Log = log;
	}
	if (params->Close)
	{
		s_Close = params->Close;
		params_.Close = close;
	}

	BT8XXEMU_run(BT8XXEMU_VERSION_API, &s_Emulator, &params_);

	if (s_Emulator)
	{
		BT8XXEMU_destroy(s_Emulator);
		s_Emulator = NULL;
	}
}

static void FT8XXEMU_stop_()
{
	BT8XXEMU_stop(s_Emulator);
}

FT8XXEMU_API void(*FT8XXEMU_stop)() = FT8XXEMU_stop_;

static uint8_t FT8XXEMU_transfer_(uint8_t data)
{
	return BT8XXEMU_transfer(s_Emulator, data);
}

FT8XXEMU_API uint8_t(*FT8XXEMU_transfer)(uint8_t data) = FT8XXEMU_transfer_;

static void FT8XXEMU_cs_(int cs)
{
	BT8XXEMU_chipSelect(s_Emulator, cs);
}

FT8XXEMU_API void(*FT8XXEMU_cs)(int cs) = FT8XXEMU_cs_;

static int FT8XXEMU_int_()
{
	return BT8XXEMU_hasInterrupt(s_Emulator);
}

FT8XXEMU_API extern int(*FT8XXEMU_int)() = FT8XXEMU_int_;

FT8XXEMU_API void FT8XXEMU_touchSetXY(int idx, int x, int y, int pressure)
{
	BT8XXEMU_touchSetXY(s_Emulator, idx, x, y, pressure);
}

FT8XXEMU_API void FT8XXEMU_touchResetXY(int idx)
{
	BT8XXEMU_touchResetXY(s_Emulator, idx);
}

uint8_t *FT8XXEMU_getRam_()
{
	return BT8XXEMU_getRam(s_Emulator);
}

FT8XXEMU_API uint8_t *(*FT8XXEMU_getRam)() = FT8XXEMU_getRam_;

const uint32_t *FT8XXEMU_getDisplayList_()
{
	return BT8XXEMU_getDisplayList(s_Emulator);
}

FT8XXEMU_API const uint32_t *(*FT8XXEMU_getDisplayList)() = FT8XXEMU_getDisplayList_;

void FT8XXEMU_poke_()
{
	BT8XXEMU_poke(s_Emulator);
}

FT8XXEMU_API void(*FT8XXEMU_poke)() = FT8XXEMU_poke_;

int *FT8XXEMU_getDisplayListCoprocessorWrites_()
{
	return BT8XXEMU_getDisplayListCoprocessorWrites(s_Emulator);
}

FT8XXEMU_API int *(*FT8XXEMU_getDisplayListCoprocessorWrites)() = FT8XXEMU_getDisplayListCoprocessorWrites_;

void FT8XXEMU_clearDisplayListCoprocessorWrites_()
{
	return BT8XXEMU_clearDisplayListCoprocessorWrites(s_Emulator);
}

FT8XXEMU_API void(*FT8XXEMU_clearDisplayListCoprocessorWrites)() = FT8XXEMU_clearDisplayListCoprocessorWrites_;

bool FT8XXEMU_getDebugLimiterEffective_()
{
	return BT8XXEMU_getDebugLimiterEffective(s_Emulator);
}

FT8XXEMU_API bool(*FT8XXEMU_getDebugLimiterEffective)() = FT8XXEMU_getDebugLimiterEffective_;

int FT8XXEMU_getDebugLimiterIndex_()
{
	return BT8XXEMU_getDebugLimiterIndex(s_Emulator);
}

FT8XXEMU_API int(*FT8XXEMU_getDebugLimiterIndex)() = FT8XXEMU_getDebugLimiterIndex_;

void FT8XXEMU_setDebugLimiter_(int debugLimiter)
{
	BT8XXEMU_setDebugLimiter(s_Emulator, debugLimiter);
}

FT8XXEMU_API void(*FT8XXEMU_setDebugLimiter)(int debugLimiter) = FT8XXEMU_setDebugLimiter_;

void FT8XXEMU_processTrace_(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize)
{
	BT8XXEMU_processTrace(s_Emulator, result, size, x, y, hsize);
}

FT8XXEMU_API void(*FT8XXEMU_processTrace)(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = FT8XXEMU_processTrace_;

/* end of file */
