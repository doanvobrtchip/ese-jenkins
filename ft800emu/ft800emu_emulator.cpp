/**
 * Emulator
 * $Id$
 * \file ft800emu_emulator.cpp
 * \brief Emulator
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_emulator.h"

#ifdef FT800EMU_DEFS_H
#pragma message(" : Error: Not allowed to include ft800emu_defs.h from ft800emu_emulator.h")
#endif

// System includes
#include <thread>

// Project includes
#include "ft8xxemu_system.h"
#include "ft8xxemu_keyboard_input.h"
#include "ft8xxemu_keyboard_keys.h"
#include "ft8xxemu_window_output.h"
#include "ft8xxemu_audio_output.h"

#include "ft800emu_bus_slave.h"
#include "ft800emu_memory.h"
#include "ft800emu_touch.h"
#include "ft800emu_graphics_processor.h"
#include "ft800emu_coprocessor.h"

#include "ft800emu_audio_render.h"
#include "ft800emu_audio_processor.h"

#include "ft8xxemu_minmax.h"

#include "ft800emu_vc.h"

// using namespace ...;

// Enable or disable debug messages
#define FT800EMU_DEBUG 0

// Reduce CPU usage when REG_PCLK is zero
#define FT800EMU_REG_PCLK_ZERO_REDUCE 1

#if defined(FTEMU_SDL2)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, name, data)
#elif defined(FTEMU_SDL)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, data)
#endif

namespace FT800EMU {

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

}

void Emulator::debugShortkeys()
{
	{
		static bool setDebugMode = false;
		if (setDebugMode)
		{
			setDebugMode = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F3);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F3))
		{
			FT800EMU::GraphicsProcessor.setDebugMode((FT800EMU::GraphicsProcessor.getDebugMode() + 1) % FT800EMU_DEBUGMODE_COUNT);
			setDebugMode = true;
		}
	}

	{
		static bool incDebugMultiplier = false;
		if (incDebugMultiplier)
		{
			incDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADPLUS);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADPLUS))
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
			decDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADMINUS);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADMINUS))
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
			resetDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADSLASH);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADSLASH))
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
			incDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F8);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F8))
		{
			FT800EMU::GraphicsProcessor.setDebugLimiter(FT800EMU::GraphicsProcessor.getDebugLimiter() + 1);
			incDebugLimiter = true;
		}
	}

	{
		static bool decDebugLimiter = false;
		if (decDebugLimiter)
		{
			decDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F7);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F7))
		{
			FT800EMU::GraphicsProcessor.setDebugLimiter(max(FT800EMU::GraphicsProcessor.getDebugLimiter() - 1, 0));
			decDebugLimiter = true;
		}
	}

	{
		static bool resetDebugLimiter = false;
		if (resetDebugLimiter)
		{
			resetDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F6);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F6))
		{
			FT800EMU::GraphicsProcessor.setDebugLimiter(0);
			resetDebugLimiter = true;
		}
	}
}

int Emulator::masterThread(bool sync)
{
	m_ThreadMaster.init();
	m_ThreadMaster.foreground();
	m_ThreadMaster.noboost();
	m_ThreadMaster.realtime();
	m_ThreadMaster.setName(sync ? "FT8XXEMU Graphics Master" : "FT8XXEMU Graphics Master Async");

	double targetSeconds = m_System->getSeconds();

	uint8_t *ram = m_Memory->getRam();
	int lastMicros = m_System->getMicros();

	if (!sync)
	{
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_InitCond.notify_one();
	}

	while (m_MasterRunning)
	{
		// FTEMU_printf("main thread\n");

		m_SwapDLMutex.lock();

		uint32_t usefreq = m_ExternalFrequency ? m_ExternalFrequency : m_Memory->rawReadU32(ram, REG_FREQUENCY);
		int curMicros = m_System->getMicros();
		int diffMicros = curMicros - lastMicros;
		lastMicros = curMicros;
		int32_t diffClock = (int32_t)(((int64_t)usefreq * (int64_t)diffMicros) / (int64_t)1000000LL);
		uint32_t curClock = (uint32_t)((int32_t)m_Memory->rawReadU32(ram, REG_CLOCK) + diffClock);
		m_Memory->rawWriteU32(ram, REG_CLOCK, curClock);

		bool renderLineSnapshot = (ram[REG_RENDERMODE] & 1) != 0;
		uint32_t reg_pclk = m_Memory->rawReadU32(ram, REG_PCLK);
		double deltaSeconds;
		// Calculate the display frequency
		if (reg_pclk == 0xFF)
		{
			deltaSeconds = 0.0;
		}
		else if (reg_pclk)
		{
			// if (!usefreq) usefreq = 48000000; // Possibility to avoid freeze in case of issues
			double frequency = (double)usefreq;
			frequency /= (double)reg_pclk;
			frequency /= (double)m_Memory->rawReadU32(ram, REG_VCYCLE);
			frequency /= (double)m_Memory->rawReadU32(ram, REG_HCYCLE);
			deltaSeconds = 1.0 / frequency;
		}
		else
		{
			deltaSeconds = 1.0;
		}

		m_System->update();
		if ((renderLineSnapshot && m_System->renderWoke()) || reg_pclk == 0xFF)
			targetSeconds = m_System->getSeconds() + deltaSeconds;
		else
			targetSeconds += deltaSeconds;

		m_Touch->setTouchScreenXYFrameTime((long)(deltaSeconds * 1000 * 1000));

		// Update display resolution
		int32_t reg_vsize = m_Memory->rawReadU32(ram, REG_VSIZE);
		if (reg_vsize > FT800EMU_SCREEN_HEIGHT_MAX) reg_vsize = FT800EMU_SCREEN_HEIGHT_MAX;
		int32_t reg_hsize = m_Memory->rawReadU32(ram, REG_HSIZE);
		if (reg_hsize > FT800EMU_SCREEN_WIDTH_MAX) reg_hsize = FT800EMU_SCREEN_WIDTH_MAX;
		if (!m_Graphics) m_WindowOutput->setMode(reg_hsize, reg_vsize);


		bool renderProcessed = false;
		bool hasChanged;

		// Render lines
		// VBlank=0
#ifdef FT810EMU_MODE
		m_Memory->rawWriteU32(ram, REG_RASTERY, 0);
#endif
		{
			FT8XXEMU::System::switchThread();

			unsigned long procStart = m_System->getMicros();
			if (reg_pclk || renderLineSnapshot)
			{
				// FTEMU_printf("%i != %i\n", lwoc, woc);
				if (ram[REG_DLSWAP] == DLSWAP_FRAME)
				{
					m_Memory->swapDisplayList();
					ram[REG_DLSWAP] = DLSWAP_DONE;
					m_Memory->flagDLSwap();
				}

				int lwoc = m_LastWriteOpCount;
				int woc = m_Memory->getWriteOpCount();
				hasChanged = (lwoc != woc);
				bool hasChanges = hasChanged || m_ChangesSkipped;
				m_LastWriteOpCount = woc;

				if (hasChanges || !m_FrameFullyDrawn)
				{
					bool mirrorHorizontal = /*m_RotateEnabled &&*/ FT800EMU_REG_ROTATE_MIRROR_HORIZONTAL(ram);
					bool mirrorVertical = /*m_RotateEnabled &&*/ FT800EMU_REG_ROTATE_MIRROR_VERTICAL(ram);
					if (renderLineSnapshot)
					{
						if ((ram[REG_SNAPSHOT] & 1) && ram[REG_BUSYBITS]) // TODO_BUSYBITS
						{
							m_Memory->rawWriteU32(ram, REG_SNAPSHOT, 0);
							// Render single line
							int32_t snapy = mirrorVertical ? (reg_vsize - m_Memory->rawReadU32(ram, REG_SNAPY)) : (m_Memory->rawReadU32(ram, REG_SNAPY));
							snapy &= FT800EMU_SCREEN_HEIGHT_MASK;
							// FTEMU_printf("SNAPY: %u\n", snapy);
							argb8888 *buffer = m_GraphicsBuffer ? m_GraphicsBuffer : m_WindowOutput->getBufferARGB8888();
							GraphicsProcessor.process(buffer,
								false, mirrorHorizontal,
#ifdef FT810EMU_MODE
								FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
								reg_hsize, snapy + 1, snapy);
							uint32_t ya = (reg_hsize * snapy);
							uint32_t wa = RAM_COMPOSITE;
							for (int32_t x = 0; x < reg_hsize; ++x)
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
									m_Memory->rawWriteU32(ram, wa, r);
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
									m_Memory->rawWriteU16(ram, wa, r);
									wa += 2;
									break;
								}*/
								case RGB565:
								{
									argb8888 c = buffer[ya + x];
									uint16_t r = (DITHERDIV8(c & 0xFF, (-x), snapy))
										| (DITHERDIV4((c >> 8) & 0xFF, x, snapy) << 5)
										| (DITHERDIV8((c >> 16) & 0xFF, x, (-snapy)) << 11);
									m_Memory->rawWriteU16(ram, wa, r);
									wa += 2;
									break;
								}
								case 0x20:
								default:
#endif
									m_Memory->rawWriteU32(ram, RAM_COMPOSITE + (x * 4), buffer[ya + x]);
#ifdef FT810EMU_MODE
									break;
								}
