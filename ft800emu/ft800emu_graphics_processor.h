/**
 * GraphicsProcessor
 * $Id$
 * \file ft800emu_graphics_processor.h
 * \brief GraphicsProcessor
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
#include <memory>

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

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Memory;
	class Touch;

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
 * GraphicsProcessor
 * \brief GraphicsProcessor
 * \date 2013-06-22 09:29GMT
 * \author Jan Boon (Kaetemi)
 */
class GraphicsProcessor
{
public:
	GraphicsProcessor(FT8XXEMU::System *system, Memory *memory, Touch *touch, bool backgroundPerformance);
	~GraphicsProcessor();

	void setThreadPriority(bool realtime);

	void process(
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
	void processBlank();

	void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

	// Enables or disables emuating REG_PWM_DUTY by fading to black
	void enableRegPwmDutyEmulation(bool enabled = true);

	// Enables multithreaded rendering, sets thread count to number of available CPU cores
	void enableMultithread(bool enabled = true);
	// Reduces the number of used threads with the specified amount
	void reduceThreads(int nb);

	void setDebugMode(int debugMode);
	int getDebugMode();
	void setDebugMultiplier(int debugMultiplier);
	int getDebugMultiplier();
	void setDebugLimiter(int debugLimiter);
	int getDebugLimiter();

	bool getDebugLimiterEffective();
	int getDebugLimiterIndex();

public:
	inline FT8XXEMU::System *system() { return m_System; }
	inline Memory *memory() { return m_Memory; }
	inline Touch *touch() { return m_Touch; }

private:
	struct ThreadInfo;
	std::vector<std::unique_ptr<ThreadInfo> > m_ThreadInfos;

	void resizeThreadInfos(int size);
	void launchGraphicsProcessorThread(ThreadInfo *li);

private:
	FT8XXEMU::System *m_System = 0;
	Memory *m_Memory = 0;
	Touch *m_Touch = 0;

private:
	GraphicsProcessor(const GraphicsProcessor &) = delete;
	GraphicsProcessor &operator=(const GraphicsProcessor &) = delete;

}; /* class GraphicsProcessor */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_PROCESSOR_H */

/* end of file */
