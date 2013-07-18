/**
 * GraphicsProcessorClass
 * $Id$
 * \file ft800emu_graphics_processor.h
 * \brief GraphicsProcessorClass
 * \date 2013-06-22 09:29GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_GRAPHICS_PROCESSOR_H
#define FT800EMU_GRAPHICS_PROCESSOR_H
// #include <...>

// System includes

// Project includes
#include "ft800emu_inttypes.h"

#define FT800EMU_DEBUGMODE_NONE 0
#define FT800EMU_DEBUGMODE_ALPHA 1
#define FT800EMU_DEBUGMODE_TAG 2
#define FT800EMU_DEBUGMODE_STENCIL 3
#define FT800EMU_DEBUGMODE_COUNT 2

namespace FT800EMU {

/**
 * GraphicsProcessorClass
 * \brief GraphicsProcessorClass
 * \date 2013-06-22 09:29GMT
 * \author Jan Boon (Kaetemi)
 */
class GraphicsProcessorClass
{
public:
	GraphicsProcessorClass() { }

	static void begin();
	static void end();

	static void process(argb8888 *screenArgb8888, bool upsideDown, uint32_t hsize, uint32_t vsize);
	
	static void setDebugMode(int debugMode);
	static int getDebugMode();

private:
	GraphicsProcessorClass(const GraphicsProcessorClass &);
	GraphicsProcessorClass &operator=(const GraphicsProcessorClass &);
	
}; /* class GraphicsProcessorClass */

extern GraphicsProcessorClass GraphicsProcessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_PROCESSOR_H */

/* end of file */
