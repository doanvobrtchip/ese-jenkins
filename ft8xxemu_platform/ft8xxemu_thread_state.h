/**
 * ThreadState
 * \file ft8xxemu_thread_state.h
 * \brief ThreadState
 * \date 2017-08-04 10:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifndef BT8XXEMU_THREAD_STATE_H
#define BT8XXEMU_THREAD_STATE_H
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
 * ThreadState
 * \brief ThreadState
 * \date 2017-08-04 19:38GMT
 * \author Jan Boon (Kaetemi)
 */
class ThreadState
{
public:
	ThreadState();
	~ThreadState();

	//! Initialize for the current thread
	bool init();
	void reset();

	//! Optimize the thread for foreground applications
	//! Should only be called when the process containing the emulator is managing the window
	bool foreground();
	bool noboost();

	// Set the priority for this thread
	bool realtime();
	bool prioritize();
	bool unprioritize();

	// Control this thread
	bool boost(); //< Bring the thread to the top of the scheduler queue temporarily
	bool yield(); //< Cause the thread to switch away and go to the end of the scheduler queue
	bool kill();

	// Is this the current thread
	bool current();

private:
#ifdef WIN32
	HANDLE m_Handle = NULL;
	DWORD m_Id = 0;
	HANDLE m_AvHandle = NULL;
	DWORD m_AvTask = 0;
#else
	pthread_t m_PThread = 0;
#endif
	
private:
	ThreadState(const ThreadState &) = delete;
	ThreadState &operator=(const ThreadState &) = delete;

}; /* class ThreadState */

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_THREAD_STATE_H */

/* end of file */
