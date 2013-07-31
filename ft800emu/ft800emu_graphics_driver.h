/**
 * GraphicsDriverClass
 * $Id$
 * \file ft800emu_graphics_driver.h
 * \brief GraphicsDriverClass
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_GRAPHICS_DRIVER_H
#define FT800EMU_GRAPHICS_DRIVER_H
// #include <...>

// System includes

// Project includes
#include "ft800emu_inttypes.h"

#define FT800EMU_WINDOW_TITLE TEXT("FT800 Emulator")
#define FT800EMU_WINDOW_WIDTH_DEFAULT 100 // 480
#define FT800EMU_WINDOW_HEIGHT_DEFAULT 50 // 272
#define FT800EMU_WINDOW_RATIO_DEFAULT 2.0f // (480.0f / 272.0f)
#define FT800EMU_WINDOW_WIDTH_MAX 512
#define FT800EMU_WINDOW_HEIGHT_MAX 512
#define FT800EMU_WINDOW_KEEPRATIO 1
#define FT800EMU_WINDOW_SCALE 1

namespace FT800EMU {

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
	static void renderBuffer(bool output);

	static argb8888 *getBufferARGB8888();
	static inline bool isUpsideDown()
	{
		#ifdef FT800EMU_SDL
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

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_DRIVER_H */

/* end of file */
