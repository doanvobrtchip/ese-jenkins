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
#include <vector>

// Project includes
#include "ft8xxemu_inttypes.h"

#ifndef BT8XXEMU_NODEFS
#ifdef FT810EMU_MODE
#define FT800EMU_SCREEN_WIDTH_MAX 2048
#define FT800EMU_SCREEN_HEIGHT_MAX 2048
#else
#define FT800EMU_SCREEN_WIDTH_MAX 512
#define FT800EMU_SCREEN_HEIGHT_MAX 512
#endif
#define FT800EMU_SCREEN_HEIGHT_MASK (FT800EMU_SCREEN_HEIGHT_MAX - 1)

#define FT800EMU_DEBUGMODE_NONE 0
#define FT800EMU_DEBUGMODE_ALPHA 1
#define FT800EMU_DEBUGMODE_TAG 2
#define FT800EMU_DEBUGMODE_STENCIL 3
#define FT800EMU_DEBUGMODE_COUNT 4
#endif

namespace FT800EMU {

struct BitmapInfo
{
	uint32_t Source;
	int LayoutFormat;
	int LayoutWidth;
	int LayoutStride;
	int LayoutHeight;
	int SizeFilter;
	int SizeWrapX;
	int SizeWrapY;
	int SizeWidth;
	int SizeHeight;
};

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

	static void setThreadPriority(bool realtime);

	static void process(
		argb8888 *screenArgb8888, 
		bool upsideDown, 
		bool mirrored, 
#ifdef FT810EMU_MODE
		bool swapXY,
#endif
		uint32_t hsize, 
		uint32_t vsize, 
		uint32_t yIdx = 0, 
		uint32_t yInc = 1);
	static void processBlank();

	static void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

	// Enables or disables emuating REG_PWM_DUTY by fading to black
	static void enableRegPwmDutyEmulation(bool enabled = true);

	// Enables multithreaded rendering, sets thread count to number of available CPU cores
	static void enableMultithread(bool enabled = true);
	// Reduces the number of used threads with the specified amount
	static void reduceThreads(int nb);

	static void setDebugMode(int debugMode);
	static int getDebugMode();
	static void setDebugMultiplier(int debugMultiplier);
	static int getDebugMultiplier();
	static void setDebugLimiter(int debugLimiter);
	static int getDebugLimiter();

	static bool getDebugLimiterEffective();
	static int getDebugLimiterIndex();

	/*
	// Sets operation trace on specified point
	static void setDebugTrace(uint32_t x, uint32_t y);
	// Disables or enables tracing
	static void setDebugTrace(bool enabled);
	// Returns the debug tracing state
	static void getDebugTrace(bool &enabled, uint32_t &x, uint32_t &y);
	// Returns a *copy* of the debug trace
	static void getDebugTrace(std::vector<int> &result);
	*/

private:
	GraphicsProcessorClass(const GraphicsProcessorClass &);
	GraphicsProcessorClass &operator=(const GraphicsProcessorClass &);

}; /* class GraphicsProcessorClass */

extern GraphicsProcessorClass GraphicsProcessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_PROCESSOR_H */

/* end of file */
