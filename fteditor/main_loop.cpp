/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6262) // Large stack
#endif

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "main_window.h"
#include "version.h"
#include "Windows.h"

// STL includes
#include <stdio.h>
#include <algorithm>
#include <math.h>

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
#include <QStyleFactory>
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
#include <QPushButton>
#include <QTextStream>

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

volatile int g_HSize = 480;
volatile int g_VSize = 272;
volatile int g_Rotate = 0;

volatile bool g_EmulatorRunning = false;

// Content manager
ContentManager *g_ContentManager = NULL;
//static BitmapSetup *s_BitmapSetup = NULL;
//static int s_BitmapSetupModNb = 0;

// Editors
DlEditor *g_DlEditor = NULL;
DlEditor *g_CmdEditor = NULL;
DlEditor *g_Macro = NULL;

// Utilization
int g_UtilizationDisplayListCmd = 0;
volatile bool g_WaitingCoprocessorAnimation = false;

// Array indexed by display list index containing coprocessor line which wrote the display list command
static int s_DisplayListCoprocessorCommandA[FTEDITOR_DL_SIZE];
static int s_DisplayListCoprocessorCommandB[FTEDITOR_DL_SIZE];
int *g_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandA;
static int *s_DisplayListCoprocessorCommandWrite = s_DisplayListCoprocessorCommandB;

static std::vector<uint32_t> s_CmdParamCache;
static std::vector<std::string> s_CmdStrParamCache;

int g_StepCmdLimit = 0;
static int s_StepCmdLimitCurrent = 0;

static int s_MediaFifoPtr = 0;
static int s_MediaFifoSize = 0;
static QFile *s_MediaFifoFile = NULL;
static QDataStream *s_MediaFifoStream = NULL;

volatile bool g_CoprocessorFaultOccured = false;
volatile bool g_CoprocessorFrameSuccess = false;
char g_CoprocessorDiagnostic[128 + 4] = { 0 };
bool g_StreamingData = false;

bool g_WarnMissingClear = false;
bool g_WarnMissingClearActive = false;
bool g_WarnMissingTestcardDLStart = false;
bool g_WarnMissingTestcardDLStartActive = false;

volatile bool g_ShowCoprocessorBusy = true;

static bool displayListSwapped = false;
static bool coprocessorSwapped = false;

static bool s_WantReloopCmd = false;

static bool s_HasContentReadCoCmd = false;

QElapsedTimer s_AbortTimer;

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

void wr8(ramaddr address, uint8_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("wr8(%i, %i)\n", (int)address, (int)value);
#endif

	swrbegin(address);
	swr8(value);
	swrend();
}

void wr16(ramaddr address, uint16_t value)
{
#if FTEDITOR_DEBUG_EMUWRITE
	printf("wr16(%i, %i)\n", (int)address, (int)value);
#endif

	swrbegin(address);
	swr16(value);
	swrend();
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

uint16_t rd16(size_t address)
{

	BT8XXEMU_chipSelect(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (address >> 16) & 0x3F);
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	BT8XXEMU_transfer(g_Emulator, 0x00);

	uint16_t value;
	value = BT8XXEMU_transfer(g_Emulator, 0);
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 8;

#if FTEDITOR_DEBUG_EMUWRITE
	printf("rd16(%i), %i\n", (int)address, (int)value);
#endif

	BT8XXEMU_chipSelect(g_Emulator, 0);
	return value;
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

	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
	{
		// Calling this on first emulator launch fixes an odd coprocessor issue with fonts not getting loaded occasionally
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD), CMD_ROMFONT);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 4, 0);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 8, 16);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 12, CMD_RESETFONTS);
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 16);
	}
}

void cleanupMediaFifo()
{
	delete s_MediaFifoStream;
	s_MediaFifoStream = NULL;
	delete s_MediaFifoFile;
	s_MediaFifoFile = NULL;
}

void resetemu()
{
	g_UtilizationDisplayListCmd = 0;
	g_WaitingCoprocessorAnimation = false;
	g_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandA;
	s_DisplayListCoprocessorCommandWrite = s_DisplayListCoprocessorCommandB;
	s_CmdParamCache.clear();
	g_StepCmdLimit = 0;
	s_StepCmdLimitCurrent = 0;
	g_CoprocessorFaultOccured = false;
	g_WarnMissingClear = false;
	g_WarnMissingClearActive = false;
	g_WarnMissingTestcardDLStart = false;
	g_WarnMissingTestcardDLStartActive = false;
	displayListSwapped = false;
	coprocessorSwapped = false;
	s_WantReloopCmd = false;
	s_MediaFifoPtr = 0;
	s_MediaFifoSize = 0;
	cleanupMediaFifo();
}

