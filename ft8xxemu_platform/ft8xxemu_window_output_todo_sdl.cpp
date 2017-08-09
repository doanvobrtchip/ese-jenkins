/**
 * WindowOutput
 * $Id$
 * \file ft8xxemu_window_output_sdl.cpp
 * \brief WindowOutput
 * \date 2012-06-27 11:49GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if 0

#ifdef FTEMU_SDL
#ifndef FTEMU_SDL2

// #include <...>
#include "ft8xxemu_window_output.h"

// System includes
#include "ft8xxemu_system.h"
#include "ft8xxemu_system_sdl.h"
#include "ft8xxemu_spi_i2c.h"

namespace FT8XXEMU {

WindowOutput WindowOutput;

void (*g_ResetTouchScreenXY)(int idx) = NULL;
void (*g_SetTouchScreenXY)(int idx, int x, int y, int pressure) = NULL;

static int s_Width = BT8XXEMU_WINDOW_WIDTH_DEFAULT;
static int s_Height = BT8XXEMU_WINDOW_HEIGHT_DEFAULT;
static float s_Ratio = BT8XXEMU_WINDOW_RATIO_DEFAULT;

static bool s_MouseEnabled;
static int s_MousePressure;

static int s_MouseX;
static int s_MouseY;
static int s_MouseDown;

#define FTEMU_SDL_THREADED_FLIP 0

namespace {

static argb8888 s_BufferARGB8888[BT8XXEMU_WINDOW_WIDTH_MAX * BT8XXEMU_WINDOW_HEIGHT_MAX];

SDL_Surface *s_Screen = NULL;
SDL_Surface *s_Buffer = NULL;

#if FTEMU_SDL_THREADED_FLIP

SDL_Thread *s_FlipThread = NULL;

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
	return 0;
}

#endif

} /* anonymous namespace */

argb8888 *WindowOutput::getBufferARGB8888()
{
	return s_BufferARGB8888;
}

bool WindowOutput::begin()
{
	s_Width = BT8XXEMU_WINDOW_WIDTH_DEFAULT;
	s_Height = BT8XXEMU_WINDOW_HEIGHT_DEFAULT;
	s_Ratio = BT8XXEMU_WINDOW_RATIO_DEFAULT;

	SDL_InitSubSystem(SDL_INIT_VIDEO);

	s_Screen = SDL_SetVideoMode(s_Width * BT8XXEMU_WINDOW_SCALE, s_Height * BT8XXEMU_WINDOW_SCALE, 32, SDL_SWSURFACE | SDL_ASYNCBLIT);
	if (s_Screen == NULL) SystemSdlClass::ErrorSdl();

	SDL_WM_SetCaption(BT8XXEMU_WINDOW_TITLE_A, NULL);

	Uint32 bpp;
	Uint32 rmask, gmask, bmask, amask;

	rmask = 0x00FF0000;
	gmask = 0x0000FF00;
	bmask = 0x000000FF;
	amask = 0x00000000;

	bpp = 32;

	s_Buffer = SDL_CreateRGBSurfaceFrom(s_BufferARGB8888, s_Width, s_Height, bpp, 4 * s_Width, rmask, gmask, bmask, amask);
	if (s_Buffer == NULL) SystemSdlClass::ErrorSdl();

#if FTEMU_SDL_THREADED_FLIP

	s_Running = true;
	s_WaitFlip = SDL_CreateCond();
	s_WaitFlipMutex = SDL_CreateMutex();
	s_FlipThread = SDL_CreateThreadFT(flipThread, NULL);

#endif

	// TODO - Error handling
	return true;
}

bool WindowOutput::update()
{
	SDL_Event event;

	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
			// don't care about other events
			case SDL_QUIT:
				return false;
		}
	}

	int mouseX, mouseY;
	int button = SDL_GetMouseState(&mouseX, &mouseY);
	s_MouseDown = button & 1;
	s_MouseX = mouseX / BT8XXEMU_WINDOW_SCALE;
	s_MouseY = mouseY / BT8XXEMU_WINDOW_SCALE;
	if (s_MouseDown)
	{
		g_SetTouchScreenXY(0, s_MouseX, s_MouseY, s_MousePressure);
	}
	else
	{
		g_ResetTouchScreenXY(0);
	}

	return true;
}

