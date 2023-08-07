/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2023  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#include "bt8xxemu.h"
#include "bt8xxemu_diag.h"
#include "bt8xxemu_pp.h"
#include "bt8xxemu_flash.h"
#include "bt8xxemu_version.h"

#include <string.h>
#include <stdio.h>

// Include BT8XXEMU_PLATFORM
#include "ft8xxemu_window_output.h"

// Include FT800EMU
#define BT8XXEMU_NODEFS
#ifdef FTEMU_HAVE_FT800EMU
#include "ft800emu_emulator.h"
#endif

// Include FT810EMU
#ifdef FTEMU_HAVE_FT810EMU
#undef FT800EMU_EMULATOR_H
#define FT800EMU FT810EMU
#define FT810EMU_MODE
#include "ft800emu_emulator.h"
#undef FT800EMU
#undef FT810EMU_MODE
#endif

// Include BT815EMU
#ifdef FTEMU_HAVE_BT880EMU
#undef FT800EMU_EMULATOR_H
#define FT800EMU BT880EMU
#define FT810EMU BT880EMU
#define FT810EMU_MODE
#define BT880EMU_MODE
#include "ft800emu_emulator.h"
#undef FT800EMU
#undef FT810EMU
#undef FT810EMU_MODE
#undef BT880EMU_MODE
#endif

// Include BT815EMU
#ifdef FTEMU_HAVE_BT815EMU
#undef FT800EMU_EMULATOR_H
#define FT800EMU BT815EMU
#define FT810EMU BT815EMU
#define FT810EMU_MODE
#define BT815EMU_MODE
#include "ft800emu_emulator.h"
#undef FT800EMU
#undef FT810EMU
#undef FT810EMU_MODE
#undef BT815EMU_MODE
#endif

// Include BT817EMU
#ifdef FTEMU_HAVE_BT817EMU
#undef FT800EMU_EMULATOR_H
#define FT800EMU BT817EMU
#define FT810EMU BT817EMU
#define BT815EMU BT817EMU
#define FT810EMU_MODE
#define BT815EMU_MODE
#define BT817EMU_MODE
#include "ft800emu_emulator.h"
#undef FT800EMU
#undef FT810EMU
#undef BT815EMU
#undef FT810EMU_MODE
#undef BT815EMU_MODE
#undef BT817EMU_MODE
#endif

static const char *c_Version =
	"BT8XX Emulator Library v" BT8XXEMU_VERSION_STRING "\n"
	"Copyright(C) 2013-2015  Future Technology Devices International Ltd\n"
	"Copyright(C) 2016-2023  Bridgetek Pte Lte\n"
	"Author: Jan Boon <jan.boon@kaetemi.be>";

BT8XXEMU_API const char *BT8XXEMU_version()
{
	return c_Version;
}

BT8XXEMU_API void BT8XXEMU_defaults(uint32_t versionApi, BT8XXEMU_EmulatorParameters *params, BT8XXEMU_EmulatorMode mode)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible bt8xxemu API version\n");
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
		// | BT8XXEMU_EmulatorEnableDynamicDegrade
		| BT8XXEMU_EmulatorEnableTouchTransformation
		| BT8XXEMU_EmulatorEnableMainPerformance;

	if (mode >= BT8XXEMU_EmulatorBT817)
		params->Flags |= BT8XXEMU_EmulatorEnableHSFPreview;

	params->Mode = mode;
}

