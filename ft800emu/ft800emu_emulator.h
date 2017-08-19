/**
 * Emulator
 * $Id$
 * \file ft800emu_emulator.h
 * \brief Emulator
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_EMULATOR_H
#define FT800EMU_EMULATOR_H
// #include <...>

#define BT8XXEMU_PROFILE_FRAMEDELTA (60 * 20)

// System includes
#include <string>
#include <condition_variable>
#include <mutex>

// Project includes (include standard stuff for user)
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"
#include "ft8xxemu_thread_state.h"

#ifndef BT8XXEMU_EMULATOR_H
#define BT8XXEMU_EMULATOR_H
namespace BT8XXEMU {

/**
* IEmulator
* \brief IEmulator
* \date 2011-08-04
* \author Jan Boon (Kaetemi)
*/
class IEmulator
{
public:
	virtual void stop() = 0;

	virtual uint8_t transfer(uint8_t data) = 0;
	virtual void cs(bool cs) = 0;
	virtual bool hasInterrupt() = 0;

	/*
	virtual void touchSetXY(int idx, int x, int y, int pressure) = 0;
	virtual void touchResetXY(int idx) = 0;
	*/

	virtual uint8_t *getRam() = 0;
	// virtual uint8_t *getFlash() = 0;
	virtual const uint32_t *getDisplayList() = 0;
	virtual void poke() = 0;
	virtual int *getDisplayListCoprocessorWrites() = 0;
	virtual void clearDisplayListCoprocessorWrites() = 0;

	virtual bool getDebugLimiterEffective() = 0;
	virtual int getDebugLimiterIndex() = 0;
	virtual void setDebugLimiter(int debugLimiter) = 0;
	virtual void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = 0;

};

}
#endif

namespace FT8XXEMU {
	class WindowOutput;
	class AudioOutput;
	class KeyboardInput;
}

namespace FT800EMU {
	class Memory;
	class Touch;
	class GraphicsProcessor;
	class BusSlave;
	class AudioProcessor;
	class AudioRender;
	class Coprocessor;

/**
 * Emulator
 * \brief Emulator
 * \date 2011-05-29 19:54GMT
 * \author Jan Boon (Kaetemi)
 */
class Emulator : public BT8XXEMU::IEmulator
{
public:
	Emulator();
	~Emulator();

	void run(const BT8XXEMU_EmulatorParameters &params);
	virtual void stop() override;

	virtual uint8_t transfer(uint8_t data) override;
	virtual void cs(bool cs) override;
	virtual bool hasInterrupt() override;

	virtual uint8_t *getRam() override;
	// virtual uint8_t *getFlash() override;
	virtual const uint32_t *getDisplayList() override;
	virtual void poke() override;
	virtual int *getDisplayListCoprocessorWrites() override;
	virtual void clearDisplayListCoprocessorWrites() override;

	virtual bool getDebugLimiterEffective() override;
	virtual int getDebugLimiterIndex() override;
	virtual void setDebugLimiter(int debugLimiter)override;
	virtual void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) override;

private:
	int masterThread(bool sync);
	int mcuThread();
	int coprocessorThread();
	int audioThread();

	void finalMasterThread(bool sync, int flags);

private:
	void debugShortkeys();

private:
	volatile bool m_EmulatorRunning = false;

	void(*m_Main)() = NULL;
	void(*m_Keyboard)() = NULL;
	void(*m_Close)() = NULL;
	int m_Flags = 0;
	volatile bool m_MasterRunning = false;
	bool m_DynamicDegrade = false;
	uint32_t m_ExternalFrequency = 0;

	bool m_DegradeOn = false;
	int m_DegradeStage = 0;

	bool m_SkipOn = false;
	int m_SkipStage = 0;
	volatile bool m_CloseCalled = false;

	// bool m_RotateEnabled = false;

	int(*m_Graphics)(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags) = NULL;
	argb8888 *m_GraphicsBuffer = NULL;

	int m_LastWriteOpCount = 0;
	int m_LastRealSwapCount = 0;
	bool m_FrameFullyDrawn = true;
	bool m_ChangesSkipped = false;

	std::condition_variable m_InitCond;
	std::mutex m_InitMutex;
	std::mutex m_InitMutexGlobal;

	FT8XXEMU::ThreadState m_ThreadMaster; //< Graphics master thread
	FT8XXEMU::ThreadState m_ThreadMCU; //< User MCU thread (optional)
	FT8XXEMU::ThreadState m_ThreadCoprocessor; //< Coprocessor thread
	FT8XXEMU::ThreadState m_ThreadAudio; //< Audio thread

	std::thread m_StdThreadMaster;
	std::thread m_StdThreadMCU;
	std::thread m_StdThreadCoprocessor;
	std::thread m_StdThreadAudio;

	bool m_BackgroundPerformance;
	bool m_MainPerformance;

#ifdef WIN32
	bool m_CoInit = false;
#endif

	std::mutex m_SwapDLMutex;

	FT8XXEMU::System *m_System = NULL;
	FT8XXEMU::WindowOutput *m_WindowOutput = NULL;
	FT8XXEMU::AudioOutput *m_AudioOutput = NULL;
	FT8XXEMU::KeyboardInput *m_KeyboardInput = NULL;

	Memory *m_Memory = NULL;
	Touch *m_Touch = NULL;
	GraphicsProcessor *m_GraphicsProcessor = NULL;
	BusSlave *m_BusSlave = NULL;
	AudioProcessor *m_AudioProcessor = NULL;
	AudioRender *m_AudioRender = NULL;
	Coprocessor *m_Coprocessor = NULL;

#ifdef BT8XXEMU_PROFILE_FRAMEDELTA
	uint32_t m_ProfileFrameDelta[BT8XXEMU_PROFILE_FRAMEDELTA] = { 0 };
	int m_ProfileFrameDeltaIndex = 0;
#endif

private:
	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

}; /* class Emulator */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
