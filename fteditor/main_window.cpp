/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "main_window.h"

// STL includes
#include <stdio.h>
#include <algorithm>

// Qt includes
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTreeView>
#include <QDirModel>
#include <QUndoStack>
#include <QUndoCommand>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTextBlock>
#include <QPlainTextEdit>
#include <QDesktopServices>
#include <QScrollBar>
#include <QComboBox>
#include <QStandardPaths>
#include <QMovie>
#include <QDirIterator>
#include <QElapsedTimer>

// Emulator includes
#include <bt8xxemu_inttypes.h>
#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>
#ifdef WIN32
#	undef min
#	undef max
#endif

// Project includes
#include "dl_editor.h"
#include "interactive_viewport.h"
#include "properties_editor.h"
#include "code_editor.h"
#include "toolbox.h"
#include "device_manager.h"
#include "inspector.h"
#include "content_manager.h"
#include "interactive_properties.h"
#include "emulator_navigator.h"
#include "constant_mapping.h"
#include "constant_common.h"
#include "constant_mapping_flash.h"

namespace FTEDITOR {

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

#define FTEDITOR_DEBUG_EMUWRITE 0

typedef int32_t ramaddr;

volatile int s_HSize = 480;
volatile int s_VSize = 272;
volatile int s_Rotate = 0;

static const int s_StandardResolutionNb[FTEDITOR_DEVICE_NB] = {
	3, // FT800
	3, // FT801
	5, // FT810
	5, // FT811
	5, // FT812
	5, // FT813
	5, // BT815
};

static const char *s_StandardResolutions[] = {
	"QVGA (320x240)",
	"WQVGA (480x272)",
	"HVGA Portrait (320x480)",
	"WVGA (800x480)",
	"SVGA (800x600)",
};

static const int s_StandardWidths[] = {
	320,
	480,
	320,
	800,
	800,
};

static const int s_StandardHeights[] = {
	240,
	272,
	480,
	480,
	600,
};

bool s_EmulatorRunning = false;

void swrbegin(ramaddr address)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("swrbegin(%i)\n", (int)address);
#endif

	BT8XXEMU_chipSelect(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (2 << 6) | ((address >> 16) & 0x3F));
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	// BT8XXEMU_transfer(0x00);
}

void swr8(uint8_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("swr8(%i)\n", (int)value);
#endif

	BT8XXEMU_transfer(g_Emulator, value);
}

void swr16(uint16_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("swr16(%i)\n", (int)value);
#endif

	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);
}

void swr32(uint32_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("swr32(%i)\n", (int)value);
#endif

	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 24) & 0xFF);
}

void swrend()
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("swrend()\n");
#endif

	BT8XXEMU_chipSelect(g_Emulator, 0);
}

void wr32(ramaddr address, uint32_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("wr32(%i, %i)\n", (int)address, (int)value);
#endif

	swrbegin(address);
	swr32(value);
	swrend();
}

uint32_t rd32(size_t address)
{

	BT8XXEMU_chipSelect(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (address >> 16) & 0x3F);
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	BT8XXEMU_transfer(g_Emulator, 0x00);

	uint32_t value;
	value = BT8XXEMU_transfer(g_Emulator, 0);
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 8;
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 16;
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 24;

#if FTEDITOR_DEBUG_EMUWRITE
	printf("rd32(%i), %i\n", (int)address, (int)value);
#endif

	BT8XXEMU_chipSelect(g_Emulator, 0);
	return value;
}

static DlEditor *s_DlEditor = NULL;
static DlEditor *s_CmdEditor = NULL;
static DlEditor *s_Macro = NULL;
// static FILE *s_F = NULL;

void closeDummy(BT8XXEMU_Emulator *sender, void *context)
{
	// no-op
	// NOTE: Used to avoid loop thread kill
}

void setup()
{
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE), 480);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE), 272);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PCLK), 5);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_INT_MASK), 0xFF); // INT_CMDEMPTY);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_INT_EN), 1);
}

// Content manager
static ContentManager *s_ContentManager = NULL;
//static BitmapSetup *s_BitmapSetup = NULL;
//static int s_BitmapSetupModNb = 0;

// Utilization
static int s_UtilizationDisplayListCmd = 0;
static volatile bool s_WaitingCoprocessorAnimation = false;

// Array indexed by display list index containing coprocessor line which wrote the display list command
static int s_DisplayListCoprocessorCommandA[FTEDITOR_DL_SIZE];
static int s_DisplayListCoprocessorCommandB[FTEDITOR_DL_SIZE];
static int *s_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandA;
static int *s_DisplayListCoprocessorCommandWrite = s_DisplayListCoprocessorCommandB;

static std::vector<uint32_t> s_CmdParamCache;
static std::vector<std::string> s_CmdStrParamCache;

int g_StepCmdLimit = 0;
static int s_StepCmdLimitCurrent = 0;

static int s_MediaFifoPtr = 0;
static int s_MediaFifoSize = 0;
static QFile *s_MediaFifoFile = NULL;
static QDataStream *s_MediaFifoStream = NULL;

static bool s_CoprocessorFaultOccured = false;
static bool s_StreamingData = false;

static bool s_WarnMissingClear = false;
static bool s_WarnMissingClearActive = false;

static bool displayListSwapped = false;
static bool coprocessorSwapped = false;

static bool s_WantReloopCmd = false;

QElapsedTimer s_AbortTimer;

static volatile bool s_ShowCoprocessorBusy = true;

void cleanupMediaFifo()
{
	delete s_MediaFifoStream;
	s_MediaFifoStream = NULL;
	delete s_MediaFifoFile;
	s_MediaFifoFile = NULL;
}

void resetemu()
{
	s_UtilizationDisplayListCmd = 0;
	s_WaitingCoprocessorAnimation = false;
	s_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandA;
	s_DisplayListCoprocessorCommandWrite = s_DisplayListCoprocessorCommandB;
	s_CmdParamCache.clear();
	g_StepCmdLimit = 0;
	s_StepCmdLimitCurrent = 0;
	s_CoprocessorFaultOccured = false;
	s_WarnMissingClear = false;
	s_WarnMissingClearActive = false;
	displayListSwapped = false;
	coprocessorSwapped = false;
	s_WantReloopCmd = false;
	s_MediaFifoPtr = 0;
	s_MediaFifoSize = 0;
	cleanupMediaFifo();
}

void resetCoprocessorFromLoop()
{
	// Enter reset state
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CPURESET), 1);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ), 0);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 0);
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
	{
		// Enable patched rom in case cmd_logo was running
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROMSUB_SEL), 3);
	}
	QThread::msleep(1); // Timing hack because we don't lock CPURESET flag at the moment with coproc thread
	// Leave reset
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CPURESET), 0);
	QThread::msleep(1); // Timing hack because we don't lock CPURESET flag at the moment with coproc thread
	// Stop playing audio in case video with audio was playing during reset
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PLAYBACK_PLAY), 0);
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
	{
		// Go back into the patched coprocessor main loop
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD), CMD_EXECUTE);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 4, 0x7ffe);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 8, 0);
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 12);
		QThread::msleep(1); // Timing hack because it's not checked when the coprocessor finished processing the CMD_EXECUTE
		// Need to manually stop previous command from repeating infinitely
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 0);
	}
	// Start display list from beginning
	wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD), CMD_DLSTART);
	wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 4);
}