#endif
							}
							m_Memory->rawWriteU32(ram, REG_BUSYBITS, 0); // TODO_BUSYBITS
						}
					}
					else
					{
						if (m_SkipOn)
						{
							++m_SkipStage;
							m_SkipStage %= 2;
						}
						if (m_SkipOn && m_SkipStage)
						{
							GraphicsProcessor.processBlank();
							m_ChangesSkipped = hasChanges;
						}
						else if (m_DegradeOn)
						{
							GraphicsProcessor.process(m_GraphicsBuffer ? m_GraphicsBuffer : m_WindowOutput->getBufferARGB8888(),
								m_GraphicsBuffer ? mirrorVertical : (mirrorVertical ? !m_WindowOutput->isUpsideDown() : m_WindowOutput->isUpsideDown()), mirrorHorizontal,
#ifdef FT810EMU_MODE
								FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
								reg_hsize, reg_vsize, m_DegradeStage, 2);
							++m_DegradeStage;
							m_DegradeStage %= 2;
							m_ChangesSkipped = false;
							m_FrameFullyDrawn = !hasChanges;
							renderProcessed = true;
						}
						else
						{
							GraphicsProcessor.process(m_GraphicsBuffer ? m_GraphicsBuffer : m_WindowOutput->getBufferARGB8888(),
								m_GraphicsBuffer ? mirrorVertical : (mirrorVertical ? !m_WindowOutput->isUpsideDown() : m_WindowOutput->isUpsideDown()), mirrorHorizontal,
#ifdef FT810EMU_MODE
								FT800EMU_REG_ROTATE_SWAP_XY(ram),
#endif
								reg_hsize, reg_vsize);
							m_ChangesSkipped = false;
							m_FrameFullyDrawn = true;
							renderProcessed = true;
						}
					}
				}
				else
				{
					// GraphicsProcessor.processBlank();
					// FTEMU_printf("no changes\n");
				}
			}
			else
			{
				m_SkipOn = false;
				m_DegradeOn = false;
			}
			unsigned long procDelta = m_System->getMicros() - procStart;
