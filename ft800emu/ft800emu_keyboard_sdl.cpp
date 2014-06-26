/**
 * KeyboardClass
 * $Id$
 * \file ft800emu_keyboard_sdl.cpp
 * \brief KeyboardClass
 * \date 2012-06-27 11:48GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2))

// #include <...>
#include "ft800emu_keyboard.h"

// System includes
#include <vector>
#include <SDL.h>

// Project includes
#include "ft800emu_system_windows.h"
#include "ft800emu_system.h"
#include "wiring.h"

// using namespace ...;

#define FT800EMU_DIGITAL_LEFT  6
#define FT800EMU_DIGITAL_RIGHT 3
#define FT800EMU_DIGITAL_UP    4
#define FT800EMU_DIGITAL_DOWN  5
#define FT800EMU_DIGITAL_SHOOT 2
#define FT800EMU_ANALOG_X      0
#define FT800EMU_ANALOG_Y      1

namespace FT800EMU {

KeyboardClass Keyboard;

void KeyboardClass::begin()
{

}

void KeyboardClass::update()
{

}

void KeyboardClass::end()
{

}

bool KeyboardClass::isKeyDown(int key)
{
#ifdef FT800EMU_SDL2
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
#else
	Uint8 *keystate = SDL_GetKeyState(NULL);
#endif
	return (keystate[key] != 0);
}

} /* namespace FT800EMU */

#endif /* (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2)) */

/* end of file */
