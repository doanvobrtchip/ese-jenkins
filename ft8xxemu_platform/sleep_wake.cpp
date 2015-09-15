/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "sleep_wake.h"

SleepWake::SleepWake() : m_WantWake(false), m_IsSleeping(false)
{
#ifdef FTEMU_SDL2
	m_SleepCond = SDL_CreateCond();
	m_SleepLock = SDL_CreateMutex();
#endif
}

SleepWake::~SleepWake()
{
	wakeInternal();
#ifdef FTEMU_SDL2
	SDL_DestroyCond(m_SleepCond);
	SDL_DestroyMutex(m_SleepLock);
#endif
}

void SleepWake::sleep(uint32_t ms)
{
	m_IsSleeping = true;
#ifdef FTEMU_SDL2
	SDL_LockMutex(m_SleepLock);
	if (m_WantWake)
	{
		m_IsSleeping = false;
		m_WantWake = false;
		SDL_UnlockMutex(m_SleepLock);
		return;
	}
	SDL_CondWaitTimeout(m_SleepCond, m_SleepLock, ms);
	SDL_UnlockMutex(m_SleepLock);
#else
	std::unique_lock<std::mutex> lock(m_SleepLock);
	if (m_WantWake)
	{
		m_IsSleeping = false;
		m_WantWake = false;
		lock.unlock();
		return;
	}
	m_SleepCond.wait_for(lock, std::chrono::milliseconds(ms));
	lock.unlock();
#endif
	m_IsSleeping = false;
	m_WantWake = false;
}

void SleepWake::wakeInternal()
{
#ifdef FTEMU_SDL2
	SDL_LockMutex(m_SleepLock);
	SDL_CondSignal(m_SleepCond);
	SDL_UnlockMutex(m_SleepLock);
#else
	m_SleepCond.notify_one();
#endif
}

/* end of file */