#ifdef BT8XXEMU_PROFILE_FRAMEDELTA
			if (m_ProfileFrameDeltaIndex < BT8XXEMU_PROFILE_FRAMEDELTA)
			{
				m_ProfileFrameDelta[m_ProfileFrameDeltaIndex] = procDelta;
				++m_ProfileFrameDeltaIndex;
				if (m_ProfileFrameDeltaIndex >= BT8XXEMU_PROFILE_FRAMEDELTA)
				{
					FILE *f = fopen("framedelta.csv", "a");
					for (int i = 0; i < BT8XXEMU_PROFILE_FRAMEDELTA; ++i)
						fprintf(f, "%u,", m_ProfileFrameDelta[i]);
					fprintf(f, "\n");
					fflush(f);
					fclose(f);
					printf("Frame delta saved\n");
				}
			}

#endif
			if (!renderLineSnapshot)
			{
				if (m_SkipOn)
				{
					if (!m_SkipStage)
					{
						unsigned long procLowLimit = (unsigned long)(deltaSeconds * 500000.0); // Under 50% of allowed time, turn off frameskip
						if (procDelta < procLowLimit)
						{
							m_SkipOn = false;
							if (FT800EMU_DEBUG) FTEMU_message("process: %i micros (%i ms)", (int)procDelta, (int)procDelta / 1000);
							if (FT800EMU_DEBUG) FTEMU_message("Frame skip switched OFF");
						}
					}
				}
				else if (m_DegradeOn)
				{
					// Note: procLowLimit must be much less than half of procHighLimit, as the render time is halved when dynamic degrade kicks in
					unsigned long procLowLimit = (unsigned long)(deltaSeconds * 250000.0); // Under 25% of allowed time, turn off degrade
					unsigned long procHighLimit = (unsigned long)(deltaSeconds * 800000.0); // Over 80% of allowed time, switch to frameskip
					if (procDelta < procLowLimit)
					{
						m_DegradeOn = false;
						if (FT800EMU_DEBUG) FTEMU_message("process: %i micros (%i ms)", (int)procDelta, (int)procDelta / 1000);
						if (FT800EMU_DEBUG) FTEMU_message("Dynamic degrade switched OFF");
					}
					else if (procDelta > procHighLimit)
					{
						m_SkipOn = true;
						if (FT800EMU_DEBUG) FTEMU_message("process: %i micros (%i ms)", (int)procDelta, (int)procDelta / 1000);
						if (FT800EMU_DEBUG) FTEMU_message("Frame skip switched ON");
					}
				}
				else
				{
					unsigned long procHighLimit = (unsigned long)(deltaSeconds * 800000.0); // Over 80% of allowed time, switch to degrade
					if (procDelta > procHighLimit)
					{
						if (FT800EMU_DEBUG) FTEMU_message("process: %i micros (%i ms)", (int)procDelta, (int)procDelta / 1000);
						if (m_DynamicDegrade)
						{
							m_DegradeOn = true;
							if (FT800EMU_DEBUG) FTEMU_message("Dynamic degrade switched ON");
						}
						else
						{
							m_ThreadMaster.prioritize();
							GraphicsProcessor.setThreadPriority(false);
						}
					}
					else
					{

						m_ThreadMaster.realtime();
						GraphicsProcessor.setThreadPriority(true);
					}
				}
			}
			else
			{
				m_ThreadMaster.realtime();
				GraphicsProcessor.setThreadPriority(true);
			}
		}

		bool hasSwapped;

		{
			int lc = m_LastRealSwapCount;
			int c = m_Memory->getRealSwapCount();
			hasSwapped = (lc != c);
			m_LastRealSwapCount = c;
		}

		m_SwapDLMutex.unlock();

		// Flip buffer and also give a slice of time to the mcu main thread
		// VBlank=1
