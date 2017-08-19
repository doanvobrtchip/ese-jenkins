/*
 * FT8XX Emulator Library
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#ifndef BT8XXEMU_H
#define BT8XXEMU_H

#include "ft8xxemu_inttypes.h"

// API version is increased whenever BT8XXEMU_EmulatorParameters format changes or functions are modified
#define BT8XXEMU_VERSION_API 11

#ifndef BT8XXEMU_STATIC
#	ifdef BT8XXEMU_EXPORT_DYNAMIC
#		ifdef WIN32
#			define BT8XXEMU_API __declspec(dllexport)
#		else
#			define BT8XXEMU_API
#		endif
#	else
#		ifdef WIN32
#			define BT8XXEMU_API __declspec(dllimport)
#		else
#			define BT8XXEMU_API
#		endif
#	endif
#else
#	define BT8XXEMU_API
#endif

typedef enum
{
	BT8XXEMU_LogError = 0,
	BT8XXEMU_LogWarning = 1,
	BT8XXEMU_LogMessage = 2,
} BT8XXEMU_LogType;

typedef enum
{
	BT8XXEMU_EmulatorFT800 = 0x0800,
	BT8XXEMU_EmulatorFT801 = 0x0801,
	BT8XXEMU_EmulatorFT810 = 0x0810,
	BT8XXEMU_EmulatorFT811 = 0x0811,
	BT8XXEMU_EmulatorFT812 = 0x0812,
	BT8XXEMU_EmulatorFT813 = 0x0813,
	// BT8XXEMU_EmulatorBT815 = 0x0815,
} BT8XXEMU_EmulatorMode;

typedef enum
{
	// enables the keyboard to be used as input (default: on)
	BT8XXEMU_EmulatorEnableKeyboard = 0x01,
	// enables audio (default: on)
	BT8XXEMU_EmulatorEnableAudio = 0x02,
	// enables coprocessor (default: on)
	BT8XXEMU_EmulatorEnableCoprocessor = 0x04,
	// enables mouse as touch (default: on)
	BT8XXEMU_EmulatorEnableMouse = 0x08,
	// enable debug shortkeys (default: on)
	BT8XXEMU_EmulatorEnableDebugShortkeys = 0x10,
	// enable graphics processor multithreading (default: on)
	BT8XXEMU_EmulatorEnableGraphicsMultithread = 0x20,
	// enable dynamic graphics quality degrading by reducing resolution and dropping frames (default: on)
	BT8XXEMU_EmulatorEnableDynamicDegrade = 0x40,
	// enable usage of REG_ROTATE (default: off)
	// BT8XXEMU_EmulatorEnableRegRotate = 0x80, // Now always on
	// enable emulating REG_PWM_DUTY by fading the rendered display to black (default: off)
	BT8XXEMU_EmulatorEnableRegPwmDutyEmulation = 0x100,
	// enable usage of touch transformation matrix (default: on) (should be disabled in editor)
	BT8XXEMU_EmulatorEnableTouchTransformation = 0x200,
	// enable output to stdout from the emulator (default: off) (note: stdout is is some cases not thread safe)
	BT8XXEMU_EmulatorEnableStdOut = 0x400,
	// enable performance adjustments for running the emulator as a background process without window (default: off)
	BT8XXEMU_EmulatorEnableBackgroundPerformance = 0x800,
	// enable performance adjustments for the main MCU thread (default: on)
	BT8XXEMU_EmulatorEnableMainPerformance = 0x1000,

} BT8XXEMU_EmulatorFlags;

typedef enum
{
	// frame render has changes since last render
	BT8XXEMU_FrameBufferChanged = 0x01,
	// frame is completely rendered (without degrade)
	BT8XXEMU_FrameBufferComplete = 0x02,
	// frame has changes since last render
	BT8XXEMU_FrameChanged = 0x04,
	// frame rendered right after display list swap
	BT8XXEMU_FrameSwap = 0x08,

	// NOTE: Difference between FrameChanged and FrameBufferChanged is that
	// FrameChanged will only be true if the content of the frame changed,
	// whereas FrameBufferChanged will be true if the rendered buffer changed.
	// For example, when the emulator renders a frame incompletely due to
	// CPU overload, it will then finish the frame in the next callback,
	// and when this is the same frame, FrameChanged will be false,
	// but FrameBufferChanged will be true as the buffer has changed.

	// NOTE: Frames can change even though no frame was swapped, due to
	// several parameters such as REG_MACRO or REG_ROTATE.

	// NOTE: If you only want completely rendered frames, turn OFF
	// the EmulatorEnableDynamicDegrade feature.

	// NOTE: To get the accurate frame after a frame swap, wait for FrameSwap
	// to be set, and get the first frame which has FrameBufferComplete set.

	// NOTE: To get the accurate frame after any frame change, wait for
	// FrameChanged, and get the first frame which has FrameBufferComplete set.

} BT8XXEMU_FrameFlags;

typedef struct
{
	// Microcontroller main function. When not provided the calling thread is assumed to be the MCU thread
	void(*Main)();
	// See EmulatorFlags.
	int Flags;
	// Emulator mode
	BT8XXEMU_EmulatorMode Mode;

	// The default mouse pressure, default 0 (maximum).
	// See REG_TOUCH_RZTRESH, etc.
	uint32_t MousePressure;
	// External frequency. See CLK, etc.
	uint32_t ExternalFrequency;

	// Reduce graphics processor threads by specified number, default 0
	// Necessary when doing very heavy work on the MCU or Coprocessor
	uint32_t ReduceGraphicsThreads;

	// Sleep function for MCU thread usage throttle. Defaults to generic system sleep
	void(*MCUSleep)(void *sender, void *context, int ms);

	// Replaces the default builtin ROM with a custom ROM from a file.
	// NOTE: String is copied and may be deallocated after call to run(...)
	char *RomFilePath;
	// Replaces the default builtin OTP with a custom OTP from a file.
	// NOTE: String is copied and may be deallocated after call to run(...)
	char *OtpFilePath;
	// Replaces the builtin coprocessor ROM.
	// NOTE: String is copied and may be deallocated after call to run(...)
	char *CoprocessorRomFilePath;

	// Graphics driverless mode
	// Setting this callback means no window will be created, and all
	// rendered graphics will be automatically sent to this function.
	// For enabling touch functionality, the functions
	// Memory.setTouchScreenXY and Memory.resetTouchScreenXY must be
	// called manually from the host application.
	// Builtin keyboard functionality is not supported and must be
	// implemented manually when using this mode.
	// The output parameter is false (0) when the display is turned off.
	// The contents of the buffer pointer are undefined after this
	// function returns. Create a copy to use it on another thread.
	// Return false (0) when the application must exit, otherwise return true (1).
	int(*Graphics)(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags);

	// Interrupt handler
	// void (*Interrupt)();

	// Log callback
	void(*Log)(void *sender, void *context, BT8XXEMU_LogType type, const char *message);

	// Safe exit. Called when the emulator window is closed
	void(*Close)();

} BT8XXEMU_EmulatorParameters;

#ifdef __cplusplus 
extern "C" {
#endif

//////////
// INIT //
//////////

// Return version information
BT8XXEMU_API const char *BT8XXEMU_version();

// Initialize the default emulator parameters
BT8XXEMU_API void BT8XXEMU_defaults(uint32_t versionApi, BT8XXEMU_EmulatorParameters *params, BT8XXEMU_EmulatorMode mode);

// Run the emulator on the current thread. Returns when the emulator is fully stopped. Parameter versionApi must be set to BT8XXEMU_VERSION_API
BT8XXEMU_API void BT8XXEMU_run(uint32_t versionApi, const BT8XXEMU_EmulatorParameters *params);

// Stop the emulator. Can be called from any thread. Returns when the emulator has fully stopped
BT8XXEMU_API void BT8XXEMU_stop();

/////////////
// RUNTIME //
/////////////

// Transfer data over the imaginary SPI bus. Call from the MCU thread (from the setup/loop callbacks). See FT8XX documentation for SPI transfer protocol
BT8XXEMU_API extern uint8_t BT8XXEMU_transfer(uint8_t data);

// Set cable select. Must be set to 1 to start data transfer, 0 to end. See FT8XX documentation for CS_N
BT8XXEMU_API extern void BT8XXEMU_cs(int cs);

// Returns 1 if there is an interrupt flag set. Depends on mask. See FT8XX documentation for INT_N
BT8XXEMU_API extern int BT8XXEMU_hasInterrupt();

//////////////
// ADVANCED //
//////////////

// Set touch XY. Param idx 0..4. Call on every frame during mouse down or touch when using custom graphics output
BT8XXEMU_API void BT8XXEMU_touchSetXY(int idx, int x, int y, int pressure);

// Reset touch XY. Call once no longer touching when using custom graphics output
BT8XXEMU_API void BT8XXEMU_touchResetXY(int idx);

#ifdef __cplusplus 
} /* extern "C" */
#endif

