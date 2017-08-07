/**
 * ThreadEx
 * \file ft8xxemu_thread_ex.cpp
 * \brief ThreadEx
 * \date 2017-08-04 10:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011-2017  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft8xxemu_thread_ex.h"

// System includes
#include "ft8xxemu_system_windows.h"

// Project includes

// using namespace ...;

namespace FT8XXEMU {

ThreadEx::ThreadEx()
{
	
}

ThreadEx::~ThreadEx()
{
	reset();
}

bool ThreadEx::init()
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

bool ThreadEx::foreground()
{
	return !m_AvHandle && (m_AvHandle = AvSetMmThreadCharacteristicsA("Games", &m_AvTask));
}

void ThreadEx::reset()
{
	if (m_AvHandle)
	{
		AvRevertMmThreadCharacteristics(m_AvHandle);
	}
	m_AvTask = 0;
	if (m_Handle)
	{
		CloseHandle(m_Handle);
		m_Handle = NULL;
	}
	m_Id = 0;
}


bool ThreadEx::prioritize()
{
	return m_Handle && SetThreadPriority(m_Handle, THREAD_PRIORITY_HIGHEST);
}

bool ThreadEx::unprioritize()
{
	return m_Handle && SetThreadPriority(m_Handle, THREAD_PRIORITY_NORMAL);
}

bool ThreadEx::hold()
{
	return m_Handle && (SuspendThread(m_Handle) > 0);
}

bool ThreadEx::resume()
{
	return m_Handle && (ResumeThread(m_Handle) > 0);
}

bool ThreadEx::poke()
{
	return m_Handle && (SuspendThread(m_Handle) > 0) && (ResumeThread(m_Handle) > 0); 
}

bool ThreadEx::kill()
{
	return m_Handle && TerminateThread(m_Handle, 0);
	// return m_Handle && (TerminateThread(m_Handle), true);
}

bool ThreadEx::current()
{
	return GetCurrentThreadId() == m_Id;
}

} /* namespace FT8XXEMU */

/* end of file */
