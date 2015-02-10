/**
 * SystemSdlClass
 * $Id$
 * \file ft8xxemu_system_sdl.h
 * \brief SystemSdlClass
 * \date 2012-06-27 11:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
#ifndef FT8XXEMU_SYSTEM_SDL_H
#define FT8XXEMU_SYSTEM_SDL_H
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

namespace FT8XXEMU {

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

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_SYSTEM_SDL_H */
#endif /* #ifdef FTEMU_SDL */

/* end of file */
