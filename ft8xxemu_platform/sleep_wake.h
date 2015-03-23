/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <SDL_mutex.h>

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
	SDL_mutex *m_SleepLock;
	SDL_cond *m_SleepCond;
};

/* end of file */