#define FT8XXEMU_VERSION_API BT8XXEMU_VERSION_API
#define FT8XXEMU_VERSION_API BT8XXEMU_VERSION_API
#define FT8XXEMU_EmulatorFT800 BT8XXEMU_EmulatorFT800
#define FT8XXEMU_EmulatorFT801 BT8XXEMU_EmulatorFT801
#define FT8XXEMU_EmulatorFT810 BT8XXEMU_EmulatorFT810
#define FT8XXEMU_EmulatorFT811 BT8XXEMU_EmulatorFT811
#define FT8XXEMU_EmulatorFT812 BT8XXEMU_EmulatorFT812
#define FT8XXEMU_EmulatorFT813 BT8XXEMU_EmulatorFT813
#define FT8XXEMU_EmulatorEnableKeyboard BT8XXEMU_EmulatorEnableKeyboard
#define FT8XXEMU_EmulatorEnableAudio BT8XXEMU_EmulatorEnableAudio
#define FT8XXEMU_EmulatorEnableCoprocessor BT8XXEMU_EmulatorEnableCoprocessor
#define FT8XXEMU_EmulatorEnableMouse BT8XXEMU_EmulatorEnableMouse
#define FT8XXEMU_EmulatorEnableDebugShortkeys BT8XXEMU_EmulatorEnableDebugShortkeys
#define FT8XXEMU_EmulatorEnableGraphicsMultithread BT8XXEMU_EmulatorEnableGraphicsMultithread
#define FT8XXEMU_EmulatorEnableDynamicDegrade BT8XXEMU_EmulatorEnableDynamicDegrade
// #define FT8XXEMU_EmulatorEnableRegRotate BT8XXEMU_EmulatorEnableRegRotate
#define FT8XXEMU_EmulatorEnableRegPwmDutyEmulation BT8XXEMU_EmulatorEnableRegPwmDutyEmulation
#define FT8XXEMU_EmulatorEnableTouchTransformation BT8XXEMU_EmulatorEnableTouchTransformation
#define FT8XXEMU_EmulatorEnableTouchTransformation BT8XXEMU_EmulatorEnableTouchTransformation
#define FT8XXEMU_EmulatorEnableStdOut BT8XXEMU_EmulatorEnableStdOut
#define FT8XXEMU_EmulatorFlags BT8XXEMU_EmulatorFlags
#define FT8XXEMU_FrameBufferChanged BT8XXEMU_FrameBufferChanged
#define FT8XXEMU_FrameBufferComplete BT8XXEMU_FrameBufferComplete
#define FT8XXEMU_FrameChanged BT8XXEMU_FrameChanged
#define FT8XXEMU_FrameSwap BT8XXEMU_FrameSwap
#define FT8XXEMU_FrameFlags BT8XXEMU_FrameFlags
#define FT8XXEMU_EmulatorParameters BT8XXEMU_EmulatorParameters
#define FT8XXEMU_version BT8XXEMU_version
#define FT8XXEMU_defaults BT8XXEMU_defaults
#define FT8XXEMU_run BT8XXEMU_run
#define FT8XXEMU_stop BT8XXEMU_stop
#define FT8XXEMU_transfer BT8XXEMU_transfer
#define FT8XXEMU_cs BT8XXEMU_cs
#define FT8XXEMU_int BT8XXEMU_int
#define FT8XXEMU_touchSetXY BT8XXEMU_touchSetXY
#define FT8XXEMU_touchResetXY BT8XXEMU_touchResetXY

#endif /* #ifndef BT8XXEMU_H */

/* end of file */
