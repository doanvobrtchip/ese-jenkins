/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifdef FTEMU_SDL2

// #include <...>
#include "ft8xxemu_graphics_driver.h"

// System includes
#ifdef WIN32
#	define NOMINMAX
#	include <Windows.h>
#endif
#include <GL/gl.h>
#include "ft8xxemu_system.h"
#include "ft8xxemu_system_sdl.h"

// Library includes
#ifdef WIN32
#	include <SDL_syswm.h>
#endif

#ifndef GL_CLAMP_TO_BORDER
#	define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_SWIZZLE_RGBA
#	define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#endif

#if defined(FTEMU_SDL2)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, name, data)
#elif defined(FTEMU_SDL)
#define SDL_CreateThreadFT(fn, name, data) SDL_CreateThread(fn, data)
#endif

using namespace std;

namespace FT8XXEMU {

GraphicsDriverClass GraphicsDriver;

void(*g_ResetTouchScreenXY)(int idx) = NULL;
void(*g_SetTouchScreenXY)(int idx, int x, int y, int pressure) = NULL;

static int s_Width = FT8XXEMU_WINDOW_WIDTH_DEFAULT;
static int s_Height = FT8XXEMU_WINDOW_HEIGHT_DEFAULT;
static float s_Ratio = FT8XXEMU_WINDOW_RATIO_DEFAULT;

static int s_WidthCur = -1;
static int s_HeightCur = -1;

static int s_WindowWidth;
static int s_WindowHeight;
static bool s_WindowResized;

static float s_LetterBoxX = 0.0f;
static int s_LetterBoxXPix = 0;
static float s_LetterBoxY = 0.0f;
static int s_LetterBoxYPix = 0;

static bool s_MouseEnabled;
static int s_MousePressure;

static int s_MouseX;
static int s_MouseY;
static int s_MouseDown;

// Threaded flip causes the buffer to be flipped from a separate thread.
// It saves time for the CPU processing
// Disabled because not compatible with dynamic degrade yet
// Must be 0 or 1
#define FT8XXEMU_THREADED_FLIP 0

// Hardware double buffer adds an additional copy to the output.
// It is technically redundant, as only one texture is drawn, but may improve performance in some window managers
#define FT8XXEMU_HARDWARE_DOUBLE_BUFFER 0

// Number of frames to skip for refreshing the window title
#define FT8XXEMU_TITLE_FRAMESKIP 3

// Whether to use a dynamic title
// It seems this is very slow under SDL2 on Linux
#if defined(WIN32)
#	define FT8XXEMU_TITLE_DYNAMIC 0 // TEMPORARILY DISABLED DUE TO LINKAGE PRIORITIES // TODO
#else
#	define FT8XXEMU_TITLE_DYNAMIC 0
#endif

namespace {

SDL_Window* s_Window = NULL;
SDL_GLContext s_GLContext = NULL;

argb8888 s_BufferARGB8888[FT8XXEMU_THREADED_FLIP + 1][FT8XXEMU_WINDOW_WIDTH_MAX * FT8XXEMU_WINDOW_HEIGHT_MAX];
#if FT8XXEMU_THREADED_FLIP
SDL_mutex *s_BufferMutex[FT8XXEMU_THREADED_FLIP + 1] = { NULL, NULL };
#endif
GLuint s_BufferTexture = 0;
#if FT8XXEMU_THREADED_FLIP
int s_BufferCurrent = 0;
#else
const int s_BufferCurrent = 0;
#endif

#if FT8XXEMU_THREADED_FLIP

SDL_Thread *s_FlipThread = NULL;

SDL_mutex *s_WaitFlipMutex = NULL;
SDL_cond *s_WaitFlip = NULL;
bool s_Running = false;

#endif

bool s_Output = false;

#if FT8XXEMU_TITLE_DYNAMIC
int s_TitleFrameSkip = 0;
#endif

void drawBuffer()
{
	if (s_WindowResized)
	{
		glViewport(0, 0, s_WindowWidth, s_WindowHeight);
		s_WindowResized = false;
		float windowRatio = (float)s_WindowWidth / (float)s_WindowHeight;
		if (windowRatio > s_Ratio) // Window is wider
		{
			float box = ((windowRatio / s_Ratio) - 1.0f) * 0.5f;
			s_LetterBoxX = box;
			s_LetterBoxY = 0;
		}
		else if (windowRatio < s_Ratio)
		{
			float box = ((s_Ratio / windowRatio) - 1.0f) * 0.5f;
			s_LetterBoxX = 0;
			s_LetterBoxY = box;
		}
		else
		{
			s_LetterBoxX = 0;
			s_LetterBoxY = 0;
		}
		s_LetterBoxXPix = (int)(s_LetterBoxX * (float)s_Width * ((float)s_WindowHeight / (float)s_Height));
		s_LetterBoxYPix = (int)(s_LetterBoxY * (float)s_Height * ((float)s_WindowWidth / (float)s_Width));
	}

	if (s_Output)
	{
#if FT8XXEMU_THREADED_FLIP
		int buffer = (s_BufferCurrent + 1) % 2;
		if (!SDL_LockMutex(s_BufferMutex[buffer]))
#else
		int buffer = s_BufferCurrent;
#endif
		{
			if (s_Width != s_WidthCur || s_Height != s_HeightCur)
			{
				s_WidthCur = s_Width;
				s_HeightCur = s_Height;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_Width, s_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_BufferARGB8888[buffer]);
			}
			else
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s_WidthCur, s_HeightCur, GL_RGBA, GL_UNSIGNED_BYTE, s_BufferARGB8888[buffer]);
			}