#ifdef FT810EMU_MODE
		m_Memory->rawWriteU32(ram, REG_RASTERY, 1 << 11);
#endif
		if (!renderLineSnapshot)
		{
			if (reg_pclk) m_Memory->rawWriteU32(ram, REG_FRAMES, m_Memory->rawReadU32(ram, REG_FRAMES) + 1); // Increase REG_FRAMES
			m_ThreadMCU.prioritize();
			m_ThreadMCU.boost(); // Swapped!

			unsigned long flipStart = m_System->getMicros();
			if (m_SkipOn && m_SkipStage)
			{
				// no-op
				// NOTE: Also skips m_Graphics
			}
			else
			{
				if (m_Graphics)
				{
					uint32_t frameFlags = 0;
					if (renderProcessed)
						frameFlags |= BT8XXEMU_FrameBufferChanged;
					if (m_FrameFullyDrawn)
						frameFlags |= BT8XXEMU_FrameBufferComplete;
					if (hasChanged)
						frameFlags |= BT8XXEMU_FrameChanged;
					if (hasSwapped)
						frameFlags |= BT8XXEMU_FrameSwap;
					if (!m_Graphics(reg_pclk != 0, m_GraphicsBuffer, reg_hsize, reg_vsize, (BT8XXEMU_FrameFlags)frameFlags))
					{
						m_CloseCalled = true;
						if (m_Close)
						{
							m_Close();
							return 0;
						}
						else
						{
							FTEMU_warning("Kill MCU thread");
							m_ThreadMCU.kill();
							return 0;
						}
					}
				}
				else
				{
					m_WindowOutput->renderBuffer(reg_pclk != 0, renderProcessed);
					/*
					if (!m_WindowOutput->update())
					{
						// FIXME: 2017-08-09: Need to update mechanism for detecting window close!
						m_CloseCalled = true;
						if (m_Close)
						{
							m_Close();
							return 0;
						}
						else
						{
							FTEMU_printf("Kill MCU thread\n");
							m_ThreadMCU.kill();
							return 0;
						}
					}
					*/
				}
			}
			unsigned long flipDelta = m_System->getMicros() - flipStart;

			if (flipDelta > 8000)
				if (FT800EMU_DEBUG) 
					FTEMU_message("flip: %i micros (%i ms)", (int)flipDelta, (int)flipDelta / 1000);

			FT8XXEMU::System::switchThread(); // ensure slice of time to mcu thread at cost of coprocessor cycles
			m_ThreadMCU.unprioritize();
		}

		m_ThreadCoprocessor.prioritize();
		if (reg_pclk == 0xFF)
		{
			m_System->renderSleep(1);
		}
		else if (reg_pclk)
		{
			//long currentMillis = millis();
			//long millisToWait = targetMillis - currentMillis;
			double currentSeconds = m_System->getSeconds();
			double secondsToWait = targetSeconds - currentSeconds;
			// FTEMU_printf("sleep %f seconds (%f current time, %f target time, %f delta seconds)\n", (float)secondsToWait, (float)currentSeconds, (float)targetSeconds, (float)deltaSeconds);
			//if (millisToWait < -100) targetMillis = millis();
			if (secondsToWait < -0.25) // Skip freeze
			{
				//FTEMU_printf("skip freeze\n");
				targetSeconds = m_System->getSeconds();
			}
			else if (secondsToWait > 0.25)
			{
				FTEMU_warning("Possible problem with REG_FREQUENCY value %u, sw %f, cs %f -> ts %f", m_Memory->rawReadU32(ram, REG_FREQUENCY), (float)secondsToWait, (float)currentSeconds, (float)targetSeconds);
				targetSeconds = m_System->getSeconds() + 0.25;
				secondsToWait = 0.25;
			}

			//FTEMU_printf("millis to wait: %i", (int)millisToWait);

			if (secondsToWait > 0.0)
			{
				m_System->renderSleep((int)(secondsToWait * 1000.0));
			}
		}
		else
		{
			// REG_PCLK is 0
#if FT800EMU_REG_PCLK_ZERO_REDUCE
			targetSeconds = m_System->getSeconds() + 0.02;
			m_System->renderSleep(20);
#else
			targetSeconds = FT8XXEMU::System.getSeconds();
			FT8XXEMU::System.switchThread();
#endif
		}
		m_ThreadCoprocessor.unprioritize();

		m_ThreadMCU.yield(); // don't let the other thread hog cpu
		m_ThreadCoprocessor.yield();
	}

	m_ThreadMaster.reset();
	return 0;
}

