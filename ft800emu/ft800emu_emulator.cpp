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

#include "ft800emu_spi_i2c.h"
#include "ft800emu_memory.h"

#include "vc.h"

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

		uint8_t *ram = Memory.getRam();

		for (;;)
		{
			double deltaSeconds = 0.1; // TODO

			//printf("main thread\n");
			System.makeRealtimePriorityThread();

			System.update();
			targetSeconds += deltaSeconds;

			// If coprocessor running on render thread, REG_CPURESET goes into effect at this point.
			// Must use the getCPUReset() function which buffers value switches.
			// ...(Memory.getCPUReset());
			
			// Render lines
			{
				// VBlank 0
				// System.delay(0);
				System.switchThread();
				
				unsigned long procStart = System.getMicros();
				// GraphicsProcessor.process();
				unsigned long procDelta = System.getMicros() - procStart;

				if (procDelta > 8000)
					printf("process: %i micros (%i ms)\r\n", (int)procDelta, (int)procDelta / 1000);
			}

			// Flip buffer and also give a slice of time to the mcu main thread
			{
				// VBlank 1
				Memory.rawWriteU32(ram, REG_FRAMES, Memory.rawReadU32(ram, REG_FRAMES) + 1); // Increase REG_FRAMES
				System.prioritizeMCUThread();

#ifndef WIN32
				System.holdMCUThread(); // vblank'd !
				System.resumeMCUThread();
#endif

				unsigned long flipStart = System.getMicros();
				GraphicsDriver.renderBuffer();
				if (!GraphicsDriver.update()) exit(0); // ...
				unsigned long flipDelta = System.getMicros() - flipStart;

				if (flipDelta > 8000)
					printf("flip: %i micros (%i ms)\r\n", (int)flipDelta, (int)flipDelta / 1000);

				//System.delay(2); // ensure slice of time to mcu thread at cost of cpu cycles
				System.switchThread();
				System.unprioritizeMCUThread();
			}

			//long currentMillis = millis();
			//long millisToWait = targetMillis - currentMillis;
			double currentSeconds = System.getSeconds();
			double secondsToWait = targetSeconds - currentSeconds;
			//if (millisToWait < -100) targetMillis = millis();
			if (secondsToWait < -0.25) // Skip freeze
			{
				//printf("skip freeze\n");
				targetSeconds = System.getSeconds();
			}

			//printf("millis to wait: %i", (int)millisToWait);

			if (secondsToWait > 0.0)
			{
				System.delay((int)(secondsToWait * 1000.0));
				// If coprocessor enabled and runs on the same thread as the rendering it would run here.
			}

#ifdef WIN32
			System.holdMCUThread(); // don't let the other thread hog cpu
			System.resumeMCUThread();
#endif
		}

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
	SPII2C.begin();
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
	SPII2C.end();
	Memory.end();
	System.end();
}

} /* namespace FT800EMU */

/* end of file */