// static int s_SwapCount = 0;
void loop()
{
	if (!s_EmulatorRunning)
	{
		QThread::yieldCurrentThread();
		return;
	}

	if (BT8XXEMU_hasInterrupt(g_Emulator))
	{
		uint32_t flags = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_INT_FLAGS));
		// printf("INTERRUPT %i\n", flags);
		if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF)
		{
			printf("COPROCESSOR FAULT\n");
			s_CoprocessorFaultOccured = true;
			resetCoprocessorFromLoop();
		}
	}

	s_ShowCoprocessorBusy = false;

	// wait
	if (coprocessorSwapped)
	{
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE)) != rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)))
		{
			if (!s_EmulatorRunning) return;
			if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) return;
		}
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_DL)) != 0)
		{
			if (!s_EmulatorRunning) return;
		}
		coprocessorSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped CMD %i\n", s_SwapCount);
	}
	else if (displayListSwapped)
	{
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_DLSWAP)) != DLSWAP_DONE)
		{
			if (!s_EmulatorRunning) return;
		}
		displayListSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped DL %i\n", s_SwapCount);
	}
	else
	{
		QThread::msleep(10);
	}

	if (!s_EmulatorRunning)
		return;

	s_ContentManager->lockContent();
	std::set<ContentInfo *> contentInfo;
	s_ContentManager->swapUploadDirty(contentInfo);
	bool reuploadFontSetup = false;
	for (std::set<ContentInfo *>::iterator it(contentInfo.begin()), end(contentInfo.end()); it != end; ++it)
	{
		ContentInfo *info = (*it);
		int loadAddr = (info->Converter == ContentInfo::Image) ? info->bitmapAddress() : info->MemoryAddress;
		QString fileName = info->DestName + ".raw";
		printf("[RAM_G] Load: '%s' to '%i'\n", info->DestName.toLocal8Bit().data(), loadAddr);
		QFile binFile(fileName);
		if (!binFile.exists())
		{
			printf("[RAM_G] Error: File '%s' does not exist\n", fileName.toLocal8Bit().data());
			continue;
		}
		bool imageCoprocessor = (info->Converter == ContentInfo::ImageCoprocessor);
		int binSize = imageCoprocessor ? info->CachedSize : (int)binFile.size();
		if (binSize + loadAddr > addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END))
		{
			printf("[RAM_G] Error: File of size '%i' exceeds RAM_G size\n", binSize);
			continue;
		}
		if (imageCoprocessor) // Load image through coprocessor
		{
			int wp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE));
			int rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
			int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
			int freespace = ((4096 - 4) - fullness);
			swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
			swr32(CMD_LOADIMAGE);
			swr32(loadAddr);
			swr32(OPT_NODL | (info->ImageMono ? OPT_MONO : 0));
			swrend();
			fullness += 12;
			freespace -= 12;
			wp += 12;
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			int fileSize = binFile.size();
			binFile.open(QIODevice::ReadOnly);
			QDataStream in(&binFile);
			int writeCount = 0;
			swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
			for (;;)
			{
				if (freespace < (4 + 8) || writeCount < 128)
				{
					swrend();
					wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
					do
					{
						if (!s_EmulatorRunning) return;
						rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
						if ((rp & 0xFFF) == 0xFFF)
						{
							printf("Error during stream at %i bytes\n", (int)writeCount);
							return;
						}
						fullness = ((wp & 0xFFF) - rp) & 0xFFF;
					} while (fullness > 1024);
					freespace = ((4096 - 4) - fullness);
					printf("Stream: %i bytes\n", (int)writeCount);
					swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				}
				uint32_t buffer;
				int nb = in.readRawData(reinterpret_cast<char *>(&buffer), 4);
				if (nb > 0)
				{
					// write
					writeCount += nb;
					wp += 4;
					freespace -= 4;
					swr32(buffer);
				}
				if (nb != 4)
				{
					// done
					printf("Stream finished: %i bytes\n", (int)writeCount);
					if (writeCount == 0)
					{
						swrend();
						resetCoprocessorFromLoop();
						return;
					}
					break;
				}
				if (!s_EmulatorRunning)
				{
					swrend();
					return;
				}
			}
			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
			continue;
		}
		// ok
		; {
			binFile.open(QIODevice::ReadOnly);
			QDataStream in(&binFile);
			/*swrbegin(info->MemoryAddress);
			char b;
			while (in.readRawData(&b, 1))
			{
				swr8(b);
			}
			swrend();*/
			char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
			int s = in.readRawData(&ram[FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G) + loadAddr], binSize);
			BT8XXEMU_poke(g_Emulator);
		}
		if (info->Converter == ContentInfo::Font)
		{
			// Write bitmap address
			wr32(loadAddr + 144, loadAddr + 148);
		}
		if (FTEDITOR_CURRENT_DEVICE < FTEDITOR_FT810)
		{
			if (info->Converter == ContentInfo::Image && info->ImageFormat == PALETTED)
			{
				QString palName = info->DestName + ".lut.raw";
				printf("[RAM_PAL] Load: '%s'\n", info->DestName.toLocal8Bit().data());
				QFile palFile(palName);
				if (!palFile.exists())
				{
					printf("[RAM_PAL] Error: File '%s' does not exist\n", palName.toLocal8Bit().data());
					continue;
				}
				int palSize = (int)palFile.size();
				if (palSize != 1024)
				{
					printf("[RAM_PAL] Error: File of size '%i' not equal to palSize\n", palSize);
					continue;
				}
				// ok
				{
					palFile.open(QIODevice::ReadOnly);
					QDataStream in(&palFile);
					char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
					int s = in.readRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL)], palSize);
					BT8XXEMU_poke(g_Emulator);
				}
			}
		}
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
		{
			if (info->Converter == ContentInfo::Image && (info->ImageFormat == PALETTED8 || info->ImageFormat == PALETTED565 || info->ImageFormat == PALETTED4444))
			{
				int palSize;
				switch (info->ImageFormat)
				{
				case PALETTED565:
				case PALETTED4444:
					palSize = 256 * 2;
					break;
				default:
					palSize = 256 * 4;
					break;
				}
				QString palName = info->DestName + ".lut.raw";
				printf("[RAM_PAL] Load: '%s'\n", info->DestName.toLocal8Bit().data());
				QFile palFile(palName);
				if (!palFile.exists())
				{
					printf("[RAM_PAL] Error: File '%s' does not exist\n", palName.toLocal8Bit().data());
					continue;
				}
				if (palSize != (int)palFile.size())
				{
					printf("[RAM_PAL] Error: File of size '%i' not equal to palSize\n", palSize);
					continue;
				}
				// ok
				{
					palFile.open(QIODevice::ReadOnly);
					QDataStream in(&palFile);
					char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
					int s = in.readRawData(&ram[FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G) + info->MemoryAddress], palSize);
					BT8XXEMU_poke(g_Emulator);
				}
			}
		}
		if (info->Converter == ContentInfo::Font)
		{
			reuploadFontSetup = true;
		}
	}
	/*bool reuploadBitmapSetup = contentInfo.size() || s_BitmapSetupModNb < s_BitmapSetup->getModificationNb();
	if (reuploadBitmapSetup)
	{
		printf("Reupload bitmap setup to RAM_DL\n");
		wr32(REG_PCLK, 0);
		swrbegin(RAM_DL);
		const FT800EMU::BitmapInfo *bitmapInfo = s_BitmapSetup->getBitmapInfos();
		const ContentInfo *const *bitmapSources = s_BitmapSetup->getBitmapSources();
		for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
		{
			const ContentInfo *info = bitmapSources[i];
			if (info && s_BitmapSetup->bitmapSourceExists(i))
			{
				swr32(BITMAP_HANDLE(i));
				// printf("%i\n", i);
				swr32(BITMAP_SOURCE(info->MemoryAddress));
				// printf("%i, %i, %i\n", info->ImageFormat, info->CachedImageStride, info->CachedImageHeight);
				if (info->Converter == ContentInfo::Image) // Always use cached data from content info for image layout
					swr32(BITMAP_LAYOUT(info->ImageFormat, info->CachedImageStride, info->CachedImageHeight));
				else
					swr32(BITMAP_LAYOUT(bitmapInfo[i].LayoutFormat, bitmapInfo[i].LayoutStride, bitmapInfo[i].LayoutHeight));
				swr32(BITMAP_SIZE(bitmapInfo[i].SizeFilter, bitmapInfo[i].SizeWrapX, bitmapInfo[i].SizeWrapY, bitmapInfo[i].SizeWidth, bitmapInfo[i].SizeHeight));
			}
		}
		swrend();
		wr32(REG_DLSWAP, DLSWAP_FRAME);
		while (rd32(REG_DLSWAP) != DLSWAP_DONE)
		{
			printf("Waiting for bitmap setup DL swap\n");
			if (!s_EmulatorRunning) return;
		}
		wr32(REG_PCLK, 5);
		s_BitmapSetupModNb = s_BitmapSetup->getModificationNb();
	}*/
	s_ContentManager->unlockContent();

	// switch to next resolution
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE)) != s_HSize)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE), s_HSize);
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE)) != s_VSize)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE), s_VSize);
	// switch to next rotation (todo: CMD_SETROTATE for coprocessor)
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROTATE)) != s_Rotate)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROTATE), s_Rotate);
	// switch to next macro list
	s_Macro->lockDisplayList();
	bool macroModified = s_Macro->isDisplayListModified();
	// if (macroModified) // Always write macros to intial user value, in case changed by coprocessor
	// {
		if (s_Macro->getDisplayListParsed()[0].ValidId
			&& rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_0)) != s_Macro->getDisplayList()[0]) // Do a read test so we don't change the ram if not necessary
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_0), s_Macro->getDisplayList()[0]); // (because ram writes cause the write count to increase and force a display render)
		if (s_Macro->getDisplayListParsed()[1].ValidId
			&& rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_1)) != s_Macro->getDisplayList()[1])
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_1), s_Macro->getDisplayList()[1]);
	// }
	s_Macro->unlockDisplayList();
	// switch to next display list
	s_DlEditor->lockDisplayList();
	s_CmdEditor->lockDisplayList();
	bool dlModified = s_DlEditor->isDisplayListModified();
	bool cmdModified = s_CmdEditor->isDisplayListModified();
	if (dlModified || cmdModified || reuploadFontSetup || (g_StepCmdLimit != s_StepCmdLimitCurrent) || s_WantReloopCmd)
	{
		bool warnMissingClear = true;
		s_WantReloopCmd = false;
		s_StepCmdLimitCurrent = g_StepCmdLimit;
		// if (dlModified) printf("dl modified\n");
		// if (cmdModified) printf("cmd modified\n");
		uint32_t *displayList = s_DlEditor->getDisplayList();
		swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_DL));
		for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
		{
			// printf("dl %i: %i\n", i, displayList[i]);
			if ((displayList[i] & ~(CLEAR(0, 0, 0) ^ CLEAR(1, 1, 1))) == CLEAR(0, 0, 0))
				warnMissingClear = false;
			swr32(displayList[i]);
		}
		swrend();
		// wr32(REG_DLSWAP, DLSWAP_FRAME);
		// displayListSwapped = true;
		s_DlEditor->unlockDisplayList();

		uint32_t cmdList[FTEDITOR_DL_SIZE];
		// DlParsed cmdParsed[FTEDITOR_DL_SIZE];
		s_CmdParamCache.clear();
		s_CmdStrParamCache.clear();
		int strParamRead = 0;
		int cmdParamCache[FTEDITOR_DL_SIZE + 1];
		bool cmdValid[FTEDITOR_DL_SIZE];
		uint32_t *cmdListPtr = s_CmdEditor->getDisplayList();
		const DlParsed *cmdParsedPtr = s_CmdEditor->getDisplayListParsed();
		// Make local copy, necessary in case of blocking commands
		for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
		{
			cmdList[i] = cmdListPtr[i];
			// cmdParsed[i] = cmdParsedPtr[i];
			cmdParamCache[i] = (int)s_CmdParamCache.size();
			DlParser::compile(FTEDITOR_CURRENT_DEVICE, s_CmdParamCache, cmdParsedPtr[i]);
			cmdValid[i] = cmdParsedPtr[i].ValidId;
			if (cmdValid[i])
			{
				switch (cmdList[i])
				{
				case CMD_LOADIMAGE:
				case CMD_PLAYVIDEO:
					s_CmdStrParamCache.push_back(cmdParsedPtr[i].StringParameter);
					break;
				}
				if ((cmdList[i] & ~(CLEAR(0, 0, 0) ^ CLEAR(1, 1, 1))) == CLEAR(0, 0, 0))
					warnMissingClear = false;
			}
		}
		cmdParamCache[FTEDITOR_DL_SIZE] = (int)s_CmdParamCache.size();
		s_CmdEditor->unlockDisplayList();
		s_WarnMissingClear = warnMissingClear;

		s_ShowCoprocessorBusy = true;
		bool validCmd = false;
		int coprocessorWrites[1024]; // array indexed by write pointer of command index in the coprocessor editor gui
		for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
		for (int i = 0; i < FTEDITOR_DL_SIZE; ++i) s_DisplayListCoprocessorCommandWrite[i] = -1;
		int wp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE));
		int rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
		int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
		// printf("fullness: %i\n", fullness); // should be 0 always (ok)
		int freespace = ((4096 - 4) - fullness);
		BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);
		swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
		swr32(CMD_COLDSTART);
		wp += 4;
		freespace -= 4;
		s_MediaFifoPtr = 0;
		s_MediaFifoSize = 0;
		for (int i = 0; i < (s_StepCmdLimitCurrent ? s_StepCmdLimitCurrent : FTEDITOR_DL_SIZE); ++i) // FIXME CMD SIZE
		{
			// const DlParsed &pa = cmdParsed[i];
			// Skip invalid lines (invalid id)
			if (!cmdValid[i]) continue;
			bool useMediaFifo;
			const char *useFileStream = NULL;
			if ((FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) && (cmdList[i] == CMD_MEDIAFIFO))
			{
				s_MediaFifoPtr = s_CmdParamCache[cmdParamCache[i]];
				s_MediaFifoSize = s_CmdParamCache[cmdParamCache[i] + 1];
			}
			else if (cmdList[i] == CMD_LOADIMAGE)
			{
				useFileStream = s_CmdStrParamCache[strParamRead].c_str();
				++strParamRead;
				useMediaFifo = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
					&& ((s_CmdParamCache[cmdParamCache[i] + 1] & OPT_MEDIAFIFO) == OPT_MEDIAFIFO);
			}
			else if (cmdList[i] == CMD_PLAYVIDEO)
			{
				useFileStream = s_CmdStrParamCache[strParamRead].c_str();
				++strParamRead;
				useMediaFifo = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
					&& ((s_CmdParamCache[cmdParamCache[i]] & OPT_MEDIAFIFO) == OPT_MEDIAFIFO);
			}
			else if (cmdList[i] == CMD_SNAPSHOT)
			{
				// Validate snapshot address range
				uint32_t addr = s_CmdParamCache[cmdParamCache[i]];
				uint32_t ramGEnd = FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END);
				uint32_t imgSize = (s_VSize * s_HSize) * 2;
				if (addr + imgSize > ramGEnd)
				{
					printf("Dropping CMD_SNAPSHOT, out of memory range\n");
					continue;
				}
			}
			if (useFileStream)
			{
				if (!QFileInfo(useFileStream).exists())
					continue;
			}
			validCmd = true;
			int paramNb = cmdParamCache[i + 1] - cmdParamCache[i];
			int cmdLen = 4 + (paramNb * 4);
			if (freespace < (cmdLen + 8)) // Wait for coprocessor ready, + 4 for swap and display afterwards
			{
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				do
				{
					if (!s_EmulatorRunning) return;
					rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
					if ((rp & 0xFFF) == 0xFFF) return;
					fullness = ((wp & 0xFFF) - rp) & 0xFFF;
				} while (fullness != 0);
				freespace = ((4096 - 4) - fullness);

				int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
				for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
				{
					if (cpWrite[i] >= 0)
					{
						// printf("A %i\n", i);
						s_DisplayListCoprocessorCommandWrite[i]
							= coprocessorWrites[cpWrite[i]];
					}
				}
				for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
				BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
			}
			int wpn = 0;
			coprocessorWrites[(wp & 0xFFF) >> 2] = i;
			swr32(cmdList[i]);
			// printf("cmd %i", cmdList[i]);
			for (int j = cmdParamCache[i]; j < cmdParamCache[i + 1]; ++j)
			{
				++wpn;
				// printf("; param %i", s_CmdParamCache[j]);
				coprocessorWrites[((wp >> 2) + wpn) & 0x3FF] = i;
				swr32(s_CmdParamCache[j]);
			}
			// printf("\n");
			wp += cmdLen;
			freespace -= cmdLen;
			// Handle special cases
			if (cmdList[i] == CMD_LOGO)
			{
				printf("Waiting for CMD_LOGO...\n");
				s_WaitingCoprocessorAnimation = true;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				printf("WP = %i\n", wp);
				QThread::msleep(100);

				do
				{
					rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
					wp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE));
					if (!s_EmulatorRunning) { s_WaitingCoprocessorAnimation = false; return; }
					if ((rp & 0xFFF) == 0xFFF) { s_WaitingCoprocessorAnimation = false; return; }
					if (s_CmdEditor->isDisplayListModified()) { s_WantReloopCmd = true; resetCoprocessorFromLoop(); s_WaitingCoprocessorAnimation = false; return; }
				} while (rp || wp);
				wp = 0;
				rp = 0;
				fullness = ((wp & 0xFFF) - rp) & 0xFFF;
				freespace = ((4096 - 4) - fullness);

				int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
				for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
				{
					if (cpWrite[i] >= 0)
					{
						// printf("D %i\n", i);
						s_DisplayListCoprocessorCommandWrite[i]
							= coprocessorWrites[cpWrite[i]];
					}
				}
				for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
				BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

				if (wp == 0) printf("WP 0\n");
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				swr32(CMD_DLSTART);
				swr32(CMD_COLDSTART);
				wp += 8;
				freespace -= 8;
				s_MediaFifoPtr = 0;
				s_MediaFifoSize = 0;
				swrend();
				if (wp == 8) printf("WP 8\n");
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) { s_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { s_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				s_WaitingCoprocessorAnimation = false;
				s_WantReloopCmd = true;
				printf("Finished CMD_LOGO\n");
			}
			else if (cmdList[i] == CMD_CALIBRATE)
			{
				printf("Waiting for CMD_CALIBRATE...\n");
				s_WaitingCoprocessorAnimation = true;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) { s_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { s_WaitingCoprocessorAnimation = false; return; }
					if (s_CmdEditor->isDisplayListModified()) { s_WantReloopCmd = true; resetCoprocessorFromLoop(); s_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				swr32(CMD_DLSTART);
				swr32(CMD_COLDSTART);
				wp += 8;
				freespace -= 8;
				s_MediaFifoPtr = 0;
				s_MediaFifoSize = 0;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) { s_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { s_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				s_WaitingCoprocessorAnimation = false;
				s_WantReloopCmd = true;
				printf("Finished CMD_CALIBRATE\n");
			}
			if (useFileStream)
			{
				printf("Flush before stream\n");
				if (true) // Flush first
				{
					swrend();
					wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
					do
					{
						if (!s_EmulatorRunning) return;
						rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
						if ((rp & 0xFFF) == 0xFFF) return;
						fullness = ((wp & 0xFFF) - rp) & 0xFFF;

						if (s_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
						{
							printf("Abort coprocessor flush (793)\n");
							s_WantReloopCmd = true;
							resetCoprocessorFromLoop();
							return;
						}
					} while (fullness > cmdLen); // Ok to keep streaming command in
					freespace = ((4096 - 4) - fullness);

					int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
					for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
					{
						if (cpWrite[i] >= 0)
						{
							// printf("A %i\n", i);
							s_DisplayListCoprocessorCommandWrite[i]
								= coprocessorWrites[cpWrite[i]];
						}
					}
					for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
					BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

					swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				}

				if (useMediaFifo)
				{
					if (s_MediaFifoSize)
					{
						swrend();

						cleanupMediaFifo();
						s_MediaFifoFile = new QFile(useFileStream);
						s_MediaFifoFile->open(QIODevice::ReadOnly);
						s_MediaFifoStream = new QDataStream(s_MediaFifoFile);

						s_StreamingData = true;
						printf("Streaming into media fifo\n");
						int writeCount = 0;
						QDataStream &mfstream = *s_MediaFifoStream;
						ramaddr mfptr = s_MediaFifoPtr;
						ramaddr mfsz = s_MediaFifoSize;
						ramaddr mfwp;
						ramaddr mfrp;
						ramaddr mffree;
#define mfwprd() mfwp = (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MEDIAFIFO_WRITE)) /*- mfptr*/) % mfsz;
#define mfwpinc(v) { mfwp += v; mffree -= v; mfwp %= mfsz; }
#define mfwpwr() wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MEDIAFIFO_WRITE), mfwp /*+ mfptr*/);
#define mffreespace ((mfrp > mfwp) ? (mfrp - mfwp - 4) : (mfrp + s_MediaFifoSize - mfwp - 4))
#define mfrprd() { mfrp = (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MEDIAFIFO_READ)) /*- mfptr*/) % mfsz; mffree = mffreespace; }
						mfwprd();
						mfrprd();

						printf("Media fifo write start at %x (%i)\n", mfptr + mfwp, mfwp);
						printf("Media fifo read start at %x (%i)\n", mfptr + mfrp, mfrp);
						printf("Media fifo freespace start at %i\n", mffree);

						swrbegin(mfptr + mfwp);
						for (;;)
						{
							if (mffree < 4)
							{
								swrend();

								printf("Media fifo stream: %i bytes\n", (int)writeCount);

								mfwpwr();
								printf("Media fifo write at %x (%i)\n", mfptr + mfwp, mfwp);
								printf("Media fifo read at %x (%i)\n", mfptr + mfrp, mfrp);
								printf("Media fifo freespace at %i\n", mffree);
								do
								{
									if (!s_EmulatorRunning) return;
									ramaddr rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
									if ((rp & 0xFFF) == 0xFFF) return;
									mfrprd();

									if (s_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
									{
										printf("Abort media fifo flush (871)\n");
										s_WantReloopCmd = true;
										resetCoprocessorFromLoop();
										return;
									}
								} while (mffree < (mfsz >> 1));

								swrbegin(mfptr + mfwp);
							}

							uint32_t buffer;
							int nb = mfstream.readRawData(reinterpret_cast<char *>(&buffer), 4);
							if (nb > 0)
							{
								// write
								swr32(buffer);
								writeCount += nb;
								mfwpinc(4);

								if (mfwp == 0)
								{
									swrend();
									swrbegin(mfptr + mfwp);
								}
							}
							if (nb != 4)
							{
								// done
								swrend();
								mfrprd();
								mfwpwr();
								printf("Media fifo stream finished: %i bytes\n", (int)writeCount);
								printf("Media fifo write end at %x (%i)\n", mfptr + mfwp, mfwp);
								printf("Media fifo read end at %x (%i)\n", mfptr + mfrp, mfrp);
								printf("Media fifo freespace end at %i\n", mffree);
								if (writeCount == 0)
								{
									resetCoprocessorFromLoop();
									return;
								}
								break;
							}
							if (!s_EmulatorRunning)
							{
								swrend();
								return;
							}
							if (s_CmdEditor->isDisplayListModified())
							{
								s_WantReloopCmd = true;
								swrend();
								resetCoprocessorFromLoop();
								return;
							}
						}
						s_StreamingData = false;
						printf("Media fifo finished\n");

#undef mfrprd
#undef mfwprd
#undef mfwpinc
#undef mfwpwr
#undef mffreespace

						swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
					}
					else
					{
						printf("Media fifo not setup\n");
						swrend();
						resetCoprocessorFromLoop();
						return;
					}
				}
				else
				{
					s_StreamingData = true;
					printf("Streaming in file '%s'...\n", useFileStream); // NOTE: abort on edit by reset
					QFile cmdFile(useFileStream);
					printf("File size: %i\n", (int)cmdFile.size());
					cmdFile.open(QIODevice::ReadOnly);
					QDataStream cmdStream(&cmdFile);
					int writeCount = 0;
					
					for (;;)
					{
						if (freespace < (4 + 8) || writeCount < 128) // Wait for coprocessor free space, + 4 for swap and display afterwards
						{
							// COPY PASTE FROM ABOVE
							
							swrend();
							wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
							do
							{
								if (!s_EmulatorRunning) return;
								rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
								if ((rp & 0xFFF) == 0xFFF)
								{
									printf("Error during stream at %i bytes\n", (int)writeCount);
									return; // DIFFER FROM ABOVE HERE
								}
								fullness = ((wp & 0xFFF) - rp) & 0xFFF;

								if (s_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
								{
									printf("Abort streaming flush (976)\n");
									s_WantReloopCmd = true;
									resetCoprocessorFromLoop();
									return;
								}
							} while (fullness > 1024); // DIFFER FROM ABOVE HERE
							freespace = ((4096 - 4) - fullness);

							int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
							for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
							{
								if (cpWrite[i] >= 0)
								{
									// printf("A %i\n", i);
									s_DisplayListCoprocessorCommandWrite[i]
										= coprocessorWrites[cpWrite[i]];
								}
							}
							for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
							BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

							printf("Stream: %i bytes\n", (int)writeCount);

							swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
						}
						
						uint32_t buffer;
						int nb = cmdStream.readRawData(reinterpret_cast<char *>(&buffer), 4);
						if (nb > 0)
						{
							// write
							coprocessorWrites[(wp >> 2) & 0x3FF] = i;
							writeCount += nb;
							wp += 4;
							freespace -= 4;
							swr32(buffer);
						}
						if (nb != 4)
						{
							// done
							printf("Stream finished: %i bytes\n", (int)writeCount);
							if (writeCount == 0)
							{
								swrend();
								resetCoprocessorFromLoop();
								return;
							}
							break;
						}
						if (!s_EmulatorRunning)
						{
							swrend();
							return;
						}
						if (s_CmdEditor->isDisplayListModified())
						{
							s_WantReloopCmd = true;
							swrend();
							resetCoprocessorFromLoop();
							return;
						}
					}
					printf("Finished streaming in file\n");
					s_StreamingData = false;
				}

				printf("Flush after stream\n");
				if (true) // Flush after
				{
					swrend();
					wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
					do
					{
						if (!s_EmulatorRunning) return;
						rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
						if ((rp & 0xFFF) == 0xFFF) return;
						fullness = ((wp & 0xFFF) - rp) & 0xFFF;

						if (s_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
						{
							printf("Abort coprocessor flush (1056)\n");
							s_WantReloopCmd = true;
							resetCoprocessorFromLoop();
							return;
						}
					} while (fullness > 0); // Want completely empty
					freespace = ((4096 - 4) - fullness);

					int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
					for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
					{
						if (cpWrite[i] >= 0)
						{
							// printf("A %i\n", i);
							s_DisplayListCoprocessorCommandWrite[i]
								= coprocessorWrites[cpWrite[i]];
						}
					}
					for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
					BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

					swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				}
			}
		}

		if (validCmd)
		{
			swr32(DISPLAY());
			swr32(CMD_SWAP);
			wp += 8;
			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			// Finish all processing
			int rpl = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
			s_AbortTimer.start();
			while ((wp & 0xFFF) != rpl)
			{
				rpl = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
				if (!s_EmulatorRunning) return;
				if ((rpl & 0xFFF) == 0xFFF) return;

				if (s_CmdEditor->isDisplayListModified() || s_WantReloopCmd) // Trap to avoid infinite flush on errors
				{
					s_WantReloopCmd = true;
					if (s_AbortTimer.elapsed() > 1000)
					{
						printf("Abort coprocessor flush (1100)\n");
						resetCoprocessorFromLoop();
						return;
					}
				}
			}
			int *cpWrite = BT8XXEMU_getDisplayListCoprocessorWrites(g_Emulator);
			for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
			{
				if (cpWrite[i] >= 0)
				{
					// printf("C %i, %i, %i\n", i, cpWrite[i], coprocessorWrites[cpWrite[i]]);
					s_DisplayListCoprocessorCommandWrite[i]
						= coprocessorWrites[cpWrite[i]];
				}
			}
			for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
			BT8XXEMU_clearDisplayListCoprocessorWrites(g_Emulator);

			for (int i = FTEDITOR_DL_SIZE - 1; i >= 0; --i)
			{
				if (s_DisplayListCoprocessorCommandWrite[i] >= 0)
				{
					s_UtilizationDisplayListCmd = i;
					break;
				}
			}

			// Test
			/*for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
			{
				if (s_DisplayListCoprocessorCommandWrite[i] >= 0)
				{
					std::string res;
					DlParser::toString(res, rd32(RAM_DL + (i * 4)));
					printf("DL %i was written by CMD %i: %s\n", i, s_DisplayListCoprocessorCommandWrite[i], res.c_str());
				}
			}*/

			swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
			swr32(CMD_DLSTART);
			wp += 4;
			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			s_CoprocessorFaultOccured = false;
			s_StreamingData = false;
			coprocessorSwapped = true;
		}
		else
		{
			// Swap frame directly if nothing was written to the coprocessor
			s_UtilizationDisplayListCmd = 0;

			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_DLSWAP), DLSWAP_FRAME);
		}

		s_ShowCoprocessorBusy = false;

		// FIXME: Not very thread-safe, but not too critical
		int *nextWrite = s_DisplayListCoprocessorCommandRead;
		s_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandWrite;
		s_DisplayListCoprocessorCommandWrite = nextWrite;
	}
	else
	{
		s_CmdEditor->unlockDisplayList();
		s_DlEditor->unlockDisplayList();
	}

	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && s_MediaFifoStream && s_MediaFifoSize)
	{
		// Write out to media FIFO


	}
}

void emuMain(BT8XXEMU_Emulator *sender, void *context)
{
	setup();

	while (BT8XXEMU_isRunning(sender))
	{
		loop();
	}
}

void keyboard(BT8XXEMU_Emulator *sender, void *context)
{

}

bool MainWindow::waitingCoprocessorAnimation()
{
	return s_WaitingCoprocessorAnimation;
}

int *MainWindow::getDlCmd()
{
	// FIXME: Not very thread-safe, but not too critical
	return s_DisplayListCoprocessorCommandRead;
}

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL),
	m_EmulatorViewport(NULL),
	m_DlEditor(NULL), m_DlEditorDock(NULL), m_CmdEditor(NULL), m_CmdEditorDock(NULL),
	m_PropertiesEditor(NULL), m_PropertiesEditorScroll(NULL), m_PropertiesEditorDock(NULL),
	m_ToolboxDock(NULL), m_Toolbox(NULL), m_ContentManagerDock(NULL), m_ContentManager(NULL),
	m_RegistersDock(NULL), m_Macro(NULL), m_HSize(NULL), m_VSize(NULL), m_Rotate(NULL), 
	m_ControlsDock(NULL), m_StepEnabled(NULL), m_StepCount(NULL), m_StepCmdEnabled(NULL), m_StepCmdCount(NULL),
	m_TraceEnabled(NULL), m_TraceX(NULL), m_TraceY(NULL),
	m_FileMenu(NULL), m_EditMenu(NULL), m_ToolsMenu(NULL), m_WidgetsMenu(NULL),
