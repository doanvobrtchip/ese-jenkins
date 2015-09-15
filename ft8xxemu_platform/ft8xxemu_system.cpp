/**
 * SystemClass
 * $Id$
 * \file ft8xxemu_system.cpp
 * \brief SystemClass
 * \date 2011-05-25 19:32GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft8xxemu_system.h"

// System includes

// Project includes
#include "sleep_wake.h"

// using namespace ...;

namespace FT8XXEMU {


#define D_FT8XXEMU_FPS_SMOOTHING 36


double SystemClass::s_FrameTime, SystemClass::s_FrameTimeDelta; //, SystemClass::s_FrameTimeOffset;
int SystemClass::s_FrameCount;
double SystemClass::s_FPSSmooth;
bool SystemClass::s_MainThreadSwitchable = false;

static SleepWake *s_RenderSleepWake = NULL;
static bool s_RenderWoke = false;

void (*g_Exception)(const char *message) = 0;


static double s_FPSSmoothValues[D_FT8XXEMU_FPS_SMOOTHING];
//static double s_FPSSmoothTotal;
static int s_FPSSmoothAt;
static int s_FPSSmoothCount;

static uint16_t s_AnalogPins[64];

static void (*s_MCUDelay)(int) = 0;

void SystemClass::begin()
{
	SystemClass::_begin();

	// Initialize Frame Count
	s_FrameCount = 0;

	// Initialize Frame Timing Information
	s_FrameTime = 0.0;
	s_FrameTimeDelta = 0.0;
	s_FPSSmooth = 0.0;
	s_FPSSmoothAt = 0;
	for (int i = 0; i < D_FT8XXEMU_FPS_SMOOTHING; ++i)
		s_FPSSmoothValues[i] = 0.0;
	//s_FPSSmoothTotal = 0.0;
	s_FPSSmoothCount = 0;
	//s_FrameTimeOffset = SystemClass::getSeconds() * -1.0;

	s_RenderSleepWake = new SleepWake();
}

void SystemClass::update()
{
	SystemClass::_update();

	// Update Frame Count
	++s_FrameCount;
	
	// Update Frame Timing Information
	double frameTime = getSeconds();
	s_FrameTimeDelta = frameTime - s_FrameTime;
	s_FrameTime = frameTime;

	//s_FPSSmoothTotal -= s_FPSSmoothValues[s_FPSSmoothAt];
	s_FPSSmoothValues[s_FPSSmoothAt] = frameTime; //getFPS();
	//s_FPSSmoothTotal += s_FPSSmoothValues[s_FPSSmoothAt];
	if (s_FPSSmoothCount > 0)
		s_FPSSmooth = (double)(s_FPSSmoothCount - 1) / (s_FPSSmoothValues[s_FPSSmoothAt] - s_FPSSmoothValues[(s_FPSSmoothAt + 1) % s_FPSSmoothCount]);
	++s_FPSSmoothAt;
	if (s_FPSSmoothCount < s_FPSSmoothAt)
		s_FPSSmoothCount = s_FPSSmoothAt;
	s_FPSSmoothAt %= D_FT8XXEMU_FPS_SMOOTHING;
	//s_FPSSmooth = //s_FPSSmoothTotal / (double)s_FPSSmoothCount;
}

void SystemClass::end()
{
	delete s_RenderSleepWake;
	s_RenderSleepWake = NULL;

	// ...

	SystemClass::_end();
}

uint16_t SystemClass::getAnalogRead(uint8_t pin)
{
	return s_AnalogPins[pin];
}

void SystemClass::setAnalogRead(uint8_t pin, uint16_t value)
{
	s_AnalogPins[pin] = value;
}

void SystemClass::overrideMCUDelay(void (*delay)(int))
{
	s_MCUDelay = delay;
}

void SystemClass::delayForMCU(int ms)
{
	if (s_MCUDelay) s_MCUDelay(ms);
	else delay(ms);
}

void SystemClass::renderSleep(int ms)
{
	s_RenderSleepWake->sleep(ms);
}

void SystemClass::renderWake()
{
	s_RenderWoke = true;
	s_RenderSleepWake->wake();
}

bool SystemClass::renderWoke()
{
	bool res = s_RenderWoke;
	s_RenderWoke = false;
	return res;
}

} /* namespace FT8XXEMU */

/* end of file */
