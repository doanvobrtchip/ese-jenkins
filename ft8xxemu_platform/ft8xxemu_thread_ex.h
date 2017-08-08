/**
 * ThreadEx
 * \file ft8xxemu_thread_ex.h
 * \brief ThreadEx
 * \date 2017-08-04 10:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011-2017  Future Technology Devices International Ltd
 */

#ifndef BT8XXEMU_THREAD_EX_H
#define BT8XXEMU_THREAD_EX_H
// #include <...>

// System includes
#ifdef WIN32
#include "ft8xxemu_system_windows.h"
#else
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

// Project includes
#include "ft8xxemu_system.h"

namespace FT8XXEMU {

/**
 * ThreadEx
 * \brief ThreadEx
 * \date 2017-08-04 19:38GMT
 * \author Jan Boon (Kaetemi)
 */
class ThreadEx
{
public:
	ThreadEx();
	~ThreadEx();

	// Initialize for the current thread
	bool init();

	// Optimize the thread for foreground applications
	bool foreground();

	// Set the priority for this thread
	bool prioritize();
	bool unprioritize();

	// Control this thread
	bool hold();
	bool resume();
	bool poke();
	bool kill();

	// Is this the current thread
	bool current();

private:
	void reset();

private:
#ifdef WIN32
	HANDLE m_Handle = NULL;
	DWORD m_Id = 0;
	HANDLE m_AvHandle = NULL;
	DWORD m_AvTask = 0;
#else
	// ...
#endif
	
private:
	ThreadEx(const ThreadEx &) = delete;
	ThreadEx &operator=(const ThreadEx &) = delete;

}; /* class ThreadEx */

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_THREAD_EX_H */

/* end of file */
