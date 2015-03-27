/*
 * Copyright (C) 2013-2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#ifndef FT800EMU_TOUCH_H
#define FT800EMU_TOUCH_H
// #include <...>

// System includes
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"

// Project includes

namespace FT800EMU {

class TouchClass
{
public:
	TouchClass() { }

	static void begin(FT8XXEMU_EmulatorMode emulatorMode);
	static void end();
	void reset();

	static bool multiTouch();
	
	static void enableTouchMatrix(bool enabled = true);
	
	// Internal call used for touch interpolation
	static void setTouchScreenXYFrameTime(long micros);

	// Set touch screen xy for interpolation in time when touch is pressed
	void setXY(int x, int y, int pressure);
	// Touch is not pressed
	void resetXY();
	
private:
	inline long jitteredTime(long micros);
	static inline void transformTouchXY(int &x, int &y);
	inline void getTouchScreenXY(long micros, int &x, int &y, bool jitter);
	inline void getTouchScreenXY(long micros, int &x, int &y);
	inline uint32_t getTouchScreenXY(long micros);
	inline uint32_t getTouchScreenXY();
	static inline uint32_t TouchClass::getTouchScreenXY(int x, int y);

private:
	int m_Index;
	long m_TouchScreenSet;
	int m_TouchScreenX1;
	int m_TouchScreenX2;
	int m_TouchScreenY1;
	int m_TouchScreenY2;
	static long s_TouchScreenFrameTime;
	static bool s_TouchScreenJitter;
	static bool s_EnableTouchMatrix;
	long m_LastJitteredTime;
	long m_LastDeltas[8];
	long m_LastDeltaI;
	static FT8XXEMU_EmulatorMode s_EmulatorMode;

	TouchClass(const TouchClass &);
	TouchClass &operator=(const TouchClass &);

}; /* class TouchClass */

extern TouchClass Touch[5];

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_TOUCH_H */

/* end of file */
