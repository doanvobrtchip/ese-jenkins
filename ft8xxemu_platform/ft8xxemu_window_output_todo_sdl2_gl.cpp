/*
BT8XX Emulator Library
Copyright (C) 2014-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#if 0
#ifdef FTEMU_SDL2

// #include <...>
#include "ft8xxemu_window_output.h"

// System includes
#ifdef WIN32
#	define NOMINMAX
#	include <Windows.h>
#endif
#include <GL/gl.h>
#undef GL_VERSION_1_1
#include <GL/gl3w.h>
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

namespace FT8XXEMU {

WindowOutput WindowOutput;

void(*g_ResetTouchScreenXY)(int idx) = NULL;
void(*g_SetTouchScreenXY)(int idx, int x, int y, int pressure) = NULL;

static int s_Width = BT8XXEMU_WINDOW_WIDTH_DEFAULT;
static int s_Height = BT8XXEMU_WINDOW_HEIGHT_DEFAULT;
static float s_Ratio = BT8XXEMU_WINDOW_RATIO_DEFAULT;

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

static bool s_AllowGL3 = true;
static bool s_GL3 = false;
static GLuint s_VBO = 0;

#if WIN32
static bool s_GDIRevert = false;
#endif

// Print detailed GL debug info
#define BT8XXEMU_GL_DEBUG_DETAIL 1

// Threaded flip causes the buffer to be flipped from a separate thread.
// It saves time for the CPU processing
// Disabled because not compatible with dynamic degrade yet
// Must be 0 or 1
#define BT8XXEMU_THREADED_FLIP 0

// Hardware double buffer adds an additional copy to the output.
// It is technically redundant, as only one texture is drawn, but may improve performance in some window managers
#define BT8XXEMU_HARDWARE_DOUBLE_BUFFER 1

// Number of frames to skip for refreshing the window title
#define BT8XXEMU_TITLE_FRAMESKIP 3

// Whether to use a dynamic title
// It seems this is very slow under SDL2 on Linux
#if defined(WIN32)
#	define BT8XXEMU_TITLE_DYNAMIC 0 // TEMPORARILY DISABLED DUE TO LINKAGE PRIORITIES // TODO
#else
#	define BT8XXEMU_TITLE_DYNAMIC 0
#endif

namespace {

const char *s_VertexProgram =
	"#version 330 \n\
	layout (location = 0) in vec2 position; \n\
	layout (location = 1) in vec2 texCoord0; \n\
	out vec2 ioTexCoord0; \n\
	void main() \n\
	{ \n\
		gl_Position = vec4(position.x, position.y, 1.0, 1.0); \n\
		ioTexCoord0 = texCoord0; \n\
	} \n\
	";

const char *s_PixelProgram =
	"#version 330 \n\
	in vec2 ioTexCoord0; \n\
	out vec4 color; \n\
	uniform sampler2D tex0; \n\
	void main() \n\
	{ \n\
		color = texture(tex0, ioTexCoord0); \n\
	} \n\
	";

GLuint s_VAO = 0;
GLuint s_Program = 0;

SDL_Window* s_Window = NULL;
SDL_GLContext s_GLContext = NULL;

argb8888 s_BufferARGB8888[BT8XXEMU_THREADED_FLIP + 1][BT8XXEMU_WINDOW_WIDTH_MAX * BT8XXEMU_WINDOW_HEIGHT_MAX];
#if BT8XXEMU_THREADED_FLIP
SDL_mutex *s_BufferMutex[BT8XXEMU_THREADED_FLIP + 1] = { NULL, NULL };
#endif
GLuint s_BufferTexture = 0;
#if BT8XXEMU_THREADED_FLIP
int s_BufferCurrent = 0;
#else
const int s_BufferCurrent = 0;
#endif

#if BT8XXEMU_THREADED_FLIP

SDL_Thread *s_FlipThread = NULL;

SDL_mutex *s_WaitFlipMutex = NULL;
SDL_cond *s_WaitFlip = NULL;
bool s_Running = false;

#endif

bool s_Output = false;

#if BT8XXEMU_TITLE_DYNAMIC
int s_TitleFrameSkip = 0;
#endif

#if BT8XXEMU_GL_DEBUG_DETAIL
BT8XXEMU_FORCE_INLINE bool debugGL(const char *message)
{
	bool errb = false;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (" << message << "): " << err << std::endl;
		errb = true;
	}
	return errb;
}
#else
BT8XXEMU_FORCE_INLINE bool debugGL(const char *message) { return false; }
#endif

void genVBO()
{
	if (s_GL3)
	{
#if BT8XXEMU_FLIP_SDL2
		float vbo[] = {
			-1, -1, -s_LetterBoxX, -s_LetterBoxY,
			1, -1, 1.0f + s_LetterBoxX, -s_LetterBoxY,
			1, 1, 1.0f + s_LetterBoxX, 1.0f + s_LetterBoxY,
			-1, 1, -s_LetterBoxX, 1.0f + s_LetterBoxY,
		};
#else
		float vbo[] = {
			-1, -1, -s_LetterBoxX, 1.0f + s_LetterBoxY,
			1, -1, 1.0f + s_LetterBoxX, 1.0f + s_LetterBoxY,
			1, 1, 1.0f + s_LetterBoxX, -s_LetterBoxY,
			-1, 1, -s_LetterBoxX, -s_LetterBoxY,
		};
#endif
		glBufferData(GL_ARRAY_BUFFER, sizeof(vbo), vbo, GL_DYNAMIC_DRAW);
		debugGL("genVBO() glBufferData");
	}
}

void drawBuffer()
{
	if (s_Output)
	{
		if (s_WindowResized || s_Width != s_WidthCur || s_Height != s_HeightCur)
		{
			glViewport(0, 0, s_WindowWidth, s_WindowHeight);
			debugGL("drawBuffer() 1: glViewport");
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
			genVBO();
		}

#if BT8XXEMU_THREADED_FLIP
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
				debugGL("drawBuffer() 2: glTexImage2D");
			}
			else
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s_WidthCur, s_HeightCur, GL_RGBA, GL_UNSIGNED_BYTE, s_BufferARGB8888[buffer]);
				debugGL("drawBuffer() 3: glTexSubImage2D");
			}

			if (s_GL3)
			{
				glDrawArrays(GL_QUADS, 0, 4);
				debugGL("drawBuffer() 4: glDrawArrays");
			}
			else
			{
				glEnable(GL_TEXTURE_2D);
				debugGL("drawBuffer() 5: glEnable");

				glBegin(GL_QUADS);

#if BT8XXEMU_FLIP_SDL2
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
				debugGL("drawBuffer() 6: glEnd");

				glDisable(GL_TEXTURE_2D);
				debugGL("drawBuffer() 7: glDisable");
			}

			glFlush();
			debugGL("drawBuffer() 8: glFlush");

#if BT8XXEMU_THREADED_FLIP
			SDL_UnlockMutex(s_BufferMutex[buffer]);
#endif
		}
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT);
		debugGL("drawBuffer() 9: glClear");
		glFlush();
		debugGL("drawBuffer() 10: glFlush");
	}
#if BT8XXEMU_HARDWARE_DOUBLE_BUFFER
	SDL_GL_SwapWindow(s_Window);
	debugGL("drawBuffer() 11: SDL_GL_SwapWindow");
#endif
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
}

#if BT8XXEMU_THREADED_FLIP

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

argb8888 *WindowOutput::getBufferARGB8888()
{
#if WIN32
	if (s_GDIRevert)
		return WindowOutputGDI.getBufferARGB8888();
#endif

	// Return the next available buffer
	// Wait in case it's still being rendered
#if BT8XXEMU_THREADED_FLIP
	if (!SDL_LockMutex(s_BufferMutex[s_BufferCurrent]))
		SDL_UnlockMutex(s_BufferMutex[s_BufferCurrent]);
#endif
	return s_BufferARGB8888[s_BufferCurrent];
}

#if 0
static int eventFilter(void *, SDL_Event *e)
{
	if (e->type == 512 && s_Output)
	{
		drawBuffer();
	}
	return 1; // return 1 so all events are added to queue
}
#endif

bool WindowOutput::begin()
{
#ifdef WIN32
	if (s_GDIRevert)
		return WindowOutputGDI.begin();
#endif

	s_Width = BT8XXEMU_WINDOW_WIDTH_DEFAULT;
	s_Height = BT8XXEMU_WINDOW_HEIGHT_DEFAULT;
	s_Ratio = BT8XXEMU_WINDOW_RATIO_DEFAULT;

	uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	
	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	
#if 0
#ifdef WIN32
	SDL_SetEventFilter(&eventFilter, NULL);
#endif
#endif

#if BT8XXEMU_THREADED_FLIP
	s_BufferMutex[0] = SDL_CreateMutex();
	s_BufferMutex[1] = SDL_CreateMutex();
#endif

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
#if !BT8XXEMU_HARDWARE_DOUBLE_BUFFER
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0); // We do our own double buffering, as we only render a single texture
#endif

	s_WindowWidth = s_Width * BT8XXEMU_WINDOW_SCALE;
	s_WindowHeight = s_Height * BT8XXEMU_WINDOW_SCALE;

	s_Window = SDL_CreateWindow(BT8XXEMU_WINDOW_TITLE_A, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		s_WindowWidth, s_WindowHeight, flags);
	s_GLContext = SDL_GL_CreateContext(s_Window);

	s_GDIRevert = false;
	s_GL3 = (gl3wInit() == 0 && gl3wIsSupported(3, 3)) && s_AllowGL3;
	debugGL("gl3wInit()");
	
	if (s_GL3) switch (1) case 1:
	{
		printf("OpenGL 3.3 mode\n");

		GLint vpsuccess;
		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vp, 1, &s_VertexProgram, NULL);
		glCompileShader(vp);
		glGetShaderiv(vp, GL_COMPILE_STATUS, &vpsuccess);
		if (vpsuccess == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vp, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(vp, maxLength, &maxLength, &errorLog[0]);
			printf("%s", &errorLog[0]);

			printf("Revert to OpenGL 1.1 mode\n");
			glDeleteShader(vp);
			s_GL3 = false;
			s_AllowGL3 = false;
		}

		GLint ppsuccess;
		GLuint pp = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(pp, 1, &s_PixelProgram, NULL);
		glCompileShader(pp);
		glGetShaderiv(pp, GL_COMPILE_STATUS, &ppsuccess);
		if (ppsuccess == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(pp, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(pp, maxLength, &maxLength, &errorLog[0]);
			printf("%s", &errorLog[0]);

			printf("Revert to OpenGL 1.1 mode\n");
			glDeleteShader(vp);
			glDeleteShader(pp);
			s_GL3 = false;
			s_AllowGL3 = false;
		}

		s_Program = glCreateProgram();
		glAttachShader(s_Program, vp);
		glAttachShader(s_Program, pp);
		glLinkProgram(s_Program);
		GLint isLinked = 0;
		glGetProgramiv(s_Program, GL_LINK_STATUS, (int *)&isLinked);
		glDeleteShader(vp);
		glDeleteShader(pp);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(s_Program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(s_Program, maxLength, &maxLength, &infoLog[0]);
			printf("%s", &infoLog[0]);

			printf("Revert to OpenGL 1.1 mode\n");
			glDeleteProgram(s_Program);
			s_GL3 = false;
			s_AllowGL3 = false;
		}

		glGenVertexArrays(1, &s_VAO);
		glBindVertexArray(s_VAO);

		glGenBuffers(1, &s_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, s_VBO);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
		glActiveTexture(GL_TEXTURE0);
		
		glUseProgram(s_Program);
		GLint tex0 = glGetUniformLocation(s_Program, "tex0");
		glUniform1i(tex0, 0);
	}
	else
	{
		printf("Legacy OpenGL 1.1 mode\n");
	}

	glGenTextures(1, &s_BufferTexture);
	debugGL("begin() 1: glGenTextures");

	glBindTexture(GL_TEXTURE_2D, s_BufferTexture);
	debugGL("begin() 2: glBindTexture");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	if (!s_GL3 && debugGL("begin() 3: glTexParameteri"))
	{
#if WIN32
		printf("Reverting to GDI mode\n");
#else
		SystemSdl.ErrorSdl();
#endif

		glDeleteTextures(1, &s_BufferTexture);
		SDL_GL_DeleteContext(s_GLContext); s_GLContext = NULL;
		SDL_DestroyWindow(s_Window); s_Window = NULL;

#if BT8XXEMU_THREADED_FLIP
		SDL_DestroyMutex(s_BufferMutex[0]); s_BufferMutex[0] = NULL;
		SDL_DestroyMutex(s_BufferMutex[1]); s_BufferMutex[1] = NULL;
#endif

		SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

#if WIN32
		s_GDIRevert = true;
		return WindowOutputGDI.begin();
#else
		return false;
#endif
	}

	static const GLint swizzleMask[] = { GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	debugGL("begin() 4: glTexParameteriv");

	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	debugGL("begin() 5: glClearColor");

#if BT8XXEMU_THREADED_FLIP
	SDL_GL_MakeCurrent(s_Window, NULL);

	s_Running = true;
	s_WaitFlip = SDL_CreateCond();
	s_WaitFlipMutex = SDL_CreateMutex();
	s_FlipThread = SDL_CreateThreadFT(flipThread, NULL, NULL);
#endif

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}

	return true;
}

bool WindowOutput::update()
{
#if WIN32
	if (s_GDIRevert)
		return WindowOutputGDI.update();
#endif

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

void WindowOutput::end()
{
#if WIN32
	if (s_GDIRevert)
	{
		WindowOutputGDI.end();
		return;
	}
#endif

#if BT8XXEMU_THREADED_FLIP
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

	if (s_GL3)
	{
		glDeleteVertexArrays(1, &s_VAO);
		glDeleteBuffers(1, &s_VBO);
		glDeleteProgram(s_Program);
	}
	glDeleteTextures(1, &s_BufferTexture);

	SDL_GL_DeleteContext(s_GLContext); s_GLContext = NULL;
	SDL_DestroyWindow(s_Window); s_Window = NULL;

#if BT8XXEMU_THREADED_FLIP
	SDL_DestroyMutex(s_BufferMutex[0]); s_BufferMutex[0] = NULL;
	SDL_DestroyMutex(s_BufferMutex[1]); s_BufferMutex[1] = NULL;
#endif

	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

void WindowOutput::setMode(int width, int height)
{
#ifdef WIN32
	if (s_GDIRevert)
	{
		WindowOutputGDI.setMode(width, height);
		return;
	}
#endif

	if (s_Width != width || s_Height != height)
	{
		s_Width = width;
		s_Height = height;
		s_Ratio = (float)width / (float)height;

		s_WindowWidth = s_Width * BT8XXEMU_WINDOW_SCALE;
		s_WindowHeight = s_Height * BT8XXEMU_WINDOW_SCALE;

		SDL_SetWindowSize(s_Window, s_WindowWidth, s_WindowHeight);
	}
}

void WindowOutput::renderBuffer(bool output, bool changed)
{
#ifdef WIN32
	if (s_GDIRevert)
	{
		WindowOutputGDI.renderBuffer(output, changed);
		return;
	}
#endif

#if BT8XXEMU_THREADED_FLIP
	if (changed)
	{
		++s_BufferCurrent;
		s_BufferCurrent %= 2;
	}
#endif

	s_Output = output;
#if BT8XXEMU_THREADED_FLIP
	SDL_CondBroadcast(s_WaitFlip);
#else
	drawBuffer();
#endif

#if BT8XXEMU_TITLE_DYNAMIC
	if (!s_TitleFrameSkip)
	{
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
		SDL_SetWindowTitle(s_Window, newTitle.str().c_str());
	}
	++s_TitleFrameSkip;
	s_TitleFrameSkip %= BT8XXEMU_TITLE_FRAMESKIP;
#endif
}

void WindowOutput::enableMouse(bool enabled)
{
#ifdef WIN32
	if (s_GDIRevert)
	{
		WindowOutputGDI.enableMouse(enabled);
		return;
	}
#endif

	s_MouseEnabled = enabled;
}

void WindowOutput::setMousePressure(int pressure)
{
#ifdef WIN32
	if (s_GDIRevert)
	{
		WindowOutputGDI.setMousePressure(pressure);
		return;
	}
#endif

	s_MousePressure = pressure;
}

} /* namespace FT8XXEMU */

#endif /* #ifndef FTEMU_SDL2 */

#endif

/* end of file */
