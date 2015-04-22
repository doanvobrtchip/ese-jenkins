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
#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))
#include <SDL.h>
#include <SDL_thread.h>
#endif

// Project includes
#include "ft8xxemu_system.h"
#include "ft8xxemu_keyboard.h"
#include "ft8xxemu_keyboard_keys.h"
#include "ft8xxemu_graphics_driver.h"
#include "ft8xxemu_audio_driver.h"

#include "ft800emu_spi_i2c.h"
#include "ft800emu_memory.h"
#include "ft800emu_touch.h"
#include "ft800emu_graphics_processor.h"
#include "ft800emu_coprocessor.h"

#include "ft800emu_audio_render.h"
#include "ft800emu_audio_processor.h"

#include "ft8xxemu_minmax.h"

#include "ft800emu_vc.h"

#ifndef FTEMU_SDL
#ifndef FTEMU_SDL2
#	include "omp.h"
#endif
#endif

// using namespace ...;

// Enable or disable debug messages
#define FT800EMU_DEBUG 1

// Reduce CPU usage when REG_PCLK is zero
#define FT800EMU_REG_PCLK_ZERO_REDUCE 1

#if defined(FTEMU_SDL2)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, name, data)
#elif defined(FTEMU_SDL)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, data)
#endif

namespace FT800EMU {

EmulatorClass Emulator;

namespace {

const uint8_t bayerDiv16[4][4] = {
	0, 8, 2, 10,
	12, 4, 14, 6,
	3, 11, 1, 9,
	15, 7, 13, 5,
};
#define DITHERDIV16(val, x, y) min(15, ((val) + bayerDiv16[(x) & 0x3][(y) & 0x3]) >> 4)
const uint8_t bayerDiv8[4][4] = {
	0, 4, 1, 5,
	6, 2, 7, 3,
	1, 5, 0, 4,
	7, 3, 6, 2,
};
#define DITHERDIV8(val, x, y) min(31, ((val) + bayerDiv8[(x) & 0x3][(y) & 0x3]) >> 3)
const uint8_t bayerDiv4[2][2] = {
	0, 2,
	3, 1,
};
#define DITHERDIV4(val, x, y) min(63, ((val) + bayerDiv4[(x) & 0x1][(y) & 0x1]) >> 2)

	void debugShortkeys()
	{
		{
			static bool setDebugMode = false;
			if (setDebugMode)
			{
				setDebugMode = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F3);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F3))
			{
				FT800EMU::GraphicsProcessor.setDebugMode((FT800EMU::GraphicsProcessor.getDebugMode() + 1) % FT800EMU_DEBUGMODE_COUNT);
				setDebugMode = true;
			}
		}

		{
			static bool incDebugMultiplier = false;
			if (incDebugMultiplier)
			{
				incDebugMultiplier = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADPLUS);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADPLUS))
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
				decDebugMultiplier = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADMINUS);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADMINUS))
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
				resetDebugMultiplier = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADSLASH);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_NUMPADSLASH))
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
				incDebugLimiter = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F8);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F8))
			{
				FT800EMU::GraphicsProcessor.setDebugLimiter(FT800EMU::GraphicsProcessor.getDebugLimiter() + 1);
				incDebugLimiter = true;
			}
		}

		{
			static bool decDebugLimiter = false;
			if (decDebugLimiter)
			{
				decDebugLimiter = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F7);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F7))
			{
				FT800EMU::GraphicsProcessor.setDebugLimiter(max(FT800EMU::GraphicsProcessor.getDebugLimiter() - 1, 0));
				decDebugLimiter = true;
			}
		}

		{
			static bool resetDebugLimiter = false;
			if (resetDebugLimiter)
			{
				resetDebugLimiter = FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F6);
			}
			else if (FT8XXEMU::Keyboard.isKeyDown(FT8XXEMU_KEY_F6))
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
	void (*s_Close)() = NULL;
	int s_Flags = 0;
	bool s_MasterRunning = false;
	bool s_DynamicDegrade = false;
	uint32_t s_ExternalFrequency = 0;

	bool s_DegradeOn = false;
	int s_DegradeStage = 0;
	
	bool s_SkipOn = false;
	int s_SkipStage = 0;

	// bool s_RotateEnabled = false;

	int (*s_Graphics)(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags) = NULL;
	argb8888 *s_GraphicsBuffer = NULL;

	int s_LastWriteOpCount = 0;
	int s_LastRealSwapCount = 0;
	bool s_FrameFullyDrawn = true;
	bool s_ChangesSkipped = false;
	
