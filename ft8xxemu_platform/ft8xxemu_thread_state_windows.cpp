/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifdef WIN32

// #include <...>
#include "ft8xxemu_thread_state.h"

// System includes
#include "ft8xxemu_system_win32.h"
#include <avrt.h>
#include <mutex>

// Project includes

// using namespace ...;

namespace FT8XXEMU {

extern std::mutex g_LogMutex;

bool ThreadState::init()
{
	reset();
	if (DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&m_Handle,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS))
	{
		m_Id = GetCurrentThreadId();
		return true;
	}
	reset();
	return false;
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR szName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  
void SetThreadName(DWORD dwThreadID, const char* threadName) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)  
}

void ThreadState::setName(const char *name)
{
	SetThreadName(m_Id, name);
}

bool ThreadState::foreground()
{
	return !m_AvHandle && m_Id && current() && (m_AvHandle = AvSetMmThreadCharacteristicsA("Games", &m_AvTask));
}

bool ThreadState::noboost()
{
	return m_Handle && SetThreadPriorityBoost(m_Handle, TRUE);
}

void ThreadState::reset()
{
	if (m_AvHandle)
	{
		if (m_Id && current())
			AvRevertMmThreadCharacteristics(m_AvHandle);
		m_AvHandle = NULL;
	}
	m_AvTask = 0;
	if (m_Handle)
	{
		SetThreadPriority(m_Handle, THREAD_PRIORITY_NORMAL);
		SetThreadPriorityBoost(m_Handle, FALSE);
		CloseHandle(m_Handle);
		m_Handle = NULL;
	}
	m_Id = 0;
}

bool ThreadState::realtime()
{
	return m_Handle && SetThreadPriority(m_Handle, THREAD_PRIORITY_TIME_CRITICAL);
}

bool ThreadState::prioritize()
{
	return m_Handle && SetThreadPriority(m_Handle, THREAD_PRIORITY_HIGHEST);
}

bool ThreadState::unprioritize()
{
	return m_Handle && SetThreadPriority(m_Handle, THREAD_PRIORITY_NORMAL);
}

bool ThreadState::boost()
{
	return false;
}

bool ThreadState::yield()
{
	return m_Handle && (SuspendThread(m_Handle) >= 0) && (ResumeThread(m_Handle) > 0); 
}

bool ThreadState::kill()
{
	std::unique_lock<std::mutex> lock(g_LogMutex);
	return m_Handle && TerminateThread(m_Handle, 0);
	// return m_Handle && (TerminateThread(m_Handle), true);
}

bool ThreadState::current()
{
	return GetCurrentThreadId() == m_Id;
}

bool ThreadState::valid()
{
	return m_Id != 0;
}

} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */

/* end of file */