#ifdef FT800EMU_PYTHON
	m_ScriptsMenu(NULL),
#endif
	m_HelpMenu(NULL),
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_NewAct(NULL), m_OpenAct(NULL), m_SaveAct(NULL), m_SaveAsAct(NULL),
	m_ImportAct(NULL), m_ExportAct(NULL), m_ProjectFolderAct(NULL), m_ResetEmulatorAct(NULL), m_SaveScreenshotAct(NULL), m_ImportDisplayListAct(NULL),
	m_DisplayListFromIntegers(NULL), m_ManualAct(NULL), m_AboutAct(NULL), m_AboutQtAct(NULL), m_QuitAct(NULL), // m_PrintDebugAct(NULL),
	m_UndoAct(NULL), m_RedoAct(NULL), //, m_SaveScreenshotAct(NULL)
	m_CursorPosition(NULL), m_CoprocessorBusy(NULL), 
	m_TemporaryDir(NULL)
{
	setObjectName("MainWindow");
	setWindowIcon(QIcon(":/icons/eve-puzzle-16.png"));

	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
	setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

	m_InitialWorkingDir = QDir::currentPath();
	m_UndoStack = new QUndoStack(this);
	connect(m_UndoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoCleanChanged(bool)));

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	m_EmulatorViewport = new InteractiveViewport(this);

	QWidget *centralWidget = new QWidget(this);
	QGridLayout *layout = new QGridLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_EmulatorViewport, 0, 0);
	layout->addWidget(m_EmulatorViewport->verticalScrollbar(), 0, 1);
	layout->addWidget(m_EmulatorViewport->horizontalScrollbar(), 1, 0);
	centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	createDockWindows();

	m_InteractiveProperties = new InteractiveProperties(this);
	m_InteractiveProperties->setVisible(false);

	incbLanguageCode();

	s_DlEditor = m_DlEditor;
	s_CmdEditor = m_CmdEditor;
	s_Macro = m_Macro;

	startEmulatorInternal();

	actNew(true);
}

MainWindow::~MainWindow()
{
	stopEmulatorInternal();

	m_ContentManager->lockContent();
	s_DlEditor = NULL;
	s_CmdEditor = NULL;
	s_Macro = NULL;
	s_ContentManager = NULL;
	m_ContentManager->unlockContent();
	//s_BitmapSetup = NULL;

	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = NULL;
}

#ifdef FT800EMU_PYTHON
void pythonError()
{
	printf("---\nPython ERROR: \n");
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyObject *errStr = PyObject_Repr(pvalue);
	char *pStrErrorMessage = PyString_AsString(errStr);
	QString error = QString::fromLocal8Bit(pStrErrorMessage);
	printf("%s\n", pStrErrorMessage);
	Py_DECREF(errStr);
	printf("---\n");
}

static QString scriptDisplayName(const QString &script)
{
	QByteArray scriptNa = script.toUtf8();
	char *scriptName = scriptNa.data();

	PyObject *pyUserScript = PyUnicode_FromString(scriptName);
	PyObject *pyUserModule = PyImport_Import(pyUserScript);
	Py_DECREF(pyUserScript); pyUserScript = NULL;

	if (!pyUserModule) { pythonError(); return script; }

	PyObject *pyUserFunc = PyObject_GetAttrString(pyUserModule, "displayName");

	if (!pyUserFunc) { pythonError(); return script; }

	if (!PyCallable_Check(pyUserFunc))
	{
		Py_DECREF(pyUserFunc); pyUserFunc = NULL;
		pythonError();
		return script;
	}

	PyObject *pyValue;
	PyObject *pyArgs = PyTuple_New(0);
	pyValue = PyObject_CallObject(pyUserFunc, pyArgs);
	Py_DECREF(pyArgs); pyArgs = NULL;

	Py_DECREF(pyUserFunc); pyUserFunc = NULL;
	Py_DECREF(pyUserModule); pyUserModule = NULL;

	if (pyValue)
	{
		char *resCStr = PyString_AsString(pyValue);
		QString res = QString::fromLocal8Bit(resCStr);
		Py_DECREF(pyValue); pyValue = NULL; // !
		return res;
	}

	pythonError();
	return script;
}
#endif

#ifdef FT800EMU_PYTHON

char *scriptFolder = "export_scripts";

char *scriptDeviceFolder[] = {
	"ft80x",
	"ft80x",
	"ft81x",
	"ft81x",
	"ft81x",
	"ft81x",
	"bt81x"
};

QString MainWindow::scriptModule()
{
	return QString(scriptFolder) 
		+ QString(".") 
		+ scriptDeviceFolder[FTEDITOR_CURRENT_DEVICE];
}

QString MainWindow::scriptDir()
{
	return m_InitialWorkingDir + "/"
		+ scriptFolder + "/"
		+ scriptDeviceFolder[FTEDITOR_CURRENT_DEVICE];
}
#endif

void MainWindow::refreshScriptsMenu()
{
#ifdef FT800EMU_PYTHON
	QDir currentDir = scriptDir();
	// printf("Look in %s\n", currentDir.absolutePath().toUtf8().data());

	QStringList filters;

	filters.push_back("*.py");
	// QStringList scriptFiles = currentDir.entryList(filters);

	std::map<QString, RunScript *> scriptActs;
	scriptActs.swap(m_ScriptActs);
	std::map<QString, QMenu *> scriptFolderMenus;
	scriptFolderMenus.swap(m_ScriptFolderMenus);

	m_ScriptFolderMenus["."] = m_ScriptsMenu;
	scriptFolderMenus.erase(".");

	//Fill up empty m_ScriptActs from previous values in scriptActs
	// for (int i = 0; i < scriptFiles.size(); ++i)
	QDirIterator it(currentDir.absolutePath(), filters, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QString scriptFile = it.next();
		// printf("script: %s\n", scriptFile.toLocal8Bit().data());
		scriptFile = currentDir.relativeFilePath(scriptFile);

		// Ignore root __init__.py
		if (scriptFile == "__init__.py")
			continue;

		QString scriptMod = scriptModule() + "." + scriptFile.left(scriptFile.size() - 3).replace('/', '.');
		// printf("module: %s\n", scriptMod.toLocal8Bit().data());
		QString scriptDir = QFileInfo(scriptFile).dir().path();
		// printf("dir: %s\n", scriptDir.toLocal8Bit().data());

		std::map<QString, QMenu *>::iterator menuIt = m_ScriptFolderMenus.find(scriptDir);
		QMenu *menu;
		if (menuIt == m_ScriptFolderMenus.end())
		{
			menuIt = scriptFolderMenus.find(scriptDir);
			if (menuIt == scriptFolderMenus.end())
			{
				menu = new QMenu(scriptDir, this);
				m_ScriptsMenu->addMenu(menu);
				m_ScriptFolderMenus[scriptDir] = menu;
				scriptFolderMenus.erase(scriptDir);
			}
			else
			{
				menu = menuIt->second;
				m_ScriptFolderMenus[scriptDir] = menu;
				scriptFolderMenus.erase(scriptDir);
			}
		}
		else
		{
			menu = menuIt->second;
		}

		if (QFileInfo(scriptFile).fileName() == "__init__.py")
		{
			// Set folder title

			menu->setTitle(scriptDisplayName(scriptMod));

			continue;
		}

		// char *fileName = scriptFiles[i].toLocal8Bit().data();
		// printf("Script: %s\n", fileName);

		//package.subpackage.module
		// printf("Module: %s\n", scriptMod.toLocal8Bit().data());

		if (scriptActs.find(scriptMod) == scriptActs.end())
		{
			// printf("new script: %s\n", scriptFile.toLocal8Bit().data());

			RunScript *rs = new RunScript();
			QAction *act = new QAction(this);

			act->setText(scriptDisplayName(scriptMod));
			act->setStatusTip(tr("Run Python script"));
			menu->addAction(act);

			if (act->text() == "")
				act->setVisible(false);

			rs->Action = act;

			rs->Script = scriptMod;
			rs->Window = this,
			connect(act, SIGNAL(triggered()), rs, SLOT(runScript()));
			m_ScriptActs[scriptMod] = rs;
		}
		else
		{
			//find existing script and add it to m_ScriptActs		
			m_ScriptActs[scriptMod] = scriptActs[scriptMod];
			scriptActs.erase(scriptMod);
		}
	}

	//delete non-existing run scripts
	for (std::map<QString, RunScript *>::iterator it = scriptActs.begin(), end = scriptActs.end(); it != end; ++it)
		delete it->second;
	for (std::map<QString, QMenu *>::iterator it = scriptFolderMenus.begin(), end = scriptFolderMenus.end(); it != end; ++it)
		delete it->second;
#endif
}

RunScript::~RunScript()
{
#ifdef FT800EMU_PYTHON
	delete Action;
#endif
}

void RunScript::runScript()
{
	Window->runScript(Script);
}