#ifdef FTEMU_SDL2
	// Make the master thread wait for MCU and Coprocessor threads to properly set themselves up
	SDL_sem *s_InitSem = NULL;
#endif

	int masterThread(void * = NULL)
	{
		FT8XXEMU::System.makeMainThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = FT8XXEMU::System.setThreadGamesCategory(&taskId);
		FT8XXEMU::System.disableAutomaticPriorityBoost();
		FT8XXEMU::System.makeRealtimePriorityThread();

		double targetSeconds = FT8XXEMU::System.getSeconds();

		uint8_t *ram = Memory.getRam();

		while (s_MasterRunning)
		{
			// printf("main thread\n");
			FT8XXEMU::System.makeRealtimePriorityThread();

			FT8XXEMU::System.enterSwapDL();

			bool renderLineSnapshot = (ram[REG_RENDERMODE] & 1) != 0;
			uint32_t reg_pclk = Memory.rawReadU32(ram, REG_PCLK);
			double deltaSeconds;
			// Calculate the display frequency
			if (reg_pclk == 0xFF)
			{
				deltaSeconds = 0.0;
			}
			else if (reg_pclk)
			{
				uint32_t usefreq = s_ExternalFrequency ? s_ExternalFrequency : Memory.rawReadU32(ram, REG_FREQUENCY);
				// if (!usefreq) usefreq = 48000000; // Possibility to avoid freeze in case of issues
				double frequency = (double)usefreq;
				frequency /= (double)reg_pclk;
				frequency /= (double)Memory.rawReadU32(ram, REG_VCYCLE);
				frequency /= (double)Memory.rawReadU32(ram, REG_HCYCLE);
				deltaSeconds = 1.0 / frequency;
			}
			else
			{
				deltaSeconds = 1.0;
			}

			FT8XXEMU::System.update();
			if ((renderLineSnapshot && FT8XXEMU::System.renderWoke()) || reg_pclk == 0xFF)
				targetSeconds = FT8XXEMU::System.getSeconds() + deltaSeconds;
			else
				targetSeconds += deltaSeconds;

			TouchClass::setTouchScreenXYFrameTime((long)(deltaSeconds * 1000 * 1000));

			// Update display resolution
			uint32_t reg_vsize = Memory.rawReadU32(ram, REG_VSIZE);
			uint32_t reg_hsize = Memory.rawReadU32(ram, REG_HSIZE);
			if (!s_Graphics) FT8XXEMU::GraphicsDriver.setMode(reg_hsize, reg_vsize);


			bool renderProcessed = false;
			bool hasChanged;

			// Render lines
			// VBlank=0
#ifdef FT810EMU_MODE
			Memory.rawWriteU32(ram, REG_RASTERY, 0);
#endif
			{
				FT8XXEMU::System.switchThread();

				int lwoc = s_LastWriteOpCount;
				int woc = Memory.getWriteOpCount();
				hasChanged = (lwoc != woc);
				bool hasChanges = hasChanged || s_ChangesSkipped;
				s_LastWriteOpCount = woc;

				unsigned long procStart = FT8XXEMU::System.getMicros();
				if (reg_pclk || renderLineSnapshot)
				{
					if (hasChanges || !s_FrameFullyDrawn)
					{
						// printf("%i != %i\n", lwoc, woc);
						if (ram[REG_DLSWAP] == DLSWAP_FRAME)
						{
							Memory.swapDisplayList();
							ram[REG_DLSWAP] = DLSWAP_DONE;
							Memory.flagDLSwap();
						}
						bool mirrorHorizontal = /*s_RotateEnabled &&*/ FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram);
						bool mirrorVertical = /*s_RotateEnabled &&*/ FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram);
						if (renderLineSnapshot)
						{
							if ((ram[REG_SNAPSHOT] & 1) && ram[REG_BUSYBITS]) // TODO_BUSYBITS
							{
								Memory.rawWriteU32(ram, REG_SNAPSHOT, 0);
								// Render single line
								uint32_t snapy = mirrorVertical ? (reg_vsize - Memory.rawReadU32(ram, REG_SNAPY)) : (Memory.rawReadU32(ram, REG_SNAPY));
								snapy &= FT800EMU_SCREEN_HEIGHT_MASK;
								// printf("SNAPY: %u\n", snapy);
								argb8888 *buffer = s_GraphicsBuffer ? s_GraphicsBuffer : FT8XXEMU::GraphicsDriver.getBufferARGB8888();
								GraphicsProcessor.process(buffer,
									false, mirrorHorizontal,
#ifdef FT810EMU_MODE
									FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
									reg_hsize, snapy + 1, snapy);
								uint32_t ya = (reg_hsize * snapy);
								uint32_t wa = RAM_COMPOSITE;
								for (uint32_t x = 0; x < reg_hsize; ++x)
								{
#ifdef FT810EMU_MODE
									switch (ram[REG_SNAPFORMAT])
									{
									case ARGB4:
									{
										argb8888 c0 = buffer[ya + x]; ++x;
										argb8888 c1 = buffer[ya + x];
										/*uint32_t r = ((c0 >> 4) & 0xF)
											| (((c0 >> 12) & 0xF) << 4)
											| (((c0 >> 20) & 0xF) << 8)
											| ((/*(c0 >> 28) &* / 0xF) << 12)
											| (((c1 >> 4) & 0xF) << 16)
											| (((c1 >> 12) & 0xF) << 20)
											| (((c1 >> 20) & 0xF) << 24)
											| ((/*(c1 >> 28) &* / 0xF) << 28);*/
										uint32_t r = (DITHERDIV16(c0 & 0xFF, (-(x - 1)), snapy))
											| (DITHERDIV16((c0 >> 8) & 0xFF, (x - 1), snapy) << 4)
											| (DITHERDIV16((c0 >> 16 & 0xFF), (x - 1), (-snapy)) << 8)
											| (DITHERDIV16((c0 >> 24 & 0xFF), (-(x - 1)), (-(snapy))) << 12)
											// | (0xF << 12)
											| (DITHERDIV16(c1 & 0xFF, (-x), snapy) << 16)
											| (DITHERDIV16((c1 >> 8) & 0xFF, x, snapy) << 20)
											| (DITHERDIV16((c1 >> 16) & 0xFF, x, (-snapy)) << 24)
											// | (0xF << 28);
											| (DITHERDIV16((c1 >> 24 & 0xFF), (-x), (-(snapy))) << 28);
										Memory.rawWriteU32(ram, wa, r);
										wa += 4;
										break;
									}
									/*case ARGB1555:
									{
										argb8888 c = buffer[ya + x];
										uint16_t r = (DITHERDIV8(c & 0xFF, x, snapy))
											| (DITHERDIV8((c >> 8) & 0xFF, x, snapy) << 5)
											| (DITHERDIV8((c >> 16) & 0xFF, x, snapy) << 10)
											| (0x1 << 15);
										Memory.rawWriteU16(ram, wa, r);
										wa += 2;
										break;
									}*/
									case RGB565:
									{
										argb8888 c = buffer[ya + x];
										uint16_t r = (DITHERDIV8(c & 0xFF, (-x), snapy))
											| (DITHERDIV4((c >> 8) & 0xFF, x, snapy) << 5)
											| (DITHERDIV8((c >> 16) & 0xFF, x, (-snapy)) << 11);
										Memory.rawWriteU16(ram, wa, r);
										wa += 2;
										break;
									}
									case 0x20:
									default:
#endif
										Memory.rawWriteU32(ram, RAM_COMPOSITE + (x * 4), buffer[ya + x]);
#ifdef FT810EMU_MODE
										break;
									}
#endif
								}
								Memory.rawWriteU32(ram, REG_BUSYBITS, 0); // TODO_BUSYBITS
							}
						}
						else
						{
							if (s_SkipOn)
							{
								++s_SkipStage;
								s_SkipStage %= 2;
							}
							if (s_SkipOn && s_SkipStage)
							{
								GraphicsProcessor.processBlank();
								s_ChangesSkipped = hasChanges;
							}
							else if (s_DegradeOn)
							{
								GraphicsProcessor.process(s_GraphicsBuffer ? s_GraphicsBuffer : FT8XXEMU::GraphicsDriver.getBufferARGB8888(),
									s_GraphicsBuffer ? mirrorVertical : (mirrorVertical ? !FT8XXEMU::GraphicsDriver.isUpsideDown() : FT8XXEMU::GraphicsDriver.isUpsideDown()), mirrorHorizontal,
#ifdef FT810EMU_MODE
									FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
									reg_hsize, reg_vsize, s_DegradeStage, 2);
								++s_DegradeStage;
								s_DegradeStage %= 2;
								s_ChangesSkipped = false;
								s_FrameFullyDrawn = !hasChanges;
								renderProcessed = true;
							}
							else
							{
								GraphicsProcessor.process(s_GraphicsBuffer ? s_GraphicsBuffer : FT8XXEMU::GraphicsDriver.getBufferARGB8888(),
									s_GraphicsBuffer ? mirrorVertical : (mirrorVertical ? !FT8XXEMU::GraphicsDriver.isUpsideDown() : FT8XXEMU::GraphicsDriver.isUpsideDown()), mirrorHorizontal,
#ifdef FT810EMU_MODE
									FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
									reg_hsize, reg_vsize);
								s_ChangesSkipped = false;
								s_FrameFullyDrawn = true;
								renderProcessed = true;
							}
						}
					}
					else
					{
						// printf("no changes\n");
					}
				}
				else
				{
					s_SkipOn = false;
					s_DegradeOn = false;
				}
				unsigned long procDelta = FT8XXEMU::System.getMicros() - procStart;
				if (!renderLineSnapshot)
				{
					if (s_SkipOn)
					{
						if (!s_SkipStage)
						{
							unsigned long procLowLimit = (unsigned long)(deltaSeconds * 500000.0); // Under 50% of allowed time, turn off frameskip
							if (procDelta < procLowLimit)
							{
								s_SkipOn = false;
								if (FT800EMU_DEBUG) printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
								if (FT800EMU_DEBUG) printf("Frame skip switched OFF\n");
							}
						}
					}
					else if (s_DegradeOn)
					{
						// Note: procLowLimit must be much less than half of procHighLimit, as the render time is halved when dynamic degrade kicks in
						unsigned long procLowLimit = (unsigned long)(deltaSeconds * 250000.0); // Under 25% of allowed time, turn off degrade
						unsigned long procHighLimit = (unsigned long)(deltaSeconds * 800000.0); // Over 80% of allowed time, switch to frameskip
						if (procDelta < procLowLimit)
						{
							s_DegradeOn = false;
							if (FT800EMU_DEBUG) printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
							if (FT800EMU_DEBUG) printf("Dynamic degrade switched OFF\n");
						}
						else if (procDelta > procHighLimit)
						{
							s_SkipOn = true;
							if (FT800EMU_DEBUG) printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
							if (FT800EMU_DEBUG) printf("Frame skip switched ON\n");
						}
					}
					else
					{
						unsigned long procHighLimit = (unsigned long)(deltaSeconds * 800000.0); // Over 80% of allowed time, switch to degrade
						if (procDelta > procHighLimit)
						{
							if (FT800EMU_DEBUG) printf("process: %i micros (%i ms)\n", (int)procDelta, (int)procDelta / 1000);
							if (s_DynamicDegrade)
							{
								s_DegradeOn = true;
								if (FT800EMU_DEBUG) printf("Dynamic degrade switched ON\n");
							}
						}
					}
				}
			}

			bool hasSwapped;

			{
				int lc = s_LastRealSwapCount;
				int c = Memory.getRealSwapCount();
				hasSwapped = (lc != c);
				s_LastRealSwapCount = c;
			}

			FT8XXEMU::System.leaveSwapDL();

			// Flip buffer and also give a slice of time to the mcu main thread
			// VBlank=1
