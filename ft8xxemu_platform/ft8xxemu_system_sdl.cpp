/**
 * SystemSdlClass
 * $Id$
 * \file ft8xxemu_system_sdl.cpp
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
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
