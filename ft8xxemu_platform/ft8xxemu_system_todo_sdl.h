/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2023  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
#ifndef BT8XXEMU_SYSTEM_SDL_H
#define BT8XXEMU_SYSTEM_SDL_H
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
	static void IgnoreErrorSdl();

private:
	SystemSdlClass(const SystemSdlClass &);
	SystemSdlClass &operator=(const SystemSdlClass &);

}; /* class SystemSdlClass */

extern SystemSdlClass SystemSdl;

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_SYSTEM_SDL_H */
#endif /* #ifdef FTEMU_SDL */

/* end of file */