void MainWindow::runScript(const QString &script)
{
#ifdef FT800EMU_PYTHON
	QByteArray scriptNa = script.toUtf8();
	char *scriptName = scriptNa.data();
	statusBar()->showMessage(tr("Executed Python script '%1'").arg(scriptName));
	QString outputName = QFileInfo(m_CurrentFile).completeBaseName();
	if (outputName.isEmpty()) outputName = "untitled";
	QByteArray outN = outputName.toUtf8();

	////////////////////////////////////////////////////////////////////
	// Initialize JSON

	bool error = true;

	PyObject *pyJsonScript = PyString_FromString("json");
	PyObject *pyJsonModule = PyImport_Import(pyJsonScript);
	Py_DECREF(pyJsonScript); pyJsonScript = NULL;
	PyObject *pyJsonLoadS = NULL;

	if (pyJsonModule)
	{
		pyJsonLoadS = PyObject_GetAttrString(pyJsonModule, "loads");
		if (pyJsonLoadS)
		{
			printf("Json available\n");
			error = false;
		}
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyObject *errStr = PyObject_Repr(pvalue);
		char *pStrErrorMessage = PyString_AsString(errStr);
		QString error = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		Py_DECREF(errStr);
		printf("---\n");
		m_PropertiesEditor->setInfo("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}

	////////////////////////////////////////////////////////////////////
	// Create python object from JSON
	error = true;
	QByteArray jsonDoc = toJson(true);
	PyObject *pyJsonDoc = PyString_FromString(jsonDoc.data());
	PyObject *pyArgs = PyTuple_New(1);
	PyTuple_SetItem(pyArgs, 0, pyJsonDoc);
	PyObject *pyDocument = PyObject_CallObject(pyJsonLoadS, pyArgs);
	if (pyDocument) error = false;
	Py_DECREF(pyArgs); pyArgs = NULL;
	Py_DECREF(pyJsonLoadS); pyJsonLoadS = NULL;
	Py_DECREF(pyJsonModule); pyJsonModule = NULL;

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyObject *errStr = PyObject_Repr(pvalue);
		char *pStrErrorMessage = PyString_AsString(errStr);
		QString error = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		Py_DECREF(errStr);
		printf("---\n");
		m_PropertiesEditor->setInfo("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}

	/*PyObject *pyDocumentString = PyObject_Repr(pyDocument);
	printf("Demo: %s\n", PyString_AsString(pyDocumentString));
	Py_DECREF(pyDocumentString); pyDocumentString = NULL;
	Py_DECREF(pyDocument); pyDocument = NULL;*/

	////////////////////////////////////////////////////////////////////
	// Run script

	error = true;

	PyObject *pyUserScript = PyUnicode_FromString(scriptName);
	PyObject *pyUserModuleOld = PyImport_Import(pyUserScript);
	if (pyUserModuleOld != NULL)
	{
		PyObject *pyUserModule = PyImport_ReloadModule(pyUserModuleOld);
		Py_DECREF(pyUserScript); pyUserScript = NULL;

		if (pyUserModule != NULL)
		{
			PyObject *pyUserFunc = PyObject_GetAttrString(pyUserModule, "run");
			if (pyUserFunc && PyCallable_Check(pyUserFunc))
			{
				PyObject *pyValue;
				PyObject *pyArgs = PyTuple_New(3);
				pyValue = PyUnicode_FromString(outN.data());;
				PyTuple_SetItem(pyArgs, 0, pyValue);
				PyTuple_SetItem(pyArgs, 1, pyDocument); pyDocument = NULL;
				char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
				pyValue = PyByteArray_FromStringAndSize(ram, addressSpace(FTEDITOR_CURRENT_DEVICE));
				PyTuple_SetItem(pyArgs, 2, pyValue);
				pyValue = PyObject_CallObject(pyUserFunc, pyArgs);
				Py_DECREF(pyArgs); pyArgs = NULL;
				if (pyValue)
				{
					printf("Ok\n");
					PyObject *resStr = PyObject_Repr(pyValue);
					char *resCStr = PyString_AsString(resStr);
					QString res = QString::fromLocal8Bit(resCStr);
					Py_DECREF(pyValue); pyValue = NULL;
					m_PropertiesEditor->setInfo(res);
					m_PropertiesEditor->setEditWidget(NULL, false, NULL);
					error = false;
				}
				else
				{
					printf("Not ok\n");
				}
			}
			else
			{
				printf("Missing run function\n");
			}

			Py_XDECREF(pyUserFunc); pyUserFunc = NULL;
			Py_XDECREF(pyUserModule); pyUserModule = NULL;
		}
		Py_XDECREF(pyUserModuleOld); pyUserModuleOld = NULL;
	}

	Py_XDECREF(pyDocument); pyDocument = NULL;

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyObject *errStr = PyObject_Repr(pvalue);
		char *pStrErrorMessage = PyString_AsString(errStr);
		QString error = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		Py_DECREF(errStr);
		printf("---\n");
		m_PropertiesEditor->setInfo("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}
#endif /* FT800EMU_PYTHON */
}

void MainWindow::frameEmu()
{
	// ...
}

void MainWindow::frameQt()
{
	m_UtilizationBitmapHandleStatus->setValue(inspector()->countHandleUsage());

	int utilizationDisplayList = std::max(s_UtilizationDisplayListCmd, m_DlEditor->codeEditor()->document()->blockCount());
	m_UtilizationDisplayList->setValue(utilizationDisplayList);
	m_UtilizationDisplayListStatus->setValue(utilizationDisplayList);

	m_UtilizationGlobalStatus->setValue(g_RamGlobalUsage);

	if (!s_StreamingData && s_CoprocessorFaultOccured && (m_PropertiesEditor->getEditWidgetSetter() == m_DlEditor || m_PropertiesEditor->getEditWidgetSetter() == m_CmdEditor || m_PropertiesEditor->getEditWidgetSetter() == NULL))
	{
		QString info;
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
		{
			uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
			if (ram)
			{
				info = "<b>Co-processor engine fault</b><br><br>";
				info += QString::fromLatin1((char *)&ram[0x309800]);
			}
			else
			{
				info = "<b>Co-processor engine fault</b><br><br>"
					"Emulator not initialized";
			}
		}
		else
		{
			info = "<b>Co-processor engine fault</b><br><br>"
				"A co-processor engine fault occurs when the co-processor engine cannot continue. Possible causes:<br><br>"
				"- An attempt is made to write more than 2048 instructions into a display list<br><br>"
				"- An invalid JPEG is supplied to CMD_LOADIMAGE<br><br>"
				"- An invalid data stream is supplied to CMD_INFLATE";
		}
		m_PropertiesEditor->setInfo(info);
		m_PropertiesEditor->setEditWidget(NULL, false, m_PropertiesEditorDock); // m_PropertiesEditorDock is a dummy
		focusProperties();
	}

	// printf("msc: %s\n", s_WarnMissingClear ? "warn" : "ok");
	if (s_WarnMissingClear != s_WarnMissingClearActive)
	{
		if (s_WarnMissingClear)
		{
			statusBar()->showMessage(tr("WARNING: Missing CLEAR instruction in display list"));
		}
		else
		{
			statusBar()->showMessage("");
		}
		s_WarnMissingClearActive = s_WarnMissingClear;
	}

	// m_CursorPosition
	uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
	uint32_t addr = reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_TOUCH_SCREEN_XY);
	uint32_t regValue = reinterpret_cast<uint32_t &>(ram[addr]);
	if (m_EmulatorViewport->mouseOver())
		m_CursorPosition->setText(QString::number(m_EmulatorViewport->mouseX()) + " x " + QString::number(m_EmulatorViewport->mouseY()));
	else
		m_CursorPosition->setText("");

	// Busy loader
	m_CoprocessorBusy->setVisible(s_ShowCoprocessorBusy && !s_WaitingCoprocessorAnimation);
}

void MainWindow::createActions()
{
	m_NewAct = new QAction(this);
	m_NewAct->setShortcuts(QKeySequence::New);
	connect(m_NewAct, SIGNAL(triggered()), this, SLOT(actNew()));
	m_OpenAct = new QAction(this);
	connect(m_OpenAct, SIGNAL(triggered()), this, SLOT(actOpen()));

	m_SaveAct = new QAction(this);
	m_SaveAct->setShortcuts(QKeySequence::Save);
	connect(m_SaveAct, SIGNAL(triggered()), this, SLOT(actSave()));
	m_SaveAsAct = new QAction(this);
	connect(m_SaveAsAct, SIGNAL(triggered()), this, SLOT(actSaveAs()));

	m_ImportAct = new QAction(this);
	connect(m_ImportAct, SIGNAL(triggered()), this, SLOT(actImport()));
	m_ImportAct->setVisible(FT_VCDUMP_VISIBLE);
	m_ExportAct = new QAction(this);
	connect(m_ExportAct, SIGNAL(triggered()), this, SLOT(actExport()));
	m_ExportAct->setVisible(FT_VCDUMP_VISIBLE);

	m_ProjectFolderAct = new QAction(this);
	connect(m_ProjectFolderAct, SIGNAL(triggered()), this, SLOT(actProjectFolder()));

	m_ResetEmulatorAct = new QAction(this);
	connect(m_ResetEmulatorAct, SIGNAL(triggered()), this, SLOT(actResetEmulator()));

	m_SaveScreenshotAct = new QAction(this);
	connect(m_SaveScreenshotAct, SIGNAL(triggered()), this, SLOT(actSaveScreenshot()));

	m_ImportDisplayListAct = new QAction(this);
	connect(m_ImportDisplayListAct, SIGNAL(triggered()), this, SLOT(actImportDisplayList()));

	m_DisplayListFromIntegers = new QAction(this);
	connect(m_DisplayListFromIntegers, SIGNAL(triggered()), this, SLOT(actDisplayListFromIntegers()));

	m_QuitAct = new QAction(this);
	m_QuitAct->setShortcuts(QKeySequence::Quit);
	connect(m_QuitAct, SIGNAL(triggered()), this, SLOT(close()));

	m_ManualAct = new QAction(this);
	connect(m_ManualAct, SIGNAL(triggered()), this, SLOT(manual()));
	m_AboutAct = new QAction(this);
	connect(m_AboutAct, SIGNAL(triggered()), this, SLOT(about()));
	m_AboutQtAct = new QAction(this);
	connect(m_AboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

	//m_PrintDebugAct = new QAction(this);
	//connect(m_PrintDebugAct, SIGNAL(triggered()), this, SLOT(printDebug()));

	m_UndoAct = m_UndoStack->createUndoAction(this);
	m_UndoAct->setShortcuts(QKeySequence::Undo);
	m_RedoAct = m_UndoStack->createRedoAction(this);
	m_RedoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));

	m_DummyAct = new QAction(this);
	connect(m_DummyAct, SIGNAL(triggered()), this, SLOT(dummyCommand()));

	// m_SaveScreenshotAct = m_EmulatorViewport->createSaveScreenshotAction(this);
}

void MainWindow::translateActions()
{
	m_NewAct->setText(tr("New"));
	m_NewAct->setStatusTip(tr("Create a new project"));
	m_NewAct->setIcon(QIcon(":/icons/document.png"));
	m_OpenAct->setText(tr("Open"));
	m_OpenAct->setStatusTip(tr("Open an existing project"));
	m_OpenAct->setIcon(QIcon(":/icons/folder-horizontal-open.png"));
	m_SaveAct->setText(tr("Save"));
	m_SaveAct->setStatusTip(tr("Save the current project"));
	m_SaveAct->setIcon(QIcon(":/icons/disk.png"));
	m_SaveAsAct->setText(tr("Save As"));
	m_SaveAsAct->setStatusTip(tr("Save the current project to a new file"));;
	m_SaveAsAct->setIcon(QIcon(":/icons/disk--pencil.png"));
	m_ImportAct->setText(tr("Import"));
	m_ImportAct->setStatusTip(tr("Import file to a new project"));
	m_ExportAct->setText(tr("Export"));
	m_ExportAct->setStatusTip(tr("Export project to file"));
	m_ProjectFolderAct->setText(tr("Browse Project Folder"));
	m_ProjectFolderAct->setStatusTip(tr("Open the project folder in the default system file browser"));
	m_ResetEmulatorAct->setText(tr("Reset Emulator"));
	m_ResetEmulatorAct->setStatusTip(tr("Reset the emulated device"));
	m_SaveScreenshotAct->setText(tr("Save Screenshot"));
	m_SaveScreenshotAct->setStatusTip(tr("Save a screenshot of the emulator output"));
	m_ImportDisplayListAct->setText(tr("Capture Display List"));
	m_ImportDisplayListAct->setStatusTip(tr("Capture the active display list from the emulator into the editor"));
	m_DisplayListFromIntegers->setText(tr("Display List from Integers (debug mode only)"));
	m_DisplayListFromIntegers->setStatusTip(tr("Developer tool (debug mode only)"));
	m_QuitAct->setText(tr("Quit"));
	m_QuitAct->setStatusTip(tr("Exit the application"));
	m_ManualAct->setText(tr("Manual"));
	m_ManualAct->setStatusTip(tr("Open the manual"));
	m_AboutAct->setText(tr("About"));
	m_AboutAct->setStatusTip(tr("Show information about the application"));
	m_AboutQtAct->setText(tr("3rd Party"));
	m_AboutQtAct->setStatusTip(tr("Show information about the 3rd party code and content"));
	// m_PrintDebugAct->setText(tr("ActionPrintDebug"));
	// m_PrintDebugAct->setStatusTip(tr("ActionPrintDebugStatusTip"));
	m_UndoAct->setText(tr("Undo"));
	m_UndoAct->setStatusTip(tr("Reverses the last action"));
	m_UndoAct->setIcon(QIcon(":/icons/arrow-return-180.png"));
	m_RedoAct->setText(tr("Redo"));
	m_RedoAct->setStatusTip(tr("Reapply the action"));
	m_RedoAct->setIcon(QIcon(":/icons/arrow-circle-315.png"));
	m_DummyAct->setText(tr("Dummy"));
	m_DummyAct->setStatusTip(tr("Does nothing"));
	// m_SaveScreenshotAct->setText(tr("ActionSaveScreenshot"));
	// m_SaveScreenshotAct->setStatusTip(tr("ActionSaveScreenshotStatusTip"));
}

void MainWindow::createMenus()
{
	m_FileMenu = menuBar()->addMenu(QString::null);
	m_FileMenu->addAction(m_NewAct);
	m_FileMenu->addAction(m_OpenAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_SaveAct);
	m_FileMenu->addAction(m_SaveAsAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_ProjectFolderAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_ImportAct);
	m_FileMenu->addAction(m_ExportAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_SaveScreenshotAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_QuitAct);

	m_EditMenu = menuBar()->addMenu(QString::null);
	m_EditMenu->addAction(m_UndoAct);
	m_EditMenu->addAction(m_RedoAct);
	//m_EditMenu->addAction(m_DummyAct);

	m_ToolsMenu = menuBar()->addMenu(QString::null);
	m_ToolsMenu->addAction(m_ResetEmulatorAct);
	// m_ToolsMenu->addAction(m_SaveScreenshotAct);
	m_ToolsMenu->addAction(m_ImportDisplayListAct);
#if _DEBUG
	m_ToolsMenu->addAction(m_DisplayListFromIntegers);
#endif

	m_WidgetsMenu = menuBar()->addMenu(QString::null);

#ifdef FT800EMU_PYTHON
	m_ScriptsMenu = menuBar()->addMenu(QString::null);
	connect(m_ScriptsMenu, SIGNAL(aboutToShow()), this, SLOT(refreshScriptsMenu()));
#endif /* FT800EMU_PYTHON */

	menuBar()->addSeparator();

	m_HelpMenu = menuBar()->addMenu(QString::null);
	m_HelpMenu->addAction(m_ManualAct);
	m_HelpMenu->addSeparator();
	m_HelpMenu->addAction(m_AboutAct);
	m_HelpMenu->addAction(m_AboutQtAct);
}

void MainWindow::translateMenus()
{
	m_FileMenu->setTitle(tr("File"));
	m_EditMenu->setTitle(tr("Edit"));
	m_ToolsMenu->setTitle(tr("Tools"));
	m_WidgetsMenu->setTitle(tr("View"));
#ifdef FT800EMU_PYTHON
	m_ScriptsMenu->setTitle(tr("Export"));
#endif /* FT800EMU_PYTHON */
	m_HelpMenu->setTitle(tr("Help"));
}

void MainWindow::createToolBars()
{
	QToolBar *mainBar = addToolBar(tr("MainBar"));
	mainBar->setIconSize(QSize(16, 16));
	mainBar->addAction(m_NewAct);
	mainBar->addAction(m_OpenAct);
	mainBar->addAction(m_SaveAct);
	mainBar->addSeparator();
	mainBar->addAction(m_UndoAct);
	mainBar->addAction(m_RedoAct);
}

void MainWindow::translateToolBars()
{
	//m_FileToolBar->setWindowTitle(tr("File"));
	//m_EditToolBar->setWindowTitle(tr("Edit"));

}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
	// Navigator
	{
		m_NavigatorDock = new QDockWidget(this);
		m_NavigatorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_NavigatorDock->setObjectName("Navigator");
		m_EmulatorNavigator = new EmulatorNavigator(this, m_EmulatorViewport);
		m_NavigatorDock->setWidget(m_EmulatorNavigator);
		m_NavigatorDock->setMinimumHeight(100);
		addDockWidget(Qt::RightDockWidgetArea, m_NavigatorDock);
		m_WidgetsMenu->addAction(m_NavigatorDock->toggleViewAction());
	}

	// Project
	{
		m_ProjectDock = new QDockWidget(this);
		m_ProjectDock->setAllowedAreas(Qt::RightDockWidgetArea);
		m_ProjectDock->setObjectName("Project");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		QWidget *widget = new QWidget(this);
		QVBoxLayout *layout = new QVBoxLayout();

		// Device
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Device"));
			QHBoxLayout *groupLayout = new QHBoxLayout();

			m_ProjectDevice = new QComboBox(this);
			for (int i = 0; i < FTEDITOR_DEVICE_NB; ++i)
				m_ProjectDevice->addItem(deviceToString(i));
			m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE);
			groupLayout->addWidget(m_ProjectDevice);
			connect(m_ProjectDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(projectDeviceChanged(int)));

			m_ProjectDisplay = new QComboBox(this);
			for (int i = 0; i < s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE]; ++i)
				m_ProjectDisplay->addItem(s_StandardResolutions[i]);
			m_ProjectDisplay->addItem("");
			groupLayout->addWidget(m_ProjectDisplay);
			connect(m_ProjectDisplay, SIGNAL(currentIndexChanged(int)), this, SLOT(projectDisplayChanged(int)));

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		// Flash
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Flash"));
			QHBoxLayout *groupLayout = new QHBoxLayout();
			m_ProjectFlashGroup = group;

			m_ProjectFlash = new QComboBox(this);
			for (int i = 0; i < FTEDITOR_FLASH_NB; ++i)
				m_ProjectFlash->addItem(flashToString(i));
			m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_FLASH);
			groupLayout->addWidget(m_ProjectFlash);
			connect(m_ProjectFlash, SIGNAL(currentIndexChanged(int)), this, SLOT(projectFlashChanged(int)));

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		layout->addStretch();

		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_ProjectDock->setWidget(scrollArea);
		addDockWidget(Qt::RightDockWidgetArea, m_ProjectDock);
		m_WidgetsMenu->addAction(m_ProjectDock->toggleViewAction());

		// Force proper size
		int h = widget->height();
		scrollArea->setMaximumHeight(h);

		// Only allow closable because otherwise it can tab into other docks which will badly resize
		m_ProjectDock->setFeatures(QDockWidget::DockWidgetClosable);
	}

