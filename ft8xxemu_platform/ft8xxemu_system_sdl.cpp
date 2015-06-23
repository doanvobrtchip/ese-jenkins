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

// Project includes
#include "sleep_wake.h"

namespace FT8XXEMU {

SystemSdlClass SystemSdl;
static SleepWake s_RenderSleepWake;
static bool s_RenderWoke = false;

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

void SystemClass::renderSleep(int ms)
{
	s_RenderSleepWake.sleep(ms);
}

void SystemClass::renderWake()
{
	s_RenderWoke = true;
	s_RenderSleepWake.wake();
}

bool SystemClass::renderWoke()
{
	bool res = s_RenderWoke;
	s_RenderWoke = false;
	return res;
}

} /* namespace FT8XXEMU */

#endif /* (defined(FTEMU_SDL) || defined(FTEMU_SDL2)) */

/* end of file */