			glEnable(GL_TEXTURE_2D);

			glBegin(GL_QUADS);

#if FT8XXEMU_FLIP_SDL2
			glTexCoord2f(-s_LetterBoxX, -s_LetterBoxY);
			glVertex3f(-1, -1, 0);

			glTexCoord2f(1.0f + s_LetterBoxX, -s_LetterBoxY);
			glVertex3f(1, -1, 0);

			glTexCoord2f(1.0f + s_LetterBoxX, 1.0f + s_LetterBoxY);
			glVertex3f(1, 1, 0);

			glTexCoord2f(-s_LetterBoxX, 1.0f + s_LetterBoxY);
			glVertex3f(-1, 1, 0);
#else
			glTexCoord2f(-s_LetterBoxX, 1.0f + s_LetterBoxY);
			glVertex3f(-1, -1, 0);

			glTexCoord2f(1.0f + s_LetterBoxX, 1.0f + s_LetterBoxY);
			glVertex3f(1, -1, 0);

			glTexCoord2f(1.0f + s_LetterBoxX, -s_LetterBoxY);
			glVertex3f(1, 1, 0);

			glTexCoord2f(-s_LetterBoxX, -s_LetterBoxY);
			glVertex3f(-1, 1, 0);
#endif

			glEnd();

			glDisable(GL_TEXTURE_2D);

			glFinish();

#if FT8XXEMU_THREADED_FLIP
			SDL_UnlockMutex(s_BufferMutex[buffer]);
#endif
		}
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();
	}
#if FT8XXEMU_HARDWARE_DOUBLE_BUFFER
	SDL_GL_SwapWindow(s_Window);
#endif
}

#if FT8XXEMU_THREADED_FLIP

int flipThread(void *)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

	if (SDL_GL_MakeCurrent(s_Window, s_GLContext))
	{
		printf("SDL_GL_MakeCurrent failed\n");
		exit(-1);
	}

	SDL_CondWait(s_WaitFlip, s_WaitFlipMutex);
	while (s_Running)
	{
		drawBuffer();
		SDL_CondWait(s_WaitFlip, s_WaitFlipMutex);
	}

	SDL_GL_MakeCurrent(s_Window, NULL);

	return 0;
}

#endif

} /* anonymous namespace */

argb8888 *GraphicsDriverClass::getBufferARGB8888()
{
	// Return the next available buffer
	// Wait in case it's still being rendered
#if FT8XXEMU_THREADED_FLIP
	if (!SDL_LockMutex(s_BufferMutex[s_BufferCurrent]))
		SDL_UnlockMutex(s_BufferMutex[s_BufferCurrent]);
#endif
	return s_BufferARGB8888[s_BufferCurrent];
}

