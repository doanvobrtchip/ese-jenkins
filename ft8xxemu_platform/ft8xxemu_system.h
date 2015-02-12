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

#ifndef FT8XXEMU_SYSTEM_H
#define FT8XXEMU_SYSTEM_H
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

	static unsigned int getCPUCount();

	static void disableAutomaticPriorityBoost();

	static void makeLowPriorityThread();
	static void makeNormalPriorityThread();
	static void makeHighPriorityThread();
	static void makeHighestPriorityThread();
	static void makeRealtimePriorityThread();

	static void makeMainThread();
	static bool isMainThread();
	static inline bool setMainThreadSwitchable(bool value) { s_MainThreadSwitchable = value; }
	static inline bool isMainThreadSwitchable() { return s_MainThreadSwitchable; }

	static void makeMCUThread();
	static int isMCUThread();
	static void prioritizeMCUThread();
	static void unprioritizeMCUThread();
	static void holdMCUThread();
	static void resumeMCUThread();
	static void killMCUThread();

	static void makeCoprocessorThread();
	static bool isCoprocessorThread();
	static void prioritizeCoprocessorThread();
	static void unprioritizeCoprocessorThread();
	static void holdCoprocessorThread();
	static void resumeCoprocessorThread();

	static void switchThread();

	static void *setThreadGamesCategory(unsigned long *);
	static void revertThreadCategory(void *);

	static void enterSwapDL();
	static void leaveSwapDL();

	// MCU
	static uint16_t getAnalogRead(uint8_t pin);
	static void setAnalogRead(uint8_t pin, uint16_t value);

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

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_SYSTEM_H */

/* end of file */