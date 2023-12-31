/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "emulator_viewport.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QAction>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QMutex>
#include <QPaintEvent>
#include <QPainter>
#include <QDir>
#include <QCoreApplication>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "constant_mapping.h"
#include "constant_mapping_flash.h"
#include "src/customize/QRuler.h"

namespace FTEDITOR {

// #define FTEDITOR_STDFLASH L"C:/source/ft800emu/reference/vc3roms/stdflash.bin"

#define FTEDITOR_FLASH_FIRMWARE_DIR "firmware"

BT8XXEMU_Emulator *g_Emulator = NULL;
BT8XXEMU_Flash *g_Flash = NULL;

static BT8XXEMU_EmulatorParameters s_EmulatorParameters;
EmulatorThread *s_EmulatorThread;
static QImage *s_Image = NULL;
static QPixmap *s_Pixmap = NULL;
QMutex g_ViewportMutex;
static EmulatorViewport *s_EmulatorViewport = NULL;

static int s_HoverX = -1, s_HoverY = -1;
static argb8888 s_HoverColor;
static bool s_HoverValid = false;
static QColor s_HoverColorQt = QColor::Invalid;

static void(*s_Main)(BT8XXEMU_Emulator *sender, void *context) = NULL;
static bool s_MainReady = false;

static bool s_LastRendered = true;

static void emuLog(BT8XXEMU_Emulator *sender, void *context, BT8XXEMU_LogType type, const char *message)
{
	printf("[BT8XXEMU] %s\n", message);
}

void overrideMain(BT8XXEMU_Emulator *sender, void *context)
{
	s_MainReady = true;

	if (s_Main)
		s_Main(sender, context);
}

static int ftqtGraphics(BT8XXEMU_Emulator *sender, void *context, int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags) // on Emulator thread
{
	// TODO: Optimize using platform specific access to QImage so we
	// don't need to copy the buffer each time.
	if (s_Image && s_Pixmap)
	{
		g_ViewportMutex.lock();
		if (hsize != s_Image->width() || vsize != s_Image->height())
		{
			// printf("Graphics resize");
			QImage *image = s_Image;
			s_Image = new QImage(hsize, vsize, QImage::Format_RGB32);
			delete image;
		}
		if (output && (flags & BT8XXEMU_FrameBufferChanged))
		{
			// printf("Graphics received");
			// This is just terrible code.
			for (uint32_t y = 0; y < vsize; ++y)
				memcpy(s_Image->scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
		}
		else
		{
			// TODO: Blank
			// ..
		}
		s_HoverValid = s_HoverX >= 0 && s_HoverY >= 0 && s_HoverX < (int)hsize && s_HoverY < (int)vsize;
		if (s_HoverValid)
			s_HoverColor = buffer[s_HoverY * hsize + s_HoverX];
		s_EmulatorViewport->graphics();
		if (s_LastRendered)
		{
			s_LastRendered = false;
			emit s_EmulatorThread->repaint();
		}
		g_ViewportMutex.unlock();
		QThread::yieldCurrentThread();
	}
	return true; // g_EmulatorThread != NULL;
}

void EmulatorThread::run()
{
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &g_Emulator, &s_EmulatorParameters);
}

EmulatorViewport::EmulatorViewport(QWidget *parent, const QString &applicationDataDir)
#ifdef FTEDITOR_OPENGL_VIEWPORT
	: QOpenGLWidget(parent)
#else
	: QWidget(parent)
#endif
	, m_ApplicationDataDir(applicationDataDir), m_ScreenScale(16)
{
	s_EmulatorViewport = this;
	s_Image = new QImage(screenWidthDefault(FTEDITOR_CURRENT_DEVICE), screenHeightDefault(FTEDITOR_CURRENT_DEVICE), QImage::Format_RGB32);
	s_Pixmap = new QPixmap(screenWidthDefault(FTEDITOR_CURRENT_DEVICE), screenHeightDefault(FTEDITOR_CURRENT_DEVICE));

	m_Vertical = new QScrollBar(Qt::Vertical, this);
	m_Horizontal = new QScrollBar(Qt::Horizontal, this);

	m_Vertical->setMinimum(-screenHeightDefault(FTEDITOR_CURRENT_DEVICE) * 8);
	m_Vertical->setMaximum(screenHeightDefault(FTEDITOR_CURRENT_DEVICE) * 8);
	m_Vertical->setValue(0);
	m_Vertical->setSingleStep(16);
	m_Vertical->setPageStep(16 * 16);
	m_Horizontal->setMinimum(-screenWidthDefault(FTEDITOR_CURRENT_DEVICE) * 8);
	m_Horizontal->setMaximum(screenWidthDefault(FTEDITOR_CURRENT_DEVICE) * 8);
	m_Horizontal->setValue(0);
	m_Horizontal->setSingleStep(16);
	m_Horizontal->setPageStep(16 * 16);

	setMinimumWidth(screenWidthDefault(FTEDITOR_CURRENT_DEVICE));
	setMinimumHeight(screenHeightDefault(FTEDITOR_CURRENT_DEVICE));

	m_HorizontalRuler = new QRuler(this, Qt::Horizontal);
	m_HorizontalRuler->setMaximumWidth(m_Horizontal->maximum());
	m_HorizontalRuler->setFixedHeight(30);
	m_VerticalRuler = new QRuler(this, Qt::Vertical);
	m_VerticalRuler->setMaximumHeight(m_Vertical->maximum());
	m_VerticalRuler->setFixedWidth(35);
}

EmulatorViewport::~EmulatorViewport()
{
	stop();
	delete s_Pixmap; s_Pixmap = NULL;
	delete s_Image; s_Image = NULL;
	s_EmulatorViewport = NULL;
}

int EmulatorViewport::hsize()
{
	return s_Pixmap->width();
}

int EmulatorViewport::vsize()
{
	return s_Pixmap->height();
}

void EmulatorViewport::run(const BT8XXEMU_EmulatorParameters &params)
{
	// There can be only one
	if (s_EmulatorThread == NULL)
	{
		// Copy the params for the new thread to use
		s_EmulatorParameters = params;

		// Attach flash
		if (flashSupport(FTEDITOR_CURRENT_DEVICE))
		{
			BT8XXEMU_FlashParameters flashParams;
			BT8XXEMU_Flash_defaults(BT8XXEMU_VERSION_API, &flashParams);
			wcscpy(flashParams.DeviceType, flashDeviceType(FTEDITOR_CURRENT_FLASH));
			flashParams.SizeBytes = flashSizeBytes(FTEDITOR_CURRENT_FLASH);
			flashParams.Persistent = false;
			flashParams.StdOut = false;
			// flashParams.Data // TODO: Need to remove this from the flash parameter block, since it's not compatible with remote process
			if (flashFirmware(FTEDITOR_CURRENT_DEVICE, FTEDITOR_CURRENT_FLASH)[0])
			{
				QString blobPath = m_ApplicationDataDir + "/" FTEDITOR_FLASH_FIRMWARE_DIR "/" + QString::fromWCharArray(flashFirmware(FTEDITOR_CURRENT_DEVICE, FTEDITOR_CURRENT_FLASH));
				if (blobPath.length() < 260)
				{
					int i = blobPath.toWCharArray(flashParams.DataFilePath);
					flashParams.DataFilePath[i] = L'\0';
				}
			}
#ifdef FTEDITOR_STDFLASH
			wcscpy(flashParams.DataFilePath, FTEDITOR_STDFLASH); // Standard flash image (for testing)
#endif
			g_Flash = BT8XXEMU_Flash_create(BT8XXEMU_VERSION_API, &flashParams);
			s_EmulatorParameters.Flash = g_Flash;
		}

		// Log output
		s_EmulatorParameters.Log = emuLog;

		// Add the graphics callback to the parameters
		s_EmulatorParameters.Graphics = ftqtGraphics;

		// Override setup to set ready flag
		s_Main = params.Main;
		s_EmulatorParameters.Main = overrideMain;
		s_MainReady = false;

		// Create the main thread for the emulator
		s_EmulatorThread = new EmulatorThread();
		s_EmulatorThread->start();

		while (!s_MainReady && s_EmulatorThread->isRunning())
			QThread::msleep(1);

		// Connect the cross thread repaint event
		s_LastRendered = true;
		connect(s_EmulatorThread, SIGNAL(repaint()), this, SLOT(threadRepaint()));
	}
}

void EmulatorViewport::stop()
{
	if (s_EmulatorThread != NULL)
	{
		BT8XXEMU_stop(g_Emulator);

		printf("Wait for emulator threads\n");
		s_EmulatorThread->wait();
		delete s_EmulatorThread;
		s_EmulatorThread = NULL;
		printf("Emulator threads finished\n");

		BT8XXEMU_destroy(g_Emulator);
		g_Emulator = NULL;

		if (g_Flash)
		{
			BT8XXEMU_Flash_destroy(g_Flash);
			g_Flash = NULL;
		}
	}
}

void EmulatorViewport::paintEvent(QPaintEvent* e) // on Qt thread
{
	QPainter painter(this);
#ifdef FTEDITOR_OPENGL_VIEWPORT
	painter.setPen(Qt::NoPen);
	painter.setBrush(painter.background());
	painter.drawRect(rect());
#endif
	painter.drawPixmap(screenLeft(), screenTop(),
		s_Pixmap->width() * screenScale() / 16, 
		s_Pixmap->height() * screenScale() / 16, 
		*s_Pixmap,
		0, 0, s_Pixmap->width(), s_Pixmap->height());
}

int EmulatorViewport::screenLeft() // offset in screenspace pixels
{
	int center = width() / 2;
	int centerFrame = s_Pixmap->width() * m_ScreenScale / 32;
	int offset = m_Horizontal->value() * m_ScreenScale / 256;
	return center - centerFrame - offset;
}

int EmulatorViewport::screenTop()
{
	int center = height() / 2;
	int centerFrame = s_Pixmap->height() * m_ScreenScale / 32;
	int offset = m_Vertical->value() * m_ScreenScale / 256;
	return center - centerFrame - offset;
}

int EmulatorViewport::screenBottom()
{
	return screenTop() + (s_Pixmap->height() * m_ScreenScale / 16);
}

int EmulatorViewport::screenRight()
{
	return screenLeft() + (s_Pixmap->width() * m_ScreenScale / 16);
}

int EmulatorViewport::screenScale()
{
	return m_ScreenScale;
}

void EmulatorViewport::setScreenScale(int screenScale)
{
	if (screenScale < 1) m_ScreenScale = 1;
	else if (screenScale > 256) m_ScreenScale = 256;
	else m_ScreenScale = screenScale;
}

void EmulatorViewport::zoomIn()
{
	// Not really used, base implementation for virtual override
	setScreenScale(screenScale() * 2);
}

void EmulatorViewport::zoomOut()
{
	// Not really used, base implementation for virtual override
	setScreenScale(screenScale() / 2);
}

void EmulatorViewport::threadRepaint() // on Qt thread
{
	g_ViewportMutex.lock();
	graphics(s_Image);
	if (s_Image->width() != s_Pixmap->width()
		|| s_Image->height() != s_Pixmap->height())
	{
		QPixmap *pixmap = s_Pixmap;
		s_Pixmap = new QPixmap(s_Image->width(), s_Image->height());
		/*setMinimumWidth(s_Pixmap->width());
		setMinimumHeight(s_Image->height());*/
		m_Vertical->setMinimum(-s_Pixmap->height() * 8);
		m_Vertical->setMaximum(s_Pixmap->height() * 8);
		m_Horizontal->setMinimum(-s_Pixmap->width() * 8);
		m_Horizontal->setMaximum(s_Pixmap->width() * 8);
		delete pixmap;
	}
	s_Pixmap->convertFromImage(*s_Image);
	if (s_HoverValid)
		s_HoverColorQt = QColor::fromRgba(s_HoverColor);
	else
		s_HoverColorQt = QColor::Invalid;
	s_LastRendered = true;
	g_ViewportMutex.unlock();
	repaint();
	emit frame();
}

void EmulatorViewport::toggleViewRuler(bool checked)
{
	horizontalRuler()->setVisible(checked);
	verticalRuler()->setVisible(checked);
}

void EmulatorViewport::fetchColorAsync(int x, int y)
{
	s_HoverX = x;
	s_HoverY = y;
}

QColor EmulatorViewport::fetchColorAsync()
{
	return s_HoverColorQt;
}

const QPixmap &EmulatorViewport::getPixMap() const
{
	return *s_Pixmap;
}

} /* namespace FTEDITOR */

/* end of file */