void WindowOutput::end()
{
#if FTEMU_SDL_THREADED_FLIP

	s_Running = false;
	SDL_CondBroadcast(s_WaitFlip);
	SDL_WaitThread(s_FlipThread, NULL);
	s_FlipThread = NULL;

	SDL_DestroyMutex(s_WaitFlipMutex);
	s_WaitFlipMutex = NULL;
	SDL_DestroyCond(s_WaitFlip);
	s_WaitFlip = NULL;

#endif

	SDL_FreeSurface(s_Buffer);
	s_Buffer = NULL;
	SDL_FreeSurface(s_Screen);
	s_Screen = NULL;

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void WindowOutput::setMode(int width, int height)
{
	if (s_Width != width || s_Height != height)
	{
#if FTEMU_SDL_THREADED_FLIP
		// Stop the flip thread
		s_Running = false;
		SDL_CondBroadcast(s_WaitFlip);
		SDL_WaitThread(s_FlipThread, NULL);
		s_FlipThread = NULL;
#endif

		// Update values
		s_Width = width;
		s_Height = height;
		s_Ratio = (float)width / (float)height;

		// Change the screen mode
		s_Screen = SDL_SetVideoMode(s_Width * BT8XXEMU_WINDOW_SCALE, s_Height * BT8XXEMU_WINDOW_SCALE, 32, SDL_SWSURFACE | SDL_ASYNCBLIT);
		if (s_Screen == NULL) SystemSdlClass::ErrorSdl();

		// Release the buffer
		SDL_FreeSurface(s_Buffer);
		s_Buffer = NULL;

		// Reinitialize the buffer
		Uint32 bpp;
		Uint32 rmask, gmask, bmask, amask;
		rmask = 0x00FF0000;
		gmask = 0x0000FF00;
		bmask = 0x000000FF;
		amask = 0x00000000;
		bpp = 32;
		s_Buffer = SDL_CreateRGBSurfaceFrom(s_BufferARGB8888, s_Width, s_Height, bpp, 4 * s_Width, rmask, gmask, bmask, amask);
		if (s_Buffer == NULL) SystemSdlClass::ErrorSdl();

#if FTEMU_SDL_THREADED_FLIP
		// Resume the flip thread
		s_FlipThread = SDL_CreateThreadFT(flipThread, NULL);
#endif
	}
}

void WindowOutput::renderBuffer(bool output, bool changed)
{
	// TODO: Allow user resize and aspect ratio

	if (output)
	{
		// FIXME: SDL_SoftStretch is terribly slow and does not
                // convert pixel format!
		if (SDL_BlitSurface(s_Buffer, NULL, s_Screen, NULL) < 0)
			SystemSdlClass::ErrorSdl();
	}
	else
	{
		if (SDL_FillRect(s_Screen, NULL, 0xFF808080) < 0)
			SystemSdlClass::ErrorSdl();
	}

#if FTEMU_SDL_THREADED_FLIP
	SDL_CondBroadcast(s_WaitFlip);
#else
	if (SDL_Flip(s_Screen) < 0)
		SystemSdlClass::ErrorSdl();
#endif

	std::stringstream newTitle;
	newTitle << BT8XXEMU_WINDOW_TITLE_A;
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
	newTitle << " [FPS: ";
	newTitle << System.getFPSSmooth();
	newTitle << " (";
	newTitle << System.getFPS();
	newTitle << ")]";
	SDL_WM_SetCaption(newTitle.str().c_str(), NULL);
}

void WindowOutput::enableMouse(bool enabled)
{
	s_MouseEnabled = enabled;
}

void WindowOutput::setMousePressure(int pressure)
{
	s_MousePressure = pressure;
}

} /* namespace FT8XXEMU */

#endif /* #ifndef FTEMU_SDL2 */
#endif /* #ifdef FTEMU_SDL */

#endif

/* end of file */