BT8XXEMU_API void BT8XXEMU_run(uint32_t versionApi, BT8XXEMU_Emulator **emulator, const BT8XXEMU_EmulatorParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible bt8xxemu API version\n");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return;
	}

	try
	{
		switch (params->Mode)
		{
		case 0:
#ifdef FTEMU_HAVE_FT800EMU
		case BT8XXEMU_EmulatorFT800:
		case BT8XXEMU_EmulatorFT801:
		{
			FT800EMU::Emulator *ft800emu = new FT800EMU::Emulator();
			*emulator = ft800emu;
			ft800emu->run(*params);
			break;
		}
#endif
#ifdef FTEMU_HAVE_FT810EMU
		case BT8XXEMU_EmulatorFT810:
		case BT8XXEMU_EmulatorFT811:
		case BT8XXEMU_EmulatorFT812:
		case BT8XXEMU_EmulatorFT813:
		{
			FT810EMU::Emulator *ft810emu = new FT810EMU::Emulator();
			*emulator = ft810emu;
			ft810emu->run(*params);
			break;
		}
#endif
#ifdef FTEMU_HAVE_BT880EMU
		case BT8XXEMU_EmulatorBT880:
		case BT8XXEMU_EmulatorBT881:
		case BT8XXEMU_EmulatorBT882:
		case BT8XXEMU_EmulatorBT883:
		{
			BT880EMU::Emulator *bt880emu = new BT880EMU::Emulator();
			*emulator = bt880emu;
			bt880emu->run(*params);
			break;
		}
#endif
#ifdef FTEMU_HAVE_BT815EMU
		case BT8XXEMU_EmulatorBT815:
		case BT8XXEMU_EmulatorBT816:
		{
			BT815EMU::Emulator *bt815emu = new BT815EMU::Emulator();
			*emulator = bt815emu;
			bt815emu->run(*params);
			break;
		}
#endif
#ifdef FTEMU_HAVE_BT817EMU
		case BT8XXEMU_EmulatorBT817:
		case BT8XXEMU_EmulatorBT818:
		{
			BT817EMU::Emulator *bt817emu = new BT817EMU::Emulator();
			*emulator = bt817emu;
			bt817emu->run(*params);
			break;
		}
#endif
		default:
		{
			fprintf(stderr, "Invalid bt8xxemu emulator mode selected\n");
			*emulator = NULL;
#if defined(_MSC_VER) && !defined(NDEBUG)
			__debugbreak();
#endif
			break;
		}
		}
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "run");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API void BT8XXEMU_stop(BT8XXEMU_Emulator *emulator)
{
	try
	{
		emulator->stop();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "stop");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API void BT8XXEMU_destroy(BT8XXEMU_Emulator *emulator)
{
	try
	{
		emulator->stop();
		delete emulator;
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "destroy");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API int BT8XXEMU_isRunning(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->isRunning();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "isRunning");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API uint8_t BT8XXEMU_transfer(BT8XXEMU_Emulator *emulator, uint8_t data)
{
	try
	{
		return emulator->transfer(data);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "transfer");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API void BT8XXEMU_chipSelect(BT8XXEMU_Emulator *emulator, int cs)
{
	try
	{
		emulator->cs(cs != 0);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "chipSelect");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API int BT8XXEMU_hasInterrupt(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->hasInterrupt();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "hasInterrupt");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API void BT8XXEMU_touchSetXY(BT8XXEMU_Emulator *emulator, int idx, int x, int y, int pressure)
{
	try
	{
		emulator->touchSetXY(idx, x, y, pressure);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "touchSetXY");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API void BT8XXEMU_touchResetXY(BT8XXEMU_Emulator *emulator, int idx)
{
	try
	{
		emulator->touchResetXY(idx);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "touchResetXY");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API int BT8XXEMU_setFlag(BT8XXEMU_Emulator *emulator, BT8XXEMU_EmulatorFlags flag, int value)
{
	try
	{
		return emulator->setFlag(flag, value);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "setFlag");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API uint8_t *BT8XXEMU_getRam(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->getRam();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "getRam");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}
}

BT8XXEMU_API const uint32_t *BT8XXEMU_getDisplayList(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->getDisplayList();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "getDisplayList");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}
}

BT8XXEMU_API void BT8XXEMU_poke(BT8XXEMU_Emulator *emulator)
{
	try
	{
		emulator->poke();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "poke");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API int *BT8XXEMU_getDisplayListCoprocessorWrites(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->getDisplayListCoprocessorWrites();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "getDisplayListCoprocessorWrites");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}
}

BT8XXEMU_API void BT8XXEMU_clearDisplayListCoprocessorWrites(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->clearDisplayListCoprocessorWrites();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "clearDisplayListCoprocessorWrites");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API int BT8XXEMU_getDebugLimiterEffective(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->getDebugLimiterEffective();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "getDebugLimiterEffective");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API int BT8XXEMU_getDebugLimiterIndex(BT8XXEMU_Emulator *emulator)
{
	try
	{
		return emulator->getDebugLimiterIndex();
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "getDebugLimiterIndex");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API void BT8XXEMU_setDebugLimiter(BT8XXEMU_Emulator *emulator, int debugLimiter)
{
	try
	{
		emulator->setDebugLimiter(debugLimiter);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "setDebugLimiter");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API void BT8XXEMU_processTrace(BT8XXEMU_Emulator *emulator, int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize)
{
	try
	{
		emulator->processTrace(result, size, x, y, hsize);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_%s\n", "processTrace");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API void BT8XXEMU_Flash_defaults(uint32_t versionApi, BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible bt8xxemu API version\n");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return;
	}

	memset(params, 0, sizeof(BT8XXEMU_FlashParameters));

	wcscpy(params->DeviceType, L"mx25lemu");
	params->SizeBytes = 8 * 1024 * 1024;
}

// Create flash emulator instance
BT8XXEMU_API BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible bt8xxemu API version\n");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}

	try
	{
		static const std::wstring libraryExt = L".dll";
		std::wstring libraryName = params->DeviceType + libraryExt;
		HMODULE module = LoadLibraryW(libraryName.c_str());

		if (!module)
		{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244) // Trim wchar to char
#endif
			fprintf(stderr, "Failed to load flash library (%s)\n", std::string(libraryName.begin(), libraryName.end()).c_str());
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if defined(_MSC_VER) && !defined(NDEBUG)
			__debugbreak();
#endif
			return NULL;
		}

		BT8XXEMU_Flash *(*create)(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
			= reinterpret_cast<BT8XXEMU_Flash *(*)(uint32_t, const BT8XXEMU_FlashParameters *)>(
				(void *)GetProcAddress(module, "BT8XXEMU_Flash_create"));

		if (!create)
		{
			fprintf(stderr, "Library does not contain BT8XXEMU_Flash_create procedure\n");
#if defined(_MSC_VER) && !defined(NDEBUG)
			__debugbreak();
#endif
			return NULL;
		}

		BT8XXEMU_Flash *flash = create(versionApi, params);

		if (!flash)
		{
			fprintf(stderr, "Unable to create flash emulator instance\n");
#if defined(_MSC_VER) && !defined(NDEBUG)
			__debugbreak();
#endif
			return NULL;
		}

		return flash;
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_Flash_%s\n", "create");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}
}

// Destroy flash emulator instance
BT8XXEMU_API void BT8XXEMU_Flash_destroy(BT8XXEMU_Flash *flash)
{
	if (!flash)
		return;

	try
	{
		flash->vTable()->Destroy(flash);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_Flash_%s\n", "destroy");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
	}
}

BT8XXEMU_API uint8_t BT8XXEMU_Flash_transferSpi4(BT8XXEMU_Flash *flash, uint8_t signal)
{
	try
	{
		return flash->vTable()->TransferSpi4(flash, signal);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_Flash_%s\n", "transferSpi4");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

BT8XXEMU_API uint8_t *BT8XXEMU_Flash_data(BT8XXEMU_Flash *flash)
{
	try
	{
		return flash->vTable()->Data(flash);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_Flash_%s\n", "data");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return NULL;
	}
}

BT8XXEMU_API size_t BT8XXEMU_Flash_size(BT8XXEMU_Flash *flash)
{
	try
	{
		return flash->vTable()->Size(flash);
	}
	catch (...)
	{
		fprintf(stderr, "Unexpected C++ exception in BT8XXEMU_Flash_%s\n", "size");
#if defined(_MSC_VER) && !defined(NDEBUG)
		__debugbreak();
#endif
		return 0;
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
