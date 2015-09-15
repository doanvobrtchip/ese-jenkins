/**
 * GraphicsDriverClass
 * $Id$
 * \file ft8xxemu_graphics_driver_gdi.cpp
 * \brief GraphicsDriverClass
 * \date 2011-05-26 18:14GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEMU_SDL
#ifdef WIN32

// #include <...>
#include "ft8xxemu_graphics_driver.h"

#ifdef FTEMU_SDL2
#define GraphicsDriverClass GraphicsDriverClassGDI
#define GraphicsDriver GraphicsDriverGDI
#endif

// System includes
#include "ft8xxemu_system_windows.h"
#include <Windowsx.h>
#include "ft8xxemu_system.h"

#define FT8XXEMU_WINDOW_CLASS_NAME TEXT("FT8XXEMUGraphicsDriver")

// Getting more CPU usage with StretchDIBits for some reason, so I don't use it.
#define FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS 0

namespace FT8XXEMU {

GraphicsDriverClass GraphicsDriver;

#ifndef FTEMU_SDL2
void (*g_ResetTouchScreenXY)(int idx) = NULL;
void (*g_SetTouchScreenXY)(int idx, int x, int y, int pressure) = NULL;
#endif

static argb8888 s_BufferARGB8888[FT8XXEMU_WINDOW_WIDTH_MAX * FT8XXEMU_WINDOW_HEIGHT_MAX];

static HINSTANCE s_HInstance = NULL;
static HWND s_HWnd = NULL;
//static int s_ViewWidth, s_ViewHeight;
//static float s_AspectRatio;
//static bool s_KeepRatio;
static std::map<UINT, WNDPROC> s_WindowProcedures;
//static ULONG_PTR s_GdiplusToken;
#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
static HBITMAP s_Buffer = NULL;
static HDC s_HDC = NULL;//, m_WindowGraphics;
#endif
//static Gdiplus::Graphics *m_GraphicsPtr;



BITMAPINFO s_BitInfo;
static int s_Width = FT8XXEMU_WINDOW_WIDTH_DEFAULT;
static int s_Height = FT8XXEMU_WINDOW_HEIGHT_DEFAULT;
static float s_Ratio = FT8XXEMU_WINDOW_RATIO_DEFAULT;

static bool s_MouseEnabled;
static int s_MousePressure;

static int s_MouseX;
static int s_MouseY;
static int s_MouseDown;

argb8888 *GraphicsDriverClass::getBufferARGB8888()
{
	return s_BufferARGB8888;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if (s_MouseEnabled)
		{
			s_MouseDown = wParam & MK_LBUTTON;
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			RECT r;
			GetClientRect(s_HWnd, &r);
			int width_r = (int)((float)r.bottom * s_Ratio); int height_r;
			if (width_r > r.right) { width_r = r.right; height_r = (int)((float)r.right / s_Ratio); }
			else height_r = r.bottom;
			int x_r = (r.right - width_r) / 2;
			int y_r = (r.bottom - height_r) / 2;
			s_MouseX = (mouseX - x_r) * s_Width / width_r;
			s_MouseY = (mouseY - y_r) * s_Height / height_r;
			if (s_MouseDown && s_MouseX >= 0 && s_MouseX < s_Width && s_MouseY >= 0 && s_MouseY < s_Height)
			{
				g_SetTouchScreenXY(0, s_MouseX, s_MouseY, s_MousePressure);
			}
			else
			{
				s_MouseDown = false;
				g_ResetTouchScreenXY(0);
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		s_HWnd = NULL;
		break;
	case WM_ERASEBKGND:
		return (LRESULT)1; // Say we handled it.
	case WM_SYSCOMMAND:
		if (wParam == SC_KEYMENU) return 0;
		return DefWindowProc(hWnd, message, wParam, lParam);
	default:
		std::map<UINT, WNDPROC>::iterator it = s_WindowProcedures.find(message);
		if (it != s_WindowProcedures.end())
			return it->second(hWnd, message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return (LRESULT)0;
}

bool GraphicsDriverClass::begin()
{
	// Defaults
	s_MouseEnabled = false;
	s_MousePressure = 0;
	s_MouseX = 0;
	s_MouseY = 0;
	s_MouseDown = false;

	// Save params
	s_HInstance = GetModuleHandle(NULL);

	// Register Display Class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpszClassName = FT8XXEMU_WINDOW_CLASS_NAME;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = s_HInstance;
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
	RECT r; r.top = 0; r.left = 0; r.bottom = (LONG)(((float)FT8XXEMU_WINDOW_WIDTH / aspectRatio) * 2); r.right = FT8XXEMU_WINDOW_WIDTH * 2; // window size
#else*/
	RECT r; r.top = 0; r.left = 0; r.bottom = s_Height * FT8XXEMU_WINDOW_SCALE; r.right = s_Width * FT8XXEMU_WINDOW_SCALE; // window size
