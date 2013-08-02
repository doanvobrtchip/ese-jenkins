/**
 * GraphicsDriverClass
 * $Id$
 * \file ft800emu_graphics_driver_sdl.cpp
 * \brief GraphicsDriverClass
 * \date 2012-06-27 11:49GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL

// #include <...>
#include "ft800emu_graphics_driver.h"

// System includes
#include "ft800emu_system.h"
#include "ft800emu_system_sdl.h"
#include "ft800emu_spi_i2c.h"

// Project includes

using namespace std;

namespace FT800EMU {

GraphicsDriverClass GraphicsDriver;

static int s_Width = FT800EMU_WINDOW_WIDTH_DEFAULT;
static int s_Height = FT800EMU_WINDOW_HEIGHT_DEFAULT;
static float s_Ratio = FT800EMU_WINDOW_RATIO_DEFAULT;

static bool s_MouseEnabled;
static int s_MousePressure;

#define FT800EMU_SDL_THREADED_FLIP 0

namespace {

static argb8888 s_BufferARGB8888[FT800EMU_WINDOW_WIDTH_MAX * FT800EMU_WINDOW_HEIGHT_MAX];

SDL_Surface *s_Screen = NULL;
SDL_Surface *s_Buffer = NULL;

#if FT800EMU_SDL_THREADED_FLIP

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

argb8888 *GraphicsDriverClass::getBufferARGB8888()
{
	return s_BufferARGB8888;
}

void GraphicsDriverClass::begin()
{
	s_Width = FT800EMU_WINDOW_WIDTH_DEFAULT;
	s_Height = FT800EMU_WINDOW_HEIGHT_DEFAULT;
	s_Ratio = FT800EMU_WINDOW_RATIO_DEFAULT;

	SDL_InitSubSystem(SDL_INIT_VIDEO);

	s_Screen = SDL_SetVideoMode(s_Width * FT800EMU_WINDOW_SCALE, s_Height * FT800EMU_WINDOW_SCALE, 32, SDL_SWSURFACE | SDL_ASYNCBLIT);
	if (s_Screen == NULL) SystemSdlClass::ErrorSdl();

	SDL_WM_SetCaption(FT800EMU_WINDOW_TITLE_A, NULL);

	Uint32 bpp;
	Uint32 rmask, gmask, bmask, amask;

	rmask = 0x00FF0000;
	gmask = 0x0000FF00;
	bmask = 0x000000FF;
	amask = 0x00000000;

	bpp = 32;

	s_Buffer = SDL_CreateRGBSurfaceFrom(s_BufferARGB8888, s_Width, s_Height, bpp, 4 * s_Width, rmask, gmask, bmask, amask);
	if (s_Buffer == NULL) SystemSdlClass::ErrorSdl();
	
#if FT800EMU_SDL_THREADED_FLIP

	s_Running = true;
	s_WaitFlip = SDL_CreateCond();
	s_WaitFlipMutex = SDL_CreateMutex();
	s_FlipThread = SDL_CreateThread(flipThread, NULL);
	
#endif

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
#if FT800EMU_SDL_THREADED_FLIP

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

void GraphicsDriverClass::setMode(int width, int height)
{
	if (s_Width != width || s_Height != height)
	{
#if FT800EMU_SDL_THREADED_FLIP
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
		s_Screen = SDL_SetVideoMode(s_Width * FT800EMU_WINDOW_SCALE, s_Height * FT800EMU_WINDOW_SCALE, 32, SDL_SWSURFACE | SDL_ASYNCBLIT);
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

#if FT800EMU_SDL_THREADED_FLIP
		// Resume the flip thread
		s_FlipThread = SDL_CreateThread(flipThread, NULL);
#endif
	}
}

void GraphicsDriverClass::renderBuffer(bool output)
{
	// TODO: Allow user resize and aspect ratio

	if (output)
	{
		// FIXME: This SDL_SoftStretch is terribly slow!
		if (SDL_SoftStretch(s_Buffer, NULL, s_Screen, NULL) < 0)
			SystemSdlClass::ErrorSdl();
	}
	else
	{
		if (SDL_FillRect(s_Screen, NULL, 0xFF808080) < 0)
			SystemSdlClass::ErrorSdl();
	}

#if FT800EMU_SDL_THREADED_FLIP
	SDL_CondBroadcast(s_WaitFlip);
#else
	if (SDL_Flip(s_Screen) < 0)
		SystemSdlClass::ErrorSdl();
#endif

	std::stringstream newTitle;
	newTitle << FT800EMU_WINDOW_TITLE_A;
	newTitle << (" [FPS: ");
	newTitle << System.getFPSSmooth();
	newTitle << (" (");
	newTitle << System.getFPS();
	newTitle << (")]");
	SDL_WM_SetCaption(newTitle.str().c_str(), NULL);
}

void GraphicsDriverClass::enableMouse(bool enabled)
{
	s_MouseEnabled = enabled;
}

void GraphicsDriverClass::setMousePressure(int pressure)
{
	s_MousePressure = pressure;
}

} /* namespace FT800EMU */

#endif /* #ifdef FT800EMU_SDL */

/* end of file */
