/**
 * SystemLinuxClass
 * $Id$
 * \file ft800emu_system_linux.h
 * \brief SystemLinuxClass
 * \date 2012-06-29 14:50GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef WIN32
#ifndef FT800EMU_SYSTEM_LINUX_H
#define FT800EMU_SYSTEM_LINUX_H
// #include <...>

// Linux Headers

// SDL
#if (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2))
#	include <SDL.h>
#endif

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
 * SystemLinuxClass
 * \brief SystemLinuxClass
 * \date 2012-06-29 14:50GMT
 * \author Jan Boon (Kaetemi)
 */
class SystemLinuxClass
{
public:
	SystemLinuxClass() { }

	static void Error(const char *message);

private:
	SystemLinuxClass(const SystemLinuxClass &);
	SystemLinuxClass &operator=(const SystemLinuxClass &);

}; /* class SystemLinuxClass */

extern SystemLinuxClass SystemLinux;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SYSTEM_LINUX_H */
#endif /* #ifndef WIN32 */

/* end of file */
