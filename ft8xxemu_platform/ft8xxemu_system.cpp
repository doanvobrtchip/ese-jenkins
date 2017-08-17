/**
 * System
 * $Id$
 * \file ft8xxemu_system.cpp
 * \brief System
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
#include "ft8xxemu_sleep_wake.h"

// using namespace ...;

namespace FT8XXEMU {

System::System()
{
#ifdef WIN32
	m_Win32 = new SystemWin32(this);
	initWindows();
#endif

	// Initialize Frame Count
	m_FrameCount = 0;

	// Initialize Frame Timing Information
	m_FrameTime = 0.0;
	m_FrameTimeDelta = 0.0;
	m_FPSSmooth = 0.0;
	m_FPSSmoothAt = 0;
	for (int i = 0; i < BT8XXEMU_FPS_SMOOTHING; ++i)
		m_FPSSmoothValues[i] = 0.0;
	//m_FPSSmoothTotal = 0.0;
	m_FPSSmoothCount = 0;
	//m_FrameTimeOffset = System::getSeconds() * -1.0;

	m_RenderSleepWake = std::make_unique<SleepWake>();
}

void System::update()
{
	// Update Frame Count
	++m_FrameCount;
	
	// Update Frame Timing Information
	double frameTime = getSeconds();
	m_FrameTimeDelta = frameTime - m_FrameTime;
	m_FrameTime = frameTime;

	//m_FPSSmoothTotal -= m_FPSSmoothValues[m_FPSSmoothAt];
	m_FPSSmoothValues[m_FPSSmoothAt] = frameTime; //getFPS();
	//m_FPSSmoothTotal += m_FPSSmoothValues[m_FPSSmoothAt];
	if (m_FPSSmoothCount > 0)
		m_FPSSmooth = (double)(m_FPSSmoothCount - 1) / (m_FPSSmoothValues[m_FPSSmoothAt] - m_FPSSmoothValues[(m_FPSSmoothAt + 1) % m_FPSSmoothCount]);
	++m_FPSSmoothAt;
	if (m_FPSSmoothCount < m_FPSSmoothAt)
		m_FPSSmoothCount = m_FPSSmoothAt;
	m_FPSSmoothAt %= BT8XXEMU_FPS_SMOOTHING;
	//m_FPSSmooth = //m_FPSSmoothTotal / (double)m_FPSSmoothCount;
}

System::~System()
{
#ifdef WIN32
	releaseWindows();
	delete m_Win32;
#endif
}

void System::delayForMCU(int ms)
{
	if (m_MCUDelay) m_MCUDelay(m_Sender, m_UserContext, ms);
	else delay(ms);
}

void System::renderSleep(int ms)
{
	m_RenderSleepWake->sleep(ms);
}

void System::renderWake()
{
	m_RenderWoke = true;
	m_RenderSleepWake->wake();
}

bool System::renderWoke()
{
	bool res = m_RenderWoke;
	m_RenderWoke = false;
	return res;
}

} /* namespace FT8XXEMU */

/* end of file */