int Emulator::mcuThread()
{
	m_ThreadMCU.init();
	if (m_MainPerformance)
	{
		if (!m_BackgroundPerformance)
		{
			m_ThreadMCU.foreground();
		}
		m_ThreadMCU.noboost();
	}
	m_ThreadMCU.unprioritize();
	m_ThreadMCU.setName("FT8XXEMU MCU");

	; {
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_InitCond.notify_one();
	}
	
	m_Main();

	m_ThreadMCU.reset();
	return 0;
}

int Emulator::coprocessorThread()
{
	m_ThreadCoprocessor.init();
	if (!m_BackgroundPerformance)
	{
		m_ThreadCoprocessor.foreground();
	}
	m_ThreadCoprocessor.noboost();
	m_ThreadCoprocessor.unprioritize();
	m_ThreadCoprocessor.setName("FT8XXEMU Coprocessor");

	FTEMU_message("Coprocessor thread begin");
	
	; {
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_InitCond.notify_one();
	}

	m_Coprocessor->executeEmulator();

	FTEMU_message("Coprocessor thread exit");

	m_ThreadCoprocessor.reset();
	return 0;
}

int Emulator::audioThread()
{
#ifdef WIN32
#ifndef FTEMU_SDL2
	HRESULT coInitHR = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
#endif
#endif

	//FTEMU_printf("go sound thread");
	m_ThreadAudio.init();
	if (!m_BackgroundPerformance)
	{
		m_ThreadAudio.foreground();
	}
	m_ThreadAudio.noboost();
	m_ThreadAudio.realtime();
	m_ThreadAudio.setName("FT8XXEMU Audio");

	while (m_MasterRunning)
	{
		//FTEMU_printf("sound thread\n");
		if (m_Flags & BT8XXEMU_EmulatorEnableAudio)
		{
			m_AudioRender->process();
		}
		if (m_Flags & BT8XXEMU_EmulatorEnableKeyboard)
		{
			m_KeyboardInput->update();
			if (m_Flags & BT8XXEMU_EmulatorEnableDebugShortkeys)
			{
				debugShortkeys();
			}
		}
		FT8XXEMU::System::delay(10);
	}

	m_ThreadAudio.reset();

#ifdef WIN32
#ifndef FTEMU_SDL2
	if (coInitHR == S_OK)
		CoUninitialize();
#endif
#endif

	return 0;
}

