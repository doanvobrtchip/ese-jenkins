/**
 * GraphicsDriverClass
 * $Id$
 * \file ft8xxemu_graphics_driver.h
 * \brief GraphicsDriverClass
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT8XXEMU_GRAPHICS_DRIVER_H
#define FT8XXEMU_GRAPHICS_DRIVER_H
// #include <...>

// System includes

// Project includes
#include "ft8xxemu_inttypes.h"

#define FT8XXEMU_WINDOW_TITLE_A "FT8XX Emulator"
#define FT8XXEMU_WINDOW_TITLE TEXT(FT8XXEMU_WINDOW_TITLE_A)
#define FT8XXEMU_WINDOW_WIDTH_DEFAULT 480
#define FT8XXEMU_WINDOW_HEIGHT_DEFAULT 272
#define FT8XXEMU_WINDOW_RATIO_DEFAULT (480.0f / 272.0f)
#define FT8XXEMU_WINDOW_WIDTH_MAX 2048
#define FT8XXEMU_WINDOW_HEIGHT_MAX 2048
#define FT8XXEMU_WINDOW_KEEPRATIO 1
#define FT8XXEMU_WINDOW_SCALE 1

// Render SDL2 upside down (makes no difference)
#define FT8XXEMU_FLIP_SDL2 0

namespace FT8XXEMU {

/**
 * GraphicsDriverClass
 * \brief GraphicsDriverClass
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */
class GraphicsDriverClass
{
public:
	GraphicsDriverClass() { }

	static void begin();
	static bool update();
	static void end();

	static void setMode(int width, int height);
	static void renderBuffer(bool output, bool changed);

	static argb8888 *getBufferARGB8888();
	static inline bool isUpsideDown()
	{
		#if (defined(FTEMU_SDL) || (defined(FTEMU_SDL2) && !FT8XXEMU_FLIP_SDL2))
		return false;
		#else
		return true;
		#endif
	}

	static void enableMouse(bool enabled = true);
	static void setMousePressure(int pressure = 0);

private:
	GraphicsDriverClass(const GraphicsDriverClass &);
	GraphicsDriverClass &operator=(const GraphicsDriverClass &);

}; /* class GraphicsDriverClass */

extern GraphicsDriverClass GraphicsDriver;

extern void (*g_ResetTouchScreenXY)();
extern void (*g_SetTouchScreenXY)(int x, int y, int pressure);

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_GRAPHICS_DRIVER_H */

/* end of file */
