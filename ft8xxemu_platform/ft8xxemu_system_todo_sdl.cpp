/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

// #include <...>
#include "ft8xxemu_system_sdl.h"
#include "ft8xxemu_system.h"

// System includes
#ifdef WIN32
#	include <ObjBase.h>
#endif

// System includes
#include <SDL_thread.h>

namespace FT8XXEMU {

SystemSdlClass SystemSdl;

void SystemSdlClass::ErrorSdl()
{
	const char *sdlError = SDL_GetError();
	printf("ErrorSdl: %s", sdlError);
#ifdef WIN32
	if (GetConsoleWindow() == NULL)
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"FT8XXEMU Error (SDL)",
			sdlError,
			NULL);
#endif
	exit(1);
}

void SystemSdlClass::IgnoreErrorSdl()
{
	const char *sdlError = SDL_GetError();
	printf("ErrorSdl: %s", sdlError);
#ifdef WIN32
	if (GetConsoleWindow() == NULL)
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"FT8XXEMU Error (SDL)",
			sdlError,
			NULL);
#endif
}

} /* namespace FT8XXEMU */

#endif /* (defined(FTEMU_SDL) || defined(FTEMU_SDL2)) */

/* end of file */
