/**
 * WindowOutput
 * $Id$
 * \file ft8xxemu_window_output_gdi.cpp
 * \brief WindowOutput
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifdef WIN32

// #include <...>
#include "ft8xxemu_window_output.h"

// System includes
#include <thread>
#include <mutex>
#include "ft8xxemu_system_windows.h"
#include <Windowsx.h>
#include "ft8xxemu_system.h"
#include <concurrent_queue.h>
#include <assert.h>

// Project includes
#include "ft8xxemu_thread_state.h"

#define BT8XXEMU_GDI_DOUBLE_BUFFER (1)

#define BT8XXEMU_WINDOW_CLASS_NAME TEXT("BT8XXEMUWindowOutput")

#define BT8XXEMU_WM_FUNCTION (WM_APP + 42)
#define BT8XXEMU_WM_STOP (WM_APP + 43)

namespace FT8XXEMU {

namespace /* anonymous */ {

std::mutex s_LoopMutex;
std::thread s_LoopThread;
int s_LoopActive = 0;
concurrency::concurrent_queue<std::function<void()>> s_LoopQueue;
DWORD s_LoopThreadId = 0;
std::map<HWND, WindowOutput *> s_WindowMap;

void immediate(std::function<void()> f)
{
	s_LoopQueue.push(f);
	while (!PostThreadMessage(s_LoopThreadId, BT8XXEMU_WM_FUNCTION, 0, 0))
		SwitchToThread();
}

void joinQueue()
{
	std::condition_variable cond;
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	immediate([&]() -> void {
		std::unique_lock<std::mutex> lock(m);
		cond.notify_one();
	});
	cond.wait(lock);
}

void immediateSync(std::function<void()> f)
{
	std::condition_variable cond;
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	immediate([&]() -> void {
		f();
		; {
			std::unique_lock<std::mutex> lock(m);
			cond.notify_one();
		}
	});
	cond.wait(lock);
}

void loopFunction()
{
	ThreadState threadState;
	threadState.init();
	threadState.foreground();
	threadState.setName("FT8XXEMU Win32 Loop");

	s_LoopThreadId = GetCurrentThreadId();
	MSG msg;
	while (BOOL bRet = GetMessage(&msg, NULL, 0, 0))
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			std::function<void()> f;
			while (s_LoopQueue.try_pop(f))
				f();
			switch (msg.message)
			{
			case BT8XXEMU_WM_FUNCTION:
				break;
			case BT8XXEMU_WM_STOP:
				return;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				break;
			}
		}
	}
}

void incLoop()
{
	std::unique_lock<std::mutex> lock(s_LoopMutex);
	if (!s_LoopActive)
	{
		assert(s_LoopQueue.empty());
		s_LoopThread = std::move(std::thread(loopFunction));
	}
	++s_LoopActive;
}

