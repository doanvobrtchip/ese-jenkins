/**
 * GraphicsDriverClass
 * $Id$
 * \file ft800emu_graphics_driver.h
 * \brief GraphicsDriverClass
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_GRAPHICS_DRIVER_H
#define FT800EMU_GRAPHICS_DRIVER_H
// #include <...>

// System includes

// Project includes

namespace FT800EMU {

typedef unsigned short argb1555;

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

	static void renderBuffer();

	static argb1555 *getBufferARGB1555();
	static inline bool isUpsideDown()
	{
		#ifdef FT800EMU_SDL
		return false;
		#else
		return true;
		#endif
	}

private:
	GraphicsDriverClass(const GraphicsDriverClass &);
	GraphicsDriverClass &operator=(const GraphicsDriverClass &);

}; /* class GraphicsDriverClass */

extern GraphicsDriverClass GraphicsDriver;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_DRIVER_H */

/* end of file */