#if FT800_DEVICE_MANAGER
	// Devices
	{
		m_DeviceManagerDock = new QDockWidget(this);
		m_DeviceManagerDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_DeviceManagerDock->setObjectName("Devices");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		m_DeviceManager = new DeviceManager(this);
		scrollArea->setWidget(m_DeviceManager);
		m_DeviceManagerDock->setWidget(scrollArea);
		addDockWidget(Qt::RightDockWidgetArea, m_DeviceManagerDock);
		m_WidgetsMenu->addAction(m_DeviceManagerDock->toggleViewAction());
	}
#endif /* FT800_DEVICE_MANAGER */

	// Cursor position
	{
		m_CursorPosition = new QLabel(statusBar());
		m_CursorPosition->setText("");
		statusBar()->addPermanentWidget(m_CursorPosition);

		/*QFrame *line = new QFrame(statusBar());
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		statusBar()->addPermanentWidget(line);*/

		QLabel *label = new QLabel(statusBar());
		label->setText("  ");
		statusBar()->addPermanentWidget(label);
	}

	// Coprocessor busy
	{
		m_CoprocessorBusy = new QLabel(statusBar());
		QMovie *movie = new QMovie(":/icons/loader.gif");
		m_CoprocessorBusy->setMovie(movie);
		movie->start();
		statusBar()->addPermanentWidget(m_CoprocessorBusy);

		QLabel *label = new QLabel(statusBar());
		label->setText("  ");
		statusBar()->addPermanentWidget(label);
	}

	// Utilization
	{
		/*QWidget *w = new QWidget(statusBar());
		w->setMinimumSize(120, 8);
		w->setMaximumSize(180, 19); // FIXME
		w->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout *l = new QHBoxLayout();
		l->setContentsMargins(0, 0, 0, 0);*/

		QLabel *dlLabelH = new QLabel(statusBar());
		dlLabelH->setText(tr("BITMAP_HANDLE: "));
		statusBar()->addPermanentWidget(dlLabelH);

		m_UtilizationBitmapHandleStatus = new QProgressBar(statusBar());
		m_UtilizationBitmapHandleStatus->setMinimum(0);
		m_UtilizationBitmapHandleStatus->setMaximum(FTED_NUM_HANDLES);
		m_UtilizationBitmapHandleStatus->setMinimumSize(60, 8);
		m_UtilizationBitmapHandleStatus->setMaximumSize(120, 19); // FIXME
		statusBar()->addPermanentWidget(m_UtilizationBitmapHandleStatus);

		QLabel *dlLabel = new QLabel(statusBar());
		dlLabel->setText(tr("RAM_DL: "));
		statusBar()->addPermanentWidget(dlLabel);

		m_UtilizationDisplayListStatus = new QProgressBar(statusBar());
		m_UtilizationDisplayListStatus->setMinimum(0);
		m_UtilizationDisplayListStatus->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE));
		m_UtilizationDisplayListStatus->setMinimumSize(60, 8);
		m_UtilizationDisplayListStatus->setMaximumSize(120, 19); // FIXME
		statusBar()->addPermanentWidget(m_UtilizationDisplayListStatus);

		QLabel *dlLabelG = new QLabel(statusBar());
		dlLabelG->setText(tr("RAM_G: "));
		statusBar()->addPermanentWidget(dlLabelG);

		m_UtilizationGlobalStatus = new QProgressBar(statusBar());
		m_UtilizationGlobalStatus->setMinimum(0);
		m_UtilizationGlobalStatus->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END));
		m_UtilizationGlobalStatus->setMinimumSize(60, 8);
		m_UtilizationGlobalStatus->setMaximumSize(120, 19); // FIXME
		statusBar()->addPermanentWidget(m_UtilizationGlobalStatus);

		/*w->setLayout(l);
		statusBar()->addPermanentWidget(w);*/

		m_UtilizationDock = new QDockWidget(this);
		m_UtilizationDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_UtilizationDock->setObjectName("Utilization");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		QWidget *widget = new QWidget(this);
		QVBoxLayout *layout = new QVBoxLayout();

		// Display List
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Display List"));
			QVBoxLayout *groupLayout = new QVBoxLayout();

			m_UtilizationDisplayList = new QProgressBar(this);
			m_UtilizationDisplayList->setMinimum(0);
			m_UtilizationDisplayList->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE));
			groupLayout->addWidget(m_UtilizationDisplayList);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		layout->addStretch();
		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_UtilizationDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_UtilizationDock);
		// m_WidgetsMenu->addAction(m_UtilizationDock->toggleViewAction()); Disabled for now

		m_UtilizationDock->setVisible(false);
	}

	// PropertiesEditor
	{
		m_PropertiesEditorDock = new QDockWidget(this);
		m_PropertiesEditorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_PropertiesEditorDock->setObjectName("PropertiesEditor");
		m_PropertiesEditorScroll = new QScrollArea(this);
		m_PropertiesEditorScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_PropertiesEditorScroll->setWidgetResizable(true);
		m_PropertiesEditorScroll->setMinimumWidth(240);
		m_PropertiesEditor = new PropertiesEditor(this);
		m_PropertiesEditorScroll->setWidget(m_PropertiesEditor);
		m_PropertiesEditorDock->setWidget(m_PropertiesEditorScroll);
		addDockWidget(Qt::RightDockWidgetArea, m_PropertiesEditorDock);
		m_WidgetsMenu->addAction(m_PropertiesEditorDock->toggleViewAction());
	}

	// Inspector
	{
		m_InspectorDock = new QDockWidget(this);
		m_InspectorDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_InspectorDock->setObjectName("Inspector");
		m_Inspector = new Inspector(this);
		m_InspectorDock->setWidget(m_Inspector);
		addDockWidget(Qt::BottomDockWidgetArea, m_InspectorDock);
		m_WidgetsMenu->addAction(m_InspectorDock->toggleViewAction());
	}

	// DlEditor (Display List)
	{
		m_DlEditorDock = new QDockWidget(this);
		m_DlEditorDock->setAllowedAreas(Qt::BottomDockWidgetArea);
		m_DlEditorDock->setObjectName("DlEditor");
		m_DlEditor = new DlEditor(this, false);
		m_DlEditor->setPropertiesEditor(m_PropertiesEditor);
		m_DlEditor->setUndoStack(m_UndoStack);
		connect(m_EmulatorViewport, SIGNAL(frame()), m_DlEditor, SLOT(frame()));
		m_DlEditorDock->setWidget(m_DlEditor);
		addDockWidget(Qt::BottomDockWidgetArea, m_DlEditorDock);
		m_WidgetsMenu->addAction(m_DlEditorDock->toggleViewAction());

		m_DlEditorDock->setVisible(false);
	}

	// CmdEditor (Coprocessor)
	{
		m_CmdEditorDock = new QDockWidget(this);
		m_CmdEditorDock->setAllowedAreas(Qt::BottomDockWidgetArea);
		m_CmdEditorDock->setObjectName("CmdEditor");
		m_CmdEditor = new DlEditor(this, true);
		m_CmdEditor->setPropertiesEditor(m_PropertiesEditor);
		m_CmdEditor->setUndoStack(m_UndoStack);
		connect(m_EmulatorViewport, SIGNAL(frame()), m_CmdEditor, SLOT(frame()));
		m_CmdEditorDock->setWidget(m_CmdEditor);
		addDockWidget(Qt::BottomDockWidgetArea, m_CmdEditorDock);
		m_WidgetsMenu->addAction(m_CmdEditorDock->toggleViewAction());
	}

	// Controls
	{
		m_ControlsDock = new QDockWidget(this);
		m_ControlsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_ControlsDock->setObjectName("Controls");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		QWidget *widget = new QWidget(this);
		QVBoxLayout *layout = new QVBoxLayout();

		// Step
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Steps"));
			QVBoxLayout *groupLayout = new QVBoxLayout();

			QHBoxLayout *dlhbox = new QHBoxLayout();
			m_StepEnabled = new QCheckBox(this);
			m_StepEnabled->setChecked(false);
			m_StepEnabled->setText("Display List");
			connect(m_StepEnabled, SIGNAL(toggled(bool)), this, SLOT(stepEnabled(bool)));
			dlhbox->addWidget(m_StepEnabled);
			m_StepCount = new QSpinBox(this);
			m_StepCount->setMinimum(1);
			m_StepCount->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE) * 64);
			m_StepCount->setEnabled(false);
			connect(m_StepCount, SIGNAL(valueChanged(int)), this, SLOT(stepChanged(int)));
			dlhbox->addWidget(m_StepCount);
			groupLayout->addLayout(dlhbox);

			QHBoxLayout *cmdhbox = new QHBoxLayout();
			m_StepCmdEnabled = new QCheckBox(this);
			m_StepCmdEnabled->setChecked(false);
			m_StepCmdEnabled->setText("Coprocessor");
			connect(m_StepCmdEnabled, SIGNAL(toggled(bool)), this, SLOT(stepCmdEnabled(bool)));
			cmdhbox->addWidget(m_StepCmdEnabled);
			m_StepCmdCount = new QSpinBox(this);
			m_StepCmdCount->setMinimum(1);
			m_StepCmdCount->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE) * 64);
			m_StepCmdCount->setEnabled(false);
			connect(m_StepCmdCount, SIGNAL(valueChanged(int)), this, SLOT(stepCmdChanged(int)));
			cmdhbox->addWidget(m_StepCmdCount);
			groupLayout->addLayout(cmdhbox);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		// Trace
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Trace"));
			QHBoxLayout *groupLayout = new QHBoxLayout();

			m_TraceEnabled = new QCheckBox(this);
			m_TraceEnabled->setChecked(false);
			connect(m_TraceEnabled, SIGNAL(toggled(bool)), this, SLOT(traceEnabledChanged(bool)));
			groupLayout->addWidget(m_TraceEnabled);
			m_TraceX = new QSpinBox(this);
			m_TraceX->setMinimum(0);
			m_TraceX->setMaximum(screenWidthMaximum(FTEDITOR_CURRENT_DEVICE) - 1);
			m_TraceX->setEnabled(false);
			groupLayout->addWidget(m_TraceX);
			m_TraceY = new QSpinBox(this);
			m_TraceY->setMinimum(0);
			m_TraceY->setMaximum(screenHeightMaximum(FTEDITOR_CURRENT_DEVICE) - 1);
			m_TraceY->setEnabled(false);
			groupLayout->addWidget(m_TraceY);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		layout->addStretch();
		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_ControlsDock->setWidget(scrollArea);
		addDockWidget(Qt::RightDockWidgetArea, m_ControlsDock);
		m_WidgetsMenu->addAction(m_ControlsDock->toggleViewAction());
	}

	// Registers
	{
		m_RegistersDock = new QDockWidget(this);
		m_RegistersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_RegistersDock->setObjectName("Registers");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		QWidget *widget = new QWidget(this);
		QVBoxLayout *layout = new QVBoxLayout();

		// Size
		{
			QGroupBox *sizeGroup = new QGroupBox(widget);
			sizeGroup->setTitle(tr("Display Size"));
			QVBoxLayout *sizeLayout = new QVBoxLayout();

			m_HSize = new QSpinBox(widget);
			m_HSize->setMinimum(1);
			m_HSize->setMaximum(screenWidthMaximum(FTEDITOR_CURRENT_DEVICE));
			connect(m_HSize, SIGNAL(valueChanged(int)), this, SLOT(hsizeChanged(int)));
			QHBoxLayout *hsizeLayout = new QHBoxLayout();
			QLabel *hsizeLabel = new QLabel(widget);
			hsizeLabel->setText(tr("Horizontal"));
			hsizeLayout->addWidget(hsizeLabel);
			hsizeLayout->addWidget(m_HSize);
			sizeLayout->addLayout(hsizeLayout);

			m_VSize = new QSpinBox(widget);
			m_VSize->setMinimum(1);
			m_VSize->setMaximum(screenHeightMaximum(FTEDITOR_CURRENT_DEVICE));
			connect(m_VSize, SIGNAL(valueChanged(int)), this, SLOT(vsizeChanged(int)));
			QHBoxLayout *vsizeLayout = new QHBoxLayout();
			QLabel *vsizeLabel = new QLabel(widget);
			vsizeLabel->setText(tr("Vertical"));
			vsizeLayout->addWidget(vsizeLabel);
			vsizeLayout->addWidget(m_VSize);
			sizeLayout->addLayout(vsizeLayout);

			sizeGroup->setLayout(sizeLayout);
			layout->addWidget(sizeGroup);
		}

		// Rotate
		{
			QLabel *label;
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Rotate (debug mode only)"));
			QVBoxLayout *groupLayout = new QVBoxLayout();
			QHBoxLayout *hboxLayout;

			m_Rotate = new QSpinBox(widget);
			m_Rotate->setMinimum(0);
			m_Rotate->setMaximum(7);
			connect(m_Rotate, SIGNAL(valueChanged(int)), this, SLOT(rotateChanged(int)));
			hboxLayout = new QHBoxLayout();
			label = new QLabel(widget);
			label->setText(tr("Rotate"));
			hboxLayout->addWidget(label);
			hboxLayout->addWidget(m_Rotate);
			groupLayout->addLayout(hboxLayout);

			group->setLayout(groupLayout);
			layout->addWidget(group);

			// NOTE: This widget does not save to project yet
			// NOTE: This widget does not downgrade rotate between device versions

#if !_DEBUG
			group->setVisible(false);
#endif
		}

		// Macro
		{
			QGroupBox *macroGroup = new QGroupBox(widget);
			macroGroup->setTitle(tr("Macro"));
			QVBoxLayout *macroLayout = new QVBoxLayout();

			m_Macro = new DlEditor(this);
			m_Macro->setPropertiesEditor(m_PropertiesEditor);
			m_Macro->setUndoStack(m_UndoStack);
			m_Macro->setModeMacro();
			//QHBoxLayout *macroLayout = new QHBoxLayout();
			//QLabel *macroLabel = new QLabel(widget);
			//macroLabel->setText(tr("Macro"));
			//layout->addWidget(macroLabel);
			macroLayout->addWidget(m_Macro);
			//layout->addLayout(macroLayout);

			macroGroup->setLayout(macroLayout);
			layout->addWidget(macroGroup);
		}

		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_RegistersDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_RegistersDock);
		m_WidgetsMenu->addAction(m_RegistersDock->toggleViewAction());
	}

	// Content
	{
		m_ContentManagerDock = new QDockWidget(this);
		m_ContentManagerDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_ContentManagerDock->setObjectName("ContentManager");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		m_ContentManager = new ContentManager(this);
		s_ContentManager = m_ContentManager;
		scrollArea->setWidget(m_ContentManager);
		m_ContentManagerDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ContentManagerDock);
		m_WidgetsMenu->addAction(m_ContentManagerDock->toggleViewAction());
	}

	// Bitmap
	/*{
		m_BitmapSetupDock = new QDockWidget(this);
		m_BitmapSetupDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_BitmapSetupDock->setObjectName("BitmapSetup");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		m_BitmapSetup = new BitmapSetup(this);
		s_BitmapSetup = m_BitmapSetup;
		scrollArea->setWidget(m_BitmapSetup);
		m_BitmapSetupDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_BitmapSetupDock);
		m_WidgetsMenu->addAction(m_BitmapSetupDock->toggleViewAction());
	}*/

	// Toolbox
	{
		m_ToolboxDock = new QDockWidget(this);
		m_ToolboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_ToolboxDock->setObjectName("Toolbox");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		m_Toolbox = new Toolbox(this);
		scrollArea->setWidget(m_Toolbox);
		m_ToolboxDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ToolboxDock);
		m_WidgetsMenu->addAction(m_ToolboxDock->toggleViewAction());
	}

	tabifyDockWidget(m_InspectorDock, m_DlEditorDock);
	tabifyDockWidget(m_DlEditorDock, m_CmdEditorDock);

	// Event for editor tab change
	/*QTabBar *editorTabbar = NULL;
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		QTabBar *tabBar = tabList.at(i);
		editorTabbar = tabBar;
		connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(editorTabChanged(int)));
	}*/

	tabifyDockWidget(m_RegistersDock, m_ContentManagerDock);
	//tabifyDockWidget(m_ContentManagerDock, m_BitmapSetupDock);
	tabifyDockWidget(m_ContentManagerDock, m_ToolboxDock);