void decLoop()
{
	std::unique_lock<std::mutex> lock(s_LoopMutex);
	--s_LoopActive;
	assert(s_LoopActive >= 0);
	assert(s_LoopQueue.empty());
	if (!s_LoopActive)
	{
		PostThreadMessage(s_LoopThreadId, BT8XXEMU_WM_STOP, 0, 0);
		s_LoopThread.join();
	}
}

}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WindowOutput *w = s_WindowMap[hWnd])
		return w->wndProc(message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

WindowOutput *WindowOutput::create()
{
	return new WindowOutput();
}

void WindowOutput::destroy()
{
	delete this;
}

WindowOutput::WindowOutput()
{
	incLoop();
	immediateSync([&]() -> void {
		// Save params
		m_HInstance = GetModuleHandle(NULL);

		// Register Display Class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpszClassName = BT8XXEMU_WINDOW_CLASS_NAME;
		wcex.lpfnWndProc = FT8XXEMU::wndProc;
		wcex.hInstance = m_HInstance;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		//wcex.hIcon = LoadIcon(m_HInstance, MAKEINTRESOURCE(IDI_JAZZ));
		//wcex.hIconSm = LoadIcon(m_HInstance, MAKEINTRESOURCE(IDI_SMALL));
		wcex.hIcon = NULL;
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex)) SystemWindows.ErrorWin32(TEXT("GDI Initialisation"));

		// Initialize and Show Display Instance
		DWORD dw_style = WS_OVERLAPPEDWINDOW;
		/*#if FT8XXEMUWIN_DISPLAY_ASPECT_RATIO
		RECT r; r.top = 0; r.left = 0; r.bottom = (LONG)(((float)BT8XXEMU_WINDOW_WIDTH / aspectRatio) * 2); r.right = BT8XXEMU_WINDOW_WIDTH * 2; // window size
		#else*/
		RECT r; r.top = 0; r.left = 0; r.bottom = m_Height * BT8XXEMU_WINDOW_SCALE; r.right = m_Width * BT8XXEMU_WINDOW_SCALE; // window size
																															   /*#endif*/
		AdjustWindowRect(&r, dw_style, FALSE);
		if (m_HWnd) SystemWindows.Error(TEXT("WindowOutput.begin()  s_HWnd != NULL"));
		if (!(m_HWnd = CreateWindow(BT8XXEMU_WINDOW_CLASS_NAME,
			/*(LPCTSTR)title.c_str()*/BT8XXEMU_WINDOW_TITLE, dw_style,
			CW_USEDEFAULT, 0, r.right - r.left, r.bottom - r.top, // x y w h
			NULL, NULL, m_HInstance, NULL)))
			SystemWindows.ErrorWin32(TEXT("GDI Initialisation"));
		ShowWindow(m_HWnd, /*nCmdShow*/ true); // If the window was previously visible, the return value is nonzero.
		if (!UpdateWindow(m_HWnd)) SystemWindows.ErrorWin32(TEXT("GDI Initialisation"));

		// Create GDI32 Buffer and Device Context
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		HDC hdc = GetDC(m_HWnd);
		if (m_HDC) SystemWindows.Error(TEXT("WindowOutput.begin()  s_HDC != NULL"));
		m_HDC = CreateCompatibleDC(hdc);
		if (!m_HDC) SystemWindows.Error(TEXT("WindowOutput.begin()  s_HDC == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
		if (m_Buffer) SystemWindows.Error(TEXT("WindowOutput.begin()  s_Buffer != NULL"));
		m_Buffer = CreateCompatibleBitmap(hdc, m_Width, m_Height);
		if (!m_Buffer) SystemWindows.Error(TEXT("WindowOutput.begin()  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
		SelectObject(m_HDC, m_Buffer);
		ReleaseDC(m_HWnd, hdc);
#endif

		// Initialize Bitmap Buffer
		m_BitInfo.bmiHeader.biSize = sizeof(m_BitInfo.bmiHeader);
		m_BitInfo.bmiHeader.biWidth = m_Width;
		m_BitInfo.bmiHeader.biHeight = m_Height;
		m_BitInfo.bmiHeader.biPlanes = 1;
		m_BitInfo.bmiHeader.biBitCount = 32;
		m_BitInfo.bmiHeader.biCompression = BI_RGB;

		// Map HWND to WindowOutput
		s_WindowMap[m_HWnd] = this;
		FT8XXEMU::SystemWindows.setHWnd(m_HWnd);
	});
}

/*
bool WindowOutput::update()
{
	// Update Window Messages
	MSG msg;
	while (PeekMessage(&msg, / * m_HWnd * / NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return s_HWnd != NULL;
}
*/

WindowOutput::~WindowOutput()
{
	immediateSync([&]() -> void {

		s_WindowMap.erase(m_HWnd);

#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		if (m_HDC) { DeleteDC(m_HDC); m_HDC = NULL; }
		else SystemWindows.Debug(TEXT("WindowOutput.end() m_HDC == NULL"));
		if (m_Buffer) { DeleteObject(m_Buffer); m_Buffer = NULL; }
		else SystemWindows.Debug(TEXT("WindowOutput.end() m_Buffer == NULL"));
#endif

		if (m_HWnd) { DestroyWindow(m_HWnd); m_HWnd = NULL; }
		else SystemWindows.Debug(TEXT("WindowOutput.end() m_HWnd == NULL"));

		//UnregisterClass(BT8XXEMU_WINDOW_CLASS_NAME, m_HInstance);
	});
	decLoop();
}

HWND WindowOutput::getHandle()
{
	return m_HWnd;
}

argb8888 *WindowOutput::getBufferARGB8888()
{
	return m_BufferSwitched ? m_BufferB : m_BufferA;
}

LRESULT WindowOutput::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	//int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	std::function<void()> f;
	while (s_LoopQueue.try_pop(f))
		f();

	switch (message)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if (m_MouseEnabled)
		{
			m_MouseDown = wParam & MK_LBUTTON;
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			RECT r;
			GetClientRect(m_HWnd, &r);
			int width_r = (int)((float)r.bottom * m_Ratio); int height_r;
			if (width_r > r.right) { width_r = r.right; height_r = (int)((float)r.right / m_Ratio); }
			else height_r = r.bottom;
			int x_r = (r.right - width_r) / 2;
			int y_r = (r.bottom - height_r) / 2;
			m_MouseX = (mouseX - x_r) * m_Width / width_r;
			m_MouseY = (mouseY - y_r) * m_Height / height_r;
			if (m_MouseDown && m_MouseX >= 0 && m_MouseX < m_Width && m_MouseY >= 0 && m_MouseY < m_Height)
			{
				m_SetTouchScreenXY(0, m_MouseX, m_MouseY, m_MousePressure);
			}
			else
			{
				m_MouseDown = false;
				m_ResetTouchScreenXY(0);
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(m_HWnd, &ps);
		EndPaint(m_HWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		m_HWnd = NULL;
		break;
	case WM_ERASEBKGND:
		return (LRESULT)1; // Say we handled it.
	case WM_SYSCOMMAND:
		if (wParam == SC_KEYMENU) return 0;
		return DefWindowProc(m_HWnd, message, wParam, lParam);
	default:
		return DefWindowProc(m_HWnd, message, wParam, lParam);
	}
	return (LRESULT)0;
}

void WindowOutput::setMode(int width, int height)
{
	if (m_Width != width || m_Height != height)
	{
		immediateSync([&]() -> void {
			m_Width = width;
			m_Height = height;
			m_Ratio = (float)width / (float)height;

#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
			if (!m_Buffer) SystemWindows.Error(TEXT("WindowOutput.setMode(2)  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
			HBITMAP oldBuffer = m_Buffer;
			m_Buffer = NULL;

			HDC hdc = GetDC(m_HWnd);
			if (m_Buffer) SystemWindows.Error(TEXT("WindowOutput.begin()  s_Buffer != NULL"));
			m_Buffer = CreateCompatibleBitmap(hdc, m_Width, m_Height);
			if (!m_Buffer) SystemWindows.Error(TEXT("WindowOutput.begin()  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
			SelectObject(m_HDC, m_Buffer);
			ReleaseDC(m_HWnd, hdc);

			DeleteObject(oldBuffer);
#endif

			DWORD dw_style = WS_OVERLAPPEDWINDOW;
			RECT r; r.top = 0; r.left = 0; r.bottom = m_Height * BT8XXEMU_WINDOW_SCALE; r.right = m_Width * BT8XXEMU_WINDOW_SCALE; // window size
			AdjustWindowRect(&r, dw_style, FALSE);

			SetWindowPos(m_HWnd, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
			m_BitInfo.bmiHeader.biWidth = m_Width;
			m_BitInfo.bmiHeader.biHeight = m_Height;
		});
	}
}

void WindowOutput::renderBuffer(bool output, bool changed)
{
#if BT8XXEMU_GDI_DOUBLE_BUFFER
	bool bufferSwitched = m_BufferSwitched ^ !changed;
	bool bufferFlipping = m_BufferFlipping;
#else
	bool bufferSwitched = false;
	bool bufferFlipping = true;
#endif
	m_BufferSwitched = !bufferSwitched;
	m_BufferFlipping = true;
	(bufferFlipping ? immediateSync : immediate)([=]() -> void {
		// Render bitmap to buffer
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		if (output)
		{
			if (!SetDIBitsToDevice(m_HDC, 0, 0,
				m_Width, m_Height,
				0, 0, 0, m_Height, bufferSwitched ? m_BufferB : m_BufferA, &m_BitInfo, DIB_RGB_COLORS))
				SystemWindows.Error(TEXT("SetDIBitsToDevice  FAILED"));
		}
#endif

		// Draw buffer to screen
		RECT r;
		GetClientRect(m_HWnd, &r);
		if (output)
#if BT8XXEMU_WINDOW_KEEPRATIO
		{
			COLORREF bgC32 = RGB(128, 128, 128); // bg outside render
			HBRUSH bgBrush = CreateSolidBrush(bgC32);
			if (bgBrush == NULL) SystemWindows.ErrorWin32(TEXT("GDI Render"));
			int width_r = (int)((float)r.bottom * m_Ratio); int height_r;
			if (width_r > r.right) { width_r = r.right; height_r = (int)((float)r.right / m_Ratio); }
			else height_r = r.bottom;
			int x_r = (r.right - width_r) / 2;
			int y_r = (r.bottom - height_r) / 2;
			HDC hdc = GetDC(m_HWnd);
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
			StretchBlt(hdc, x_r, y_r, width_r, height_r, m_HDC, 0, 0, m_Width, m_Height, SRCCOPY);
#else
			StretchDIBits(hdc, x_r, y_r, width_r, height_r, 0, 0, s_Width, s_Height, s_BufferARGB8888, &s_BitInfo, DIB_RGB_COLORS, SRCCOPY);
#endif
			RECT rect;
			if (x_r > 0)
			{
				rect.top = 0; rect.left = 0;
				rect.top = 0; rect.left = 0;
				rect.right = (r.right - width_r) / 2;
				rect.bottom = r.bottom;
				FillRect(hdc, &rect, bgBrush); // (HBRUSH)(COLOR_WINDOW + 1));
				rect.left = rect.right + width_r;
				rect.right += rect.left;
				FillRect(hdc, &rect, bgBrush); // (HBRUSH)(COLOR_WINDOW + 1));
			}
			if (y_r > 0)
			{
				rect.top = 0; rect.left = 0;
				rect.right = r.right;
				rect.bottom = (r.bottom - height_r) / 2;
				FillRect(hdc, &rect, bgBrush); // (HBRUSH)(COLOR_WINDOW + 1));
				rect.top = rect.bottom + height_r;
				rect.bottom += rect.top;
				FillRect(hdc, &rect, bgBrush); // (HBRUSH)(COLOR_WINDOW + 1));
			}
			ReleaseDC(m_HWnd, hdc);
			if (!DeleteObject(bgBrush)) SystemWindows.ErrorWin32(TEXT("GDI Render"));
		}
#else
		{
			HDC hdc = GetDC(s_HWnd);
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
			StretchBlt(hdc, 0, 0, r.right, r.bottom, s_HDC, 0, 0, s_Width, s_Height, SRCCOPY);
#else
			StretchDIBits(hdc, 0, 0, r.right, r.bottom, 0, 0, s_Width, s_Height, s_BufferARGB1555, &s_BitInfo, DIB_RGB_COLORS, SRCCOPY);
#endif
			ReleaseDC(s_HWnd, hdc);
		}
#endif
		else
		{
			// no output
			HDC hdc = GetDC(m_HWnd);
			COLORREF bgC32 = RGB(128, 128, 128); // bg off render
			HBRUSH bgBrush = CreateSolidBrush(bgC32);
			if (bgBrush == NULL) SystemWindows.ErrorWin32(TEXT("GDI Render"));
			FillRect(hdc, &r, bgBrush);
			if (!DeleteObject(bgBrush)) SystemWindows.ErrorWin32(TEXT("GDI Render"));
			ReleaseDC(m_HWnd, hdc);
		}

		m_BufferFlipping = false;

		// Update title
		/*
		tstringstream newTitle;
		newTitle << BT8XXEMU_WINDOW_TITLE;
		switch (GraphicsProcessor.getDebugMode())
		{
		case BT8XXEMU_DEBUGMODE_ALPHA:
			newTitle << " [ALPHA";
			break;
		case BT8XXEMU_DEBUGMODE_TAG:
			newTitle << " [TAG";
			break;
		case BT8XXEMU_DEBUGMODE_STENCIL:
			newTitle << " [STENCIL";
			break;
		}
		if (GraphicsProcessor.getDebugMode())
		{
			if (GraphicsProcessor.getDebugMultiplier() > 1)
			{
				newTitle << " (" << GraphicsProcessor.getDebugMultiplier() << "x)";
			}
			newTitle << "]";
		}
		if (GraphicsProcessor.getDebugLimiter())
		{
			newTitle << " [LIMIT: " << GraphicsProcessor.getDebugLimiter() << "]";
		}
		if (s_MouseEnabled)
		{
			newTitle << " [X: " << s_MouseX << ", Y: " << s_MouseY;
			if (s_MouseDown)
			{
				newTitle << " (";
				newTitle << Memory.rawReadU32(Memory.getRam(), REG_TOUCH_TAG);
				newTitle << ")";
			}
			newTitle << "]";
		}
		if (!output) newTitle << " [NO OUTPUT]";
		newTitle << TEXT(" [FPS: ");
		newTitle << System.getFPSSmooth();
		newTitle << TEXT(" (");
		newTitle << System.getFPS();
		newTitle << TEXT(")]");
		SetWindowText(s_HWnd, (LPCTSTR)newTitle.str().c_str());
		*/
	});
}

void WindowOutput::enableMouse(bool enabled)
{
	m_MouseEnabled = enabled;
}

void WindowOutput::setMousePressure(int pressure)
{
	m_MousePressure = pressure;
}

} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */

/* end of file */
