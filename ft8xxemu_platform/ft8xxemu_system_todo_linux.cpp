/**
 * SystemLinuxClass
 * $Id$
 * \file ft8xxemu_system_linux.cpp
 * \brief SystemLinuxClass
 * \date 2012-06-29 14:50GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef WIN32

// #include <...>
#include "ft8xxemu_system_linux.h"
#include "ft8xxemu_system.h"

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

namespace FT8XXEMU {

System System;
SystemLinuxClass SystemLinux;

static pthread_t s_MCUThread = 0;
static pthread_t s_CoprocessorThread = 0;
static pthread_t s_MainThread = 0;

static sigset_t s_SIGUSR1SigSet;
static pthread_mutex_t s_MCUSuspendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_CoprocessorSuspendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_SwapMutex = PTHREAD_MUTEX_INITIALIZER;

static time_t s_BeginTime = 0;

void System::_begin()
{
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	SDL_Init(0);
#endif

	sigemptyset(&s_SIGUSR1SigSet);
	sigaddset(&s_SIGUSR1SigSet, SIGUSR1);

	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	s_BeginTime = t.tv_sec;
}

void System::_update()
{

}

void System::_end()
{
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
	SDL_Quit();
#endif
}

unsigned int System::getCPUCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void System::disableAutomaticPriorityBoost()
{
	//SetThreadPriorityBoost(GetCurrentThread(), TRUE);
}
void System::makeLowPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
}
void System::makeNormalPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
}

void System::makeHighPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
}

void System::makeHighestPriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void System::makeRealtimePriorityThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void System::makeMainThread()
{
	s_MainThread = pthread_self();
	sigprocmask(SIG_BLOCK, &s_SIGUSR1SigSet, NULL);
}

bool System::isMainThread()
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
		fprintf(stderr, "Bad mcu suspend signal");
	}
}

// MCU thread control
void System::makeMCUThread()
{
	s_MCUThread = pthread_self();
	signal(SIGUSR1, suspendMCUWait);
}

int System::isMCUThread()
{
	return (pthread_self() == s_MCUThread) ? 1 : 0;
}

void System::prioritizeMCUThread()
{
	//if (s_MCUThread != NULL)
	//	SetThreadPriority(s_MCUThread, THREAD_PRIORITY_HIGHEST);
}

void System::unprioritizeMCUThread()
{
	//if (s_MCUThread != NULL)
	//	SetThreadPriority(s_MCUThread, THREAD_PRIORITY_NORMAL);
}

void System::holdMCUThread()
{
	if (!pthread_mutex_trylock(&s_MCUSuspendMutex))
	{
		if (pthread_kill(s_MCUThread, SIGUSR1))
			fprintf(stderr, "Send user signal suspend fail");
	}
	else
	{
		printf("Trylock mcu fail");
	}
}

void System::resumeMCUThread()
{
	pthread_mutex_unlock(&s_MCUSuspendMutex);
}

void System::killMCUThread()
{
	void *retval;
	pthread_cancel(s_MCUThread);
	pthread_join(s_MCUThread, &retval);
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
		fprintf(stderr, "Bad Coprocessor suspend signal");
	}
}

// Coprocessor thread control
void System::makeCoprocessorThread()
{
	s_CoprocessorThread = pthread_self();
	signal(SIGUSR1, suspendCoprocessorWait);
}

void System::forgetCoprocessorThread()
{
	printf("Forget coprocessor thread\n");
	pthread_mutex_lock(&s_CoprocessorSuspendMutex);
	s_CoprocessorThread = 0;
	pthread_mutex_unlock(&s_CoprocessorSuspendMutex);
	printf("Coprocessor thread forgotten ok\n");
}

bool System::isCoprocessorThread()
{
	return pthread_self() == s_CoprocessorThread;
}

void System::prioritizeCoprocessorThread()
{
	//if (s_CoprocessorThread != NULL)
	//	SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_HIGHEST);
}

void System::unprioritizeCoprocessorThread()
{
	//if (s_CoprocessorThread != NULL)
	//	SetThreadPriority(s_CoprocessorThread, THREAD_PRIORITY_NORMAL);
}

void System::holdCoprocessorThread()
{
	if (s_CoprocessorThread)
	{
		if (!pthread_mutex_trylock(&s_CoprocessorSuspendMutex))
		{
			if (pthread_kill(s_CoprocessorThread, SIGUSR1))
				fprintf(stderr, "Send user signal suspend fail");
		}
		else
		{
			printf("Trylock Coprocessor fail");
		}
	}
}

void System::resumeCoprocessorThread()
{
	if (s_CoprocessorThread)
	{
		pthread_mutex_unlock(&s_CoprocessorSuspendMutex);
	}
}

void System::enterSwapDL()
{
	pthread_mutex_lock(&s_SwapMutex);
}

void System::leaveSwapDL()
{
	pthread_mutex_unlock(&s_SwapMutex);
}

void *System::setThreadGamesCategory(unsigned long *refId)
{
	//HANDLE h = AvSetMmThreadCharacteristics(TEXT("Games"), refId);
	//if (!h) SystemLinux.ErrorWin32();
	//return h;
	return NULL;
}

void System::revertThreadCategory(void *taskHandle)
{
	//AvRevertMmThreadCharacteristics(taskHandle);
}

void System::switchThread()
{
	sched_yield();
}

double System::getSeconds()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)(t.tv_sec - s_BeginTime) + (double)t.tv_nsec * 0.000000001;
}

long System::getMillis()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_nsec / 1000000 + (t.tv_sec - s_BeginTime) * 1000;
}

long System::getMicros()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_nsec / 1000) + (t.tv_sec - s_BeginTime) * 1000000;
}

long System::getFreqTick(int hz)
{
	return getMicros() * (long)hz / 1000000; // FIXME - Higher Resolution
}

void System::delay(int ms)
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

void System::delayMicros(int us)
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

} /* namespace FT8XXEMU */

#endif /* #ifndef WIN32 */

/* end of file */