#ifdef FT810EMU_MODE
			Memory.rawWriteU32(ram, REG_RASTERY, 1 << 11);
#endif
			if (!renderLineSnapshot)
			{
				if (reg_pclk) Memory.rawWriteU32(ram, REG_FRAMES, Memory.rawReadU32(ram, REG_FRAMES) + 1); // Increase REG_FRAMES
				FT8XXEMU::System.prioritizeMCUThread();
				//printf("fr %u\n",  Memory.rawReadU32(ram, REG_FRAMES));

#ifndef WIN32
				FT8XXEMU::System.holdMCUThread(); // vblank'd !
				FT8XXEMU::System.resumeMCUThread();
#endif

				unsigned long flipStart = FT8XXEMU::System.getMicros();
				if (s_SkipOn && s_SkipStage)
				{
					// no-op
					// NOTE: Also skips s_Graphics
				}
				else
				{
					if (s_Graphics)
					{
						uint32_t frameFlags = 0;
						if (renderProcessed)
							frameFlags |= FT8XXEMU_FrameBufferChanged;
						if (s_FrameFullyDrawn)
							frameFlags |= FT8XXEMU_FrameBufferComplete;
						if (hasChanged)
							frameFlags |= FT8XXEMU_FrameChanged;
						if (hasSwapped)
							frameFlags |= FT8XXEMU_FrameSwap;
						if (!s_Graphics(reg_pclk != 0, s_GraphicsBuffer, reg_hsize, reg_vsize, (FT8XXEMU_FrameFlags)frameFlags))
						{
							if (s_Close)
							{
								s_Close();
								return 0;
							}
							else
							{
								printf("Kill MCU thread\n");
								FT8XXEMU::System.killMCUThread();
								return 0;
							}
						}
					}
					else
					{
						FT8XXEMU::GraphicsDriver.renderBuffer(reg_pclk != 0, renderProcessed);
						if (!FT8XXEMU::GraphicsDriver.update())
						{
							if (s_Close)
							{
								s_Close();
								return 0;
							}
							else
							{
								printf("Kill MCU thread\n");
								FT8XXEMU::System.killMCUThread();
								return 0;
							}
						}
					}
				}
				unsigned long flipDelta = FT8XXEMU::System.getMicros() - flipStart;

				if (flipDelta > 8000)
					if (FT800EMU_DEBUG) printf("flip: %i micros (%i ms)\r\n", (int)flipDelta, (int)flipDelta / 1000);

				FT8XXEMU::System.switchThread(); // ensure slice of time to mcu thread at cost of coprocessor cycles
				FT8XXEMU::System.unprioritizeMCUThread();
			}

			FT8XXEMU::System.prioritizeCoprocessorThread();
			if (reg_pclk == 0xFF)
			{
				FT8XXEMU::System.renderSleep(1);
			}
			else if (reg_pclk)
			{
				//long currentMillis = millis();
				//long millisToWait = targetMillis - currentMillis;
				double currentSeconds = FT8XXEMU::System.getSeconds();
				double secondsToWait = targetSeconds - currentSeconds;
				// printf("sleep %f seconds (%f current time, %f target time, %f delta seconds)\n", (float)secondsToWait, (float)currentSeconds, (float)targetSeconds, (float)deltaSeconds);
				//if (millisToWait < -100) targetMillis = millis();
				if (secondsToWait < -0.25) // Skip freeze
				{
					//printf("skip freeze\n");
					targetSeconds = FT8XXEMU::System.getSeconds();
				}
				else if (secondsToWait > 0.25)
				{
					printf("Possible problem with REG_FREQUENCY value %u, sw %f, cs %f -> ts %f\n", Memory.rawReadU32(ram, REG_FREQUENCY), (float)secondsToWait, (float)currentSeconds, (float)targetSeconds);
					targetSeconds = FT8XXEMU::System.getSeconds() + 0.25;
					secondsToWait = 0.25;
				}

				//printf("millis to wait: %i", (int)millisToWait);

				if (secondsToWait > 0.0)
				{
					FT8XXEMU::System.renderSleep((int)(secondsToWait * 1000.0));
				}
			}
			else
			{
				// REG_PCLK is 0
#if FT800EMU_REG_PCLK_ZERO_REDUCE
				targetSeconds = FT8XXEMU::System.getSeconds() + 0.02;
				FT8XXEMU::System.renderSleep(20);
#else
				targetSeconds = FT8XXEMU::System.getSeconds();
				FT8XXEMU::System.switchThread();
#endif
			}
			FT8XXEMU::System.unprioritizeCoprocessorThread();

