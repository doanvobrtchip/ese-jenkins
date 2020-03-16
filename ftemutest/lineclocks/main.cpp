/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#include <bt8xxemu.h>
#include <stdio.h>

void setup();
void loop();

extern BT8XXEMU_Emulator *g_Emulator;

bool graphics(BT8XXEMU_Emulator *sender, void *context, bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags)
{
	return true;
}

void mcu(BT8XXEMU_Emulator *sender, void *context)
{
	setup();
	while (BT8XXEMU_isRunning(g_Emulator))
		loop();
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, BT8XXEMU_EmulatorBT817);
	params.Main = mcu;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &g_Emulator, &params);
	return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
