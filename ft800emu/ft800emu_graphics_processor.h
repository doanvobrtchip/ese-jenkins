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

private:
	GraphicsProcessorClass(const GraphicsProcessorClass &);
	GraphicsProcessorClass &operator=(const GraphicsProcessorClass &);
	
}; /* class GraphicsProcessorClass */

extern GraphicsProcessorClass GraphicsProcessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_PROCESSOR_H */

/* end of file */
