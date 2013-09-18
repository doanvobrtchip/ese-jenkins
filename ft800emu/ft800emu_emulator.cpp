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
#include "ft800emu_system.h"
#include "ft800emu_keyboard.h"
#include "ft800emu_keyboard_keys.h"
#include "ft800emu_graphics_driver.h"
#include "ft800emu_audio_driver.h"

#include "ft800emu_spi_i2c.h"
#include "ft800emu_memory.h"
#include "ft800emu_graphics_processor.h"
#include "ft800emu_coprocessor.h"

#include "vc.h"

#ifndef FT800EMU_SDL
#	include "omp.h"
#endif

// using namespace ...;

#define FT800EMU_REG_PCLK_ZERO_REDUCE 1

namespace FT800EMU {

EmulatorClass Emulator;

namespace {
	void debugShortkeys()
	{
		{
			static bool setDebugMode = false;
			if (setDebugMode)
			{
				setDebugMode = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F3);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F3))
			{
				FT800EMU::GraphicsProcessor.setDebugMode((FT800EMU::GraphicsProcessor.getDebugMode() + 1) % FT800EMU_DEBUGMODE_COUNT);
				setDebugMode = true;
			}
		}

		{
			static bool incDebugMultiplier = false;
			if (incDebugMultiplier)
			{
				incDebugMultiplier = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADPLUS);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADPLUS))
			{
				if (FT800EMU::GraphicsProcessor.getDebugMode())
					FT800EMU::GraphicsProcessor.setDebugMultiplier(FT800EMU::GraphicsProcessor.getDebugMultiplier() + 1);
				incDebugMultiplier = true;
			}
		}

		{
			static bool decDebugMultiplier = false;
			if (decDebugMultiplier)
			{
				decDebugMultiplier = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADMINUS);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADMINUS))
			{
				if (FT800EMU::GraphicsProcessor.getDebugMode())
					FT800EMU::GraphicsProcessor.setDebugMultiplier(max(FT800EMU::GraphicsProcessor.getDebugMultiplier() - 1, 1));
				decDebugMultiplier = true;
			}
		}

		{
			static bool resetDebugMultiplier = false;
			if (resetDebugMultiplier)
			{
				resetDebugMultiplier = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADSLASH);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_NUMPADSLASH))
			{
				if (FT800EMU::GraphicsProcessor.getDebugMode())
					FT800EMU::GraphicsProcessor.setDebugMultiplier(1);
				resetDebugMultiplier = true;
			}
		}

		{
			static bool incDebugLimiter = false;
			if (incDebugLimiter)
			{
				incDebugLimiter = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F8);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F8))
			{
				FT800EMU::GraphicsProcessor.setDebugLimiter(FT800EMU::GraphicsProcessor.getDebugLimiter() + 1);
				incDebugLimiter = true;
			}
		}

		{
			static bool decDebugLimiter = false;
			if (decDebugLimiter)
			{
				decDebugLimiter = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F7);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F7))
			{
				FT800EMU::GraphicsProcessor.setDebugLimiter(max(FT800EMU::GraphicsProcessor.getDebugLimiter() - 1, 0));
				decDebugLimiter = true;
			}
		}

		{
			static bool resetDebugLimiter = false;
			if (resetDebugLimiter)
			{
				resetDebugLimiter = FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F6);
			}
			else if (FT800EMU::Keyboard.isKeyDown(FT800EMU_KEY_F6))
			{
				FT800EMU::GraphicsProcessor.setDebugLimiter(0);
				resetDebugLimiter = true;
			}
		}
	}

	void (*s_Setup)() = NULL;
	void (*s_Loop)() = NULL;
	void (*s_Keyboard)() = NULL;
	int s_Flags = 0;
	bool s_MasterRunning = false;
	bool s_DynamicDegrade = false;
	
	bool s_DegradeOn = false;
	int s_DegradeStage = 0;

	bool s_RotateEnabled = false;

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
			//printf("main thread\n");
			System.makeRealtimePriorityThread();

			uint32_t reg_pclk = Memory.rawReadU32(ram, REG_PCLK);
			double deltaSeconds;
			// Calculate the display frequency
			if (reg_pclk)
			{
				double frequency = (double)Memory.rawReadU32(ram, REG_FREQUENCY);
				frequency /= (double)reg_pclk;
				frequency /= (double)Memory.rawReadU32(ram, REG_VCYCLE);
				frequency /= (double)Memory.rawReadU32(ram, REG_HCYCLE);
				deltaSeconds = 1.0 / frequency;
			}
			else deltaSeconds = 1.0;

			System.update();
			targetSeconds += deltaSeconds;

			Memory.setTouchScreenXYFrameTime((long)(deltaSeconds * 1000 * 1000));

			// Update display resolution
			uint32_t reg_vsize = Memory.rawReadU32(ram, REG_VSIZE);
			uint32_t reg_hsize = Memory.rawReadU32(ram, REG_HSIZE);
			GraphicsDriver.setMode(reg_hsize, reg_vsize);
			
			// Render lines
			{
				// VBlank=0
				System.switchThread();
				
				unsigned long procStart = System.getMicros();
				if (reg_pclk) 
				{
					if (ram[REG_DLSWAP] == DLSWAP_FRAME)
					{
						Memory.swapDisplayList();
						ram[REG_DLSWAP] = DLSWAP_DONE;
					}
					bool rotate = s_RotateEnabled && ram[REG_ROTATE];
					if (s_DegradeOn)
					{
						GraphicsProcessor.process(GraphicsDriver.getBufferARGB8888(), 
							rotate ? !GraphicsDriver.isUpsideDown() : GraphicsDriver.isUpsideDown(), rotate, 
							reg_hsize, reg_vsize, s_DegradeStage, 2);
						++s_DegradeStage;
						s_DegradeStage %= 2;
					}
					else
					{
						GraphicsProcessor.process(GraphicsDriver.getBufferARGB8888(), 
							rotate ? !GraphicsDriver.isUpsideDown() : GraphicsDriver.isUpsideDown(), rotate, 
							reg_hsize, reg_vsize);
					}

				}
				unsigned long procDelta = System.getMicros() - procStart;

				if (s_DegradeOn)
				{
					if (procDelta < 4000)
					{
						s_DegradeOn = false;
						printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
						printf("Dynamic degrade switched OFF\n");
					}
				}
				else
				{
					if (procDelta > 8000)
					{
						printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
						if (s_DynamicDegrade)
						{
							s_DegradeOn = true;
							printf("Dynamic degrade switched ON\n");
						}
					}
				}
			}

			// Flip buffer and also give a slice of time to the mcu main thread
			{
				// VBlank=1
				if (reg_pclk) Memory.rawWriteU32(ram, REG_FRAMES, Memory.rawReadU32(ram, REG_FRAMES) + 1); // Increase REG_FRAMES
				System.prioritizeMCUThread();

#ifndef WIN32
				System.holdMCUThread(); // vblank'd !
				System.resumeMCUThread();
#endif

				unsigned long flipStart = System.getMicros();
				GraphicsDriver.renderBuffer(reg_pclk != 0);
				if (!GraphicsDriver.update()) exit(0); // ...
				unsigned long flipDelta = System.getMicros() - flipStart;

				if (flipDelta > 8000)
					printf("flip: %i micros (%i ms)\r\n", (int)flipDelta, (int)flipDelta / 1000);

				System.switchThread(); // ensure slice of time to mcu thread at cost of coprocessor cycles
				System.unprioritizeMCUThread();
			}

			System.prioritizeCoprocessorThread();
			if (reg_pclk)
			{
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
				}
			}
			else
			{
				// REG_PCLK is 0
#if FT800EMU_REG_PCLK_ZERO_REDUCE
				targetSeconds = System.getSeconds() + 0.1;
				System.delay(100);
#else
				targetSeconds = System.getSeconds();
				System.switchThread();
#endif
			}
			System.unprioritizeCoprocessorThread();