bool hasOTP()
{
	return FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT815;
}

void resetCoprocessorFromLoop()
{
	printf("Reset coprocessor from loop\n");

	// BT81X video patch, see ftdichipsg/FT8XXEMU#76 and #136
	uint16_t coproPatchPtr;
	bool needCoproPatch = FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT815 || FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT816;
	if (needCoproPatch)
	{
		coproPatchPtr = rd16(0x309162);
	}

	// Enter reset state
	wr8(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CPURESET), 1);
	QThread::msleep(10); // Timing hack because we don't lock CPURESET flag at the moment with coproc thread
	// Leave reset
	if (hasOTP())
	{
		// Enable patched rom in case cmd_logo was running
		wr8(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROMSUB_SEL), 3);
	}
	wr16(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ), 0);
	wr16(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 0);
	for (int i = 0; i < 4096; i += 4)
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + i, CMD_STOP);
	wr8(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CPURESET), 0);
	// Stop playing audio in case video with audio was playing during reset
	wr8(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_PLAYBACK_PLAY), 0);

	QThread::msleep(100); // Timing hack because we don't lock CPURESET flag at the moment with coproc thread
	if (hasOTP())
	{
		// Go back into the patched coprocessor main loop
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD), CMD_EXECUTE);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 4, 0x7ffe);
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 8, 0);
		wr16(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 12);
		QThread::msleep(10); // Timing hack because it's not checked when the coprocessor finished processing the CMD_EXECUTE
		// Need to manually stop previous command from repeating infinitely
		wr16(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 0);
		QThread::msleep(100);
	}

	// BT81X video patch, see ftdichipsg/FT8XXEMU#76 and #136
	if (needCoproPatch)
	{
		wr16(0x309162, coproPatchPtr);
	}

	// Start display list from beginning
	wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD), CMD_DLSTART);

	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
	{
		// Attach flash
		wr32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + 4, CMD_FLASHATTACH);
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 8);
	}
	else
	{
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), 4);
	}
}

