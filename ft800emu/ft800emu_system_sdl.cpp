/**
 * SystemSdlClass
 * $Id$
 * \file ft800emu_system_sdl.cpp
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2))

// #include <...>
#include "ft800emu_system_sdl.h"
#include "ft800emu_system.h"

// System includes
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

#endif /* (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2)) */

/* end of file */
