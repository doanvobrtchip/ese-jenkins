/**
 * SystemWindowsClass
 * $Id$
 * \file ft800emu_system_windows.cpp
 * \brief SystemWindowsClass
 * \date 2011-05-25 19:28GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef WIN32

// #include <...>
#include "ft800emu_system_windows.h"
#include "ft800emu_system.h"

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

using namespace std;

namespace FT800EMU {


SystemClass System;
SystemWindowsClass SystemWindows;


static LARGE_INTEGER s_PerformanceFrequency = { 0 };
static LARGE_INTEGER s_PerformanceCounterBegin = { 0 };


static HANDLE s_CoprocessorThread = NULL;
static HANDLE s_MCUThread = NULL;
static HANDLE s_MainThread = NULL;

//static CRITICAL_SECTION s_CriticalSection;
static CRITICAL_SECTION s_SwapCriticalSection;


void SystemClass::_begin()
{
#if (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2))
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
#if (defined(FT800EMU_SDL) || defined(FT800EMU_SDL2))
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

void SystemClass::disableAutomaticPriorityBoost()
{
	SetThreadPriorityBoost(GetCurrentThread(), TRUE);
}
void SystemClass::makeLowPriorityThread()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
}
void SystemClass::makeNormalPriorityThread()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
}

void SystemClass::makeHighPriorityThread()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
}

void SystemClass::makeHighestPriorityThread()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void SystemClass::makeRealtimePriorityThread()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void SystemClass::makeMainThread()
{
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&s_MainThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
}

bool SystemClass::isMainThread()
{
	HANDLE currentThread;
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&currentThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
	return currentThread == s_MainThread;
}

// MCU thread control
void SystemClass::makeMCUThread()
{
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&s_MCUThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
}

bool SystemClass::isMCUThread()
{
	HANDLE currentThread;
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&currentThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
	return currentThread == s_MCUThread;
}

void SystemClass::prioritizeMCUThread()
{
	if (s_MCUThread != NULL)
		SetThreadPriority(s_MCUThread, THREAD_PRIORITY_HIGHEST);
}

void SystemClass::unprioritizeMCUThread()
{
	if (s_MCUThread != NULL)
		SetThreadPriority(s_MCUThread, THREAD_PRIORITY_NORMAL);
}

void SystemClass::holdMCUThread()
{
	if (0 > SuspendThread(s_MCUThread))
		SystemWindows.Error(TEXT("SuspendThread  FAILED"));
}

void SystemClass::resumeMCUThread()
{
	if (0 > ResumeThread(s_MCUThread))
		SystemWindows.Error(TEXT("ResumeThread  FAILED"));
}

void SystemClass::killMCUThread()
{
	if (TerminateThread(s_MCUThread, 0) == FALSE)
		SystemWindows.Error(TEXT("ExitThread  FAILED"));
	printf("(1) mcu thread killed\n");
}


// Coprocessor thread control
void SystemClass::makeCoprocessorThread()
{
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&s_CoprocessorThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
}

bool SystemClass::isCoprocessorThread()
{
	HANDLE currentThread;
	if (!DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&currentThread,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
		SystemWindows.ErrorWin32();
	return currentThread == s_CoprocessorThread;
}

void SystemClass::prioritizeCoprocessorThread()
{
	if (s_CoprocessorThread != NULL)
		SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_HIGHEST);
}

void SystemClass::unprioritizeCoprocessorThread()
{
	if (s_CoprocessorThread != NULL)
		SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_NORMAL);
}

void SystemClass::holdCoprocessorThread()
{
	if (0 > SuspendThread(s_CoprocessorThread))
		SystemWindows.Error(TEXT("SuspendThread  FAILED"));
}

void SystemClass::resumeCoprocessorThread()
{
	if (0 > ResumeThread(s_CoprocessorThread))
		SystemWindows.Error(TEXT("ResumeThread  FAILED"));
}

void *SystemClass::setThreadGamesCategory(unsigned long *refId)
{
#ifndef __MINGW32__
	HANDLE h = AvSetMmThreadCharacteristics(TEXT("Games"), refId);
	if (!h) SystemWindows.ErrorWin32();
	return h;
#else
	return NULL;
#endif
}

void SystemClass::revertThreadCategory(void *taskHandle)
{
#ifndef __MINGW32__
	AvRevertMmThreadCharacteristics(taskHandle);
#endif
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
		<< TEXT(" (error: ") << dwError << ")";
	return buffer.str();
}

void SystemWindowsClass::Error(const tstring &message)
{
	// exit with message
	MessageBox(NULL, (LPCTSTR)message.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
	tcout << TEXT("Error: ") << message << endl;
	exit(EXIT_FAILURE);
}

void SystemWindowsClass::Warning(const tstring &message)
{
	// show a warning box and send to output
	MessageBox(NULL, (LPCTSTR)message.c_str(), TEXT("Warning"), MB_OK | MB_ICONWARNING);
	tcout << TEXT("Warning: ") << message << endl;
}

void SystemWindowsClass::Debug(const tstring &message)
{
	// send a debug to output
	tcout << TEXT("Debug: ") << message << endl;
}

void SystemWindowsClass::ErrorWin32()
{
	// crash with last win32 error string
	Error(GetWin32LastErrorString());
}

void SystemWindowsClass::ErrorHResult(HRESULT hr)
{
	Error(TEXT("ErrorHResult")); // fixme :p
}

#ifdef _UNICODE
tstring SystemWindowsClass::ToTString(const std::wstring &s) { return s; }
tstring SystemWindowsClass::ToTString(const std::string &s)
#else
tstring SystemWindowsClass::ToTString(const std::string &s) { return s; }
tstring SystemWindowsClass::ToTString(const std::wstring &s)
#endif
{
	tstring result(s.length(), 0);
	// copy from one to another
	std::copy(s.begin(), s.end(), result.begin());
	return result;
}

#ifdef _UNICODE
std::wstring SystemWindowsClass::ToWString(const tstring &s) { return s; }
std::string SystemWindowsClass::ToAString(const tstring &s)
#else
std::string SystemWindowsClass::ToAString(const tstring &s) { return s; }
std::wstring SystemWindowsClass::ToWString(const tstring &s)
#endif
{
#ifdef _UNICODE
	string
#else
	wstring
#endif
		result(s.length(), 0);
	// copy from one to another
	std::copy(s.begin(), s.end(), result.begin());
	return result;
}


} /* namespace FT800EMU */

#endif /* #ifdef WIN32 */

/* end of file */
