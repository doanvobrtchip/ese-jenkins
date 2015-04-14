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

// using namespace ...;

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