void Emulator::finalMasterThread(bool sync, int flags)
{
	masterThread(sync);

	m_MasterRunning = false;
	if (flags & BT8XXEMU_EmulatorEnableCoprocessor)
	{
		m_ThreadCoprocessor.reset();
		m_Coprocessor->stopEmulator();
	}

	FTEMU_message("Wait for Coprocessor");
	if (flags & BT8XXEMU_EmulatorEnableCoprocessor)
		m_StdThreadCoprocessor.join();

	if (!m_CloseCalled && m_StdThreadMCU.joinable())
	{
		FTEMU_message("Late kill MCU thread");
		if (m_Close)
		{
			m_Close();
		}
		else
		{
			FTEMU_warning("Kill MCU thread");
			m_ThreadMCU.kill();
		}
	}

	FTEMU_message("Wait for MCU");
	m_StdThreadMCU.join();

	FTEMU_message("Wait for Audio");
	m_StdThreadAudio.join();

	FTEMU_message("Threads finished, cleaning up");

	delete[] m_GraphicsBuffer;
	m_GraphicsBuffer = NULL;
	if ((!m_Graphics) && (flags & BT8XXEMU_EmulatorEnableKeyboard))
	{
		m_KeyboardInput->destroy();
		m_KeyboardInput = NULL;
	}
	if (flags & BT8XXEMU_EmulatorEnableCoprocessor)
	{
		delete m_Coprocessor;
		m_Coprocessor = NULL;
	}
	assert(!m_Coprocessor);
	if (m_Flags & BT8XXEMU_EmulatorEnableAudio)
	{
		m_Memory->setAudioRender(NULL);
		delete m_AudioRender;
		m_AudioRender = NULL;
		m_Memory->setAudioProcessor(NULL);
		delete m_AudioProcessor;
		m_AudioProcessor = NULL;
		m_AudioOutput->destroy();
		m_AudioOutput = NULL;
	}
	if (!m_Graphics) m_WindowOutput->destroy();
	m_WindowOutput = NULL;
	delete m_BusSlave;
	m_BusSlave = NULL;
	GraphicsProcessor.end();
	delete m_Touch;
	m_Touch = NULL;
	delete m_Memory;
	m_Memory = NULL;

	m_EmulatorRunning = false;
	FTEMU_message("Emulator stopped running\n");

#ifdef WIN32
	if (m_CoInit)
		CoUninitialize();
#endif
}

Emulator::Emulator()
{
	m_System = new FT8XXEMU::System();
}

Emulator::~Emulator()
{
	delete m_System;
	m_System = NULL;
}

void Emulator::run(const BT8XXEMU_EmulatorParameters &params)
{
	m_InitMutexGlobal.lock();

#ifdef WIN32
	m_CoInit = (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) == S_OK);
