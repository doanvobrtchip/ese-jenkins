/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon (jan.boon@kaetemi.be)
 */

#ifndef FT8XXEMU_H
#define FT8XXEMU_H

#include "ft8xxemu_inttypes.h"

// API version is increased whenever FT8XXEMU_EmulatorParameters format changes
#define FT8XXEMU_VERSION_API 1

#ifndef FT8XXEMU_STATIC
#	ifdef FT8XXEMU_EXPORT_DYNAMIC
#		ifdef WIN32
#			define FT8XXEMU_API __declspec(dllexport)
#		else
#			define FT8XXEMU_API
#		endif
#	else
#		ifdef WIN32
#			define FT8XXEMU_API __declspec(dllimport)
#		else
#			define FT8XXEMU_API
#		endif
#	endif
#else
#	define FT8XXEMU_API
#endif

typedef enum
{
	FT8XXEMU_EmulatorFT800 = 800,
	FT8XXEMU_EmulatorFT801 = 801,
	FT8XXEMU_EmulatorFT810 = 810,
} FT8XXEMU_EmulatorMode;

typedef enum
{
	// enables the keyboard to be used as input
	FT8XXEMU_EmulatorEnableKeyboard = 0x01,
	// enables audio
	FT8XXEMU_EmulatorEnableAudio = 0x02,
	// enables coprocessor
	FT8XXEMU_EmulatorEnableCoprocessor = 0x04,
	// enables mouse as touch
	FT8XXEMU_EmulatorEnableMouse = 0x08,
	// enable debug shortkeys
	FT8XXEMU_EmulatorEnableDebugShortkeys = 0x10,
	// enable graphics processor multithreading
	FT8XXEMU_EmulatorEnableGraphicsMultithread = 0x20,
	// enable dynamic graphics quality degrading by interlacing
	FT8XXEMU_EmulatorEnableDynamicDegrade = 0x40,
	// enable usage of REG_ROTATE
	FT8XXEMU_EmulatorEnableRegRotate = 0x80,
	// enable emulating REG_PWM_DUTY by fading the rendered display to black
	FT8XXEMU_EmulatorEnableRegPwmDutyEmulation = 0x100,
} FT8XXEMU_EmulatorFlags;

typedef enum
{
	// frame render has changes since last render
	FT8XXEMU_FrameBufferChanged = 0x01,
	// frame is completely rendered (without degrade)
	FT8XXEMU_FrameBufferComplete = 0x02,
	// frame has changes since last render
	FT8XXEMU_FrameChanged = 0x04,
	// frame rendered right after display list swap
	FT8XXEMU_FrameSwap = 0x08,

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
} FT8XXEMU_FrameFlags;

typedef struct
{
	// Microcontroller function called before loop.
	void (*Setup)();
	// Microcontroller continuous loop.
	void (*Loop)();
	// See EmulatorFlags.
	int Flags;
	// Emulator mode
	FT8XXEMU_EmulatorMode Mode;

	// Called after keyboard update.
	// Supplied function can use Keyboard.isKeyDown(FT8XXEMU_KEY_F3)
	// or FT8XXEMU_isKeyDown(FT8XXEMU_KEY_F3) functions.
	void (*Keyboard)();
	// The default mouse pressure, default 0 (maximum).
	// See REG_TOUCH_RZTRESH, etc.
	uint32_t MousePressure;
	// External frequency. See CLK, etc.
	uint32_t ExternalFrequency;

	// Reduce graphics processor threads by specified number, default 0
	// Necessary when doing very heavy work on the MCU or Coprocessor
	uint32_t ReduceGraphicsThreads;

	// Replaces the default builtin ROM with a custom ROM from a file.
	// NOTE: String is copied and may be deallocated after call to run(...)
	char *RomFilePath;
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
	// function returns.
	// Return false (0) when the application must exit, otherwise return true (1).
	int (*Graphics)(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags);

	// Interrupt handler
	// void (*Interrupt)();

	// Exception callback
	void (*Exception)(const char *message);

	// Safe exit
	void (*Close)();

} FT8XXEMU_EmulatorParameters;

#ifdef __cplusplus 
extern "C" {
#endif

// Run the emulator on the current thread. Returns when the emulator is fully stopped. Parameter versionApi must be set to FT8XXEMU_VERSION_API
FT8XXEMU_API void FT8XXEMU_run(uint32_t versionApi, const FT8XXEMU_EmulatorParameters *params);

// Stop the emulator. Can be called from any thread
FT8XXEMU_API extern void (*FT8XXEMU_stop)();

// Transfer data over the imaginary SPI bus. Call from the MCU thread (from the setup/loop callbacks)
FT8XXEMU_API extern uint8_t (*FT8XXEMU_transfer)(uint8_t data);
FT8XXEMU_API extern void (*FT8XXEMU_csLow)(int low);
FT8XXEMU_API extern void (*FT8XXEMU_csHigh)(int high);

#ifdef __cplusplus 
} /* extern "C" */
#endif

#endif /* #ifndef FT8XXEMU_H */

/* end of file */