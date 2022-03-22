/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
BT815 Emulator Library
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26812) // Unscoped enum
#endif

// #include <...>
#include "ft800emu_touch.h"
#include "ft8xxemu_system_win32.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "bt8xxemu.h"
#include "ft800emu_vc.h"
#include "ft8xxemu_system.h"
#include "ft800emu_memory.h"
#include "ft8xxemu_window_output.h"

// using namespace ...;

#ifdef BT815EMU_MODE
#define REG_CTOUCH_TOUCH1_XY REG_CTOUCH_TOUCHA_XY
#define REG_CTOUCH_TOUCH2_XY REG_CTOUCH_TOUCHB_XY
#define REG_CTOUCH_TOUCH3_XY REG_CTOUCH_TOUCHC_XY
#endif

namespace FT800EMU {

inline long TouchPoint::jitteredTime(long micros)
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

inline void Touch::transformTouchXY(int &x, int &y)
{
	uint8_t *ram = m_Memory->getRam();
	// Transform (currently just depending on REG_ROTATE, ignoring TRANSFORM)
	if (m_EnableTouchMatrix) // s15.16 matrix
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
		// FTEMU_printf("x: %i ,y: %i\n", x, y);
	}

	/*
	if (Touch.rawReadU32(ram, REG_ROTATE) & 0x01)
	{
		// FTEMU_printf("rotated\n");
		x = Touch.rawReadU32(ram, REG_HSIZE) - x - 1;
		y = Touch.rawReadU32(ram, REG_VSIZE) - y - 1;
	}
	*/
}

inline void TouchPoint::getTouchScreenXY(long micros, int &x, int &y, bool jitter)
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
	else if (delta >= m_Touch->m_TouchScreenFrameTime)
	{
		x = m_TouchScreenX2;
		y = m_TouchScreenY2;
	}
	else
	{
		long xdelta = m_TouchScreenX2 - m_TouchScreenX1;
		long ydelta = m_TouchScreenY2 - m_TouchScreenY1;
		x = m_TouchScreenX1 + (int)(xdelta * delta / m_Touch->m_TouchScreenFrameTime);
		y = m_TouchScreenY1 + (int)(ydelta * delta / m_Touch->m_TouchScreenFrameTime);
	}
	if (jitter)
	{
		x += ((rand() % 2000) - 1000) / 1000;
		y += ((rand() % 2000) - 1000) / 1000;
	}
	// transformTouchXY(x, y);
}

inline void TouchPoint::getTouchScreenXY(long micros, int &x, int &y)
{
	getTouchScreenXY(micros, x, y, m_Touch->m_TouchScreenJitter);
}

inline uint32_t Touch::getTouchScreenXY(int x, int y)
{
	uint16_t const touch_screen_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_screen_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_screen_xy = (uint32_t)touch_screen_x << 16 | touch_screen_y;
	return touch_screen_xy;
}

inline uint32_t TouchPoint::getTouchScreenXY(long micros)
{
	if (m_TouchScreenSet)
	{
		int x, y;
		getTouchScreenXY(micros, x, y);
		m_Touch->transformTouchXY(x, y);
		return Touch::getTouchScreenXY(x, y);
	}
	else
	{
		return 0x80008000;
	}
}

inline uint32_t TouchPoint::getTouchScreenXY()
{
	long micros = m_Touch->m_System->getMicros();
	return getTouchScreenXY(micros);
}

void TouchPoint::setXY(int x, int y, int pressure)
{
	uint8_t *ram = m_Touch->m_Memory->getRam();
	uint16_t const touch_raw_x = ((uint32_t &)x) & 0xFFFF;
	uint16_t const touch_raw_y = ((uint32_t &)y) & 0xFFFF;
	uint32_t const touch_raw_xy = (uint32_t)touch_raw_x << 16 | touch_raw_y;
	if (m_Touch->multiTouch())
	{
		// no-op
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_RAW_XY, touch_raw_xy);
			break;
		}
	}
	if (m_TouchScreenSet)
	{
		if (m_Touch->m_TouchScreenJitter)
		{
			long micros = jitteredTime(m_Touch->m_System->getMicros());
			getTouchScreenXY(micros, m_TouchScreenX1, m_TouchScreenY1, false);
			//s_TouchScreenX1 = s_TouchScreenX2;
			//s_TouchScreenY1 = s_TouchScreenY2;
			m_TouchScreenSet = micros;
		}
		else
		{
			long micros = m_Touch->m_System->getMicros();
			m_TouchScreenX1 = m_TouchScreenX2;
			m_TouchScreenY1 = m_TouchScreenY2;
			m_TouchScreenSet = micros;
		}
	}
	else
	{
		long micros = m_Touch->m_System->getMicros();
		m_LastJitteredTime = micros;
		m_TouchScreenX1 = x;
		m_TouchScreenY1 = y;
		m_TouchScreenSet = micros;
	}
	m_TouchScreenX2 = x;
	m_TouchScreenY2 = y;

	m_Touch->transformTouchXY(x, y);
	if (m_Touch->multiTouch())
	{
		switch (m_Index)
		{
		case 0:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH0_XY, Touch::getTouchScreenXY(x, y));
			break;
		case 1:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH1_XY, Touch::getTouchScreenXY(x, y));
			break;
		case 2:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH2_XY, Touch::getTouchScreenXY(x, y));
			break;
		case 3:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH3_XY, Touch::getTouchScreenXY(x, y));
			break;
		case 4:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH4_X, x & 0xFFFF);
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH4_Y, y & 0xFFFF);
			break;
		}
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_SCREEN_XY, Touch::getTouchScreenXY(x, y));
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_RZ, pressure);
			break;
		}
	}
	m_Touch->m_Memory->poke();
}