#ifdef WIN32
			FT8XXEMU::System.holdMCUThread(); // don't let the other thread hog cpu
			FT8XXEMU::System.resumeMCUThread();
			FT8XXEMU::System.holdCoprocessorThread();
			FT8XXEMU::System.resumeCoprocessorThread();
#endif
		}

		FT8XXEMU::System.revertThreadCategory(taskHandle);
		return 0;
	}

	int mcuThread(void * = NULL)
	{
		FT8XXEMU::System.makeMCUThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = FT8XXEMU::System.setThreadGamesCategory(&taskId);
		FT8XXEMU::System.disableAutomaticPriorityBoost();
		FT8XXEMU::System.makeNormalPriorityThread();
		
#ifdef FTEMU_SDL2
		SDL_SemPost(s_InitSem);
#endif
		
		s_Setup();
		while (s_MasterRunning)
		{
			//printf("mcu thread\n");
			s_Loop();
		}
		FT8XXEMU::System.revertThreadCategory(taskHandle);
		return 0;
	}

	int coprocessorThread(void * = NULL)
	{
		FT8XXEMU::System.makeCoprocessorThread();

		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = FT8XXEMU::System.setThreadGamesCategory(&taskId);
		FT8XXEMU::System.disableAutomaticPriorityBoost();
		FT8XXEMU::System.makeNormalPriorityThread();
		
#ifdef FTEMU_SDL2
		SDL_SemPost(s_InitSem);
#endif

		Coprocessor.executeEmulator();

		printf("Coprocessor thread exit\n");

		FT8XXEMU::System.revertThreadCategory(taskHandle);
		return 0;
	}

	int audioThread(void * = NULL)
	{
		//printf("go sound thread");
		unsigned long taskId = 0;
		void *taskHandle;
		taskHandle = FT8XXEMU::System.setThreadGamesCategory(&taskId);
		FT8XXEMU::System.disableAutomaticPriorityBoost();
		FT8XXEMU::System.makeHighPriorityThread();
		while (s_MasterRunning)
		{
			//printf("sound thread\n");
			if (s_Flags & FT8XXEMU_EmulatorEnableAudio)
			{
				AudioRender.process();
			}
			if (s_Flags & FT8XXEMU_EmulatorEnableKeyboard)
			{
				FT8XXEMU::Keyboard.update();
				if (s_Keyboard)
				{
					s_Keyboard();
				}
				if (s_Flags & FT8XXEMU_EmulatorEnableDebugShortkeys)
				{
					debugShortkeys();
				}
			}
			FT8XXEMU::System.delay(10);
		}
		FT8XXEMU::System.revertThreadCategory(taskHandle);
		return 0;
	}
}

