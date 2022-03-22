/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef BT8XXEMU_THREAD_STATE_H
#define BT8XXEMU_THREAD_STATE_H
// #include <...>

// System includes
#include "ft8xxemu_system.h"

#ifndef WIN32
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

// Project includes

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

	//! Set thread name
	void setName(const char *name);

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

	// Is this state valid
	bool valid();

private:
#ifdef WIN32
	HANDLE m_Handle = NULL;
	HANDLE m_AvHandle = NULL;
	DWORD m_AvTask = 0;
	DWORD m_Id = 0;
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
