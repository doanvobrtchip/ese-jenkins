/**
 * SystemWindowsClass
 * $Id$
 * \file ft8xxemu_system_windows.cpp
 * \brief SystemWindowsClass
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef WIN32

// #include <...>
#include "ft8xxemu_system_windows.h"
#include "ft8xxemu_system.h"

// Libraries
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

// System includes

// Project includes

namespace FT8XXEMU {


SystemClass System;
SystemWindowsClass SystemWindows;


static LARGE_INTEGER s_PerformanceFrequency = { 0 };
static LARGE_INTEGER s_PerformanceCounterBegin = { 0 };


static HANDLE s_CoprocessorThread = NULL;
static HANDLE s_MCUThread = NULL;
static HANDLE s_MainThread = NULL;

static HANDLE s_CoprocessorThreadPseudo = NULL;
static HANDLE s_MCUThreadPseudo = NULL;
static HANDLE s_MainThreadPseudo = NULL;

//static CRITICAL_SECTION s_CriticalSection;
static CRITICAL_SECTION s_SwapCriticalSection;

void SystemClass::_begin()
{
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	SDL_Init(0);
#endif
	QueryPerformanceFrequency(&s_PerformanceFrequency);
	QueryPerformanceCounter(&s_PerformanceCounterBegin);
	//InitializeCriticalSection(&s_CriticalSection);
	InitializeCriticalSection(&s_SwapCriticalSection);
}

void SystemClass::_update()
{

}

void SystemClass::_end()
{
	//DeleteCriticalSection(&s_CriticalSection);
	DeleteCriticalSection(&s_SwapCriticalSection);
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	SDL_Quit();
#endif
}

unsigned int SystemClass::getCPUCount()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

/*
void SystemClass::enterCriticalSection()
{
	//EnterCriticalSection(&s_CriticalSection);
}

void SystemClass::leaveCriticalSection()
{
	//LeaveCriticalSection(&s_CriticalSection);
}
*/

void SystemClass::enterSwapDL()
{
	EnterCriticalSection(&s_SwapCriticalSection);
}

void SystemClass::leaveSwapDL()
{
	LeaveCriticalSection(&s_SwapCriticalSection);
}

void SystemClass::switchThread()
{
	SwitchToThread();
}

double SystemClass::getSeconds()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (double)(counter.QuadPart - s_PerformanceCounterBegin.QuadPart) / (double)s_PerformanceFrequency.QuadPart;
}

long SystemClass::getMillis()
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	c.QuadPart -= s_PerformanceCounterBegin.QuadPart;
	c.QuadPart *= (LONGLONG)1000;
	c.QuadPart /= s_PerformanceFrequency.QuadPart;
	return (long)c.QuadPart;
}

long SystemClass::getMicros()
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	c.QuadPart -= s_PerformanceCounterBegin.QuadPart;
	c.QuadPart *= (LONGLONG)1000000;
	c.QuadPart /= s_PerformanceFrequency.QuadPart;
	return (long)c.QuadPart;
}

long SystemClass::getFreqTick(int hz)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	c.QuadPart -= s_PerformanceCounterBegin.QuadPart;
	c.QuadPart *= (LONGLONG)hz;
	c.QuadPart /= s_PerformanceFrequency.QuadPart;
	return (long)c.QuadPart;
}

void SystemClass::delay(int ms)
{
	Sleep(ms);
}

void SystemClass::delayMicros(int us)
{
	long endMicros = getMicros() + (long)us;;
	do
	{
		switchThread();
	} while (getMicros() < endMicros);
	//Sleep(us / 1000);
}



tstring SystemWindowsClass::GetWin32ErrorString(DWORD dwError)
{
	// convert win32 error number to string

	LPTSTR lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

	tstring result = tstring(lpMsgBuf);

    LocalFree(lpMsgBuf);

	return result;
}

tstring SystemWindowsClass::GetWin32LastErrorString()
{
	// put the last win32 error in a string and add the error number too
	DWORD dwError = GetLastError();
	tstringstream buffer;
	buffer << GetWin32ErrorString(dwError)
		<< TEXT(" (error: ") << dwError << TEXT(")");
	return buffer.str();
}

// http://stackoverflow.com/questions/22233527/how-to-convert-hresult-into-an-error-description
DWORD Win32FromHResult(HRESULT hr)
{
	if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
	{
		return HRESULT_CODE(hr);
	}

	if (hr == S_OK)
	{
		return ERROR_SUCCESS;
	}

	// Not a Win32 HRESULT so return a generic error code.
	return ERROR_CAN_NOT_COMPLETE;
}

void SystemWindowsClass::Error(const tstring &message)
{
	// exit with message
	if (::FT8XXEMU::g_PrintStd)
	{
		MessageBox(NULL, (LPCTSTR)message.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
		tcout << TEXT("Error: ") << message << std::endl;
	}
	exit(EXIT_FAILURE);
}

void SystemWindowsClass::Warning(const tstring &message)
{
	// show a warning box and send to output
	if (::FT8XXEMU::g_PrintStd)
	{
		MessageBox(NULL, (LPCTSTR)message.c_str(), TEXT("Warning"), MB_OK | MB_ICONWARNING);
		tcout << TEXT("Warning: ") << message << std::endl;
	}
}

void SystemWindowsClass::Debug(const tstring &message)
{
	// send a debug to output
	if (::FT8XXEMU::g_PrintStd)
	{
		tcout << TEXT("Debug: ") << message << std::endl;
	}
}

void SystemWindowsClass::ErrorWin32(const tstring &message)
{
	// crash with last win32 error string
	tstringstream buffer;
	buffer << message << TEXT("\n") << GetWin32LastErrorString();
	Error(buffer.str());
}

void SystemWindowsClass::ErrorHResult(const tstring &message, HRESULT hr)
{
	tstringstream buffer;
	buffer << message << TEXT("\n") << GetWin32ErrorString(Win32FromHResult(hr));
	Error(buffer.str());
}

void SystemWindowsClass::WarningHResult(const tstring &message, HRESULT hr)
{
	tstringstream buffer;
	buffer << message << TEXT("\n") << GetWin32ErrorString(Win32FromHResult(hr));
	Warning(buffer.str());
}


} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */

/* end of file */
