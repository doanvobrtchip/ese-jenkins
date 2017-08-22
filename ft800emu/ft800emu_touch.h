/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#ifndef FT800EMU_TOUCH_H
#define FT800EMU_TOUCH_H
// #include <...>

// System includes
#include "bt8xxemu.h"
#include "bt8xxemu_inttypes.h"

// Project includes

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Memory;

class Touch;

class TouchPoint
{
	friend class Touch;

public:
	TouchPoint();
	~TouchPoint();

	void reset();

	// Set touch screen xy for interpolation in time when touch is pressed
	void setXY(int x, int y, int pressure);
	// Touch is not pressed
	void resetXY();
	// Get XY value interpolated etc
	uint32_t getXY();

private:
	inline long jitteredTime(long micros);
	inline void getTouchScreenXY(long micros, int &x, int &y, bool jitter);
	inline void getTouchScreenXY(long micros, int &x, int &y);
	inline uint32_t getTouchScreenXY(long micros);
	inline uint32_t getTouchScreenXY();

private:
	Touch *m_Touch;

	int m_Index;
	long m_TouchScreenSet;
	int m_TouchScreenX1;
	int m_TouchScreenX2;
	int m_TouchScreenY1;
	int m_TouchScreenY2;

	long m_LastJitteredTime;
	long m_LastDeltas[8];
	long m_LastDeltaI;

private:
	TouchPoint(const TouchPoint &) = delete;
	TouchPoint &operator=(const TouchPoint &) = delete;

}; /* class TouchPoint */

class Touch
{
	friend class TouchPoint;
public:
	Touch(FT8XXEMU::System *system, BT8XXEMU_EmulatorMode emulatorMode, Memory *memory);
	~Touch();

	bool multiTouch();

	void enableTouchMatrix(bool enabled = true);

	// Internal call used for touch interpolation
	void setTouchScreenXYFrameTime(long micros);

	inline TouchPoint &touch(int i) { return m_Touch[i]; }

private:
	inline void transformTouchXY(int &x, int &y);
	inline static uint32_t getTouchScreenXY(int x, int y);

private:
	FT8XXEMU::System *m_System;
	Memory *m_Memory;
	TouchPoint m_Touch[5];

	long m_TouchScreenFrameTime;
	bool m_TouchScreenJitter;
	bool m_EnableTouchMatrix;

	BT8XXEMU_EmulatorMode m_EmulatorMode;

private:
	Touch(const Touch &) = delete;
	Touch &operator=(const Touch &) = delete;

};

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_TOUCH_H */

/* end of file */
