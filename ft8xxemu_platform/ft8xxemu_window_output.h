/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifdef WIN32

#ifndef BT8XXEMU_WINDOW_OUTPUT_H
#define BT8XXEMU_WINDOW_OUTPUT_H
#include "bt8xxemu_inttypes.h"

// System includes
#include <functional>
#include <mutex>

// Project includes
#include "ft8xxemu_system.h"

#define BT8XXEMU_WINDOW_TITLE_A "BT8XX Emulator"
#define BT8XXEMU_WINDOW_TITLE TEXT(BT8XXEMU_WINDOW_TITLE_A)
#define BT8XXEMU_WINDOW_WIDTH_DEFAULT 480
#define BT8XXEMU_WINDOW_HEIGHT_DEFAULT 272
#define BT8XXEMU_WINDOW_RATIO_DEFAULT (480.0f / 272.0f)
#define BT8XXEMU_WINDOW_WIDTH_MAX 2048
#define BT8XXEMU_WINDOW_HEIGHT_MAX 2048
#define BT8XXEMU_WINDOW_KEEPRATIO 1
#define BT8XXEMU_WINDOW_SCALE 1

// Render SDL2 upside down (makes no difference)
#define BT8XXEMU_FLIP_SDL2 1

// (GDI) Getting more CPU usage with StretchDIBits for some reason, so I don't use it.
#define BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS 0

namespace FT8XXEMU {

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/**
 * WindowOutput
 * \brief WindowOutput
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */
class WindowOutput
{
public:
	static WindowOutput *create(System *system);
	void destroy();

private:
	WindowOutput(System *system);
	virtual ~WindowOutput();

public:
	HWND getHandle();

	void onResetTouchScreenXY(std::function<void(int)> f) { m_ResetTouchScreenXY = f; }
	void onSetTouchScreenXY(std::function<void(int, int, int, int)> f) { m_SetTouchScreenXY = f; }

	// Update window output dimensions
	void setMode(int width, int height);

	// Render current buffer, return false if window is already closed
	bool renderBuffer(bool output, bool changed);


	argb8888 *getBufferARGB8888();
	inline bool isUpsideDown()
	{
		#if (defined(FTEMU_SDL) || (defined(FTEMU_SDL2) && !BT8XXEMU_FLIP_SDL2))
		return false;
		#else
		return true;
		#endif
	}

	void enableMouse(bool enabled = true);
	void setMousePressure(int pressure = 0);

private:
	friend LRESULT CALLBACK FT8XXEMU::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	argb8888 m_BufferA[BT8XXEMU_WINDOW_WIDTH_MAX * BT8XXEMU_WINDOW_HEIGHT_MAX];
	argb8888 m_BufferB[BT8XXEMU_WINDOW_WIDTH_MAX * BT8XXEMU_WINDOW_HEIGHT_MAX];
	bool m_BufferSwitched = false;
	volatile bool m_BufferFlipping = false;

	System *m_System;

	std::function<void(int idx)> m_ResetTouchScreenXY = [](int) -> void { };
	std::function<void(int idx, int x, int y, int pressure)> m_SetTouchScreenXY = [](int, int, int, int) -> void { };

	HINSTANCE m_HInstance = NULL;
	HWND m_HWnd = NULL;

	bool m_Close = false;

#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
	HBITMAP m_Buffer = NULL;
	HDC m_HDC = NULL;
#endif

	BITMAPINFO m_BitInfo = { 0 };
	int m_Width = BT8XXEMU_WINDOW_WIDTH_DEFAULT;
	int m_Height = BT8XXEMU_WINDOW_HEIGHT_DEFAULT;
	float m_Ratio = BT8XXEMU_WINDOW_RATIO_DEFAULT;

	bool m_MouseEnabled = false;
	int m_MousePressure = 0;

	int m_MouseX = 0;
	int m_MouseY = 0;
	int m_MouseDown = false;

private:
	WindowOutput(const WindowOutput &) = delete;
	WindowOutput &operator=(const WindowOutput &) = delete;

}; /* class WindowOutput */

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_WINDOW_OUTPUT_H */

#endif /* #ifdef WIN32 */

/* end of file */
