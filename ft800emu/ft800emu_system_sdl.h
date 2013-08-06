/**
 * SystemSdlClass
 * $Id$
 * \file ft800emu_system_sdl.h
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL
#ifndef FT800EMU_SYSTEM_SDL_H
#define FT800EMU_SYSTEM_SDL_H
// #include <...>

// Sdl Headers
#include <SDL.h>
/*#include <sdl.h>
#include <avrt.h>*/

// GDI+
/*#include <gdiplus.h>*/

// WASAPI
/*#include <objbase.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>*/

// C Headers
#include <cstdlib>

// STL Headers
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

namespace FT800EMU {

/**
 * SystemSdlClass
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */
class SystemSdlClass
{
public:
	SystemSdlClass() { }

	static void ErrorSdl();

private:
	SystemSdlClass(const SystemSdlClass &);
	SystemSdlClass &operator=(const SystemSdlClass &);

}; /* class SystemSdlClass */

extern SystemSdlClass SystemSdl;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SYSTEM_SDL_H */
#endif /* #ifdef FT800EMU_SDL */

/* end of file */
