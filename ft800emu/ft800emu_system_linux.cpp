/**
 * SystemLinuxClass
 * $Id$
 * \file ft800emu_system_linux.cpp
 * \brief SystemLinuxClass
 * \date 2012-06-29 14:50GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef WIN32

// #include <...>
#include "ft800emu_system_linux.h"
#include "ft800emu_system.h"

// System includes
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
#include <mach/clock.h>
#include <mach/mach.h>

static void clock_gettime(int, timespec *ts)
{
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
}
#define CLOCK_MONOTONIC 0
#endif

// Project includes

using namespace std;

namespace FT800EMU {

SystemClass System;
SystemLinuxClass SystemLinux;

static pthread_t s_MCUThread = 0;
static pthread_t s_CoprocessorThread = 0;
static pthread_t s_MainThread = 0;

static sigset_t s_SIGUSR1SigSet;
static pthread_mutex_t s_MCUSuspendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_CoprocessorSuspendMutex = PTHREAD_MUTEX_INITIALIZER;

static time_t s_BeginTime = 0;

void SystemClass::_begin()
{
#ifdef FT800EMU_SDL
	SDL_Init(0);
#endif

	sigemptyset(&s_SIGUSR1SigSet);
	sigaddset(&s_SIGUSR1SigSet, SIGUSR1);
	
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	s_BeginTime = t.tv_sec;
}

void SystemClass::_update()
{

}

void SystemClass::_end()
{
#ifdef FT800EMU_SDL
	SDL_Quit();
#endif
}

unsigned int SystemClass::getCPUCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void SystemClass::disableAutomaticPriorityBoost()
{
	//SetThreadPriorityBoost(GetCurrentThread(), TRUE);
}
void SystemClass::makeLowPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
}
void SystemClass::makeNormalPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
}

void SystemClass::makeHighPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
}

void SystemClass::makeHighestPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void SystemClass::makeRealtimePriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void SystemClass::makeMainThread()
{
	s_MainThread = pthread_self();
	sigprocmask(SIG_BLOCK, &s_SIGUSR1SigSet, NULL);
}

bool SystemClass::isMainThread()
{
	return pthread_self() == s_MainThread;
}

static void suspendMCUWait(int signum)
{
	if (signum == SIGUSR1)
	{
		if (pthread_mutex_lock(&s_MCUSuspendMutex))
			printf("Cannot lock");
		pthread_mutex_unlock(&s_MCUSuspendMutex);
	}
	else
	{
		SystemLinux.Error("Bad mcu suspend signal");
	}
}

// MCU thread control
void SystemClass::makeMCUThread()
{
	s_MCUThread = pthread_self();
	signal(SIGUSR1, suspendMCUWait);
}

bool SystemClass::isMCUThread()
{
	return pthread_self() == s_MCUThread;
}

void SystemClass::prioritizeMCUThread()
{
	//if (s_MCUThread != NULL)
	//	SetThreadPriority(s_MCUThread, THREAD_PRIORITY_HIGHEST);
}

void SystemClass::unprioritizeMCUThread()
{
	//if (s_MCUThread != NULL)
	//	SetThreadPriority(s_MCUThread, THREAD_PRIORITY_NORMAL);
}

void SystemClass::holdMCUThread()
{
	if (!pthread_mutex_trylock(&s_MCUSuspendMutex))
	{
		if (pthread_kill(s_MCUThread, SIGUSR1))
			SystemLinux.Error("Send user signal suspend fail");
	}
	else
	{
		printf("Trylock mcu fail");
	}
}

void SystemClass::resumeMCUThread()
{
	pthread_mutex_unlock(&s_MCUSuspendMutex);
}


static void suspendCoprocessorWait(int signum)
{
	if (signum == SIGUSR1)
	{
		if (pthread_mutex_lock(&s_CoprocessorSuspendMutex))
			printf("Cannot lock");
		pthread_mutex_unlock(&s_CoprocessorSuspendMutex);
	}
	else
	{
		SystemLinux.Error("Bad Coprocessor suspend signal");
	}
}

// Coprocessor thread control
void SystemClass::makeCoprocessorThread()
{
	s_CoprocessorThread = pthread_self();
	signal(SIGUSR1, suspendCoprocessorWait);
}

bool SystemClass::isCoprocessorThread()
{
	return pthread_self() == s_CoprocessorThread;
}

void SystemClass::prioritizeCoprocessorThread()
{
	//if (s_CoprocessorThread != NULL)
	//	SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_HIGHEST);
}

void SystemClass::unprioritizeCoprocessorThread()
{
	//if (s_CoprocessorThread != NULL)
	//	SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_NORMAL);
}

void SystemClass::holdCoprocessorThread()
{
	if (s_CoprocessorThread)
	{
		if (!pthread_mutex_trylock(&s_CoprocessorSuspendMutex))
		{
			if (pthread_kill(s_CoprocessorThread, SIGUSR1))
				SystemLinux.Error("Send user signal suspend fail");
		}
		else
		{
			printf("Trylock Coprocessor fail");
		}
	}
}

void SystemClass::resumeCoprocessorThread()
{
	if (s_CoprocessorThread)
	{
		pthread_mutex_unlock(&s_CoprocessorSuspendMutex);
	}
}

void *SystemClass::setThreadGamesCategory(unsigned long *refId)
{
	//HANDLE h = AvSetMmThreadCharacteristics(TEXT("Games"), refId);
	//if (!h) SystemLinux.ErrorWin32();
	//return h;
	return NULL;
}

void SystemClass::revertThreadCategory(void *taskHandle)
{
	//AvRevertMmThreadCharacteristics(taskHandle);
}

void SystemClass::switchThread()
{
	sched_yield();
}

double SystemClass::getSeconds()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)(t.tv_sec - s_BeginTime) + (double)t.tv_nsec * 0.000000001;
}

long SystemClass::getMillis()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_nsec / 1000000 + (t.tv_sec - s_BeginTime) * 1000;
}

long SystemClass::getMicros()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_nsec / 1000) + (t.tv_sec - s_BeginTime) * 1000000;
}

long SystemClass::getFreqTick(int hz)
{
	return getMicros() * (long)hz / 1000000; // FIXME - Higher Resolution
}

void SystemClass::delay(int ms)
{
	long endMillis = getMillis() + (long)ms;
	int sleepMillis = endMillis - getMillis();
	do
	{
		if (sleepMillis >= 1000) sleep(sleepMillis / 1000);
		else usleep(sleepMillis * 1000);
		sleepMillis = endMillis - getMillis();
	} while (sleepMillis > 0);
}

void SystemClass::delayMicros(int us)
{
	long endMicros = getMicros() + (long)us;
	int sleepMicros = (long)us;
	do
	{
		if (sleepMicros >= 1000000) sleep(sleepMicros / 1000000);
		else usleep(sleepMicros);
		sleepMicros = endMicros - getMicros();
	} while (sleepMicros > 0);

	/*if (us >= 1000000) sleep(us / 1000000);
	else usleep(us);*/

	/*

	long endMicros = getMicros() + (long)us;;
	do
	{
		switchThread();
	} while (getMicros() < endMicros);

	*/
}

void SystemLinuxClass::Error(const char *message)
{
	printf("Error: %s", message);
	exit(1);
}

} /* namespace FT800EMU */

#endif /* #ifndef WIN32 */

/* end of file */