// static int s_SwapCount = 0;
void loop()
{
	if (!g_EmulatorRunning)
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
			g_CoprocessorFaultOccured = true;
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
			{
				uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
				memcpy(g_CoprocessorDiagnostic, (char *)&ram[0x309800], 128);
				g_CoprocessorDiagnostic[128] = '\0';
				printf("COPROCESSOR FAULT: '%s'\n", g_CoprocessorDiagnostic);
			}
			else
			{
				g_CoprocessorDiagnostic[0] = '\0';
				printf("COPROCESSOR FAULT\n");
			}
			resetCoprocessorFromLoop();
		}
	}

	g_ShowCoprocessorBusy = false;

	bool contentPoked = false; // Whether the contents of RAM_G or Flash has been changed

	// wait
	if (coprocessorSwapped)
	{
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE)) != rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)))
		{
			if (!g_EmulatorRunning) return;
			if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) return;
		}
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_DL)) != 0)
		{
			if (!g_EmulatorRunning) return;
			if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) return;
		}
		coprocessorSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped CMD %i\n", s_SwapCount);
	}
	else if (displayListSwapped)
	{
		while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_DLSWAP)) != DLSWAP_DONE)
		{
			if (!g_EmulatorRunning) return;
		}
		displayListSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped DL %i\n", s_SwapCount);
	}
	else
	{
		QThread::msleep(10);
	}

	if (!g_EmulatorRunning)
		return;

	g_ContentManager->lockContent();
	if (g_Flash)
	{
		std::set<ContentInfo *> contentInfoFlash;
		g_ContentManager->swapUploadFlashDirty(contentInfoFlash);
		size_t flashSize = BT8XXEMU_Flash_size(g_Flash);
		for (std::set<ContentInfo *>::iterator it(contentInfoFlash.begin()), end(contentInfoFlash.end()); it != end; ++it)
		{
			ContentInfo *info = (*it);
			int loadAddr = info->FlashAddress; // (info->Converter == ContentInfo::Image) ? info->bitmapAddress() : info->MemoryAddress;
			if (loadAddr < FTEDITOR_FLASH_FIRMWARE_SIZE)
			{
				// Safety to avoid breaking functionality, never allow overriding the provided firmware from the content manager
				printf("[Flash] Error: Load address not permitted for '%s' to '%i'\n", info->DestName.toLocal8Bit().data(), loadAddr);
				continue;
			}
			bool dataCompressed = (info->Converter != ContentInfo::ImageCoprocessor && info->Converter != ContentInfo::FlashMap)
				? info->DataCompressed : false;
			QString fileName = info->DestName + (dataCompressed ? ".bin" : ".raw");
			printf("[Flash] Load: '%s' to '%i'\n", fileName.toLocal8Bit().constData(), loadAddr);
			QFile binFile(fileName);
			if (!binFile.exists())
			{
				printf("[Flash] Error: File '%s' does not exist\n", fileName.toLocal8Bit().constData());
				continue;
			}
			int64_t binSize = binFile.size();
			if ((uint64_t)binSize + loadAddr > flashSize)
			{
				printf("[Flash] Error: File of size '%i' exceeds flash size\n", (int)binSize);
				continue;
			}
			; {
				binFile.open(QIODevice::ReadOnly);
				QDataStream in(&binFile);
				char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_Flash_data(g_Flash)));
				int s = in.readRawData(&ram[loadAddr], (int)binSize);
				// FIXME: Pad 0x00 to end for 64 byte-aligned size
				BT8XXEMU_poke(g_Emulator);
				contentPoked = true;
                binFile.close();
			}
		}
	}
	std::set<ContentInfo *> contentInfoMemory;
	g_ContentManager->swapUploadMemoryDirty(contentInfoMemory);
	//bool reuploadFontSetup = false;
	for (std::set<ContentInfo *>::iterator it(contentInfoMemory.begin()), end(contentInfoMemory.end()); it != end; ++it)
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
		int binSize = imageCoprocessor ? info->CachedMemorySize : binFile.size();
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
						if (!g_EmulatorRunning) return;
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
				if (!g_EmulatorRunning)
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
			contentPoked = true;
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
					contentPoked = true;
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
				//if (palSize != (int)palFile.size())
				//{
				//	printf("[RAM_PAL] Error: File of size '%i' not equal to palSize\n", palSize);
				//	continue;
				//}
				// ok
				{
					palFile.open(QIODevice::ReadOnly);
					QDataStream in(&palFile);
					char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
					int s = in.readRawData(&ram[FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G) + info->MemoryAddress], palSize);
					BT8XXEMU_poke(g_Emulator);
					contentPoked = true;
				}
			}
		}
		/*if (info->Converter == ContentInfo::Font)
		{*/ // Always reupload, since raw data may change too
			//reuploadFontSetup = true;
		/*}*/
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
			if (!g_EmulatorRunning) return;
		}
		wr32(REG_PCLK, 5);
		s_BitmapSetupModNb = s_BitmapSetup->getModificationNb();
	}*/
	g_ContentManager->unlockContent();

	// switch to next resolution
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE)) != g_HSize)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE), g_HSize);
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE)) != g_VSize)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE), g_VSize);
	// switch to next rotation (todo: CMD_SETROTATE for coprocessor)
	if (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROTATE)) != g_Rotate)
		wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_ROTATE), g_Rotate);
	// switch to next macro list
	g_Macro->lockDisplayList();
	bool macroModified = g_Macro->isDisplayListModified();
	// if (macroModified) // Always write macros to intial user value, in case changed by coprocessor
	// {
		if (g_Macro->getDisplayListParsed()[0].ValidId
			&& rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_0)) != g_Macro->getDisplayList()[0]) // Do a read test so we don't change the ram if not necessary
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_0), g_Macro->getDisplayList()[0]); // (because ram writes cause the write count to increase and force a display render)
		if (g_Macro->getDisplayListParsed()[1].ValidId
			&& rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_1)) != g_Macro->getDisplayList()[1])
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_MACRO_1), g_Macro->getDisplayList()[1]);
	// }
	g_Macro->unlockDisplayList();
	// switch to next display list
	g_DlEditor->lockDisplayList();
	g_CmdEditor->lockDisplayList();
	bool dlModified = g_DlEditor->isDisplayListModified();
	bool cmdModified = g_CmdEditor->isDisplayListModified();
	if (dlModified || cmdModified || /*reuploadFontSetup ||*/ (g_StepCmdLimit != s_StepCmdLimitCurrent) || s_WantReloopCmd || (s_HasContentReadCoCmd && contentPoked))
	{
		bool warnMissingClear = true;
		s_WantReloopCmd = false;
		s_HasContentReadCoCmd = false;
		s_StepCmdLimitCurrent = g_StepCmdLimit;
		// if (dlModified) printf("dl modified\n");
		// if (cmdModified) printf("cmd modified\n");
		uint32_t *displayList = g_DlEditor->getDisplayList();
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
		g_DlEditor->unlockDisplayList();

		uint32_t cmdList[FTEDITOR_DL_SIZE];
		// DlParsed cmdParsed[FTEDITOR_DL_SIZE];
		s_CmdParamCache.clear();
		s_CmdStrParamCache.clear();
		int strParamRead = 0;
		int cmdParamIdx[FTEDITOR_DL_SIZE + 1];
		bool cmdValid[FTEDITOR_DL_SIZE];
		uint32_t *cmdListPtr = g_CmdEditor->getDisplayList();
		const DlParsed *cmdParsedPtr = g_CmdEditor->getDisplayListParsed();
		// Make local copy, necessary in case of blocking commands
		for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
		{
			cmdList[i] = cmdListPtr[i];
			// cmdParsed[i] = cmdParsedPtr[i];
			cmdParamIdx[i] = (int)s_CmdParamCache.size();
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
		cmdParamIdx[FTEDITOR_DL_SIZE] = (int)s_CmdParamCache.size();
		g_CmdEditor->unlockDisplayList();
		g_WarnMissingClear = warnMissingClear;

		g_ShowCoprocessorBusy = true;
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
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
		{
			// int flashStatus = rd32(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_FLASH_STATUS));
			swr32(CMD_FLASHATTACH);
			//swr32(CMD_FLASHFAST);
			//swr32(~0); // result
			wp += 4;
			freespace -= 4;
		}
		s_MediaFifoPtr = 0;
		s_MediaFifoSize = 0;
		int lastCmd = -1;
		bool warnMissingTestcardDLStart = false;
		for (int i = 0; i < (s_StepCmdLimitCurrent ? s_StepCmdLimitCurrent : FTEDITOR_DL_SIZE); ++i) // FIXME CMD SIZE
		{
			// const DlParsed &pa = cmdParsed[i];
			// Skip invalid lines (invalid id)
			if (!cmdValid[i]) continue;
			// if (lastCmd == CMD_TESTCARD) break; // Make CMD_TESTCARD show
			bool useMediaFifo = false;
			bool useFlash = false;
			const char *useFileStream = NULL;
			switch (cmdList[i])
			{
				// Track when the command list is reading from RAM_G and Flash.
				// (That is, the coprocessor itself is reading from RAM_G or Flash, not just the display engine.)
				// TODO: Some of these commands only read from content 
				// RAM_G and Flash depending on OPT_FLASH or other options.
				// Needs to be checked.
			case CMD_MEMCRC:
			case CMD_REGREAD:
			case CMD_MEMCPY:
			case CMD_APPEND:
			case CMD_INFLATE:
			case CMD_FLASHREAD:
			case CMD_FLASHPROGRAM:
			case CMD_FLASHWRITE:
			case CMD_FLASHUPDATE:
			case CMD_LOADIMAGE:
			case CMD_SETFONT:
			case CMD_SETFONT2:
			case CMD_VIDEOSTART:
			case CMD_VIDEOFRAME:
			case CMD_INFLATE2:
			case CMD_ANIMSTART:
			case CMD_ANIMDRAW:
			case CMD_ANIMFRAME:
			case CMD_APPENDF:
			case CMD_VIDEOSTARTF:
			case CMD_ANIMFRAMERAM:
			case CMD_ANIMSTARTRAM:
			case CMD_RUNANIM:
				s_HasContentReadCoCmd = true;
				break;
			}
			if ((FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) && (cmdList[i] == CMD_MEDIAFIFO))
			{
				s_MediaFifoPtr = s_CmdParamCache[cmdParamIdx[i]];
				s_MediaFifoSize = s_CmdParamCache[(size_t)cmdParamIdx[i] + 1];
			}
			else if (cmdList[i] == CMD_LOADIMAGE)
			{
				useFlash = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					&& (s_CmdParamCache[(size_t)cmdParamIdx[i] + 1] & OPT_FLASH);
				useFileStream = useFlash ? NULL : s_CmdStrParamCache[strParamRead].c_str();
				++strParamRead;
				useMediaFifo = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810) 
					&& (s_CmdParamCache[(size_t)cmdParamIdx[i] + 1] & OPT_MEDIAFIFO);
			}
			else if (cmdList[i] == CMD_PLAYVIDEO)
			{
				useFlash = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					&& (s_CmdParamCache[cmdParamIdx[i]] & OPT_FLASH);
				useFileStream = useFlash ? NULL : s_CmdStrParamCache[strParamRead].c_str();
				++strParamRead;
				useMediaFifo = (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
					&& ((s_CmdParamCache[cmdParamIdx[i]] & OPT_MEDIAFIFO) == OPT_MEDIAFIFO);
			}
			else if (cmdList[i] == CMD_SNAPSHOT)
			{
				// Validate snapshot address range
				uint32_t addr = s_CmdParamCache[cmdParamIdx[i]];
				uint32_t ramGEnd = FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END);
				uint32_t imgSize = (g_VSize * g_HSize) * 2;
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
			// Warn the user to insert CMD_DLSTART when other commands follow CMD_TESTCARD
			if (lastCmd == CMD_TESTCARD && cmdList[i] != CMD_DLSTART)
			{
				warnMissingTestcardDLStart = true;
			}
			validCmd = true;
			lastCmd = cmdList[i];
			int paramNb = cmdParamIdx[i + 1] - cmdParamIdx[i];
			int cmdLen = 4 + (paramNb * 4);
			if (freespace < (cmdLen + 8)) // Wait for coprocessor ready, + 4 for swap and display afterwards
			{
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				do
				{
					if (!g_EmulatorRunning) return;
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
			for (int j = cmdParamIdx[i]; j < cmdParamIdx[i + 1]; ++j)
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
			if (cmdList[i] == CMD_TESTCARD)
			{
				// Avoid display list overrun on trailing commands
				// Might want to warn the user if this is not the last command, and cmdList[i + 1] isn't CMD_DLSTART...
				swr32(CMD_DLSTART);
				// No need to check space for this, since the regular 
				// CMD_SWAP won't be called anymore if this is the last command. 
				// We're using it's space
				wp += 4;
				freespace -= 4;
			}
			else if (cmdList[i] == CMD_LOGO)
			{
				printf("Waiting for CMD_LOGO...\n");
				g_WaitingCoprocessorAnimation = true;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				printf("WP = %i\n", wp);
				QThread::msleep(100);

				do
				{
					rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
					wp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE));
					if (!g_EmulatorRunning) { g_WaitingCoprocessorAnimation = false; return; }
					if ((rp & 0xFFF) == 0xFFF) { g_WaitingCoprocessorAnimation = false; return; }
					if (g_CmdEditor->isDisplayListModified()) { s_WantReloopCmd = true; resetCoprocessorFromLoop(); g_WaitingCoprocessorAnimation = false; return; }
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
					if (!g_EmulatorRunning) { g_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { g_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				g_WaitingCoprocessorAnimation = false;
				s_WantReloopCmd = true;
				printf("Finished CMD_LOGO\n");
			}
			else if (cmdList[i] == CMD_CALIBRATE)
			{
				printf("Waiting for CMD_CALIBRATE...\n");
				g_WaitingCoprocessorAnimation = true;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) != (wp & 0xFFF))
				{
					if (!g_EmulatorRunning) { g_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { g_WaitingCoprocessorAnimation = false; return; }
					if (g_CmdEditor->isDisplayListModified()) { s_WantReloopCmd = true; resetCoprocessorFromLoop(); g_WaitingCoprocessorAnimation = false; return; }
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
					if (!g_EmulatorRunning) { g_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { g_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				g_WaitingCoprocessorAnimation = false;
				s_WantReloopCmd = true;
				printf("Finished CMD_CALIBRATE\n");
			}
			else if ((cmdList[i] == CMD_PLAYVIDEO) && useFlash)
			{
				printf("Waiting for CMD_PLAYVIDEO...\n");
				g_WaitingCoprocessorAnimation = true;
				swrend();
				wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
				while (rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) != (wp & 0xFFF))
				{
					if (!g_EmulatorRunning) { printf("Abort wait, restarting\n"); g_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { printf("Wait fault\n"); g_WaitingCoprocessorAnimation = false; return; }
					if (g_CmdEditor->isDisplayListModified()) { printf("Abort wait, modified\n"); s_WantReloopCmd = true; resetCoprocessorFromLoop(); g_WaitingCoprocessorAnimation = false; return; }
				}
				printf("Waiting for CMD_COLDSTART...\n");
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
					if (!g_EmulatorRunning) { g_WaitingCoprocessorAnimation = false; return; }
					if ((rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ)) & 0xFFF) == 0xFFF) { printf("Wait fault 2\n"); g_WaitingCoprocessorAnimation = false; return; }
					if (g_CmdEditor->isDisplayListModified()) { printf("Abort wait, modified\n"); s_WantReloopCmd = true; resetCoprocessorFromLoop(); g_WaitingCoprocessorAnimation = false; return; }
				}
				swrbegin(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_CMD) + (wp & 0xFFF));
				g_WaitingCoprocessorAnimation = false;
				printf("Finished CMD_PLAYVIDEO\n");
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
						if (!g_EmulatorRunning) return;
						rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
						if ((rp & 0xFFF) == 0xFFF) return;
						fullness = ((wp & 0xFFF) - rp) & 0xFFF;

						if (g_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
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

						g_StreamingData = true;
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
									if (!g_EmulatorRunning) return;
									ramaddr rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
									if ((rp & 0xFFF) == 0xFFF) return;
									mfrprd();

									if (g_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
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
							if (!g_EmulatorRunning)
							{
								swrend();
								return;
							}
							if (g_CmdEditor->isDisplayListModified())
							{
								s_WantReloopCmd = true;
								swrend();
								resetCoprocessorFromLoop();
								return;
							}
						}
						g_StreamingData = false;
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
					g_StreamingData = true;
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
								if (!g_EmulatorRunning) return;
								rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
								if ((rp & 0xFFF) == 0xFFF)
								{
									printf("Error during stream at %i bytes\n", (int)writeCount);
									return; // DIFFER FROM ABOVE HERE
								}
								fullness = ((wp & 0xFFF) - rp) & 0xFFF;

								if (g_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
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
						if (!g_EmulatorRunning)
						{
							swrend();
							return;
						}
						if (g_CmdEditor->isDisplayListModified())
						{
							s_WantReloopCmd = true;
							swrend();
							resetCoprocessorFromLoop();
							return;
						}
					}
					printf("Finished streaming in file\n");
					g_StreamingData = false;
				}

				printf("Flush after stream\n");
				if (true) // Flush after
				{
					swrend();
					wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));
					do
					{
						if (!g_EmulatorRunning) return;
						rp = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
						if ((rp & 0xFFF) == 0xFFF) return;
						fullness = ((wp & 0xFFF) - rp) & 0xFFF;

						if (g_CmdEditor->isDisplayListModified()) // Trap to avoid infinite flush on errors
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

		g_WarnMissingTestcardDLStart = warnMissingTestcardDLStart;

		if (validCmd)
		{
			if (lastCmd != CMD_TESTCARD)
			{
			 	// Testcard already swaps
				swr32(DISPLAY());
				swr32(CMD_SWAP);
			 	wp += 8;
			}
			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			// Finish all processing
			int rpl = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
			s_AbortTimer.start();
			while ((wp & 0xFFF) != rpl)
			{
				rpl = rd32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_READ));
				if (!g_EmulatorRunning) return;
				if ((rpl & 0xFFF) == 0xFFF) return;

				if (g_CmdEditor->isDisplayListModified() || s_WantReloopCmd) // Trap to avoid infinite flush on errors
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
					g_UtilizationDisplayListCmd = i;
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

			g_CoprocessorFaultOccured = false;
			g_StreamingData = false;
			coprocessorSwapped = true;
		}
		else
		{
			// Swap frame directly if nothing was written to the coprocessor
			g_UtilizationDisplayListCmd = 0;

			swrend();
			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_CMD_WRITE), (wp & 0xFFF));

			wr32(reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_DLSWAP), DLSWAP_FRAME);
		}

		g_ShowCoprocessorBusy = false;
		g_CoprocessorFrameSuccess = true;

		// FIXME: Not very thread-safe, but not too critical
		int *nextWrite = g_DisplayListCoprocessorCommandRead;
		g_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandWrite;
		s_DisplayListCoprocessorCommandWrite = nextWrite;
	}
	else
	{
		g_CmdEditor->unlockDisplayList();
		g_DlEditor->unlockDisplayList();
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

} /* namespace FTEDITOR */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
