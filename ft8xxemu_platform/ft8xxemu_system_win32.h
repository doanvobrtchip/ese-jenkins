/**
 * SystemWin32
 * $Id$
 * \file ft8xxemu_system_win32.h
 * \brief SystemWin32
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef WIN32
#ifndef BT8XXEMU_SYSTEM_WINDOWS_H
#define BT8XXEMU_SYSTEM_WINDOWS_H

#include "ft8xxemu_system.h"

/*

#ifdef BT8XXEMU_INTTYPES_DEFINED_NULL
#undef NULL
#define NULL 0
#endif

// Windows Headers
#include <windows.h>
#include <avrt.h>

// SDL
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
#	include <SDL.h>
#endif

// GDI+
#include <gdiplus.h>

// WASAPI
#include <objbase.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

// C Headers
#include <cstdlib>
#include <tchar.h>

// STL Headers
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cmath>

// DirectInput Headers
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"


// Types
#include "bt8xxemu_inttypes.h"

*/

#include <string>

namespace FT8XXEMU {
	class System;

/**
 * SystemWin32
 * \brief SystemWin32
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */
class SystemWin32
{
public:
	SystemWin32(System *system) : m_System(system) { }

	static std::string getWin32ErrorString(DWORD hr);
	static std::string getWin32LastErrorString();
	static std::string getHResultErrorString(HRESULT hResult);

private:
	System *m_System = NULL;

private:
	SystemWin32(const SystemWin32 &) = delete;
	SystemWin32 &operator=(const SystemWin32 &) = delete;

}; /* class SystemWin32 */

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_SYSTEM_WINDOWS_H */
#endif /* #ifdef WIN32 */

/* end of file */