/*#endif*/
	AdjustWindowRect(&r, dw_style, FALSE);
	if (s_HWnd) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_HWnd != NULL"));
	if (!(s_HWnd = CreateWindow(FT8XXEMU_WINDOW_CLASS_NAME,
		/*(LPCTSTR)title.c_str()*/FT8XXEMU_WINDOW_TITLE, dw_style,
		CW_USEDEFAULT, 0, r.right - r.left, r.bottom - r.top, // x y w h
		NULL, NULL, s_HInstance, NULL)))
		SystemWindows.ErrorWin32(TEXT("GDI Initialisation"));
	ShowWindow(s_HWnd, /*nCmdShow*/ true); // If the window was previously visible, the return value is nonzero.
	if (!UpdateWindow(s_HWnd)) SystemWindows.ErrorWin32(TEXT("GDI Initialisation"));

	// Create GDI32 Buffer and Device Context
#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
	HDC hdc = GetDC(s_HWnd);
	if (s_HDC) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_HDC != NULL"));
	s_HDC = CreateCompatibleDC(hdc);
	if (!s_HDC) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_HDC == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
	if (s_Buffer) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_Buffer != NULL"));
	s_Buffer = CreateCompatibleBitmap(hdc, s_Width, s_Height);
	if (!s_Buffer) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
	SelectObject(s_HDC, s_Buffer);
	ReleaseDC(s_HWnd, hdc);
#endif

	// Verify Maps
	//if (!Bitmap::m_HBitmaps.empty()) SystemWindows.Error(TEXT("GraphicsDriver.begin()  Bitmap::m_HBitmaps.empty() == false"));
	if (!s_WindowProcedures.empty()) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_WindowProcedures.empty() == false"));

	// Initialize Bitmap Buffer
	ZeroMemory(&s_BitInfo, sizeof(s_BitInfo));
    s_BitInfo.bmiHeader.biSize = sizeof(s_BitInfo.bmiHeader);
	s_BitInfo.bmiHeader.biWidth = s_Width;
    s_BitInfo.bmiHeader.biHeight = s_Height;
    s_BitInfo.bmiHeader.biPlanes = 1;
    s_BitInfo.bmiHeader.biBitCount = 32;
    s_BitInfo.bmiHeader.biCompression = BI_RGB;

	// Meh
	SystemWindows.setHWnd(s_HWnd);

	return true;
}

