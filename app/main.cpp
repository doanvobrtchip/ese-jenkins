/**
 * main.cpp
 * $Id$
 * \file main.cpp
 * \brief main.cpp
 * \date 2013-06-21 21:51GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <ft800emu_emulator.h>
#include <wiring.h>

void setup();
void loop();

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
	FT800EMU::ARDUINO::setCSPin(9);
	FT800EMU::EmulatorParameters params;
	params.Setup = setup;
	params.Loop = loop;
	params.Flags = 
		FT800EMU::EmulatorEnableKeyboard 
		| FT800EMU::EmulatorEnableMouse 
		| FT800EMU::EmulatorEnableDebugShortkeys
		| FT800EMU::EmulatorEnableRegRotate
		// | FT800EMU::EmulatorEnableCoprocessor
		| FT800EMU::EmulatorEnableGraphicsMultithread;
	FT800EMU::Emulator.run(params);
	return 0;
}
