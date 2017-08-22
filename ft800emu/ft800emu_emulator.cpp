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

// Enable or disable debug messages
#define FT800EMU_DEBUG 0

namespace FT800EMU {

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
	m_Memory->setGraphicsProcessor(NULL);
	delete m_GraphicsProcessor;
	m_GraphicsProcessor = NULL;
	m_Memory->setTouch(NULL);
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
	m_Memory->setTouch(m_Touch);
	assert(!m_GraphicsProcessor);
	m_GraphicsProcessor = new GraphicsProcessor(m_System, m_Memory, m_Touch, m_BackgroundPerformance);
	m_Memory->setGraphicsProcessor(m_GraphicsProcessor);
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
		m_GraphicsProcessor->enableMultithread();
		m_GraphicsProcessor->reduceThreads(params.ReduceGraphicsThreads);
	}
	if (params.Flags & BT8XXEMU_EmulatorEnableRegPwmDutyEmulation) m_GraphicsProcessor->enableRegPwmDutyEmulation();

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

void Emulator::touchSetXY(int idx, int x, int y, int pressure)
{
	m_Touch->touch(idx).setXY(x, y, pressure);
}

void Emulator::touchResetXY(int idx)
{
	m_Touch->touch(idx).resetXY();
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

bool Emulator::getDebugLimiterEffective()
{
	return m_GraphicsProcessor->getDebugLimiterEffective();
}

int Emulator::getDebugLimiterIndex()
{
	return m_GraphicsProcessor->getDebugLimiterIndex();
}

void Emulator::setDebugLimiter(int debugLimiter)
{
	m_GraphicsProcessor->setDebugLimiter(debugLimiter);
}

void Emulator::processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize)
{
	m_GraphicsProcessor->processTrace(result, size, x, y, hsize);
}

} /* namespace FT800EMU */

/* end of file */