bool GraphicsDriverClass::update()
{
	// Update Window Messages
	MSG msg;
	while (PeekMessage(&msg, /*m_HWnd*/ NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return s_HWnd != NULL;
}

void GraphicsDriverClass::end()
{
	SystemWindows.setHWnd(NULL);

	if (!s_WindowProcedures.empty())
	{
		SystemWindows.Debug(TEXT("GraphicsDriver.end() s_WindowProcedures.empty() == false"));
		s_WindowProcedures.clear();
	}

#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
	if (s_HDC) { DeleteDC(s_HDC); s_HDC = NULL; }
	else SystemWindows.Debug(TEXT("GraphicsDriver.end() s_HDC == NULL"));
	if (s_Buffer) { DeleteObject(s_Buffer); s_Buffer = NULL; }
	else SystemWindows.Debug(TEXT("GraphicsDriver.end() s_Buffer == NULL"));
#endif

	if (s_HWnd) { DestroyWindow(s_HWnd); s_HWnd = NULL; }
	else SystemWindows.Debug(TEXT("GraphicsDriver.end() s_HWnd == NULL"));

	UnregisterClass(FT8XXEMU_WINDOW_CLASS_NAME, s_HInstance);
}

void GraphicsDriverClass::setMode(int width, int height)
{
	if (s_Width != width || s_Height != height)
	{
		s_Width = width;
		s_Height = height;
		s_Ratio = (float)width / (float)height;

#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		if (!s_Buffer) SystemWindows.Error(TEXT("GraphicsDriver.setMode(2)  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
		HBITMAP oldBuffer = s_Buffer;
		s_Buffer = NULL;

		HDC hdc = GetDC(s_HWnd);
		if (s_Buffer) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_Buffer != NULL"));
		s_Buffer = CreateCompatibleBitmap(hdc, s_Width, s_Height);
		if (!s_Buffer) SystemWindows.Error(TEXT("GraphicsDriver.begin()  s_Buffer == NULL\r\n") + SystemWindows.GetWin32LastErrorString());
		SelectObject(s_HDC, s_Buffer);
		ReleaseDC(s_HWnd, hdc);

		DeleteObject(oldBuffer);
#endif

		DWORD dw_style = WS_OVERLAPPEDWINDOW;
		RECT r; r.top = 0; r.left = 0; r.bottom = s_Height * FT8XXEMU_WINDOW_SCALE; r.right = s_Width * FT8XXEMU_WINDOW_SCALE; // window size
		AdjustWindowRect(&r, dw_style, FALSE);

		SetWindowPos(s_HWnd, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
		s_BitInfo.bmiHeader.biWidth = s_Width;
		s_BitInfo.bmiHeader.biHeight = s_Height;
	}
}

void GraphicsDriverClass::renderBuffer(bool output, bool changed)
{
	// Render bitmap to buffer
#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
	if (output)
	{
		if (!SetDIBitsToDevice(s_HDC, 0, 0,
			s_Width, s_Height,
			0, 0, 0, s_Height, s_BufferARGB8888, &s_BitInfo, DIB_RGB_COLORS))
			SystemWindows.Error(TEXT("SetDIBitsToDevice  FAILED"));
	}
#endif

	// Draw buffer to screen
	RECT r;
	GetClientRect(s_HWnd, &r);
	if (output)
#if FT8XXEMU_WINDOW_KEEPRATIO
	{
		COLORREF bgC32 = RGB(128, 128, 128); // bg outside render
		HBRUSH bgBrush = CreateSolidBrush(bgC32);
		if (bgBrush == NULL) SystemWindows.ErrorWin32(TEXT("GDI Render"));
		int width_r = (int)((float)r.bottom * s_Ratio); int height_r;
		if (width_r > r.right) { width_r = r.right; height_r = (int)((float)r.right / s_Ratio); }
		else height_r = r.bottom;
		int x_r = (r.right - width_r) / 2;
		int y_r = (r.bottom - height_r) / 2;
		HDC hdc = GetDC(s_HWnd);
#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
		StretchBlt(hdc, x_r, y_r, width_r, height_r, s_HDC, 0, 0, s_Width, s_Height, SRCCOPY);
#else
		StretchDIBits(hdc, x_r, y_r, width_r, height_r,	0, 0, s_Width, s_Height, s_BufferARGB8888, &s_BitInfo, DIB_RGB_COLORS, SRCCOPY);
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
		ReleaseDC(s_HWnd, hdc);
		if (!DeleteObject(bgBrush)) SystemWindows.ErrorWin32(TEXT("GDI Render"));
	}
#else
	{
		HDC hdc = GetDC(s_HWnd);
#if !FT8XXEMU_GRAPHICS_USE_STRETCHDIBITS
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
		HDC hdc = GetDC(s_HWnd);
		COLORREF bgC32 = RGB(128, 128, 128); // bg off render
		HBRUSH bgBrush = CreateSolidBrush(bgC32);
		if (bgBrush == NULL) SystemWindows.ErrorWin32(TEXT("GDI Render"));
		FillRect(hdc, &r, bgBrush);
		if (!DeleteObject(bgBrush)) SystemWindows.ErrorWin32(TEXT("GDI Render"));
		ReleaseDC(s_HWnd, hdc);
	}

	// Update title
	/*
	tstringstream newTitle;
	newTitle << FT8XXEMU_WINDOW_TITLE;
	switch (GraphicsProcessor.getDebugMode())
	{
	case FT8XXEMU_DEBUGMODE_ALPHA:
		newTitle << " [ALPHA";
		break;
	case FT8XXEMU_DEBUGMODE_TAG:
		newTitle << " [TAG";
		break;
	case FT8XXEMU_DEBUGMODE_STENCIL:
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
}

void GraphicsDriverClass::enableMouse(bool enabled)
{
	s_MouseEnabled = enabled;
}

void GraphicsDriverClass::setMousePressure(int pressure)
{
	s_MousePressure = pressure;
}

} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */
#endif /* #ifndef FTEMU_SDL */

/* end of file */
