/**
 * EmulatorClass
 * $Id$
 * \file ft800emu_emulator.cpp
 * \brief EmulatorClass
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_emulator.h"

// System includes
#include <istream>
#include <ostream>
#include <iostream>
#ifdef FT800EMU_SDL
#include <SDL.h>
#include <SDL_thread.h>
#endif

// Project includes
#include "WProgram.h"
#include "ft800emu_system.h"
#include "ft800emu_keyboard.h"
#include "ft800emu_graphics_driver.h"
#include "ft800emu_audio_driver.h"

// #include "vc.h"

#include "ft800emu_spi.h"
#include "ft800emu_memory.h"

#include "omp.h"

// using namespace ...;

namespace FT800EMU {

EmulatorClass Emulator;

namespace {
	void (*s_Setup)() = NULL;
	void (*s_Loop)() = NULL;
	int s_Flags = 0;
	bool s_MasterRunning = false;

	uint64_t s_Clock = 48 * 000 * 000; // 48 MHz

	int masterThread(void * = NULL)
	{
		System.makeMainThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = System.setThreadGamesCategory(&taskId);
		System.disableAutomaticPriorityBoost();
		System.makeRealtimePriorityThread();

		double targetSeconds = System.getSeconds();

		// ... TODO ...

		System.revertThreadCategory(taskHandle);
		return 0;
	}

	int mcuThread(void * = NULL)
	{
		System.makeMCUThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = System.setThreadGamesCategory(&taskId);
		System.disableAutomaticPriorityBoost();
		System.makeNormalPriorityThread();
		s_Setup();
		while (s_MasterRunning)
		{
			//printf("mcu thread\n");
			s_Loop();
		}
		System.revertThreadCategory(taskHandle);
		return 0;
	}

	int audioThread(void * = NULL)
	{
		//printf("go sound thread");
		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = System.setThreadGamesCategory(&taskId);
		System.disableAutomaticPriorityBoost();
		System.makeHighPriorityThread();
		while (s_MasterRunning)
		{
			//printf("sound thread\n");
			// TODO_AUDIO if (s_Flags & EmulatorEnableAudio) AudioMachine.process();
			if (s_Flags & EmulatorEnableKeyboard) Keyboard.update();
			System.delay(10);
		}
		System.revertThreadCategory(taskHandle);
		return 0;
	}
}

void EmulatorClass::run(void (*setup)(), void (*loop)(), int flags)
{
	s_Setup = setup;
	s_Loop = loop;
	s_Flags = flags;

	System.begin();
	Memory.begin();
	SPI.begin();
	GraphicsDriver.begin();
	// TODO_AUDIO if (flags & EmulatorEnableAudio) AudioDriver.begin();
	if (flags & EmulatorEnableKeyboard) Keyboard.begin();

	s_MasterRunning = true;

#ifdef FT800EMU_SDL

	SDL_Thread *threadD = SDL_CreateThread(mcuThread, NULL);
	// TODO - Error handling

	SDL_Thread *threadA = SDL_CreateThread(audioThread, NULL);
	// TODO - Error handling

	masterThread();

	s_MasterRunning = false;
	SDL_WaitThread(threadD, NULL);
	SDL_WaitThread(threadA, NULL);

#else
	#pragma omp parallel num_threads(3)
	{
		// graphics
		#pragma omp master
		{
			masterThread();
			s_MasterRunning = false;
		}

		// arduino
		if (omp_get_thread_num() == 1)
		{
			mcuThread();
		}

		// sound
		if (omp_get_thread_num() == 2)
		{
			audioThread();
		}
	}
#endif /* #ifdef FT800EMU_SDL */

	if (flags & EmulatorEnableKeyboard) Keyboard.end();
	if (flags & EmulatorEnableAudio) AudioDriver.end();
	GraphicsDriver.end();
	SPI.end();
	Memory.end();
	System.end();
}

} /* namespace FT800EMU */

/* end of file */
