/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include "sleep_wake.h"

SleepWake::SleepWake() : m_WantWake(false), m_IsSleeping(false)
{
	m_SleepCond = SDL_CreateCond();
	m_SleepLock = SDL_CreateMutex();
}

SleepWake::~SleepWake()
{
	SDL_DestroyCond(m_SleepCond);
	SDL_DestroyMutex(m_SleepLock);
}

void SleepWake::sleep(uint32_t ms)
{
	m_IsSleeping = true;
	SDL_LockMutex(m_SleepLock);
	if (m_WantWake)
	{
		m_IsSleeping = false;
		m_WantWake = false;
		SDL_UnlockMutex(m_SleepLock);
		return;
	}
	SDL_CondWaitTimeout(m_SleepCond, m_SleepLock, ms);
	// SDL_Delay(ms);
	SDL_UnlockMutex(m_SleepLock);
	m_IsSleeping = false;
}

void SleepWake::wakeInternal()
{
	SDL_LockMutex(m_SleepLock);
	SDL_CondSignal(m_SleepCond);
	SDL_UnlockMutex(m_SleepLock);
}

/* end of file */