void EmulatorClass::run(const FT8XXEMU_EmulatorParameters &params)
{
	FT8XXEMU_EmulatorMode mode = params.Mode;
	if (mode == 0) mode = FT8XXEMU_EmulatorFT800;

#ifdef FT810EMU_MODE
	if (mode < FT8XXEMU_EmulatorFT810)
	{
		printf("Invalid emulator version selected, this library is built in FT810 mode\n");
		return;
	}
#else
	if (mode > FT8XXEMU_EmulatorFT801)
	{
		printf("Invalid emulator version selected, this library is built in FT800/FT800 mode\n");
		return;
	}
#endif

	s_EmulatorRunning = true;

	s_Setup = params.Setup;
	s_Loop = params.Loop;
	s_Flags = params.Flags;
	s_Keyboard = params.Keyboard;
	s_Graphics = params.Graphics;
	FT8XXEMU::g_Exception = params.Exception;
	s_Close = params.Close;
	s_ExternalFrequency = params.ExternalFrequency;

	FT8XXEMU::System.begin();
	FT8XXEMU::System.overrideMCUDelay(params.MCUSleep);
	Memory.begin(mode, params.RomFilePath, params.OtpFilePath);
	TouchClass::begin(mode);
	GraphicsProcessor.begin();
	SPII2C.begin();
	if (!s_Graphics) FT8XXEMU::GraphicsDriver.begin();
	if (params.Flags & FT8XXEMU_EmulatorEnableAudio)
	{
		AudioProcessor.begin();
		AudioRender.begin();
		FT8XXEMU::AudioDriver.begin();
	}
	if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor)
		Coprocessor.begin(
			params.CoprocessorRomFilePath ? NULL : params.CoprocessorRomFilePath,
			mode);
	if ((!s_Graphics) && (params.Flags & FT8XXEMU_EmulatorEnableKeyboard)) FT8XXEMU::Keyboard.begin();

	if (s_Graphics)
	{
		s_GraphicsBuffer = new argb8888[FT800EMU_SCREEN_WIDTH_MAX * FT800EMU_SCREEN_HEIGHT_MAX];
	}

	if (!s_Graphics) FT8XXEMU::GraphicsDriver.enableMouse((params.Flags & FT8XXEMU_EmulatorEnableMouse) == FT8XXEMU_EmulatorEnableMouse);
	Memory.enableReadDelay();

	if (params.Flags & FT8XXEMU_EmulatorEnableGraphicsMultithread)
	{
		GraphicsProcessor.enableMultithread();
		GraphicsProcessor.reduceThreads(params.ReduceGraphicsThreads);
	}
	if (params.Flags & FT8XXEMU_EmulatorEnableRegPwmDutyEmulation) GraphicsProcessor.enableRegPwmDutyEmulation();

	s_DynamicDegrade = (params.Flags & FT8XXEMU_EmulatorEnableDynamicDegrade) == FT8XXEMU_EmulatorEnableDynamicDegrade;
	// s_RotateEnabled = (params.Flags & FT8XXEMU_EmulatorEnableRegRotate) == FT8XXEMU_EmulatorEnableRegRotate;
	TouchClass::enableTouchMatrix((params.Flags & FT8XXEMU_EmulatorEnableTouchTransformation) == FT8XXEMU_EmulatorEnableTouchTransformation);

	s_MasterRunning = true;

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

	SDL_Thread *threadA = SDL_CreateThreadFT(audioThread, "FT800EMU::Audio", NULL);
	// TODO - Error handling

