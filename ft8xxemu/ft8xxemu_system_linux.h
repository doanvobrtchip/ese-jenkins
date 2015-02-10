/**
 * SystemLinuxClass
 * $Id$
 * \file ft8xxemu_system_linux.h
 * \brief SystemLinuxClass
 * \date 2012-06-29 14:50GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef WIN32
#ifndef FT8XXEMU_SYSTEM_LINUX_H
#define FT8XXEMU_SYSTEM_LINUX_H
// #include <...>

// Linux Headers

// SDL
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
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

namespace FT8XXEMU {

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

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_SYSTEM_LINUX_H */
#endif /* #ifndef WIN32 */

/* end of file */
