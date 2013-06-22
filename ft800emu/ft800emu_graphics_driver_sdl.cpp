/**
 * GraphicsDriverClass
 * $Id$
 * \file ft800emu_graphics_driver_sdl.cpp
 * \brief GraphicsDriverClass
 * \date 2012-06-27 11:49GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011-2012  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL

// #include <...>
#include "ft800emu_graphics_driver.h"

// System includes
#include "ft800emu_system.h"
#include "ft800emu_system_sdl.h"
#include "ft800emu_ft800_spi_i2c.h"

// Project includes

using namespace std;

#define FT800EMU_WINDOW_TITLE "FT800 Emulator"
#define FT800EMU_WINDOW_WIDTH 480
#define FT800EMU_WINDOW_HEIGHT 272
#define FT800EMU_WINDOW_RATIO (480.0f / 272.0f)
#define FT800EMU_WINDOW_KEEPRATIO 1

namespace FT800EMU {


GraphicsDriverClass GraphicsDriver;

namespace {

static argb1555 s_BufferARGB1555[FT800EMU_WINDOW_WIDTH * FT800EMU_WINDOW_HEIGHT];

SDL_Surface *s_Screen = NULL;
SDL_Surface *s_Buffer = NULL;

SDL_mutex *s_WaitFlipMutex = NULL;
SDL_cond *s_WaitFlip = NULL;
bool s_Running = false;

int flipThread(void *)
{
	SDL_CondWait(s_WaitFlip, s_WaitFlipMutex);
	while (s_Running)
	{
		if (SDL_Flip(s_Screen) < 0)
			SystemSdlClass::ErrorSdl();
		SDL_CondWait(s_WaitFlip, s_WaitFlipMutex);
	}
}

} /* anonymous namespace */

argb1555 *GraphicsDriverClass::getBufferARGB1555()
{
	return s_BufferARGB1555;
}

void GraphicsDriverClass::begin()
{
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	s_Screen = SDL_SetVideoMode(FT800EMU_WINDOW_WIDTH * 2, FT800EMU_WINDOW_HEIGHT * 2, 15, SDL_SWSURFACE | SDL_ASYNCBLIT);
	if (s_Screen == NULL) SystemSdlClass::ErrorSdl();

	SDL_WM_SetCaption(FT800EMU_WINDOW_TITLE, NULL);

	Uint32 bpp;
	Uint32 rmask, gmask, bmask, amask;

	rmask = 0x001F;
	gmask = 0x03E0;
	bmask = 0x7C00;
	amask = 0x0000;

	bpp = 15;

	s_Buffer = SDL_CreateRGBSurfaceFrom(s_BufferARGB1555, FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT, bpp, 2 * FT800EMU_WINDOW_WIDTH, rmask, gmask, bmask, amask);
	if (s_Buffer == NULL) SystemSdlClass::ErrorSdl();

	s_Running = true;

	s_WaitFlip = SDL_CreateCond();
	s_WaitFlipMutex = SDL_CreateMutex();
	SDL_CreateThread(flipThread, NULL);
	// TODO - Error handling

}

bool GraphicsDriverClass::update()
{
	SDL_Event event;

	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
			// don't care about other events
			case SDL_QUIT:
				return false;
		}
	}
	return true;
}

