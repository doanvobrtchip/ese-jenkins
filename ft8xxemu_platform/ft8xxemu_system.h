/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef BT8XXEMU_SYSTEM_H
#define BT8XXEMU_SYSTEM_H
// #include <...>

// System includes
#include <memory>

// Project includes
#include "bt8xxemu.h"

#ifdef WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	if !defined(NTDDI_VERSION) && !defined(_WIN32_WINNT) && !defined(WINVER)
#		define NTDDI_VERSION 0x05010000 /* NTDDI_WINXP */
#		define _WIN32_WINNT 0x0501 /* _WIN32_WINNT_WINXP */
#		define WINVER 0x0501 /* _WIN32_WINNT_WINXP */
#	endif
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#endif

namespace FT8XXEMU {
	class SleepWake;

#ifdef WIN32
	class SystemWin32;
#endif

#define BT8XXEMU_FPS_SMOOTHING 36

/**
 * System
 * \brief System
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */
class System
{
public:
	System();
	~System();

	void update();

	// OS Specific
	double getSeconds();
	long getMillis();
	long getMicros();
	long getFreqTick(int hz);

	void log(BT8XXEMU_LogType type, const char *message, ...);

	void setSender(BT8XXEMU_Emulator *sender) { m_Sender = sender; }
	void setUserContext(void *context) { m_UserContext = context; }
	void onLog(void(*log)(BT8XXEMU_Emulator *sender, void *context, BT8XXEMU_LogType type, const char *message)) { m_Log = log; }
	void overrideMCUDelay(void(*delay)(BT8XXEMU_Emulator *sender, void *context, int ms)) { m_MCUDelay = delay; }

	void delayForMCU(int ms);
	static void delay(int ms);
	void delayMicros(int us);

	void renderSleep(int ms);
	void renderWake();
	bool renderWoke();

	static unsigned int getCPUCount();

	static void switchThread();

#ifdef WIN32
	inline SystemWin32 *win32() { return m_Win32; }
#endif

	// Debugging
	inline double getFrameTime() { return m_FrameTime; }
	inline double getFrameTimeDelta() { return m_FrameTimeDelta; }

	inline double getFPS() { return 1.0 / m_FrameTimeDelta; }
	inline double getFPSSmooth() { return m_FPSSmooth; }
	inline int getFrameCount() { return m_FrameCount; }

	inline void setPrintStd(bool printStd) { m_PrintStd = printStd; }
	inline bool getPrintStd() { return m_PrintStd; }

private:
#ifdef WIN32
	void initWindows();
	void releaseWindows();
#endif

private:
	double m_FrameTime, m_FrameTimeDelta;
	int m_FrameCount;
	double m_FPSSmooth;
	// bool m_MainThreadSwitchable = false;

	std::unique_ptr<SleepWake> m_RenderSleepWake;
	bool m_RenderWoke = false;

	bool m_PrintStd = false;

	double m_FPSSmoothValues[BT8XXEMU_FPS_SMOOTHING];
	//static double m_FPSSmoothTotal;
	int m_FPSSmoothAt;
	int m_FPSSmoothCount;

	void(*m_Log)(BT8XXEMU_Emulator *sender, void *context, BT8XXEMU_LogType type, const char *message) = NULL;
	void(*m_MCUDelay)(BT8XXEMU_Emulator *sender, void *context, int ms) = NULL;
	BT8XXEMU_Emulator *m_Sender = NULL;
	void *m_UserContext = NULL;

#ifdef WIN32
	SystemWin32 *m_Win32 = NULL;
	LARGE_INTEGER m_PerformanceFrequency = { 0 };
	LARGE_INTEGER m_PerformanceCounterBegin = { 0 };
#endif

private:
	System(const System &);
	System &operator=(const System &);

}; /* class System */

// #define FTEMU_printf(s, ...) do { if (m_System->getPrintStd()) { printf(s, __VA_ARGS__); } } while (false)
#define FTEMU_error(s, ...)  m_System->log(BT8XXEMU_LogError, s, __VA_ARGS__)  
#define FTEMU_warning(s, ...)   m_System->log(BT8XXEMU_LogWarning, s, __VA_ARGS__) 
#ifdef _DEBUG
	#define FTEMU_message(s, ...)    m_System->log(BT8XXEMU_LogMessage, s, __VA_ARGS__) 
#else
	#define FTEMU_message(s, ...)	{}
#endif

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_SYSTEM_H */

/* end of file */