void GraphicsDriverClass::begin()
{
	s_Width = FT8XXEMU_WINDOW_WIDTH_DEFAULT;
	s_Height = FT8XXEMU_WINDOW_HEIGHT_DEFAULT;
	s_Ratio = FT8XXEMU_WINDOW_RATIO_DEFAULT;

	uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

#if FT8XXEMU_THREADED_FLIP
	s_BufferMutex[0] = SDL_CreateMutex();
	s_BufferMutex[1] = SDL_CreateMutex();
#endif

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
#if !FT8XXEMU_HARDWARE_DOUBLE_BUFFER
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0); // We do our own double buffering, as we only render a single texture
#endif

	s_WindowWidth = s_Width * FT8XXEMU_WINDOW_SCALE;
	s_WindowHeight = s_Height * FT8XXEMU_WINDOW_SCALE;

	s_Window = SDL_CreateWindow(FT8XXEMU_WINDOW_TITLE_A, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		s_WindowWidth, s_WindowHeight, flags);
	s_GLContext = SDL_GL_CreateContext(s_Window);

	glMatrixMode(GL_COLOR);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glGenTextures(1, &s_BufferTexture);
	glBindTexture(GL_TEXTURE_2D, s_BufferTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	static const GLint swizzleMask[] = { GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );

#if FT8XXEMU_THREADED_FLIP
	SDL_GL_MakeCurrent(s_Window, NULL);

	s_Running = true;
	s_WaitFlip = SDL_CreateCond();
	s_WaitFlipMutex = SDL_CreateMutex();
	s_FlipThread = SDL_CreateThreadFT(flipThread, NULL, NULL);
#endif
}

bool GraphicsDriverClass::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		// don't care about other events
		case SDL_QUIT:
			return false;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				s_WindowWidth = event.window.data1;
				s_WindowHeight = event.window.data2;
				s_WindowResized = true;
				break;
			}
			break;
		}
	}

	int mouseX, mouseY;
	int button = SDL_GetMouseState(&mouseX, &mouseY);
	s_MouseDown = button & 1;

	mouseX -= s_LetterBoxXPix;
	mouseX *= s_Width;
	mouseX /= (s_WindowWidth - (s_LetterBoxXPix * 2));

	mouseY -= s_LetterBoxYPix;
	mouseY *= s_Height;
	mouseY /= (s_WindowHeight - (s_LetterBoxYPix * 2));

	s_MouseX = mouseX;
	s_MouseY = mouseY;

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

void GraphicsDriverClass::end()
{
#if FT8XXEMU_THREADED_FLIP
	s_Running = false;
	SDL_CondBroadcast(s_WaitFlip);
	SDL_WaitThread(s_FlipThread, NULL);
	s_FlipThread = NULL;

	SDL_DestroyMutex(s_WaitFlipMutex);
	s_WaitFlipMutex = NULL;
	SDL_DestroyCond(s_WaitFlip);
	s_WaitFlip = NULL;

	SDL_GL_MakeCurrent(s_Window, s_GLContext);
#endif

	glDeleteTextures(1, &s_BufferTexture);

	SDL_GL_DeleteContext(s_GLContext); s_GLContext = NULL;
	SDL_DestroyWindow(s_Window); s_Window = NULL;

#if FT8XXEMU_THREADED_FLIP
	SDL_DestroyMutex(s_BufferMutex[0]); s_BufferMutex[0] = NULL;
	SDL_DestroyMutex(s_BufferMutex[1]); s_BufferMutex[1] = NULL;
#endif

	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

void GraphicsDriverClass::setMode(int width, int height)
{
	if (s_Width != width || s_Height != height)
	{
		s_Width = width;
		s_Height = height;
		s_Ratio = (float)width / (float)height;

		s_WindowWidth = s_Width * FT8XXEMU_WINDOW_SCALE;
		s_WindowHeight = s_Height * FT8XXEMU_WINDOW_SCALE;

		SDL_SetWindowSize(s_Window, s_WindowWidth, s_WindowHeight);
	}
}

void GraphicsDriverClass::renderBuffer(bool output, bool changed)
{
#if FT8XXEMU_THREADED_FLIP
	if (changed)
	{
		++s_BufferCurrent;
		s_BufferCurrent %= 2;
	}
#endif

	s_Output = output;
#if FT8XXEMU_THREADED_FLIP
	SDL_CondBroadcast(s_WaitFlip);
#else
	drawBuffer();
#endif

#if FT8XXEMU_TITLE_DYNAMIC
	if (!s_TitleFrameSkip)
	{
		std::stringstream newTitle;
		newTitle << FT8XXEMU_WINDOW_TITLE_A;
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
		newTitle << " [FPS: ";
		newTitle << System.getFPSSmooth();
		newTitle << " (";
		newTitle << System.getFPS();
		newTitle << ")]";
		SDL_SetWindowTitle(s_Window, newTitle.str().c_str());
	}
	++s_TitleFrameSkip;
	s_TitleFrameSkip %= FT8XXEMU_TITLE_FRAMESKIP;
#endif
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

#endif /* #ifndef FTEMU_SDL2 */

/* end of file */