void Touch::setTouchScreenXYFrameTime(long micros)
{
	m_TouchScreenFrameTime = micros; // * 3 / 2;
}

void TouchPoint::resetXY()
{
	bool doPoke = m_TouchScreenSet != 0;
	m_TouchScreenSet = 0;
	uint8_t *ram = m_Touch->m_Memory->getRam();
	switch (m_Index)
	{
	case 0:
		m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_TAG, 0);
		break;
#ifdef FT810EMU_MODE
	case 1:
		m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_TAG1, 0);
		break;
	case 2:
		m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_TAG2, 0);
		break;
	case 3:
		m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_TAG3, 0);
		break;
	case 4:
		m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_TAG4, 0);
		break;
#endif
	}
	if (m_Touch->multiTouch())
	{
		// No touch detected
		switch (m_Index)
		{
		case 0:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH0_XY, 0x80008000);
			break;
		case 1:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH1_XY, 0x80008000);
			break;
		case 2:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH2_XY, 0x80008000);
			break;
		case 3:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH3_XY, 0x80008000);
			break;
		case 4:
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH4_X, 0x8000);
			m_Touch->m_Memory->rawWriteU32(ram, REG_CTOUCH_TOUCH4_Y, 0x8000);
			break;
		}
	}
	else
	{
		switch (m_Index)
		{
		case 0:
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_RZ, 32767);
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_RAW_XY, 0xFFFFFFFF);
			m_Touch->m_Memory->rawWriteU32(ram, REG_TOUCH_SCREEN_XY, 0x80008000);
			break;
		}
	}
	m_LastJitteredTime = m_Touch->m_System->getMicros();
	if (doPoke)
		m_Touch->m_Memory->poke();
}

// Get XY value interpolated etc
uint32_t TouchPoint::getXY()
{
	return getTouchScreenXY();
}

void Touch::enableTouchMatrix(bool enabled)
{
	m_EnableTouchMatrix = enabled;
}

/*
void setTouchScreenXY(int idx, int x, int y, int pressure)
{
	m_Touch[idx].setXY(x, y, pressure);
}

void resetTouchScreenXY(int idx)
{
	Touch[idx].resetXY();
}
*/

Touch::Touch(FT8XXEMU::System *system, BT8XXEMU_EmulatorMode emulatorMode, Memory *memory)
{
	m_System = system;
	m_Memory = memory;
	m_EmulatorMode = emulatorMode;
	m_TouchScreenFrameTime = 0;
	m_TouchScreenJitter = true;
	m_EnableTouchMatrix = false;
	m_Touch[0].m_Touch = this;
	m_Touch[0].m_Index = 0;
	m_Touch[0].reset();
	m_Touch[1].m_Touch = this;
	m_Touch[1].m_Index = 1;
	m_Touch[1].reset();
	m_Touch[2].m_Touch = this;
	m_Touch[2].m_Index = 2;
	m_Touch[2].reset();
	m_Touch[3].m_Touch = this;
	m_Touch[3].m_Index = 3;
	m_Touch[3].reset();
	m_Touch[4].m_Touch = this;
	m_Touch[4].m_Index = 4;
	m_Touch[4].reset();
}

Touch::~Touch()
{

}

TouchPoint::TouchPoint()
{
	// FT8XXEMU::g_SetTouchScreenXY = &setTouchScreenXY;
	// FT8XXEMU::g_ResetTouchScreenXY = &resetTouchScreenXY; // FIXME

	/*s_TouchScreenSet = 0;
	s_LastJitteredTime = FT8XXEMU::System.getMicros();*/
}

void TouchPoint::reset()
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
	m_LastJitteredTime = m_Touch->m_System->getMicros();
	resetXY();
}

TouchPoint::~TouchPoint()
{

}

bool Touch::multiTouch()
{
	uint8_t *ram = m_Memory->getRam();
	return (m_EmulatorMode & 0x01)
		&& ((m_Memory->rawReadU32(ram, REG_CTOUCH_EXTENDED) & 0x01) == CTOUCH_MODE_EXTENDED);
}

} /* namespace FT800EMU */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