#if FT800_DEVICE_MANAGER
	tabifyDockWidget(m_DeviceManagerDock, m_ControlsDock);
#endif /* FT800_DEVICE_MANAGER */
	tabifyDockWidget(m_ControlsDock, m_UtilizationDock);
	tabifyDockWidget(m_UtilizationDock, m_PropertiesEditorDock);
	
	// Event for all tab changes
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		// printf("tab bar found\n");
		QTabBar *tabBar = tabList.at(i);
		/*if (tabBar != editorTabbar)
		{*/
			//connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
			connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(editorTabChanged(int)));
			connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(editorTabChanged(int))); // this is not working FIXME
		//}

		// FIX-----ME: Figure out and connect when new tab bars are created... done
		m_HookedTabs.push_back(tabBar);
	}

	editorTabChangedGo(true);

	cmdEditor()->codeEditor()->setKeyHandler(m_EmulatorViewport);
	dlEditor()->codeEditor()->setKeyHandler(m_EmulatorViewport);
	cmdEditor()->codeEditor()->setKeyHandler(NULL);
	dlEditor()->codeEditor()->setKeyHandler(NULL);
}

void MainWindow::translateDockWindows()
{
	m_InspectorDock->setWindowTitle(tr("Inspector"));
	m_DlEditorDock->setWindowTitle(tr("Display List"));
	m_CmdEditorDock->setWindowTitle(tr("Coprocessor"));
	m_ProjectDock->setWindowTitle(tr("Project"));
#if FT800_DEVICE_MANAGER
	m_DeviceManagerDock->setWindowTitle(tr("Devices"));
#endif /* FT800_DEVICE_MANAGER */
	m_UtilizationDock->setWindowTitle(tr("Utilization"));
	m_NavigatorDock->setWindowTitle(tr("Navigator"));
	m_PropertiesEditorDock->setWindowTitle(tr("Properties"));
	m_ToolboxDock->setWindowTitle(tr("Toolbox"));
	m_ContentManagerDock->setWindowTitle(tr("Content"));
	m_RegistersDock->setWindowTitle(tr("Registers"));
	m_ControlsDock->setWindowTitle(tr("Controls"));
	// m_BitmapSetupDock->setWindowTitle(tr("Bitmaps"));
	// m_BitmapSetupDock->setWindowTitle(tr("Handles"));
}

void MainWindow::incbLanguageCode()
{
	setWindowTitle(tr("EVE Screen Editor"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
}

static QIcon processIcon(QTabBar *tabBar, QIcon icon)
{
	if (tabBar->shape() == QTabBar::RoundedEast
		|| tabBar->shape() == QTabBar::RoundedWest)
	{
		QPixmap pix = icon.pixmap(16, 16);
		QTransform trans;
		if (tabBar->shape() == QTabBar::RoundedEast)
			trans.rotate(-90);
		else if (tabBar->shape() == QTabBar::RoundedWest)
			trans.rotate(+90);
		pix = pix.transformed(trans);
		return QIcon(pix);
	}
	return icon;
}

void MainWindow::editorTabChanged(int i)
{
	editorTabChangedGo(false);
}

void MainWindow::editorTabChangedGo(bool load)
{
	//printf("blip\n");
	//m_EmulatorViewport->unsetEditorLine();
	//m_Toolbox->unsetEditorLine();
	bool cmdExist = false;
	bool dlExist = false;
	bool cmdTop = true;
	bool dlTop = true;
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		QTabBar *tabBar = tabList.at(i);
		if (!load)
		{
			if (std::find(m_HookedTabs.begin(), m_HookedTabs.end(), tabBar) == m_HookedTabs.end())
			{
				connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(editorTabChanged(int)));
				connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(editorTabChanged(int))); // this is not working FIXME
				m_HookedTabs.push_back(tabBar);
			}
		}
		for (int j = 0; j < tabBar->count(); ++j)
		{
			QDockWidget *dw = reinterpret_cast<QDockWidget *>(qvariant_cast<quintptr>(tabBar->tabData(j)));
			if (dw == m_CmdEditorDock)
			{
				cmdExist = true;
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/script-text.png")));
				if (tabBar->currentIndex() != j)
					cmdTop = false;
			}
			else if (dw == m_DlEditorDock)
			{
				dlExist = true;
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/script-text.png")));
				if (tabBar->currentIndex() != j)
					dlTop = false;
			}
			else if (dw == m_PropertiesEditorDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/property.png")));
			}
			else if (dw == m_ContentManagerDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/photo-album-blue.png")));
			}
			else if (dw == m_ToolboxDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/wrench-screwdriver.png")));
			}
			else if (dw == m_InspectorDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/node-magnifier.png")));
			}
#if FT800_DEVICE_MANAGER
			else if (dw == m_DeviceManagerDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/game-monitor.png")));
			}
#endif /* FT800_DEVICE_MANAGER */
			else if (dw == m_RegistersDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/cheque.png")));
			}
			else if (dw == m_ControlsDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/switch.png")));
			}
		}
	}
	if (!cmdExist) cmdTop = false;
	if (!dlExist) dlTop = false;
	// printf("x: %i,y: %i\n", cmdTop, dlTop);
	if (cmdTop != dlTop)
	{
		if (cmdTop)
		{
			// printf("focus cmd\n");
			m_CmdEditor->codeEditor()->setFocus(Qt::OtherFocusReason);
		}
		else
		{
			// printf("focus dl\n");
			m_DlEditor->codeEditor()->setFocus(Qt::OtherFocusReason);
		}
	}
	else if (!cmdTop)
	{
		// m_EmulatorViewport->unsetEditorLine();
		// m_Toolbox->unsetEditorLine();
		setFocus(Qt::OtherFocusReason);
	}
}

void MainWindow::tabChanged(int i)
{
	// Defocus editor
	// setFocus(Qt::OtherFocusReason);
}

void MainWindow::focusDlEditor(bool forceOnly)
{
	if (forceOnly)
	{
		m_DlEditorDock->setVisible(true);
		m_CmdEditorDock->setVisible(false);
	}
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		QTabBar *tabBar = tabList.at(i);
		for (int j = 0; j < tabBar->count(); ++j)
		{
			QDockWidget *dw = reinterpret_cast<QDockWidget *>(qvariant_cast<quintptr>(tabBar->tabData(j)));
			if (dw == m_DlEditorDock)
			{
				tabBar->setCurrentIndex(j);
				goto Out;
			}
		}
	}
Out:
	m_DlEditor->codeEditor()->setFocus(Qt::OtherFocusReason);
}
void MainWindow::focusCmdEditor()
{
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		QTabBar *tabBar = tabList.at(i);
		for (int j = 0; j < tabBar->count(); ++j)
		{
			QDockWidget *dw = reinterpret_cast<QDockWidget *>(qvariant_cast<quintptr>(tabBar->tabData(j)));
			if (dw == m_CmdEditorDock)
			{
				tabBar->setCurrentIndex(j);
				goto Out;
			}
		}
	}
Out:
	m_CmdEditor->codeEditor()->setFocus(Qt::OtherFocusReason);
}

void MainWindow::focusProperties()
{
	QList<QTabBar *> tabList = findChildren<QTabBar *>();
	for (int i = 0; i < tabList.size(); ++i)
	{
		QTabBar *tabBar = tabList.at(i);
		for (int j = 0; j < tabBar->count(); ++j)
		{
			QDockWidget *dw = reinterpret_cast<QDockWidget *>(qvariant_cast<quintptr>(tabBar->tabData(j)));
			if (dw == m_PropertiesEditorDock)
			{
				tabBar->setCurrentIndex(j);
				return;
			}
		}
	}
}

static bool s_UndoRedoWorking = false;

class HSizeCommand : public QUndoCommand
{
public:
	HSizeCommand(int hsize, QSpinBox *spinbox) : QUndoCommand(), m_NewHSize(hsize), m_OldHSize(s_HSize), m_SpinBox(spinbox) { }
	virtual ~HSizeCommand() { }
	virtual void undo() { s_HSize = m_OldHSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_HSize); s_UndoRedoWorking = false; }
	virtual void redo() { s_HSize = m_NewHSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_HSize); s_UndoRedoWorking = false; }
	virtual int id() const { printf("id get\n"); return 41517686; }
	virtual bool mergeWith(const QUndoCommand *command) { m_NewHSize = static_cast<const HSizeCommand *>(command)->m_NewHSize; return true; }

private:
	int m_NewHSize;
	int m_OldHSize;
	QSpinBox *m_SpinBox;

};

class VSizeCommand : public QUndoCommand
{
public:
	VSizeCommand(int hsize, QSpinBox *spinbox) : QUndoCommand(), m_NewVSize(hsize), m_OldVSize(s_VSize), m_SpinBox(spinbox) { }
	virtual ~VSizeCommand() { }
	virtual void undo() { s_VSize = m_OldVSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_VSize); s_UndoRedoWorking = false; }
	virtual void redo() { s_VSize = m_NewVSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_VSize); s_UndoRedoWorking = false; }
	virtual int id() const { return 78984351; }
	virtual bool mergeWith(const QUndoCommand *command) { m_NewVSize = static_cast<const VSizeCommand *>(command)->m_NewVSize; return true; }

private:
	int m_NewVSize;
	int m_OldVSize;
	QSpinBox *m_SpinBox;

};

void MainWindow::userChangeResolution(int hsize, int vsize)
{
	m_UndoStack->beginMacro("Change resolution");
	m_HSize->setValue(hsize);
	m_VSize->setValue(vsize);
	m_UndoStack->endMacro();
}

void MainWindow::hsizeChanged(int hsize)
{
	updateProjectDisplay(hsize, m_VSize->value());
	
	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new HSizeCommand(hsize, m_HSize));
}

void MainWindow::vsizeChanged(int vsize)
{
	updateProjectDisplay(m_HSize->value(), vsize);
	
	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new VSizeCommand(vsize, m_VSize));
}

class RotateCommand : public QUndoCommand
{
public:
	RotateCommand(int rotate, QSpinBox *spinbox) : QUndoCommand(), m_NewRotate(rotate), m_OldRotate(s_Rotate), m_SpinBox(spinbox) { }
	virtual ~RotateCommand() { }
	virtual void undo() { s_Rotate = m_OldRotate; s_UndoRedoWorking = true; m_SpinBox->setValue(s_Rotate); s_UndoRedoWorking = false; }
	virtual void redo() { s_Rotate = m_NewRotate; s_UndoRedoWorking = true; m_SpinBox->setValue(s_Rotate); s_UndoRedoWorking = false; }
	virtual int id() const { return 78994352; }
	virtual bool mergeWith(const QUndoCommand *command) { m_NewRotate = static_cast<const RotateCommand *>(command)->m_NewRotate; return true; }

private:
	int m_NewRotate;
	int m_OldRotate;
	QSpinBox *m_SpinBox;

};

void MainWindow::rotateChanged(int rotate)
{
	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new RotateCommand(rotate, m_Rotate));
}

void MainWindow::updateProjectDisplay(int hsize, int vsize)
{
	m_ProjectDisplay->setItemText(s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE], tr("Custom") + " (" + QString::number(hsize) + "x" + QString::number(vsize) + ")");
	for (int i = 0; i < s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE]; ++i)
	{
		if (s_StandardWidths[i] == hsize && s_StandardHeights[i] == vsize)
		{
			m_ProjectDisplay->setCurrentIndex(i);
			return;
		}
	}
	m_ProjectDisplay->setCurrentIndex(s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE]);
	return;
}

void MainWindow::stepEnabled(bool enabled)
{
	m_StepCount->setEnabled(enabled);
	if (enabled)
	{
		m_StepCmdEnabled->setChecked(false);
		BT8XXEMU_setDebugLimiter(g_Emulator, m_StepCount->value());
		BT8XXEMU_poke(g_Emulator);
	}
	else
	{
		BT8XXEMU_setDebugLimiter(g_Emulator, 2048 * 64);
		BT8XXEMU_poke(g_Emulator);
	}
}

void MainWindow::stepChanged(int step)
{
	if (m_StepEnabled->isChecked())
	{
		BT8XXEMU_setDebugLimiter(g_Emulator, step);
		BT8XXEMU_poke(g_Emulator);
	}
}

void MainWindow::stepCmdEnabled(bool enabled)
{
	m_StepCmdCount->setEnabled(enabled);
	if (enabled)
	{
		m_StepEnabled->setChecked(false);
		g_StepCmdLimit = m_StepCmdCount->value();
	}
	else
	{
		g_StepCmdLimit = 0;
	}
}

void MainWindow::stepCmdChanged(int step)
{
	if (m_StepCmdEnabled->isChecked())
	{
		g_StepCmdLimit = m_StepCmdCount->value();
	}
}

void MainWindow::setTraceEnabled(bool enabled)
{
	m_TraceEnabled->setChecked(enabled);
}

void MainWindow::setTraceX(int x)
{
	m_TraceX->setValue(x);
}

void MainWindow::setTraceY(int y)
{
	m_TraceY->setValue(y);
}

bool MainWindow::traceEnabled()
{
	return m_TraceEnabled->isChecked();
}

int MainWindow::traceX()
{
	return m_TraceX->value();
}

int MainWindow::traceY()
{
	return m_TraceY->value();
}

void MainWindow::traceEnabledChanged(bool enabled)
{
	m_TraceX->setEnabled(enabled);
	m_TraceY->setEnabled(enabled);
}

#define FTEDITOR_INITIAL_HELP tr("Start typing in the <b>Coprocessor</b> editor, or drag and drop items from the <b>Toolbox</b> onto the display viewport.")

void MainWindow::clearEditor()
{
	m_ProjectDevice->setCurrentIndex(FTEDITOR_DEFAULT_DEVICE);
	m_HSize->setValue(screenWidthDefault(FTEDITOR_CURRENT_DEVICE));
	m_VSize->setValue(screenHeightDefault(FTEDITOR_CURRENT_DEVICE));
	m_StepEnabled->setChecked(false);
	m_StepCount->setValue(1);
	m_StepCmdEnabled->setChecked(false);
	m_StepCmdCount->setValue(1);
	setTraceEnabled(false);
	setTraceX(0);
	setTraceY(0);
	m_DlEditor->clear();
	m_CmdEditor->clear();
	m_Macro->clear();
	m_ContentManager->clear();
	//m_BitmapSetup->clear();
	m_ProjectDock->setVisible(true);
}

void MainWindow::clearUndoStack()
{
	m_UndoStack->clear();
	m_DlEditor->clearUndoStack();
	m_CmdEditor->clearUndoStack();
	m_Macro->clearUndoStack();
}

void MainWindow::updateWindowTitle()
{
	setWindowTitle(QString(m_CleanUndoStack ? "" : "*") + (m_CurrentFile.isEmpty() ? "New Project" : QFileInfo(m_CurrentFile).completeBaseName()) + " - " + tr("EVE Screen Editor") + " - (" + QDir::currentPath() + ")");
}

void MainWindow::undoCleanChanged(bool clean)
{
	m_CleanUndoStack = clean;
	updateWindowTitle();
}

bool MainWindow::maybeSave()
{
	if (!m_UndoStack->isClean())
	{
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("EVE Screen Editor"),
			tr("The project has been modified.\n"
				"Do you want to save your changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
		{
			actSave();
			return true;
		}
		else if (ret == QMessageBox::Cancel)
		{
			return false;
		}
	}
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) event->accept();
	else event->ignore();
}

void MainWindow::actNew()
{
	actNew(true);
}

void MainWindow::actNew(bool addClear)
{
	if (!maybeSave()) return;

	printf("** New **\n");

	// reset filename
	m_CurrentFile = QString();

	// reset editors to their default state
	clearEditor();

	// add clear
	int editLine;
	if (addClear)
	{
		DlParsed pa;
		pa.ValidId = true;
		pa.IdLeft = 0;
		pa.IdRight = FTEDITOR_DL_CLEAR;
		pa.ExpectedStringParameter = false;
		pa.Parameter[0].U = 1;
		pa.Parameter[1].U = 1;
		pa.Parameter[2].U = 1;
		pa.ExpectedParameterCount = 3;
		m_CmdEditor->insertLine(0, pa);
		editLine = 1;
	}
	else
	{
		editLine = 0;
	}

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusCmdEditor();
	m_PropertiesEditor->setInfo(FTEDITOR_INITIAL_HELP);
	m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	m_Toolbox->setEditorLine(m_CmdEditor, editLine);
	m_CmdEditor->selectLine(editLine);

	// set working directory to temporary directory
#ifdef FTEDITOR_TEMP_DIR
	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = new QTemporaryDir("ft800editor-");
	QDir::setCurrent(m_TemporaryDir->path());
#else
	QDir::setCurrent(m_InitialWorkingDir);
#endif
	updateWindowTitle();
	printf("Current path: %s\n", QDir::currentPath().toLocal8Bit().data());
}

