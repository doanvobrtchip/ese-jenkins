/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FT800EMU_EMULATOR_H
#define FT800EMU_EMULATOR_H
// #include <...>

// Number of frame times to include in profiling log
#ifndef BT8XXEMU_PROFILE_FRAMEDELTA
// #define BT8XXEMU_PROFILE_FRAMEDELTA (60 * 20)
#endif

// System includes
#include <string>
#include <condition_variable>
#include <mutex>

// Project includes (include standard stuff for user)
#include "bt8xxemu_pp.h"
#include "ft8xxemu_thread_state.h"

#ifndef BT8XXEMU_EMULATOR_H
#define BT8XXEMU_EMULATOR_H

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
class Emulator : public BT8XXEMU::Emulator
{
public:
	Emulator();
	~Emulator();

	void run(const BT8XXEMU_EmulatorParameters &params);
	virtual void stop() override;
	virtual bool isRunning() override;

	virtual uint8_t transfer(uint8_t data) override;
	virtual void cs(bool cs) override;
	virtual bool hasInterrupt() override;

	virtual void touchSetXY(int idx, int x, int y, int pressure);
	virtual void touchResetXY(int idx);

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

	void(*m_Main)(BT8XXEMU_Emulator *sender, void *context) = NULL;
	void(*m_Keyboard)(BT8XXEMU_Emulator *sender, void *context) = NULL;
	void(*m_Close)(BT8XXEMU_Emulator *sender, void *context) = NULL;
	int m_Flags = 0;
	volatile bool m_MasterRunning = false;
	bool m_DynamicDegrade = false;
	uint32_t m_ExternalFrequency = 0;
	void *m_UserContext;

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

	bool m_KeySetDebugMode = false;
	bool m_KeyIncDebugMultiplier = false;
	bool m_KeyDecDebugMultiplier = false;
	bool m_KeyResetDebugMultiplier = false;
	bool m_KeyIncDebugLimiter = false;
	bool m_KeyDecDebugLimiter = false;
	bool m_KeyResetDebugLimiter = false;

private:
	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

}; /* class Emulator */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