#ifdef WIN32
			System.holdMCUThread(); // don't let the other thread hog cpu
			System.resumeMCUThread();
			System.holdCoprocessorThread();
			System.resumeCoprocessorThread();
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

	int coprocessorThread(void * = NULL)
	{
		System.makeCoprocessorThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = System.setThreadGamesCategory(&taskId);
		System.disableAutomaticPriorityBoost();
		System.makeNormalPriorityThread();
		
		Coprocessor.executeEmulator();

		printf("Coprocessor thread exit\n");

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
			if (s_Flags & EmulatorEnableKeyboard) 
			{
				Keyboard.update();
				if (s_Keyboard)
				{
					s_Keyboard();
				}
				if (s_Flags & EmulatorEnableDebugShortkeys)
				{
					debugShortkeys();
				}
			}
			System.delay(10);
		}
		System.revertThreadCategory(taskHandle);
		return 0;
	}
}

void EmulatorClass::run(const EmulatorParameters &params)
{
	s_Setup = params.Setup;
	s_Loop = params.Loop;
	s_Flags = params.Flags;
	s_Keyboard = params.Keyboard;

	System.begin();
	Memory.begin(params.RomFilePath.empty() ? NULL : params.RomFilePath.c_str());
	GraphicsProcessor.begin();
	SPII2C.begin();
	GraphicsDriver.begin();
	// TODO_AUDIO if (flags & EmulatorEnableAudio) AudioDriver.begin();
	if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.begin(params.CoprocessorRomFilePath.empty() ? NULL : params.CoprocessorRomFilePath.c_str());
	if (params.Flags & EmulatorEnableKeyboard) Keyboard.begin();

	GraphicsDriver.enableMouse((params.Flags & EmulatorEnableMouse) == EmulatorEnableMouse);
	Memory.enableReadDelay();
	
	if (params.Flags & EmulatorEnableGraphicsMultithread)
	{
		GraphicsProcessor.enableMultithread();
		GraphicsProcessor.reduceThreads(params.ReduceGraphicsThreads);
	}
	
	s_DynamicDegrade = params.Flags & EmulatorEnableDynamicDegrade;
	s_RotateEnabled = params.Flags & EmulatorEnableRegRotate;

	s_MasterRunning = true;

#ifdef FT800EMU_SDL

	SDL_Thread *threadD = SDL_CreateThread(mcuThread, NULL);
	// TODO - Error handling

	SDL_Thread *threadA = SDL_CreateThread(audioThread, NULL);
	// TODO - Error handling

	SDL_Thread *threadC = NULL;
	if (params.Flags & EmulatorEnableCoprocessor) threadC = SDL_CreateThread(coprocessorThread, NULL);
	// TODO - Error handling

	masterThread();

	s_MasterRunning = false;
	if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.stopEmulator();

	SDL_WaitThread(threadD, NULL);
	SDL_WaitThread(threadA, NULL);
	if (params.Flags & EmulatorEnableCoprocessor) SDL_WaitThread(threadC, NULL);

#else
	#pragma omp parallel num_threads(params.Flags & EmulatorEnableCoprocessor ? 4 : 3)
	{
		// graphics
		#pragma omp master
		{
			masterThread();
			s_MasterRunning = false;
			if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.stopEmulator();
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

		// Coprocessor
		if (omp_get_thread_num() == 3)
		{
			coprocessorThread();
		}
	}
#endif /* #ifdef FT800EMU_SDL */

	if (params.Flags & EmulatorEnableKeyboard) Keyboard.end();
	if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.end();
	// TODO_AUDIO if (params.Flags & EmulatorEnableAudio) AudioDriver.end();
	GraphicsDriver.end();
	SPII2C.end();
	GraphicsProcessor.end();
	Memory.end();
	System.end();
}

} /* namespace FT800EMU */

/* end of file */