void documentFromJsonArray(QPlainTextEdit *textEditor, const QJsonArray &arr)
{
	bool firstLine = true;
	for (int i = 0; i < arr.size(); ++i)
	{
		if (firstLine) firstLine = false;
		else textEditor->textCursor().insertText("\n");
		textEditor->textCursor().insertText(arr[i].toString());
	}
}

static void bitmapSetupfromJson(MainWindow *mainWindow, DlEditor *dlEditor, QJsonArray &bitmaps)
{
	// Compatibility code updating from an old version of ft800proj format, does not need update for new features

	int hline = 0;

	for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
	{
		QJsonObject j = bitmaps[i].toObject();

		// Source
		ContentInfo *contentInfo = mainWindow->contentManager()->find(j["sourceContent"].toString());
		bool exists = (contentInfo != NULL);

		if (exists)
		{
			// printf("exist %i\n", i);
			DlParsed pa;
			pa.ValidId = true;
			pa.IdLeft = 0;

			pa.IdRight = FTEDITOR_DL_BITMAP_HANDLE;
			pa.Parameter[0].I = i;
			pa.ExpectedParameterCount = 1;
			dlEditor->insertLine(hline, pa);
			++hline;

			pa.IdRight = FTEDITOR_DL_BITMAP_SOURCE;
			pa.Parameter[0].U = contentInfo->MemoryAddress;
			pa.ExpectedParameterCount = 1;
			dlEditor->insertLine(hline, pa);
			++hline;

			pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT;
			if (contentInfo->Converter == ContentInfo::Image)
			{
				pa.Parameter[0].U = contentInfo->ImageFormat;
				pa.Parameter[1].U = contentInfo->CachedImageStride;
				pa.Parameter[2].U = contentInfo->CachedImageHeight;
			}
			else
			{
				pa.Parameter[0].U = ((QJsonValue)j["layoutFormat"]).toVariant().toInt();
				pa.Parameter[1].U = ((QJsonValue)j["layoutStride"]).toVariant().toInt();
				pa.Parameter[2].U = ((QJsonValue)j["layoutHeight"]).toVariant().toInt();
			}
			pa.ExpectedParameterCount = 3;
			dlEditor->insertLine(hline, pa);
			++hline;

			pa.IdRight = FTEDITOR_DL_BITMAP_SIZE;
			pa.Parameter[0].U = ((QJsonValue)j["sizeFilter"]).toVariant().toInt();
			pa.Parameter[1].U = ((QJsonValue)j["sizeWrapX"]).toVariant().toInt();
			pa.Parameter[2].U = ((QJsonValue)j["sizeWrapY"]).toVariant().toInt();
			pa.Parameter[3].U = ((QJsonValue)j["sizeWidth"]).toVariant().toInt();
			pa.Parameter[4].U = ((QJsonValue)j["sizeHeight"]).toVariant().toInt();
			pa.ExpectedParameterCount = 5;
			dlEditor->insertLine(hline, pa);
			++hline;
		}
	}
}

void postProcessEditor(DlEditor *editor)
{
	DlParsed pa;
	for (int i = 0; i < editor->getLineCount(); ++i)
	{
		const DlParsed &parsed = editor->getLine(i);
		if (parsed.ValidId)
		{
			switch (parsed.IdLeft)
			{
			case FTEDITOR_CO_COMMAND:
				switch (parsed.IdRight | FTEDITOR_CO_COMMAND)
				{
				case CMD_BGCOLOR:
				case CMD_FGCOLOR: 
				case CMD_GRADCOLOR: {
					DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa, editor->getLineText(i), editor->isCoprocessor(), true);
					if (pa.ExpectedParameterCount == 3) // Old RGB, upgrade
					{
						uint32_t rgb =
							pa.Parameter[0].U << 16
							| pa.Parameter[1].U << 8
							| pa.Parameter[2].U;
						pa.Parameter[0].U = rgb;
						pa.ExpectedParameterCount = 1;
						editor->replaceLine(i, pa);
					}
					} break;
				case CMD_GRADIENT: {
					DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa, editor->getLineText(i), editor->isCoprocessor(), true);
					if (pa.ExpectedParameterCount == 10) // Old RGB, upgrade
					{
						uint32_t rgb0 =
							pa.Parameter[2].U << 16
							| pa.Parameter[3].U << 8
							| pa.Parameter[4].U;
						pa.Parameter[2].U = rgb0;
						pa.Parameter[3].U = pa.Parameter[5].U;
						pa.Parameter[4].U = pa.Parameter[6].U;
						uint32_t rgb1 =
							pa.Parameter[7].U << 16
							| pa.Parameter[8].U << 8
							| pa.Parameter[9].U;
						pa.Parameter[5].U = rgb1;
						pa.ExpectedParameterCount = 6;
						editor->replaceLine(i, pa);
					}
					} break;
				}
				break;
			}
		}
	}
}

QString MainWindow::getFileDialogPath()
{
	if (m_LastProjectDir.isEmpty())
	{
		return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	}
	else
	{
		QDir dir(m_LastProjectDir);
		return dir.path();
	}
	// return m_TemporaryDir ? m_InitialWorkingDir : QDir::currentPath();
}

void MainWindow::actOpen()
{
	if (!maybeSave()) return;

	printf("*** Open ***\n");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), getFileDialogPath(),
		tr("EVE Screen Editor Project (*.ft800proj  *.ft8xxproj)"));
	if (fileName.isNull())
		return;

	openFile(fileName);
}


void MainWindow::openFile(const QString &fileName)
{
	printf("*** Open file ***\n");

	// Reset editors to their default state
	clearEditor();

	// Remove temporary paths
	if (m_TemporaryDir)
	{
		QDir::setCurrent(QDir::tempPath());
		delete m_TemporaryDir; m_TemporaryDir = NULL;
	}

	// Set current project path
	m_CurrentFile = fileName;
	QDir dir(fileName);
	dir.cdUp();
	QString dstPath = dir.path();
	QDir::setCurrent(dstPath);
	m_LastProjectDir = QDir::currentPath();

	// Load the data
	bool loadOk = false;
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QByteArray data = file.readAll();
	file.close();
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
	if (parseError.error == QJsonParseError::NoError)
	{
		QJsonObject root = doc.object();
		if (root.contains("project"))
		{
			QJsonObject project = root["project"].toObject();
			m_ProjectDevice->setCurrentIndex(deviceToIntf((BT8XXEMU_EmulatorMode)((QJsonValue)project["device"]).toVariant().toInt()));
		}
		else
		{
			m_ProjectDevice->setCurrentIndex(FTEDITOR_FT801);
		}
		QJsonObject registers = root["registers"].toObject();
		m_HSize->setValue(((QJsonValue)registers["hSize"]).toVariant().toInt());
		m_VSize->setValue(((QJsonValue)registers["vSize"]).toVariant().toInt());
		documentFromJsonArray(m_Macro->codeEditor(), registers["macro"].toArray());
		documentFromJsonArray(m_DlEditor->codeEditor(), root["displayList"].toArray());
		documentFromJsonArray(m_CmdEditor->codeEditor(), root["coprocessor"].toArray());
		QJsonArray content = root["content"].toArray();
		for (int i = 0; i < content.size(); ++i)
		{
			ContentInfo *ci = new ContentInfo("");
			QJsonObject cio = content[i].toObject();
			ci->fromJson(cio, false);
			m_ContentManager->add(ci);
		}
		if (root.contains("bitmaps") || root.contains("handles"))
		{
			// Compatibility loading BITMAP_SETUP
			QJsonArray bitmaps = root[root.contains("bitmaps") ? "bitmaps" : "handles"].toArray();
			bitmapSetupfromJson(this, m_CmdEditor, bitmaps);
			// m_BitmapSetup->fromJson(bitmaps);
		}
		postProcessEditor(m_Macro);
		postProcessEditor(m_DlEditor);
		postProcessEditor(m_CmdEditor);
		statusBar()->showMessage(tr("Opened EVE Screen Editor project"));
		loadOk = true;
	}
	else
	{
		statusBar()->showMessage(parseError.errorString());
	}

	// Fallback
	if (!loadOk)
	{
		// Reset editor and paths
		clearEditor();
		m_CurrentFile = QString();
#ifdef FTEDITOR_TEMP_DIR
		QDir::setCurrent(QDir::tempPath());
		m_TemporaryDir = new QTemporaryDir("ft800editor-");
		QDir::setCurrent(m_TemporaryDir->path());
#else
		QDir::setCurrent(m_InitialWorkingDir);
#endif
		m_LastProjectDir = QDir::currentPath();
	}

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusCmdEditor();
	m_PropertiesEditor->setInfo(FTEDITOR_INITIAL_HELP);
	m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	m_Toolbox->setEditorLine(m_CmdEditor, m_CmdEditor->getLineCount() - 1);
	m_CmdEditor->selectLine(m_CmdEditor->getLineCount() - 1);
	printf("Current path: %s\n", QDir::currentPath().toLocal8Bit().data());
}

QJsonArray documentToJsonArray(const QTextDocument *textDocument, bool coprocessor, bool exportScript)
{
	QJsonArray result;
	for (int i = 0; i < textDocument->blockCount(); ++i)
	{
		QString line = textDocument->findBlockByNumber(i).text();
		if (exportScript)
		{
			DlParsed parsed;
			DlParser::parse(FTEDITOR_CURRENT_DEVICE, parsed, line, coprocessor);
			if (!parsed.ValidId) line = "";
			else line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
		}
		result.push_back(line);
	}
	return result;
}

QByteArray MainWindow::toJson(bool exportScript)
{
	QJsonObject root;
	QJsonObject project;
	project["device"] = (int)deviceToEnum(FTEDITOR_CURRENT_DEVICE);
	root["project"] = project;
	QJsonObject registers;
	registers["hSize"] = s_HSize;
	registers["vSize"] = s_VSize;
	registers["macro"] = documentToJsonArray(m_Macro->codeEditor()->document(), false, exportScript);
	root["registers"] = registers;
	root["displayList"] = documentToJsonArray(m_DlEditor->codeEditor()->document(), false, exportScript);
	root["coprocessor"] = documentToJsonArray(m_CmdEditor->codeEditor()->document(), true, exportScript);
	QJsonArray content;
	std::vector<ContentInfo *> contentInfos;
	m_ContentManager->getContentInfos(contentInfos);
	for (std::vector<ContentInfo *>::iterator it(contentInfos.begin()), end(contentInfos.end()); it != end; ++it)
	{
		ContentInfo *info = (*it);
		content.push_back(info->toJson(false));
	}
	root["content"] = content;
	//root["handles"] = m_BitmapSetup->toJson(exportScript);
	/*if (exportScript)
	{
		// dump of ram... this is too heavy
		QJsonArray dump;
		char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam()));
		for (int i = 0; i < addressSpace(FTEDITOR_CURRENT_DEVICE); ++i)
			dump.push_back((int)ram[i]);
		root["dump"] = dump;
	}*/
	QJsonDocument doc(root);

	return doc.toJson();
}

void MainWindow::actSave()
{
	if (m_CurrentFile.isEmpty()) { actSaveAs(); return; }

	QFile file(m_CurrentFile);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	QByteArray data = toJson();

	out.writeRawData(data, data.size());

	m_UndoStack->setClean();
}

void MainWindow::actSaveAs()
{
	QString filterft8xxproj = tr("ESE Project (*.ft8xxproj)");

	QString filter = filterft8xxproj;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project"), getFileDialogPath(), filter, &filter);
	if (fileName.isNull())
		return;


	if (filter == filterft8xxproj)
	{
		if (!fileName.endsWith(".ft8xxproj"))
			fileName = fileName + ".ft8xxproj";
	}

	// Copy asset files, abort if already exists (check first)
	QDir dir(fileName);
	dir.cdUp();
	QString dstPath = dir.path();
	QString srcPath = QDir::currentPath();
	bool wantRebuildAll = false;
	if (dstPath != srcPath)
	{
		printf("From: %s\n", srcPath.toLocal8Bit().data());
		printf("To: %s\n", dstPath.toLocal8Bit().data());
		m_ContentManager->lockContent();
		// Copy assets from srcPath to dstPath
		std::vector<ContentInfo *> contentInfos;
		m_ContentManager->getContentInfos(contentInfos);
		const std::vector<QString> &fileExt = ContentManager::getFileExtensions();
		// printf("Number: %i\n", (int)contentInfos.size());
		for (std::vector<ContentInfo *>::iterator it1(contentInfos.begin()), end1(contentInfos.end()); it1 != end1; ++it1)
		{
			ContentInfo *info = (*it1);
			// Create destination directory
			if (info->DestName.contains('/'))
			{
				QString destDir = info->DestName.left(info->DestName.lastIndexOf('/'));
				if (!QDir(dstPath).mkpath(destDir))
				{
					// Will fail at copy, and a rebuild will happen which will also fail with mkpath failure
					printf("Unable to create destination path\n");
				}
			}
			// Copy related files
			// printf("Content: %s\n", info->DestName.toUtf8().data());
			for (std::vector<QString>::const_iterator it2(fileExt.begin()), end2(fileExt.end()); it2 != end2; ++it2)
			{
				QString src = srcPath + "/" + info->DestName + (*it2);
				QString dst = dstPath + "/" + info->DestName + (*it2);
				// printf("Move from '%s' to '%s'", src.toUtf8().data(), dst.toUtf8().data());
				QFile::copy(src, dst);
			}
		}
		m_ContentManager->unlockContent();
		wantRebuildAll = true;
	}

	if (m_TemporaryDir)
	{
		// Delete temporary directory
		QDir::setCurrent(QDir::tempPath());
		delete m_TemporaryDir; m_TemporaryDir = NULL;
	}

	// Set the folder to be the project folder
	QDir::setCurrent(dstPath);

	// Rebuild content to ensure correct functionality
	if (wantRebuildAll)
	{
		m_ContentManager->rebuildAll();
	}

	// Save the project itself
	m_CurrentFile = fileName;
	actSave();

	// Update window title in case project folder changed
	updateWindowTitle();
}

void MainWindow::actImport()
{
	if (!maybeSave()) return;

	printf("*** Import ***\n");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), getFileDialogPath(),
		tr("Memory dump, *.vc1dump (*.vc1dump)"));
	if (fileName.isNull())
		return;

	m_CurrentFile = QString();

	// reset editors to their default state
	clearEditor();

	// set working directory to temporary directory
#ifdef FTEDITOR_TEMP_DIR
	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = new QTemporaryDir("ft800editor-");
	QDir::setCurrent(m_TemporaryDir->path());
#else
	QDir::setCurrent(m_InitialWorkingDir);
