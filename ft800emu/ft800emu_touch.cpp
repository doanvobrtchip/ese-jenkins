/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

// #include <...>
#include "ft800emu_touch.h"
#include "ft8xxemu_system_windows.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft8xxemu.h"
#include "ft800emu_vc.h"
#include "ft8xxemu_system.h"
#include "ft800emu_memory.h"
#include "ft8xxemu_graphics_driver.h"

// using namespace ...;

namespace FT800EMU {

TouchClass Touch[5];

inline long TouchClass::jitteredTime(long micros)
{
	long delta = micros - m_LastJitteredTime;
	if (delta > 0)
	{
		const long od = delta;
		long avgLastDelta = 0;
		for (int i = 0; i < 8; ++i)
			avgLastDelta += m_LastDeltas[i];
		avgLastDelta /= 8;
		if (avgLastDelta < delta)
		{
			delta = avgLastDelta;
		}
		static const long deltadiv = 3;
		if (delta >= deltadiv)
		{
			delta /= deltadiv;
			delta = (long)rand() % delta;
			m_LastJitteredTime += delta;
		}
		if (od > 0)
		{
			m_LastDeltas[m_LastDeltaI] = od;
			++m_LastDeltaI;
			m_LastDeltaI %= 8;
		}
	}
	return m_LastJitteredTime;
}

inline void TouchClass::transformTouchXY(int &x, int &y)
{
	uint8_t *ram = Memory.getRam();
	// Transform (currently just depending on REG_ROTATE, ignoring TRANSFORM)
	if (s_EnableTouchMatrix) // s15.16 matrix
	{
		const int64_t xe = x;
		const int64_t ye = y;
		const int64_t transformA = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_A]);
		const int64_t transformB = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_B]);
		const int64_t transformC = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_C]);
		const int64_t transformD = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_D]);
		const int64_t transformE = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_E]);
		const int64_t transformF = reinterpret_cast<const int32_t &>(ram[REG_TOUCH_TRANSFORM_F]);
		int64_t xtf = (transformA * xe) + (transformB * ye) + transformC;
		int64_t ytf = (transformD * xe) + (transformE * ye) + transformF;
		x = (int)(xtf >> 16);
		y = (int)(ytf >> 16);
		// printf("x: %i ,y: %i\n", x, y);
	}

	/*
	if (Touch.rawReadU32(ram, REG_ROTATE) & 0x01)
	{
		// printf("rotated\n");
		x = Touch.rawReadU32(ram, REG_HSIZE) - x - 1;
		y = Touch.rawReadU32(ram, REG_VSIZE) - y - 1;
	}
	*/
}

inline void TouchClass::getTouchScreenXY(long micros, int &x, int &y, bool jitter)
{
	long delta;//
	if (jitter)
	{
		delta = jitteredTime(micros) - m_TouchScreenSet;
	}
	else
	{
		delta = micros - m_TouchScreenSet;
	}
	/*if (s_TouchScreenJitter)
	{
		delta += ((rand() % 2000) - 1000) * s_TouchScreenFrameTime / 1000; // add some time jitter
	}*/
	if (delta < 0)
	{
		x = m_TouchScreenX1;
		y = m_TouchScreenY1;
	}
	else if (delta >= s_TouchScreenFrameTime)
	{
		x = m_TouchScreenX2;
		y = m_TouchScreenY2;
	}
	else
	{
		long xdelta = m_TouchScreenX2 - m_TouchScreenX1;
		long ydelta = m_TouchScreenY2 - m_TouchScreenY1;
		x = m_TouchScreenX1 + (int)(xdelta * delta / s_TouchScreenFrameTime);
		y = m_TouchScreenY1 + (int)(ydelta * delta / s_TouchScreenFrameTime);
	}
	if (jitter)
	{
		x += ((rand() % 2000) - 1000) / 1000;
		y += ((rand() % 2000) - 1000) / 1000;
	}
	// transformTouchXY(x, y);
}

inline void TouchClass::getTouchScreenXY(long micros, int &x, int &y)
{
	getTouchScreenXY(micros, x, y, s_TouchScreenJitter);
}

inline uint32_t TouchClass::getTouchScreenXY(int x, int y)
{
	uint16_t const touch_screen_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_screen_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_screen_xy = (uint32_t)touch_screen_x << 16 | touch_screen_y;
	return touch_screen_xy;
}

inline uint32_t TouchClass::getTouchScreenXY(long micros)
{
	if (m_TouchScreenSet)
	{
		int x, y;
		getTouchScreenXY(micros, x, y);
		transformTouchXY(x, y);
		return getTouchScreenXY(x, y);
	}
	else
	{
		return 0x80008000;
	}
}

inline uint32_t TouchClass::getTouchScreenXY()
{
	long micros = FT8XXEMU::System.getMicros();
	return getTouchScreenXY(micros);
}

