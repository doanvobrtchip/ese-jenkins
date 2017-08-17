/**
 * ThreadState
 * \file ft8xxemu_thread_state.cpp
 * \brief ThreadState
 * \date 2017-08-04 10:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifndef WIN32

// #include <...>
#include "ft8xxemu_thread_state.h"

// System includes
#include "ft8xxemu_system_win32.h"

// Project includes

// using namespace ...;

namespace /* anonymous */ {

void signalYield(int signum)
{
	pthread_yield();
}

void signalBoost(int signum)
{

}

}

namespace FT8XXEMU {

bool ThreadState::init()
{
	reset();
	m_PThread = pthread_self();
	signal(SIGUSR1, signalYield);
	signal(SIGUSR2, signalBoost);
	return true;
}

bool ThreadState::foreground()
{
	return false;
}

bool ThreadState::noboost()
{
	return false;
}

void ThreadState::reset()
{
	m_PThread = 0;
}

bool ThreadState::realtime()
{
	return false;
}

bool ThreadState::prioritize()
{
	return false;
}

bool ThreadState::unprioritize()
{
	return false;
}

bool ThreadState::boost()
{
	return !pthread_kill(m_PThread, SIGUSR2);
}

bool ThreadState::yield()
{
	return !pthread_kill(m_PThread, SIGUSR1);
}

bool ThreadState::kill()
{
	void *retval;
	pthread_cancel(m_PThread);
	pthread_join(m_PThread, &retval);
	return true;
}

bool ThreadState::current()
{
	return pthread_self() == s_MCUThread;
}

} /* namespace FT8XXEMU */

#endif /* #ifndef WIN32 */

/* end of file */
