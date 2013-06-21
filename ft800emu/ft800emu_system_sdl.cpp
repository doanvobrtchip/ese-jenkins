/**
 * SystemSdlClass
 * $Id$
 * \file ft800emu_system_sdl.cpp
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011-2012  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL

// #include <...>
#include "ft800emu_system_sdl.h"
#include "ft800emu_system.h"

// Libraries
/*#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")*/

// System includes
#ifdef WIN32
#else
#	include <sched.h>
#endif
#include <SDL_thread.h>

// Project includes

using namespace std;

namespace FT800EMU {

SystemSdlClass SystemSdl;

void SystemSdlClass::ErrorSdl()
{
	printf("ErrorSdl: %s", SDL_GetError());
	exit(1);
}

} /* namespace FT800EMU */

#endif /* #ifdef FT800EMU_SDL */

/* end of file */