#endif

	BT8XXEMU_EmulatorMode mode = params.Mode;
	if (mode == 0) mode = BT8XXEMU_EmulatorFT800;

#ifdef FT810EMU_MODE
	if (mode < BT8XXEMU_EmulatorFT810)
	{
		FTEMU_error("Invalid emulator version selected, this library is built in FT810 mode");
		m_InitMutexGlobal.unlock();
		return;
	}
#else
	if (mode > BT8XXEMU_EmulatorFT801)
	{
		FTEMU_error("Invalid emulator version selected, this library is built in FT800/FT800 mode");
		m_InitMutexGlobal.unlock();
		return;
	}
#endif

	m_EmulatorRunning = true;

	m_Main = params.Main;
	m_Flags = params.Flags;
	m_Graphics = params.Graphics;
	m_Close = params.Close;
	m_CloseCalled = false;
	m_ExternalFrequency = params.ExternalFrequency;
	m_BackgroundPerformance = (params.Flags & BT8XXEMU_EmulatorEnableBackgroundPerformance) == BT8XXEMU_EmulatorEnableBackgroundPerformance;
	m_MainPerformance = (params.Flags & BT8XXEMU_EmulatorEnableMainPerformance) == BT8XXEMU_EmulatorEnableMainPerformance;

	m_System->onLog(params.Log);
	m_System->setPrintStd((params.Flags & BT8XXEMU_EmulatorEnableStdOut) == BT8XXEMU_EmulatorEnableStdOut);
	m_System->overrideMCUDelay(params.MCUSleep);
	m_Memory = new Memory(m_System, mode, m_SwapDLMutex, m_ThreadMCU, m_ThreadCoprocessor, params.RomFilePath, params.OtpFilePath);
	assert(!m_Touch);
	m_Touch = new Touch(m_System, mode, m_Memory);
	GraphicsProcessor.begin(m_System, m_Memory, m_Touch, m_BackgroundPerformance);
	assert(!m_BusSlave);
	m_BusSlave = new BusSlave(m_System, m_Memory);
	if (!m_Graphics)
	{
		m_WindowOutput = FT8XXEMU::WindowOutput::create(m_System);
		m_WindowOutput->onSetTouchScreenXY([this](int idx, int x, int y, int pressure) -> void {
			m_Touch->touch(idx).setXY(x, y, pressure);
		});
		m_WindowOutput->onResetTouchScreenXY([this](int idx) -> void {
			m_Touch->touch(idx).resetXY();
		});
	}
	if (params.Flags & BT8XXEMU_EmulatorEnableAudio)
	{
		assert(!m_AudioOutput);
		m_AudioOutput = FT8XXEMU::AudioOutput::create(m_System);
		assert(!m_AudioProcessor);
		m_AudioProcessor = new AudioProcessor();
		m_Memory->setAudioProcessor(m_AudioProcessor);
		m_AudioRender = new AudioRender(m_AudioOutput, m_Memory, m_AudioProcessor);
		m_Memory->setAudioRender(m_AudioRender);
		/*
		// TODO: 2017-08-09: Output creation failure manage
		if (!FT8XXEMU::AudioOutput.begin())
		{
			AudioRender.end();
			AudioProcessor.end();
			m_Flags &= ~BT8XXEMU_EmulatorEnableAudio;
		}
		*/
	}
	if (params.Flags & BT8XXEMU_EmulatorEnableCoprocessor)
	{
		assert(!m_Coprocessor);
		m_Coprocessor = new Coprocessor(m_System, m_Memory,
			params.CoprocessorRomFilePath ? NULL : params.CoprocessorRomFilePath,
			mode);
	}
	if ((!m_Graphics) && (params.Flags & BT8XXEMU_EmulatorEnableKeyboard))
	{
		m_KeyboardInput = FT8XXEMU::KeyboardInput::create(m_System, m_WindowOutput);
	}
	if (m_Graphics) m_Flags &= ~BT8XXEMU_EmulatorEnableKeyboard;

	if (m_Graphics)
	{
		m_GraphicsBuffer = new argb8888[FT800EMU_SCREEN_WIDTH_MAX * FT800EMU_SCREEN_HEIGHT_MAX];
		memset(m_GraphicsBuffer, 0, FT800EMU_SCREEN_WIDTH_MAX * FT800EMU_SCREEN_HEIGHT_MAX * sizeof(argb8888));
	}

	if (!m_Graphics) m_WindowOutput->enableMouse((params.Flags & BT8XXEMU_EmulatorEnableMouse) == BT8XXEMU_EmulatorEnableMouse);
	if (m_Graphics) m_Flags &= ~BT8XXEMU_EmulatorEnableMouse;
	m_Memory->enableReadDelay();

	if (params.Flags & BT8XXEMU_EmulatorEnableGraphicsMultithread)
	{
		GraphicsProcessor.enableMultithread();
		GraphicsProcessor.reduceThreads(params.ReduceGraphicsThreads);
	}
	if (params.Flags & BT8XXEMU_EmulatorEnableRegPwmDutyEmulation) GraphicsProcessor.enableRegPwmDutyEmulation();

	m_DynamicDegrade = (params.Flags & BT8XXEMU_EmulatorEnableDynamicDegrade) == BT8XXEMU_EmulatorEnableDynamicDegrade;
	// m_RotateEnabled = (params.Flags & BT8XXEMU_EmulatorEnableRegRotate) == BT8XXEMU_EmulatorEnableRegRotate;
	m_Touch->enableTouchMatrix((params.Flags & BT8XXEMU_EmulatorEnableTouchTransformation) == BT8XXEMU_EmulatorEnableTouchTransformation);

	m_MasterRunning = true;

	m_StdThreadAudio = std::thread(&Emulator::audioThread, this);

	; {
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_StdThreadCoprocessor = std::thread(&Emulator::coprocessorThread, this);
		m_InitCond.wait(lock); // FIXME: 2017-08-09: conditional_variable are subject to random wake-ups...
	}

	if (params.Main)
	{
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_StdThreadMCU = std::thread(&Emulator::mcuThread, this);
		m_InitCond.wait(lock);
	}
	else
	{
		// Asynchronous run function, calling thread is MCU
		if (m_MainPerformance && !m_BackgroundPerformance)
		{
			// Different behaviour for MainPerformance option from when MCU is threaded
			m_ThreadMCU.init();
			m_ThreadMCU.foreground();
			m_ThreadMCU.noboost();
			m_ThreadMCU.unprioritize();
		}
		std::unique_lock<std::mutex> lock(m_InitMutex);
		m_StdThreadMaster = std::thread(&Emulator::finalMasterThread, this, false, params.Flags);
		m_InitCond.wait(lock);
	}

	m_InitMutexGlobal.unlock();

	if (params.Main)
	{
		// Synchronous run function, calling thread is graphics
		finalMasterThread(true, params.Flags);
	}
}

