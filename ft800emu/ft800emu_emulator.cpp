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

#include "ft800emu_audio_render.h"
#include "ft800emu_audio_processor.h"

#include "ft800emu_minmax.h"

#include "vc.h"

#ifndef FT800EMU_SDL
#	include "omp.h"
#endif

// using namespace ...;

#define FT800EMU_REG_PCLK_ZERO_REDUCE 1

namespace FT800EMU {

EmulatorClass Emulator;

void (*g_Exception)(const char *message) = NULL;

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

	bool s_EmulatorRunning = false;

	void (*s_Setup)() = NULL;
	void (*s_Loop)() = NULL;
	void (*s_Keyboard)() = NULL;
	int s_Flags = 0;
	bool s_MasterRunning = false;
	bool s_DynamicDegrade = false;

	bool s_DegradeOn = false;
	int s_DegradeStage = 0;

	bool s_RotateEnabled = false;

	bool (*s_Graphics)(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize) = NULL;
	argb8888 *s_GraphicsBuffer = NULL;

	int s_LastWriteOpCount = 0;
	bool s_FrameFullyDrawn = true;

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

		while (s_MasterRunning)
		{
			//printf("main thread\n");
			System.makeRealtimePriorityThread();

			System.enterSwapDL();

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
			if (!s_Graphics) GraphicsDriver.setMode(reg_hsize, reg_vsize);

			// Render lines
			{
				// VBlank=0
				System.switchThread();

				int lwoc = s_LastWriteOpCount;
				int woc = Memory.getWriteOpCount();
				bool hasChanges = lwoc != woc;
				s_LastWriteOpCount = woc;

				unsigned long procStart = System.getMicros();
				if (reg_pclk)
				{
					if (hasChanges || !s_FrameFullyDrawn)
					{
						// printf("%i != %i\n", lwoc, woc);
						if (ram[REG_DLSWAP] == DLSWAP_FRAME)
						{
							Memory.swapDisplayList();
							ram[REG_DLSWAP] = DLSWAP_DONE;
						}
						bool rotate = s_RotateEnabled && ram[REG_ROTATE];
						if (s_DegradeOn)
						{
							GraphicsProcessor.process(s_GraphicsBuffer ? s_GraphicsBuffer : GraphicsDriver.getBufferARGB8888(),
								s_GraphicsBuffer ? rotate : (rotate ? !GraphicsDriver.isUpsideDown() : GraphicsDriver.isUpsideDown()), rotate,
								reg_hsize, reg_vsize, s_DegradeStage, 2);
							++s_DegradeStage;
							s_DegradeStage %= 2;
							s_FrameFullyDrawn = !hasChanges;
						}
						else
						{
							GraphicsProcessor.process(s_GraphicsBuffer ? s_GraphicsBuffer : GraphicsDriver.getBufferARGB8888(),
								s_GraphicsBuffer ? rotate : (rotate ? !GraphicsDriver.isUpsideDown() : GraphicsDriver.isUpsideDown()), rotate,
								reg_hsize, reg_vsize);
							s_FrameFullyDrawn = true;
						}
					}
					else
					{
						// printf("no changes\n");
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

			System.leaveSwapDL();

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
				if (s_Graphics)
				{
					if (!s_Graphics(reg_pclk != 0, s_GraphicsBuffer, reg_hsize, reg_vsize))
					{
						exit(0); // TODO: Properly exit!!!
					}
				}
				else
				{
					GraphicsDriver.renderBuffer(reg_pclk != 0);
					if (!GraphicsDriver.update()) exit(0); // ...
				}
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
				targetSeconds = System.getSeconds() + 0.02;
				System.delay(20);
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
			if (s_Flags & EmulatorEnableAudio)
			{
				AudioRender.process();
			}
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
	s_EmulatorRunning = true;

	s_Setup = params.Setup;
	s_Loop = params.Loop;
	s_Flags = params.Flags;
	s_Keyboard = params.Keyboard;
	s_Graphics = params.Graphics;
	g_Exception = params.Exception;

	System.begin();
	Memory.begin(params.RomFilePath.empty() ? NULL : params.RomFilePath.c_str());
	GraphicsProcessor.begin();
	SPII2C.begin();
	if (!s_Graphics) GraphicsDriver.begin();
	if (params.Flags & EmulatorEnableAudio)
	{
		AudioProcessor.begin();
		AudioDriver.begin();
	}
	if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.begin(params.CoprocessorRomFilePath.empty() ? NULL : params.CoprocessorRomFilePath.c_str());
	if ((!s_Graphics) && (params.Flags & EmulatorEnableKeyboard)) Keyboard.begin();

	if (s_Graphics)
	{
		s_GraphicsBuffer = new argb8888[FT800EMU_WINDOW_WIDTH_MAX * FT800EMU_WINDOW_HEIGHT_MAX];
	}

	if (!s_Graphics) GraphicsDriver.enableMouse((params.Flags & EmulatorEnableMouse) == EmulatorEnableMouse);
	Memory.enableReadDelay();

	if (params.Flags & EmulatorEnableGraphicsMultithread)
	{
		GraphicsProcessor.enableMultithread();
		GraphicsProcessor.reduceThreads(params.ReduceGraphicsThreads);
	}
	if (params.Flags & EmulatorEnableRegPwmDutyEmulation) GraphicsProcessor.enableRegPwmDutyEmulation();

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
			// System.killMCUThread();
			if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.stopEmulator();
			printf("(0) master thread exit\n");
		}

		// arduino
		if (omp_get_thread_num() == 1)
		{
			mcuThread();
			printf("(1) mcu thread exit\n");
		}

		// sound
		if (omp_get_thread_num() == 2)
		{
			audioThread();
			printf("(2) sound thread exit\n");
		}

		// Coprocessor
		if (omp_get_thread_num() == 3)
		{
			coprocessorThread();
			printf("(3) coproc thread exit\n");
		}
	}
#endif /* #ifdef FT800EMU_SDL */

	printf("ending\n");

	delete[] s_GraphicsBuffer;
	s_GraphicsBuffer = NULL;
	if ((!s_Graphics) && (params.Flags & EmulatorEnableKeyboard)) Keyboard.end();
	if (params.Flags & EmulatorEnableCoprocessor) Coprocessor.end();
	if (params.Flags & EmulatorEnableAudio)
	{
		AudioDriver.end();
		AudioProcessor.end();
	}
	if (!s_Graphics) GraphicsDriver.end();
	SPII2C.end();
	GraphicsProcessor.end();
	Memory.end();
	System.end();

	s_EmulatorRunning = false;
	printf("emulator stop running\n");
}

void EmulatorClass::stop()
{
	s_MasterRunning = false;

	while (s_EmulatorRunning) // TODO: Mutex?
	{
		System.delay(1);
	}

	printf("stop ok\n");
}

} /* namespace FT800EMU */

/* end of file */
