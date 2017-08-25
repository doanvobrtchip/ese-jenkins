/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

// #include <...>
#include "ft8xxemu_system.h"
#include "ft8xxemu_system_win32.h"

// System includes
#include <mutex>

// Project includes
#include "ft8xxemu_sleep_wake.h"

// using namespace ...;

namespace FT8XXEMU {

namespace /* anonymous */ {

std::mutex s_LogMutex;

} /* anonymous namespace */

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

void System::log(BT8XXEMU_LogType type, const char *message, ...)
{
	char buffer[2048];
	va_list args;
	va_start(args, message);
	vsnprintf(buffer, 2047, message, args);
	; {
		std::unique_lock<std::mutex> lock(s_LogMutex);
		m_Log && (m_Log(m_Sender, m_UserContext, type, buffer), true);
		const char *level =
			(type == BT8XXEMU_LogMessage
				? "Message"
				: (type == BT8XXEMU_LogWarning
					? "Warning" : "Error"));
		m_PrintStd && (printf("[%s] %s\n", level, buffer));
	}
	va_end(args);
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