void TouchClass::setXY(int x, int y, int pressure)
{
	uint8_t *ram = Memory.getRam();
	uint16_t const touch_raw_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_raw_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_raw_xy = (uint32_t)touch_raw_x << 16 | touch_raw_y;
	if (multiTouch())
	{
		// no-op
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			Memory.rawWriteU32(ram, REG_TOUCH_RAW_XY, touch_raw_xy);
			break;
		}
	}
	if (m_TouchScreenSet)
	{
		if (s_TouchScreenJitter)
		{
			long micros = jitteredTime(FT8XXEMU::System.getMicros());
			getTouchScreenXY(micros, m_TouchScreenX1, m_TouchScreenY1, false);
			//s_TouchScreenX1 = s_TouchScreenX2;
			//s_TouchScreenY1 = s_TouchScreenY2;
			m_TouchScreenSet = micros;
		}
		else
		{
			long micros = FT8XXEMU::System.getMicros();
			m_TouchScreenX1 = m_TouchScreenX2;
			m_TouchScreenY1 = m_TouchScreenY2;
			m_TouchScreenSet = micros;
		}
	}
	else
	{
		long micros = FT8XXEMU::System.getMicros();
		m_LastJitteredTime = micros;
		m_TouchScreenX1 = x;
		m_TouchScreenY1 = y;
		m_TouchScreenSet = micros;
	}
	m_TouchScreenX2 = x;
	m_TouchScreenY2 = y;

	transformTouchXY(x, y);
	if (multiTouch())
	{
		switch (m_Index)
		{
		case 0:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH0_XY, getTouchScreenXY(x, y));
			break;
		case 1:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH1_XY, getTouchScreenXY(x, y));
			break;
		case 2:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH2_XY, getTouchScreenXY(x, y));
			break;
		case 3:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH3_XY, getTouchScreenXY(x, y));
			break;
		case 4:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH4_X, x & 0xFFFF);
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH4_Y, y & 0xFFFF);
			break;
		}
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			Memory.rawWriteU32(ram, REG_TOUCH_SCREEN_XY, getTouchScreenXY(x, y));
			Memory.rawWriteU32(ram, REG_TOUCH_RZ, pressure);
			break;
		}
	}
	Memory.poke();
}

void TouchClass::setTouchScreenXYFrameTime(long micros)
{
	s_TouchScreenFrameTime = micros; // * 3 / 2;
}

void TouchClass::resetXY()
{
	m_TouchScreenSet = 0;
	uint8_t *ram = Memory.getRam();
	switch (m_Index)
	{
	case 0:
		Memory.rawWriteU32(ram, REG_TOUCH_TAG, 0);
		break;
#ifdef FT810EMU_MODE
	case 1:
		Memory.rawWriteU32(ram, REG_TOUCH_TAG1, 0);
		break;
	case 2:
		Memory.rawWriteU32(ram, REG_TOUCH_TAG2, 0);
		break;
	case 3:
		Memory.rawWriteU32(ram, REG_TOUCH_TAG3, 0);
		break;
	case 4:
		Memory.rawWriteU32(ram, REG_TOUCH_TAG4, 0);
		break;
#endif
	}
	if (multiTouch())
	{
		// No touch detected
		switch (m_Index)
		{
		case 0:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH0_XY, 0x80008000);
			break;
		case 1:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH1_XY, 0x80008000);
			break;
		case 2:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH2_XY, 0x80008000);
			break;
		case 3:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH3_XY, 0x80008000);
			break;
		case 4:
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH4_X, 0x8000);
			Memory.rawWriteU32(ram, REG_CTOUCH_TOUCH4_Y, 0x8000);
			break;
		}
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			Memory.rawWriteU32(ram, REG_TOUCH_RZ, 32767);
			Memory.rawWriteU32(ram, REG_TOUCH_RAW_XY, 0xFFFFFFFF);
			Memory.rawWriteU32(ram, REG_TOUCH_SCREEN_XY, 0x80008000);
			break;
		}
	}
	m_LastJitteredTime = FT8XXEMU::System.getMicros();
	Memory.poke();
}

void TouchClass::enableTouchMatrix(bool enabled)
{
	s_EnableTouchMatrix = enabled;
}

void setTouchScreenXY(int idx, int x, int y, int pressure)
{
	Touch[idx].setXY(x, y, pressure);
}

void resetTouchScreenXY(int idx)
{
	Touch[idx].resetXY();
}

void TouchClass::begin(FT8XXEMU_EmulatorMode emulatorMode)
{
	s_EmulatorMode = emulatorMode;
	s_TouchScreenFrameTime = 0;
	s_TouchScreenJitter = true;
	s_EnableTouchMatrix = false;
	Touch[0].m_Index = 0;
	Touch[0].reset();
	Touch[1].m_Index = 1;
	Touch[1].reset();
	Touch[2].m_Index = 2;
	Touch[2].reset();
	Touch[3].m_Index = 3;
	Touch[3].reset();
	Touch[4].m_Index = 4;
	Touch[4].reset();

	FT8XXEMU::g_SetTouchScreenXY = &setTouchScreenXY;
	FT8XXEMU::g_ResetTouchScreenXY = &resetTouchScreenXY; // FIXME

	/*s_TouchScreenSet = 0;
	s_LastJitteredTime = FT8XXEMU::System.getMicros();*/
}

void TouchClass::reset()
{
	m_TouchScreenSet = 0;
	m_LastDeltas[0] = 1000;
	m_LastDeltas[1] = 1000;
	m_LastDeltas[2] = 1000;
	m_LastDeltas[3] = 1000;
	m_LastDeltas[4] = 1000;
	m_LastDeltas[5] = 1000;
	m_LastDeltas[6] = 1000;
	m_LastDeltas[7] = 1000;
	m_LastDeltaI = 0;
	resetXY();
}

void TouchClass::end()
{

}

bool TouchClass::multiTouch()
{
	uint8_t *ram = Memory.getRam();
#ifdef FT810EMU_MODE
	return (Memory.rawReadU32(ram, REG_CTOUCH_EXTENDED) & 0x01) == CTOUCH_MODE_EXTENDED;
#else
	return s_EmulatorMode >= FT8XXEMU_EmulatorFT801
		&& ((Memory.rawReadU32(ram, REG_CTOUCH_EXTENDED) & 0x01) == CTOUCH_MODE_EXTENDED);
#endif
}

} /* namespace FT800EMU */

/* end of file */
