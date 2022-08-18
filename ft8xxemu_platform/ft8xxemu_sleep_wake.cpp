/*
BT8XX Emulator Library
Copyright (C) 2015-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "ft8xxemu_sleep_wake.h"

namespace FT8XXEMU {

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
	; {
		std::unique_lock<std::mutex> lock(m_SleepLock);
		if (m_WantWake)
		{
			m_IsSleeping = false;
			m_WantWake = false;
			// lock.unlock(); in destructor
			return;
		}
		m_SleepCond.wait_for(lock, std::chrono::milliseconds(ms));
		// lock.unlock(); in destructor
	}
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
	std::unique_lock<std::mutex> lock(m_SleepLock);
	m_SleepCond.notify_one();
#endif
}

} /* namespace FT8XXEMU */

/* end of file */
