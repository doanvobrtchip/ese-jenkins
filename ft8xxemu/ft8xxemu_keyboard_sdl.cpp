/**
 * KeyboardClass
 * $Id$
 * \file ft8xxemu_keyboard_sdl.cpp
 * \brief KeyboardClass
 * \date 2012-06-27 11:48GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

// #include <...>
#include "ft8xxemu_keyboard.h"

// System includes
#include <vector>
#include <SDL.h>

// Project includes
#include "ft8xxemu_system_windows.h"
#include "ft8xxemu_system.h"
#include "wiring.h"

// using namespace ...;

#define FT8XXEMU_DIGITAL_LEFT  6
#define FT8XXEMU_DIGITAL_RIGHT 3
#define FT8XXEMU_DIGITAL_UP    4
#define FT8XXEMU_DIGITAL_DOWN  5
#define FT8XXEMU_DIGITAL_SHOOT 2
#define FT8XXEMU_ANALOG_X      0
#define FT8XXEMU_ANALOG_Y      1

namespace FT8XXEMU {

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
#ifdef FTEMU_SDL2
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	return (keystate[SDL_GetScancodeFromKey(key)] != 0);
#else
	Uint8 *keystate = SDL_GetKeyState(NULL);
	return (keystate[key] != 0);
#endif
}

} /* namespace FT8XXEMU */

#endif /* (defined(FTEMU_SDL) || defined(FTEMU_SDL2)) */

/* end of file */
