/**
 * SystemClass
 * $Id$
 * \file ft800emu_system.cpp
 * \brief SystemClass
 * \date 2011-05-25 19:32GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_system.h"

// System includes

// Project includes

// using namespace ...;

namespace FT800EMU {


#define D_FT800EMU_FPS_SMOOTHING 36


double SystemClass::s_FrameTime, SystemClass::s_FrameTimeDelta; //, SystemClass::s_FrameTimeOffset;
int SystemClass::s_FrameCount;
double SystemClass::s_FPSSmooth;
bool SystemClass::s_MainThreadSwitchable = false;


static double s_FPSSmoothValues[D_FT800EMU_FPS_SMOOTHING];
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
	for (int i = 0; i < D_FT800EMU_FPS_SMOOTHING; ++i)
		s_FPSSmoothValues[i] = 0.0;
	//s_FPSSmoothTotal = 0.0;
	s_FPSSmoothCount = 0;
	//s_FrameTimeOffset = SystemClass::getSeconds() * -1.0;
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
	s_FPSSmoothAt %= D_FT800EMU_FPS_SMOOTHING;
	//s_FPSSmooth = //s_FPSSmoothTotal / (double)s_FPSSmoothCount;
}

void SystemClass::end()
{
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

} /* namespace FT800EMU */

/* end of file */
