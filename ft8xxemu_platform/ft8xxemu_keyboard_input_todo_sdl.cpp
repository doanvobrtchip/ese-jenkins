/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#if 0
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

// #include <...>
#include "ft8xxemu_keyboard_input.h"

// System includes
#include <vector>
#include <SDL.h>

// Project includes
#include "ft8xxemu_system_win32.h"
#include "ft8xxemu_system.h"

// using namespace ...;

namespace FT8XXEMU {

KeyboardInput Keyboard;

void KeyboardInput::begin()
{

}

void KeyboardInput::update()
{

}

void KeyboardInput::end()
{

}

bool KeyboardInput::isKeyDown(int key)
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
#endif

/* end of file */