void GraphicsDriverClass::end()
{
	// ... TODO ...
	s_Running = false;
	SDL_CondBroadcast(s_WaitFlip);
	// TODO WAIT FOR THREAD!!!!!
	// TODO DESTROY COND MUTEX

	SDL_FreeSurface(s_Buffer);
	SDL_FreeSurface(s_Screen);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GraphicsDriverClass::renderBuffer()
{
	// TODO: Allow resize and aspect ratio

	/*SDL_Rect destRect;
	destRect.x = 0;
	destRect.y = FT800EMU_WINDOW_HEIGHT * 2;
	destRect.w = FT800EMU_WINDOW_WIDTH * 2;
	destRect.h = FT800EMU_WINDOW_HEIGHT * -2;*/

	// FIXME: This SDL_SoftStretch is terribly slow!
	if (SDL_SoftStretch(s_Buffer, NULL, s_Screen, NULL) < 0)
		SystemSdlClass::ErrorSdl();

	//if (SDL_Flip(s_Screen) < 0)
	//	SystemSdlClass::ErrorSdl();
	//SDL_UpdateRect(s_Screen, 0, 0, 0, 0);
	SDL_CondBroadcast(s_WaitFlip);

	std::stringstream newTitle;
	newTitle << FT800EMU_WINDOW_TITLE;
	if (FT800SPI.getRam()[0x2809] == 0)
		newTitle << (" [+J1]");
	newTitle << (" [FPS: ");
	newTitle << System.getFPSSmooth();
	newTitle << (" (");
	newTitle << System.getFPS();
	newTitle << (")]");
	SDL_WM_SetCaption(newTitle.str().c_str(), NULL);

	/*
	// Render bitmap to buffer
#if !FT800EMU_GRAPHICS_USE_STRETCHDIBITS
	if (!SetDIBitsToDevice(s_HDC, 0, 0,
		FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT,
		0, 0, 0, FT800EMU_WINDOW_HEIGHT, s_BufferARGB1555, &s_BitInfo, DIB_RGB_COLORS))
		SystemWindows.Error(TEXT("SetDIBitsToDevice  FAILED"));
#endif

	// Draw buffer to screen
	RECT r;
	GetClientRect(s_HWnd, &r);
#if FT800EMU_WINDOW_KEEPRATIO
	{
		argb1555 bgC16 = ((argb1555 *)(void *)(&FT800SPI.getRam()[0x280e]))[0];
		COLORREF bgC32 = RGB((((bgC16) & 0x1F) * 255 / 31),
			(((bgC16 >> 5) & 0x1F) * 255 / 31),
			(((bgC16 >> 10) & 0x1F) * 255 / 31));
		HBRUSH bgBrush = CreateSolidBrush(bgC32);
		if (bgBrush == NULL) SystemWindows.ErrorWin32();
		int width_r = (int)((float)r.bottom * FT800EMU_WINDOW_RATIO); int height_r;
		if (width_r > r.right) { width_r = r.right; height_r = (int)((float)r.right / FT800EMU_WINDOW_RATIO); }
		else height_r = r.bottom;
		int x_r = (r.right - width_r) / 2;
		int y_r = (r.bottom - height_r) / 2;
		HDC hdc = GetDC(s_HWnd);
#if !FT800EMU_GRAPHICS_USE_STRETCHDIBITS
		StretchBlt(hdc, x_r, y_r, width_r, height_r, s_HDC, 0, 0, FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT, SRCCOPY);
#else
		StretchDIBits(hdc, x_r, y_r, width_r, height_r,	0, 0, FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT, s_BufferARGB1555, &s_BitInfo, DIB_RGB_COLORS, SRCCOPY);
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
		if (!DeleteObject(bgBrush)) SystemWindows.ErrorWin32();
	}
#else
	{
		HDC hdc = GetDC(s_HWnd);
#if !FT800EMU_GRAPHICS_USE_STRETCHDIBITS
		StretchBlt(hdc, 0, 0, r.right, r.bottom, s_HDC, 0, 0, FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT, SRCCOPY);
#else
		StretchDIBits(hdc, 0, 0, r.right, r.bottom, 0, 0, FT800EMU_WINDOW_WIDTH, FT800EMU_WINDOW_HEIGHT, s_BufferARGB1555, &s_BitInfo, DIB_RGB_COLORS, SRCCOPY);
#endif
		ReleaseDC(s_HWnd, hdc);
	}
#endif

	// Update title
	tstringstream newTitle;
	newTitle << FT800EMU_WINDOW_TITLE;
	if (FT800SPI.getRam()[0x2809] == 0)
		newTitle << TEXT(" [+J1]");
	newTitle << TEXT(" [FPS: ");
	newTitle << System.getFPSSmooth();
	newTitle << TEXT(" (");
	newTitle << System.getFPS();
	newTitle << TEXT(")]");
	SetWindowText(s_HWnd, (LPCTSTR)newTitle.str().c_str());*/
}

} /* namespace FT800EMU */

#endif /* #ifdef FT800EMU_SDL */

/* end of file */