#ifdef FTEMU_SDL2
	s_InitSem = SDL_CreateSemaphore(0);
#endif

	SDL_Thread *threadC = NULL;
	if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor)
		threadC = SDL_CreateThreadFT(coprocessorThread, "FT800EMU::Coprocessor", NULL);
	// TODO - Error handling
#ifdef FTEMU_SDL2
	SDL_SemWait(s_InitSem);
#endif

	SDL_Thread *threadD = SDL_CreateThreadFT(mcuThread, "FT800EMU::MCU", NULL);
	// TODO - Error handling
#ifdef FTEMU_SDL2
	SDL_SemWait(s_InitSem);
	SDL_DestroySemaphore(s_InitSem);
	s_InitSem = NULL;
#endif

	masterThread();

	s_MasterRunning = false;
	if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor)
		Coprocessor.stopEmulator();

	printf("Wait for Coprocessor\n");
	if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor)
		SDL_WaitThread(threadC, NULL);

	printf("Wait for MCU\n");
	SDL_WaitThread(threadD, NULL);

	printf("Wait for Audio\n");
	SDL_WaitThread(threadA, NULL);

#else
	#pragma omp parallel num_threads(params.Flags & FT8XXEMU_EmulatorEnableCoprocessor ? 4 : 3)
	{
		// graphics
		#pragma omp master
		{
			masterThread();
			s_MasterRunning = false;
			// System.killMCUThread();
			if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor) Coprocessor.stopEmulator();
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
#endif /* #ifdef FTEMU_SDL */

	printf("Threads finished, cleaning up\n");

	delete[] s_GraphicsBuffer;
	s_GraphicsBuffer = NULL;
	if ((!s_Graphics) && (params.Flags & FT8XXEMU_EmulatorEnableKeyboard)) FT8XXEMU::Keyboard.end();
	if (params.Flags & FT8XXEMU_EmulatorEnableCoprocessor) Coprocessor.end();
	if (params.Flags & FT8XXEMU_EmulatorEnableAudio)
	{
		FT8XXEMU::AudioDriver.end();
		AudioRender.end();
		AudioProcessor.end();
	}
	if (!s_Graphics) FT8XXEMU::GraphicsDriver.end();
	SPII2C.end();
	GraphicsProcessor.end();
	TouchClass::end();
	Memory.end();
	FT8XXEMU::System.end();

	s_EmulatorRunning = false;
	printf("Emulator stopped running\n");
}

void EmulatorClass::stop()
{
	s_MasterRunning = false;

	if (!FT8XXEMU::System.isMCUThread())
	{
		printf("Wait for MCU thread\n");
		while (s_EmulatorRunning) // TODO: Mutex?
		{
			FT8XXEMU::System.delay(1);
		}
	}

	printf("Stop ok\n");
}

} /* namespace FT800EMU */

/* end of file */
