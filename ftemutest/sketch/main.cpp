/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <bt8xxemu.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#endif

void setup();
void loop();

extern BT8XXEMU_Emulator *g_Emulator;

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

void mcu(BT8XXEMU_Emulator *sender, void *context)
{
	setup();
	while (BT8XXEMU_isRunning(g_Emulator))
		loop();
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char*[])
{
	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, BT8XXEMU_EmulatorBT880);
	params.Flags |= BT8XXEMU_EmulatorEnableStdOut;
	params.Main = mcu;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &g_Emulator, &params);
	return 0;
}

/* end of file */
