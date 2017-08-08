/**
 * SystemClass
 * $Id$
 * \file ft8xxemu_system.h
 * \brief SystemClass
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef BT8XXEMU_SYSTEM_H
#define BT8XXEMU_SYSTEM_H
// #include <...>

// System includes

// Project includes
#include "ft8xxemu_inttypes.h"

namespace FT8XXEMU {

/**
 * SystemClass
 * \brief SystemClass
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */
class SystemClass
{
public:
	SystemClass() { }

	static void begin();
	static void update();
	static void end();

	// OS Specific
	static double getSeconds();
	static long getMillis();
	static long getMicros();
	static long getFreqTick(int hz);

	static void overrideMCUDelay(void (*delay)(int));

	static void delayForMCU(int ms);
	static void delay(int ms);
	static void delayMicros(int us);

	static void renderSleep(int ms);
	static void renderWake();
	static bool renderWoke();

	static unsigned int getCPUCount();

	static void switchThread();

	static void enterSwapDL();
	static void leaveSwapDL();

	// Debugging
	static inline double getFrameTime() { return s_FrameTime; }
	static inline double getFrameTimeDelta() { return s_FrameTimeDelta; }

	static inline double getFPS() { return 1.0 / s_FrameTimeDelta; }
	static inline double getFPSSmooth() { return s_FPSSmooth; }
	static inline int getFrameCount() { return s_FrameCount; }

private:
	static void _begin();
	static void _update();
	static void _end();

private:
	static double s_FrameTime, s_FrameTimeDelta;
	static int s_FrameCount;
	static double s_FPSSmooth;
	static bool s_MainThreadSwitchable;

private:
	SystemClass(const SystemClass &);
	SystemClass &operator=(const SystemClass &);

}; /* class SystemClass */

extern SystemClass System;

extern void (*g_Exception)(const char *message);
extern bool g_PrintStd;

#define FTEMU_printf(s, ...) do { if (::FT8XXEMU::g_PrintStd) { printf(s, __VA_ARGS__); } } while (false)

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_SYSTEM_H */

/* end of file */
