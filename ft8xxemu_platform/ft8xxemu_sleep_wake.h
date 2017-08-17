/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#ifdef FTEMU_SDL2
#include <SDL_mutex.h>
#else
#include <mutex>
#include <condition_variable>
#endif

namespace FT8XXEMU {

class SleepWake
{
public:
	SleepWake();
	~SleepWake();

	// Sleep. This can only be used by one thread at once
	void sleep(uint32_t ms);
	// Wake up the sleeping thread from any other thread
	inline void wake() { m_WantWake = true; if (m_IsSleeping) { wakeInternal(); } }

private:
	void wakeInternal();

private:
	volatile bool m_WantWake;
	volatile bool m_IsSleeping;

#ifdef FTEMU_SDL2
	SDL_mutex *m_SleepLock;
	SDL_cond *m_SleepCond;
#else
	std::mutex m_SleepLock;
	std::condition_variable m_SleepCond;
#endif

};

} /* namespace FT8XXEMU */

/* end of file */
