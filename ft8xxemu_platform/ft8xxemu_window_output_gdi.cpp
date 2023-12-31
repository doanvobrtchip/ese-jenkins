/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef WIN32
#include "ft8xxemu_window_output.h"

// System includes
#include <thread>
#include <mutex>
#include <map>
#include <cassert>
#include <condition_variable>

#include "ft8xxemu_system.h"

#ifdef BT8XXEMU_INTTYPES_DEFINED_NULL
#undef NULL
#define NULL 0
#endif

#include <Windowsx.h>
#ifdef __GNUC__
#include <queue>
#else
#include <concurrent_queue.h>
#endif

// Project includes
#include "bt8xxemu_inttypes.h"
#include "ft8xxemu_system_win32.h"
#include "ft8xxemu_thread_state.h"

#define BT8XXEMU_GDI_DOUBLE_BUFFER (1)

#define BT8XXEMU_WINDOW_CLASS_NAME TEXT("BT8XXEMUWindowOutput")

#define BT8XXEMU_WM_FUNCTION (WM_APP + 42)
#define BT8XXEMU_WM_STOP (WM_APP + 43)

namespace FT8XXEMU {

namespace /* anonymous */ {

std::mutex s_LoopMutex;
struct AutoDetach { ~AutoDetach() { if (Thread.joinable()) Thread.detach(); } std::thread Thread; };
AutoDetach s_AutoDetach;
std::thread &s_LoopThread = s_AutoDetach.Thread;
int s_LoopActive = 0;
#ifdef __GNUC__
std::mutex s_LoopQueueMutex;
std::queue<std::function<void()>> s_LoopQueue;
#else
concurrency::concurrent_queue<std::function<void()>> s_LoopQueue;
#endif
DWORD s_LoopThreadId = 0;
std::map<HWND, WindowOutput *> s_WindowMap;
int s_ClassRegCount = 0;

void immediate(std::function<void()> f)
{
	; {
#ifdef __GNUC__
		std::unique_lock<std::mutex> lock(s_LoopQueueMutex);
#endif
		s_LoopQueue.push(f);
	}
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
#ifdef __GNUC__
			s_LoopQueueMutex.lock();
			while (!s_LoopQueue.empty())
			{
				f = s_LoopQueue.front();
				s_LoopQueue.pop();
				s_LoopQueueMutex.unlock();
				f();
				s_LoopQueueMutex.lock();
			}
			s_LoopQueueMutex.unlock();
#else
			while (s_LoopQueue.try_pop(f))
				f();
#endif
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
#ifdef _DEBUG
		; {
#ifdef __GNUC__
			std::unique_lock<std::mutex> lock(s_LoopQueueMutex);
#endif
			assert(s_LoopQueue.empty());
		}
#endif
		s_LoopThread = std::move(std::thread(loopFunction));
	}
	++s_LoopActive;
}

void decLoop()
{
	std::unique_lock<std::mutex> lock(s_LoopMutex);
	--s_LoopActive;
	assert(s_LoopActive >= 0);
#ifdef _DEBUG
	; {
#ifdef __GNUC__
		std::unique_lock<std::mutex> lock(s_LoopQueueMutex);
#endif
		assert(s_LoopQueue.empty());
	}
#endif
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

WindowOutput *WindowOutput::create(System *system)
{
	return new WindowOutput(system);
}

void WindowOutput::destroy()
{
	delete this;
}

WindowOutput::WindowOutput(System *system) : m_System(system)
{
	incLoop();
	immediateSync([&]() -> void {
		// Save params
		m_HInstance = GetModuleHandle(NULL);

		assert(s_ClassRegCount >= 0);
		if (!s_ClassRegCount)
		{
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
			if (!RegisterClassEx(&wcex))
			{
				FTEMU_error("GDI Initialisation: %s", SystemWin32::getWin32LastErrorString().c_str());
			}
		}
		++s_ClassRegCount;

		// Initialize and Show Display Instance
		DWORD dw_style = WS_OVERLAPPEDWINDOW;
		/*#if FT8XXEMUWIN_DISPLAY_ASPECT_RATIO
		RECT r; r.top = 0; r.left = 0; r.bottom = (LONG)(((float)BT8XXEMU_WINDOW_WIDTH / aspectRatio) * 2); r.right = BT8XXEMU_WINDOW_WIDTH * 2; // window size
		#else*/
		RECT r; r.top = 0; r.left = 0; r.bottom = m_Height * BT8XXEMU_WINDOW_SCALE; r.right = m_Width * BT8XXEMU_WINDOW_SCALE; // window size
																															   /*#endif*/
		AdjustWindowRect(&r, dw_style, FALSE);
		if (m_HWnd)
		{
			FTEMU_error("WindowOutput.begin()  s_HWnd != NULL");
		}
		if (!(m_HWnd = CreateWindow(BT8XXEMU_WINDOW_CLASS_NAME,
			/*(LPCTSTR)title.c_str()*/BT8XXEMU_WINDOW_TITLE, dw_style,
			CW_USEDEFAULT, 0, r.right - r.left, r.bottom - r.top, // x y w h
			NULL, NULL, m_HInstance, NULL)))
		{
			FTEMU_error("GDI Initialisation: %s", SystemWin32::getWin32LastErrorString().c_str());
			return;
		}
		ShowWindow(m_HWnd, /*nCmdShow*/ true); // If the window was previously visible, the return value is nonzero.
		if (!UpdateWindow(m_HWnd)) FTEMU_error("GDI Initialisation: %s", SystemWin32::getWin32LastErrorString().c_str());

		// Create GDI32 Buffer and Device Context
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		HDC hdc = GetDC(m_HWnd);
		if (m_HDC) FTEMU_error("WindowOutput.begin()  s_HDC != NULL");
		m_HDC = CreateCompatibleDC(hdc);
		if (!m_HDC) FTEMU_error("WindowOutput.begin()  s_HDC == NULL: %s", SystemWin32::getWin32LastErrorString().c_str());
		if (m_Buffer) FTEMU_error("WindowOutput.begin()  s_Buffer != NULL");
		m_Buffer = CreateCompatibleBitmap(hdc, m_Width, m_Height);
		if (!m_Buffer) FTEMU_error("WindowOutput.begin()  s_Buffer == NULL: %s", SystemWin32::getWin32LastErrorString().c_str());
		if (m_HDC && m_Buffer) SelectObject(m_HDC, m_Buffer);
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
	FTEMU_message("Destroy GDI window output");

	immediateSync([&]() -> void {

		s_WindowMap.erase(m_HWnd);

#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		if (m_HDC) { DeleteDC(m_HDC); m_HDC = NULL; }
		else FTEMU_warning("WindowOutput::~WindowOutput() m_HDC == NULL");
		if (m_Buffer) { DeleteObject(m_Buffer); m_Buffer = NULL; }
		else FTEMU_warning("WindowOutput::~WindowOutput() m_Buffer == NULL");
#endif

		if (m_HWnd) { DestroyWindow(m_HWnd); m_HWnd = NULL; }
		else FTEMU_warning("WindowOutput::~WindowOutput() m_HWnd == NULL");

		--s_ClassRegCount;
		if (!s_ClassRegCount)
		{
			UnregisterClass(BT8XXEMU_WINDOW_CLASS_NAME, m_HInstance);
		}
		assert(s_ClassRegCount >= 0);
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
#ifdef __GNUC__
	s_LoopQueueMutex.lock();
	while (!s_LoopQueue.empty())
	{
		f = s_LoopQueue.front();
		s_LoopQueue.pop();
		s_LoopQueueMutex.unlock();
		f();
		s_LoopQueueMutex.lock();
	}
	s_LoopQueueMutex.unlock();
#else
	while (s_LoopQueue.try_pop(f))
		f();
#endif

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
		// PostQuitMessage(0);
		break;
	case WM_CLOSE:
		// m_HWnd = NULL;
		m_Close = true;
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
			if (!m_Buffer) FTEMU_error("WindowOutput.setMode(2)  s_Buffer == NULL: %s", SystemWin32::getWin32LastErrorString().c_str());
			HBITMAP oldBuffer = m_Buffer;
			m_Buffer = NULL;

			HDC hdc = GetDC(m_HWnd);
			if (m_Buffer) FTEMU_error("WindowOutput.begin()  s_Buffer != NULL");
			m_Buffer = CreateCompatibleBitmap(hdc, m_Width, m_Height);
			if (!m_Buffer) FTEMU_error("WindowOutput.begin()  s_Buffer == NULL: %s", SystemWin32::getWin32LastErrorString().c_str());
			if (m_HDC && m_Buffer) SelectObject(m_HDC, m_Buffer);
			ReleaseDC(m_HWnd, hdc);

			if (m_HDC && m_Buffer && oldBuffer) DeleteObject(oldBuffer);
			else m_Buffer = oldBuffer;
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

bool WindowOutput::renderBuffer(bool output, bool changed)
{
	if (m_Close)
		return false;

#if BT8XXEMU_GDI_DOUBLE_BUFFER
	bool bufferSwitched = m_BufferSwitched ^ !changed;
	bool bufferFlipping = m_BufferFlipping;
	m_BufferSwitched = !bufferSwitched;
#else
	bool bufferSwitched = false;
	bool bufferFlipping = true;
	m_BufferSwitched = false;
#endif
	m_BufferFlipping = true;
	(bufferFlipping ? immediateSync : immediate)([=]() -> void {
		// Render bitmap to buffer
#if !BT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		if (output)
		{
			if (!SetDIBitsToDevice(m_HDC, 0, 0,
				m_Width, m_Height,
				0, 0, 0, m_Height, bufferSwitched ? m_BufferB : m_BufferA, &m_BitInfo, DIB_RGB_COLORS))
				FTEMU_error("SetDIBitsToDevice  FAILED");
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
			if (!bgBrush) FTEMU_error("GDI Render: %s", SystemWin32::getWin32LastErrorString().c_str());
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
			if (bgBrush)
			{
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
			}
			ReleaseDC(m_HWnd, hdc);
			if (bgBrush && !DeleteObject(bgBrush)) FTEMU_error("GDI Render: %s", SystemWin32::getWin32LastErrorString().c_str());
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
			if (!bgBrush) FTEMU_error("GDI Render: %s", SystemWin32::getWin32LastErrorString().c_str());
			else
			{
				FillRect(hdc, &r, bgBrush);
				if (!DeleteObject(bgBrush)) FTEMU_error("GDI Render: %s", SystemWin32::getWin32LastErrorString().c_str());
				ReleaseDC(m_HWnd, hdc);
			}
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

	return true;
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