void Emulator::stop()
{
	m_InitMutexGlobal.lock();

	m_MasterRunning = false;

	m_InitMutexGlobal.unlock();

	if (!m_ThreadMCU.current())
	{
		FTEMU_message("Wait for MCU thread");
		while (m_EmulatorRunning) // TODO: Mutex?
		{
			FT8XXEMU::System::delay(1);
		}
	}

	if (m_MainPerformance && !m_BackgroundPerformance && !m_Main && m_ThreadMCU.current())
	{
		m_ThreadMCU.reset();
	}

	FTEMU_message("Stop ok\n");
}


uint8_t Emulator::transfer(uint8_t data)
{
	return m_BusSlave->transfer(data);
}

void Emulator::cs(bool cs)
{
	m_BusSlave->cs(cs);
}

bool Emulator::hasInterrupt()
{
	return m_Memory->hasInterrupt();
}

uint8_t *Emulator::getRam()
{
	return m_Memory->getRam();
}

// uint8_t *Emulator::getFlash();

const uint32_t *Emulator::getDisplayList()
{
	return m_Memory->getDisplayList();
}

void Emulator::poke()
{
	m_Memory->poke();
}

int *Emulator::getDisplayListCoprocessorWrites()
{
	return m_Memory->getDisplayListCoprocessorWrites();
}

void Emulator::clearDisplayListCoprocessorWrites()
{
	m_Memory->clearDisplayListCoprocessorWrites();
}

} /* namespace FT800EMU */

/* end of file */