#endif
	printf("Current path: %s\n", QDir::currentPath().toLocal8Bit().data());

	// open a project
	// http://qt-project.org/doc/qt-5.0/qtcore/qdatastream.html
	// int QDataStream::readRawData(char * s, int len)
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	bool loadOk = false;
	if (true) // todo: if .vc1dump
	{
		const size_t headersz = 6;
		uint32_t header[headersz];
		int s = in.readRawData(static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz);
		if (s != sizeof(uint32_t) * headersz)
		{
			QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete header"));
		}
		else
		{
			if (header[0] == 100)
			{
				m_ProjectDevice->setCurrentIndex(FTEDITOR_FT800);
				m_HSize->setValue(header[1]);
				m_VSize->setValue(header[2]);
				m_Macro->lockDisplayList();
				m_Macro->getDisplayList()[0] = header[3];
				m_Macro->getDisplayList()[1] = header[4];
				m_Macro->reloadDisplayList(false);
				m_Macro->unlockDisplayList();
				char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
				ContentInfo *ramG = m_ContentManager->add(fileName);
				m_ContentManager->changeConverter(ramG, ContentInfo::Raw);
				m_ContentManager->changeMemoryAddress(ramG, 0);
				m_ContentManager->changeMemoryLoaded(ramG, true);
				m_ContentManager->changeRawStart(ramG, sizeof(uint32_t) * headersz);
				m_ContentManager->changeRawLength(ramG, 262144);
				// s = in.skipRawData(262144);
				s = in.readRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], 262144);
				if (s != 262144) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_G"));
				else
				{
					ramaddr ramPal = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL);
					if (ramPal < 0) ramPal = 262144;
					s = in.readRawData(&ram[ramPal], 1024); // FIXME_GUI PALETTE
					if (s != 1024) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_PAL"));
					else
					{
						m_DlEditor->lockDisplayList();
						s = in.readRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FTEDITOR_DL_SIZE * sizeof(uint32_t));
						m_DlEditor->reloadDisplayList(false);
						m_DlEditor->unlockDisplayList();
						if (s != 8192) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_DL"));
						else
						{
							/*
							// FIXME: How is the CRC32 for the .vc1dump calculated?
							uint32_t crc;
							crc = Crc32_ComputeBuf(0, &ram[RAM_G], 262144);
							crc = Crc32_ComputeBuf(crc, &ram[RAM_PAL], 1024);
							crc = Crc32_ComputeBuf(crc, m_DlEditor->getDisplayList(), 8192);
							if (crc != header[5])
							{
								QString message;
								message.sprintf(tr("CRC32 mismatch, %u, %u").toUtf8().constData(), header[5], crc);
								QMessageBox::critical(this, tr("Import .vc1dump"), message);
							}
							*/
							loadOk = true;
							statusBar()->showMessage(tr("Imported project from .vc1dump file"));
							focusDlEditor(true);
						}
					}
				}
			}
			else if (header[0] == 110)
			{
				m_ProjectDevice->setCurrentIndex(FTEDITOR_BT815);
				m_HSize->setValue(header[1]);
				m_VSize->setValue(header[2]);
				m_Macro->lockDisplayList();
				m_Macro->getDisplayList()[0] = header[3];
				m_Macro->getDisplayList()[1] = header[4];
				m_Macro->reloadDisplayList(false);
				m_Macro->unlockDisplayList();
				char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
				ContentInfo *ramG = m_ContentManager->add(fileName);
				m_ContentManager->changeConverter(ramG, ContentInfo::Raw);
				m_ContentManager->changeMemoryAddress(ramG, 0);
				m_ContentManager->changeMemoryLoaded(ramG, true);
				m_ContentManager->changeRawStart(ramG, sizeof(uint32_t) * headersz);
				m_ContentManager->changeRawLength(ramG, 1048576);
				// s = in.skipRawData(1048576);
				s = in.readRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], 1048576);
				if (s != 1048576) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_G"));
				else
				{
					m_DlEditor->lockDisplayList();
					s = in.readRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FTEDITOR_DL_SIZE * sizeof(uint32_t));
					m_DlEditor->reloadDisplayList(false);
					m_DlEditor->unlockDisplayList();
					if (s != 8192) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_DL"));
					else
					{
						loadOk = true;
						statusBar()->showMessage(tr("Imported project from .vc1dump file"));
						focusDlEditor(true);
					}
				}
			}
			else
			{
				QString message;
				message.sprintf(tr("Invalid header version: %i").toUtf8().constData(), header[0]);
				QMessageBox::critical(this, tr("Import .vc1dump"), message);
			}
		}
	}

	if (!loadOk)
	{
		clearEditor();
	}

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusDlEditor();
	m_PropertiesEditor->setInfo(tr("Imported project from .vc1dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);
	m_Toolbox->setEditorLine(m_DlEditor, 0);
}

void MainWindow::actExport()
{
	QString filtervc1dump = tr("Memory dump, *.vc1dump (*.vc1dump)");
	QString filter = filtervc1dump;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"), getFileDialogPath(), filter, &filter);
	if (fileName.isNull())
		return;

	if (filter == filtervc1dump)
	{
		if (!fileName.endsWith(".vc1dump"))
			fileName = fileName + ".vc1dump";
	}

	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	if (true) // todo: if .vc1dump
	{
		const size_t headersz = 6;
		uint32_t header[headersz];
		header[0] = 100;
		header[1] = s_HSize;
		header[2] = s_VSize;
		m_Macro->lockDisplayList();
		header[3] = m_Macro->getDisplayList()[0];
		header[4] = m_Macro->getDisplayList()[1];
		m_Macro->unlockDisplayList();
		header[5] = 0; // FIXME: CRC32
		char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
		int s = out.writeRawData(static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz);
		if (s != sizeof(uint32_t) * headersz) goto ExportWriteError;
		s = out.writeRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], 262144); // FIXME_GUI GLOBAL MEMORY
		if (s != 262144) goto ExportWriteError;
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) // FIXME_FT810
			s = out.writeRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], 1024); // WRITE INVALID DUMMY DATA // FIXME_GUI PALETTE
		else
			s = out.writeRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL)], 1024); // FIXME_GUI PALETTE
		if (s != 1024) goto ExportWriteError;
		m_DlEditor->lockDisplayList();
		// s = out.writeRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FTEDITOR_DL_SIZE * sizeof(uint32_t));
		s = out.writeRawData(static_cast<const char *>(static_cast<const void *>(BT8XXEMU_getDisplayList(g_Emulator))), FTEDITOR_DL_SIZE * sizeof(uint32_t));
		m_DlEditor->unlockDisplayList();
		if (s != FTEDITOR_DL_SIZE * sizeof(uint32_t)) goto ExportWriteError;
		statusBar()->showMessage(tr("Exported project to .vc1dump file"));
	}

	m_PropertiesEditor->setInfo(tr("Exported project to .vc1dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);

	return;
ExportWriteError:
	QMessageBox::critical(this, tr("Export"), tr("Failed to write file"));
}

void MainWindow::actProjectFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
}

void MainWindow::actResetEmulator()
{
	// Stop the emulator
	stopEmulatorInternal();

	// Reset data
	printf("Reset emulator parameters\n");
	resetemu();
	m_ContentManager->reuploadAll();
	m_DlEditor->poke();
	m_CmdEditor->poke();
	m_Macro->poke();

	// Start the emulator
	startEmulatorInternal();
}

void MainWindow::stopEmulatorInternal()
{
	printf("Stop the emulator\n");
	s_EmulatorRunning = false;
	m_EmulatorViewport->stop();
	cleanupMediaFifo();
}

void MainWindow::startEmulatorInternal()
{
	printf("Start the emulator\n");
	BT8XXEMU_EmulatorParameters params;
	memset(&params, 0, sizeof(BT8XXEMU_EmulatorParameters));
	params.Main  =  emuMain;
	params.Flags =  BT8XXEMU_EmulatorEnableMouse
				  | BT8XXEMU_EmulatorEnableAudio
				  | BT8XXEMU_EmulatorEnableCoprocessor
				  | BT8XXEMU_EmulatorEnableGraphicsMultithread
				  | BT8XXEMU_EmulatorEnableMainPerformance
					;
	params.Mode = deviceToEnum(FTEDITOR_CURRENT_DEVICE);
	params.Close = closeDummy;
	s_EmulatorRunning = true;
	m_EmulatorViewport->run(params);

	BT8XXEMU_setDebugLimiter(g_Emulator, 2048 * 64);
}

void MainWindow::changeEmulatorInternal(int deviceIntf, int flashIntf)
{
	bool changeDevice = deviceIntf != FTEDITOR_CURRENT_DEVICE;
	bool changeFlash = flashIntf != FTEDITOR_CURRENT_FLASH && flashSupport(deviceIntf);

	if (!changeDevice && !changeFlash)
		return;

	// Remove any references to the current emulator device version
	m_Inspector->unbindCurrentDevice();

	// Stop the emulator
	stopEmulatorInternal();

	// Set the new emulator version
	if (changeDevice) FTEDITOR_CURRENT_DEVICE = deviceIntf;
	if (changeFlash) FTEDITOR_CURRENT_FLASH = flashIntf;

	// Reset emulator data
	printf("Reset emulator parameters\n");
	resetemu();
	m_ContentManager->reuploadAll();
	m_DlEditor->poke();
	m_CmdEditor->poke();
	m_Macro->poke();

	// Start the emulator
	startEmulatorInternal();

	// Re-establish the current emulator device
	m_Inspector->bindCurrentDevice();
	m_DlEditor->bindCurrentDevice();
	m_CmdEditor->bindCurrentDevice();
	m_Macro->bindCurrentDevice();
	m_Toolbox->bindCurrentDevice();
	m_ContentManager->bindCurrentDevice();
	m_InteractiveProperties->bindCurrentDevice();

	// Update resolution list
	if (changeDevice)
	{
		s_UndoRedoWorking = true;
		m_ProjectDisplay->clear();
		for (int i = 0; i < s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE]; ++i)
			m_ProjectDisplay->addItem(s_StandardResolutions[i]);
		m_ProjectDisplay->addItem("");
		updateProjectDisplay(m_HSize->value(), m_VSize->value());
		s_UndoRedoWorking = false;
	}

	// Update flash support
	if (changeDevice)
	{
		m_ProjectFlashGroup->setVisible(flashSupport(FTEDITOR_CURRENT_DEVICE));
	}

	// Reconfigure emulator controls
	stepEnabled(m_StepEnabled->isChecked());
	stepCmdEnabled(m_StepCmdEnabled->isChecked());

	// Update interface ranges
	if (changeDevice)
	{
		m_UtilizationDisplayListStatus->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE));
		m_UtilizationGlobalStatus->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END));
		m_UtilizationDisplayList->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE));
		m_StepCount->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE) * 64);
		m_StepCmdCount->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE) * 64);
		m_TraceX->setMaximum(screenWidthMaximum(FTEDITOR_CURRENT_DEVICE) - 1);
		m_TraceY->setMaximum(screenHeightMaximum(FTEDITOR_CURRENT_DEVICE) - 1);
		m_HSize->setMaximum(screenWidthMaximum(FTEDITOR_CURRENT_DEVICE));
		m_VSize->setMaximum(screenHeightMaximum(FTEDITOR_CURRENT_DEVICE));
		m_Rotate->setMaximum(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 ? 7 : 1);
	}

	// TODO:
	// Inside ProjectDeviceCommand store the original display lists (incl macro) plus a backup of the current ContentInfo settings
	// During ProjectDeviceCommand->redo() calculate the new display lists (incl macro) plus the new contentinfo settings for image formats etc
	// - Coprocessor list, display list, macro list
	// - Content info bitmap format L2, palette (plus palette addr) (font and image)
	// - Display width and height limit
	// Correctly set bitmap address in font header block during content upload
}

class ProjectDeviceCommand : public QUndoCommand
{
public:
	ProjectDeviceCommand(int deviceIntf, MainWindow *mainWindow) : QUndoCommand(), m_NewProjectDevice(deviceIntf), m_OldProjectDevice(FTEDITOR_CURRENT_DEVICE), m_MainWindow(mainWindow) { }
	virtual ~ProjectDeviceCommand() { }
	virtual void undo() { m_MainWindow->changeEmulatorInternal(m_OldProjectDevice, FTEDITOR_CURRENT_FLASH); s_UndoRedoWorking = true; m_MainWindow->m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE); s_UndoRedoWorking = false; }
	virtual void redo() { m_MainWindow->changeEmulatorInternal(m_NewProjectDevice, FTEDITOR_CURRENT_FLASH); s_UndoRedoWorking = true; m_MainWindow->m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE); s_UndoRedoWorking = false; }
	virtual int id() const { return 98919600; }
	virtual bool mergeWith(const QUndoCommand *command) { m_NewProjectDevice = static_cast<const ProjectDeviceCommand *>(command)->m_NewProjectDevice; return true; }

private:
	int m_NewProjectDevice;
	int m_OldProjectDevice;
	MainWindow *m_MainWindow;

};

void MainWindow::projectDeviceChanged(int deviceIntf)
{
	if (s_UndoRedoWorking)
		return;
	
	m_UndoStack->push(new ProjectDeviceCommand(deviceIntf, this));
}

class ProjectFlashCommand : public QUndoCommand
{
public:
	ProjectFlashCommand(int flashIntf, MainWindow *mainWindow) : QUndoCommand(), m_NewProjectFlash(flashIntf), m_OldProjectFlash(FTEDITOR_CURRENT_FLASH), m_MainWindow(mainWindow) { }
	virtual ~ProjectFlashCommand() { }
	virtual void undo() { m_MainWindow->changeEmulatorInternal(FTEDITOR_CURRENT_DEVICE, m_OldProjectFlash); s_UndoRedoWorking = true; m_MainWindow->m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_DEVICE); s_UndoRedoWorking = false; }
	virtual void redo() { m_MainWindow->changeEmulatorInternal(FTEDITOR_CURRENT_DEVICE, m_NewProjectFlash); s_UndoRedoWorking = true; m_MainWindow->m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_DEVICE); s_UndoRedoWorking = false; }
	virtual int id() const { return 98919601; }
	virtual bool mergeWith(const QUndoCommand *command) { m_NewProjectFlash = static_cast<const ProjectFlashCommand *>(command)->m_NewProjectFlash; return true; }

private:
	int m_NewProjectFlash;
	int m_OldProjectFlash;
	MainWindow *m_MainWindow;

};

void MainWindow::projectFlashChanged(int flashIntf)
{
	if (s_UndoRedoWorking)
		return;
	
	m_UndoStack->push(new ProjectFlashCommand(flashIntf, this));
}

void MainWindow::projectDisplayChanged(int i)
{
	if (s_UndoRedoWorking)
		return;
	
	if (i < s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE])
	{
		m_UndoStack->beginMacro(tr("Change display"));
		m_HSize->setValue(s_StandardWidths[i]);
		m_VSize->setValue(s_StandardHeights[i]);
		m_UndoStack->endMacro();
	}
}

void MainWindow::actSaveScreenshot()
{
	QString filterpng = tr("PNG image (*.png)");
	QString filterjpg = tr("JPG image (*.jpg)");
	QString filter = filterpng + ";;" + filterjpg;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), getFileDialogPath(), filter, &filter);
	if (fileName.isNull())
		return;

	if (filter == filterpng)
	{
		if (!fileName.endsWith(".png"))
			fileName = fileName + ".png";
	}
	if (filter == filterjpg)
	{
		if (!fileName.endsWith(".jpg"))
			fileName = fileName + ".jpg";
	}

	// Save screenshot
	const QPixmap &pixmap = m_EmulatorViewport->getPixMap();
	pixmap.save(fileName);
}

void MainWindow::actImportDisplayList()
{
	m_UndoStack->beginMacro(tr("Import display list"));
	m_DlEditor->lockDisplayList();
	memcpy(m_DlEditor->getDisplayList(), BT8XXEMU_getDisplayList(g_Emulator), FTEDITOR_DL_SIZE * sizeof(uint32_t));
	m_DlEditor->reloadDisplayList(false);
	m_DlEditor->unlockDisplayList();
	m_CmdEditor->removeAll();
	m_UndoStack->endMacro();
	m_DlEditorDock->setVisible(true);
	focusDlEditor();
}

void MainWindow::actDisplayListFromIntegers()
{
	m_UndoStack->beginMacro(tr("Display list from integers"));
	int bc = m_DlEditor->codeEditor()->document()->blockCount();
	for (int i = 0; i < bc; ++i)
	{
		QTextBlock block = m_DlEditor->codeEditor()->document()->findBlockByNumber(i);
		uint32_t v = (uint32_t)block.text().toUInt();
		if (v)
		{
			QString str = DlParser::toString(FTEDITOR_CURRENT_DEVICE, v);
			DlParsed pa;
			DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa, str, false, false);
			m_DlEditor->replaceLine(i, pa);
		}
	}
	m_UndoStack->endMacro();
}

void MainWindow::dummyCommand()
{
	printf("!!!!!!!! dummy action !!!!!!!!!!!!!!!!\n");
	m_UndoStack->push(new QUndoCommand());
}

void MainWindow::manual()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(m_InitialWorkingDir + "/FT8XX Screen Editor.chm"));
}

void MainWindow::about()
{
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("About EVE Screen Editor v2.5.0"));
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setText(tr(
		"Copyright (C) 2013-2015  Future Technology Devices International Ltd<br>"		
		"<br>"
		"Copyright (C) 2016-2017  Bridgetek Pte Ltd<br>"
		"<br>"
		"Support and updates:<br>"
		"<a href='http://www.ftdichip.com/Support/Utilities.htm'>http://www.ftdichip.com/Support/Utilities.htm</a><br>"
		"<br>"
		"<a href='http://brtchip.com/utilities/#evescreeneditor'>http://brtchip.com/utilities/#evescreeneditor</a>"
		));
	msgBox.exec();
}

void MainWindow::aboutQt()
{
	QMessageBox::about(this, tr("3rd Party"), tr(
		"The Qt GUI Toolkit is Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).\n"
		"Contact: http://www.qt-project.org/legal\n"
		"Qt is available under the LGPL.\n"
		"\n"
		"Portions part of the examples of the Qt Toolkit, under the BSD license.\n"
		"Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).\n"
		"Contact: http://www.qt-project.org/legal\n"
		"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "
		"\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT "
		"LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR "
		"A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT "
		"OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, "
		"SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT "
		"LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, "
		"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY "
		"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT "
		"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
		"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
		"\n"
		"Fugue Icons\n"
		"(C) 2013 Yusuke Kamiyamane. All rights reserved.\n"
		"These icons are licensed under a Creative Commons"
		"Attribution 3.0 License.\n"
		"<http://creativecommons.org/licenses/by/3.0/>"));
}

} /* namespace FTEDITOR */

/* end of file */
