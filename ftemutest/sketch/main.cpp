
/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <ft8xxemu.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#endif

void setup();
void loop();

#ifdef WIN32
// enable memory leak checks, trick to get _CrtSetBreakAlloc in before main
#define DEBUG_ALLOC_HOOK
#if defined(DEBUG_ALLOC_HOOK)
int debugAllocHook(int allocType, void *userData, size_t size, int
	blockType, long requestNumber, const unsigned char *filename, int
	lineNumber);
#endif
class CEnableCrtDebug
{
public:
	CEnableCrtDebug()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(0);
#if defined(DEBUG_ALLOC_HOOK)
		LastSize = 0;
		_CrtSetAllocHook(debugAllocHook);
#endif
	}
#if defined(DEBUG_ALLOC_HOOK)
	size_t LastSize;
#endif
};
static CEnableCrtDebug _EnableCrtDebug;
#if defined(DEBUG_ALLOC_HOOK)
int debugAllocHook(int allocType, void *userData, size_t size, int
	blockType, long requestNumber, const unsigned char *filename, int
	lineNumber)
{
	if (allocType == _HOOK_ALLOC)
	{
		//if (requestNumber == 14806)
		//	_CrtSetBreakAlloc(14809);
		//if (_EnableCrtDebug.LastSize == 4 && size == 40 && requestNumber > 291000 && requestNumber < 292000)
		//	_CrtDbgBreak();
		//if (_EnableCrtDebug.LastSize == 36 && size == 112 && requestNumber > 300000)
		//	_CrtDbgBreak();
		_EnableCrtDebug.LastSize = size;
	}
	return TRUE;
}
#endif
#endif

bool graphics(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags)
{
	return true;
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	FT8XXEMU_EmulatorParameters params;
	FT8XXEMU_defaults(FT8XXEMU_VERSION_API, &params, FT8XXEMU_EmulatorFT810);
	params.Setup = setup;
	params.Loop = loop;
	// params.Graphics = graphics;
	FT8XXEMU_run(FT8XXEMU_VERSION_API, &params);
	return 0;
}

/* end of file */
