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

// Enable or disable debug messages
#define FT800EMU_DEBUG 0

// System includes
#include <thread>

// Project includes
#include "ft8xxemu_system.h"
#include "ft8xxemu_keyboard_input.h"
#include "ft8xxemu_keyboard_keys.h"
#include "ft8xxemu_window_output.h"

#include "ft800emu_memory.h"
#include "ft800emu_touch.h"
#include "ft800emu_graphics_processor.h"

#include "ft800emu_vc.h"

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
							m_GraphicsProcessor->process(buffer,
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
							m_GraphicsProcessor->processBlank();
							m_ChangesSkipped = hasChanges;
						}
						else if (m_DegradeOn)
						{
							m_GraphicsProcessor->process(m_GraphicsBuffer ? m_GraphicsBuffer : m_WindowOutput->getBufferARGB8888(),
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
							m_GraphicsProcessor->process(m_GraphicsBuffer ? m_GraphicsBuffer : m_WindowOutput->getBufferARGB8888(),
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
					// m_GraphicsProcessor->processBlank();
					// FTEMU_printf("no changes\n");
				}
			}
			else
			{
				m_SkipOn = false;
				m_DegradeOn = false;
				hasChanged = false; // Inaccurate
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
							m_GraphicsProcessor->setThreadPriority(false);
						}
					}
					else
					{

						m_ThreadMaster.realtime();
						m_GraphicsProcessor->setThreadPriority(true);
					}
				}
			}
			else
			{
				m_ThreadMaster.realtime();
				m_GraphicsProcessor->setThreadPriority(true);
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
						FTEMU_message("Graphics output closed");
						m_CloseCalled = true;
						if (m_Close)
						{
							m_Close();
							return 0;
						}
						else if (sync)
						{
							FTEMU_warning("Kill controlled MCU thread, no Close callback");
							m_ThreadMCU.kill();
							return 0;
						}
						else
						{
							return 0;
						}
					}
				}
				else
				{
					if (!m_WindowOutput->renderBuffer(reg_pclk != 0, renderProcessed))
					{
						FTEMU_message("Window output closed");
						m_CloseCalled = true;
						if (m_Close)
						{
							m_Close();
							return 0;
						}
						else if (sync)
						{
							FTEMU_warning("Kill controlled MCU thread, no Close callback");
							m_ThreadMCU.kill();
							return 0;
						}
						else
						{
							return 0;
						}
					}
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


void Emulator::debugShortkeys()
{
	{
		if (m_KeySetDebugMode)
		{
			m_KeySetDebugMode = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F3);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F3))
		{
			m_GraphicsProcessor->setDebugMode((m_GraphicsProcessor->getDebugMode() + 1) % FT800EMU_DEBUGMODE_COUNT);
			m_KeySetDebugMode = true;
		}
	}

	{
		if (m_KeyIncDebugMultiplier)
		{
			m_KeyIncDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADPLUS);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADPLUS))
		{
			if (m_GraphicsProcessor->getDebugMode())
				m_GraphicsProcessor->setDebugMultiplier(m_GraphicsProcessor->getDebugMultiplier() + 1);
			m_KeyIncDebugMultiplier = true;
		}
	}

	{
		static bool m_KeyDecDebugMultiplier = false;
		if (m_KeyDecDebugMultiplier)
		{
			m_KeyDecDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADMINUS);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADMINUS))
		{
			if (m_GraphicsProcessor->getDebugMode())
				m_GraphicsProcessor->setDebugMultiplier(max(m_GraphicsProcessor->getDebugMultiplier() - 1, 1));
			m_KeyDecDebugMultiplier = true;
		}
	}

	{
		static bool m_KeyResetDebugMultiplier = false;
		if (m_KeyResetDebugMultiplier)
		{
			m_KeyResetDebugMultiplier = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADSLASH);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_NUMPADSLASH))
		{
			if (m_GraphicsProcessor->getDebugMode())
				m_GraphicsProcessor->setDebugMultiplier(1);
			m_KeyResetDebugMultiplier = true;
		}
	}

	{
		if (m_KeyIncDebugLimiter)
		{
			m_KeyIncDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F8);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F8))
		{
			m_GraphicsProcessor->setDebugLimiter(m_GraphicsProcessor->getDebugLimiter() + 1);
			m_KeyIncDebugLimiter = true;
		}
	}

	{
		if (m_KeyDecDebugLimiter)
		{
			m_KeyDecDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F7);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F7))
		{
			m_GraphicsProcessor->setDebugLimiter(max(m_GraphicsProcessor->getDebugLimiter() - 1, 0));
			m_KeyDecDebugLimiter = true;
		}
	}

	{
		if (m_KeyResetDebugLimiter)
		{
			m_KeyResetDebugLimiter = m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F6);
		}
		else if (m_KeyboardInput->isKeyDown(BT8XXEMU_KEY_F6))
		{
			m_GraphicsProcessor->setDebugLimiter(0);
			m_KeyResetDebugLimiter = true;
		}
	}
}

} /* namespace FT800EMU */

/* end of file */
