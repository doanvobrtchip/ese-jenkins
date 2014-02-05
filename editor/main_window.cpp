/**
 * main_window.cpp
 * $Id$
 * \file main_window.cpp
 * \brief main_window.cpp
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "main_window.h"

// STL includes
#include <stdio.h>

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

// Emulator includes
#include <ft800emu_inttypes.h>
#ifdef WIN32
#	include <ft800emu_minmax.h>
#endif
#include <ft800emu_keyboard_keys.h>
#include <ft800emu_emulator.h>
#include <ft800emu_keyboard.h>
#include <ft800emu_system.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <ft800emu_graphics_driver.h>
#include <vc.h>
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

namespace FT800EMUQT {

int s_HSize = FT800EMU_WINDOW_WIDTH_DEFAULT;
int s_VSize = FT800EMU_WINDOW_HEIGHT_DEFAULT;

bool s_EmulatorRunning = false;

void swrbegin(size_t address)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((2 << 6) | ((address >> 16) & 0x3F));
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);
	// FT800EMU::SPII2C.transfer(0x00);
}

void swr8(uint8_t value)
{
	FT800EMU::SPII2C.transfer(value);
}

void swr16(uint16_t value)
{
	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
}

void swr32(uint32_t value)
{
	FT800EMU::SPII2C.transfer(value & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 16) & 0xFF);
	FT800EMU::SPII2C.transfer((value >> 24) & 0xFF);
}

void swrend()
{
	FT800EMU::SPII2C.csHigh();
}

void wr32(size_t address, uint32_t value)
{
	swrbegin(address);
	swr32(value);
	swrend();
}

uint32_t rd32(size_t address)
{
	FT800EMU::SPII2C.csLow();

	FT800EMU::SPII2C.transfer((address >> 16) & 0x3F);
	FT800EMU::SPII2C.transfer((address >> 8) & 0xFF);
	FT800EMU::SPII2C.transfer(address & 0xFF);
	FT800EMU::SPII2C.transfer(0x00);

	uint32_t value;
	value = FT800EMU::SPII2C.transfer(0);
	value |= FT800EMU::SPII2C.transfer(0) << 8;
	value |= FT800EMU::SPII2C.transfer(0) << 16;
	value |= FT800EMU::SPII2C.transfer(0) << 24;

	FT800EMU::SPII2C.csHigh();
	return value;
}

static DlEditor *s_DlEditor = NULL;
static DlEditor *s_CmdEditor = NULL;
static DlEditor *s_Macro = NULL;
// static FILE *s_F = NULL;

void setup()
{
	wr32(REG_HSIZE, 480);
	wr32(REG_VSIZE, 272);
	wr32(REG_PCLK, 5);
}

// Utilization
static int s_UtilizationDisplayListCmd = 0;

// Array indexed by display list index containing coprocessor line which wrote the display list command
static int s_DisplayListCoprocessorCommandA[FT800EMU_DL_SIZE];
static int s_DisplayListCoprocessorCommandB[FT800EMU_DL_SIZE];
static int *s_DisplayListCoprocessorCommandRead = s_DisplayListCoprocessorCommandA;
static int *s_DisplayListCoprocessorCommandWrite = s_DisplayListCoprocessorCommandB;

static std::vector<uint32_t> s_CmdParamCache;

static bool displayListSwapped = false;
static bool coprocessorSwapped = false;
// static int s_SwapCount = 0;
void loop()
{
	// wait
	if (coprocessorSwapped)
	{
		int wp = rd32(REG_CMD_WRITE);
		while (wp != rd32(REG_CMD_READ))
		{
			if (!s_EmulatorRunning) return;
		}
		while (rd32(REG_CMD_DL) != 0)
		{
			if (!s_EmulatorRunning) return;
		}
		coprocessorSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped CMD %i\n", s_SwapCount);
	}
	else if (displayListSwapped)
	{
		while (rd32(REG_DLSWAP) != DLSWAP_DONE)
		{
			if (!s_EmulatorRunning) return;
		}
		displayListSwapped = false;
		// ++s_SwapCount;
		// printf("Swapped DL %i\n", s_SwapCount);
	}
	else
	{
		FT800EMU::System.delay(10);
	}

	// switch to next resolution
	if (rd32(REG_HSIZE) != s_HSize)
		wr32(REG_HSIZE, s_HSize);
	if (rd32(REG_VSIZE) != s_VSize)
		wr32(REG_VSIZE, s_VSize);
	// switch to next macro list
	s_Macro->lockDisplayList();
	bool macroModified = s_Macro->isDisplayListModified();
	// if (macroModified) // Always write macros to intial user value, in case changed by coprocessor
	// {
		if (s_Macro->getDisplayListParsed()[0].ValidId
			&& rd32(REG_MACRO_0) != s_Macro->getDisplayList()[0]) // Do a read test so we don't change the ram if not necessary
			wr32(REG_MACRO_0, s_Macro->getDisplayList()[0]); // (because ram writes cause the write count to increase and force a display render)
		if (s_Macro->getDisplayListParsed()[1].ValidId
			&& rd32(REG_MACRO_1) != s_Macro->getDisplayList()[1])
			wr32(REG_MACRO_1, s_Macro->getDisplayList()[1]);
	// }
	s_Macro->unlockDisplayList();
	// switch to next display list
	s_DlEditor->lockDisplayList();
	s_CmdEditor->lockDisplayList();
	bool dlModified = s_DlEditor->isDisplayListModified();
	bool cmdModified = s_CmdEditor->isDisplayListModified();
	if (dlModified || cmdModified)
	{
		// if (dlModified) printf("dl modified\n");
		// if (cmdModified) printf("cmd modified\n");
		uint32_t *displayList = s_DlEditor->getDisplayList();
		swrbegin(RAM_DL);
		for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
		{
			// printf("dl %i: %i\n", i, displayList[i]);
			swr32(displayList[i]);
		}
		swrend();
		// wr32(REG_DLSWAP, DLSWAP_FRAME);
		// displayListSwapped = true;
		s_DlEditor->unlockDisplayList();

		uint32_t cmdList[FT800EMU_DL_SIZE];
		// DlParsed cmdParsed[FT800EMU_DL_SIZE];
		s_CmdParamCache.clear();
		int cmdParamCache[FT800EMU_DL_SIZE + 1];
		bool cmdValid[FT800EMU_DL_SIZE];
		uint32_t *cmdListPtr = s_CmdEditor->getDisplayList();
		const DlParsed *cmdParsedPtr = s_CmdEditor->getDisplayListParsed();
		// Make local copy, necessary in case of blocking commands
		for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
		{
			cmdList[i] = cmdListPtr[i];
			// cmdParsed[i] = cmdParsedPtr[i];
			cmdParamCache[i] = s_CmdParamCache.size();
			DlParser::compile(s_CmdParamCache, cmdParsedPtr[i]);
			cmdValid[i] = cmdParsedPtr[i].ValidId;
		}
		cmdParamCache[FT800EMU_DL_SIZE] = s_CmdParamCache.size();
		s_CmdEditor->unlockDisplayList();

		bool validCmd = false;
		int coprocessorWrites[1024]; // array indexed by write pointer of command index in the coprocessor editor gui
		for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
		for (int i = 0; i < FT800EMU_DL_SIZE; ++i) s_DisplayListCoprocessorCommandWrite[i] = -1;
		int wp = rd32(REG_CMD_WRITE);
		int rp = rd32(REG_CMD_READ);
		int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
		// printf("fullness: %i\n", fullness); // should be 0 always (ok)
		int freespace = ((4096 - 4) - fullness);
		FT800EMU::Memory.clearDisplayListCoprocessorWrites();
		swrbegin(RAM_CMD + (wp & 0xFFF));
		for (int i = 0; i < FT800EMU_DL_SIZE; ++i) // FIXME CMD SIZE
		{
			// const DlParsed &pa = cmdParsed[i];
			// Skip invalid lines (invalid id)
			if (!cmdValid[i]) continue;
			validCmd = true;
			int paramNb = cmdParamCache[i + 1] - cmdParamCache[i];
			int cmdLen = 4 + (paramNb * 4);
			if (freespace < (cmdLen + 8)) // Wait for coprocessor ready, + 4 for swap and display afterwards
			{
				swrend();
				wr32(REG_CMD_WRITE, (wp & 0xFFF));
				do
				{
					if (!s_EmulatorRunning) return;
					rp = rd32(REG_CMD_READ);
					fullness = ((wp & 0xFFF) - rp) & 0xFFF;
				} while (fullness != 0);
				freespace = ((4096 - 4) - fullness);

				int *cpWrite = FT800EMU::Memory.getDisplayListCoprocessorWrites();
				for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
				{
					if (cpWrite[i] >= 0)
					{
						// printf("A %i\n", i);
						s_DisplayListCoprocessorCommandWrite[i]
							= coprocessorWrites[cpWrite[i]];
					}
				}
				for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
				FT800EMU::Memory.clearDisplayListCoprocessorWrites();

				swrbegin(RAM_CMD + (wp & 0xFFF));
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
				swrend();
				wr32(REG_CMD_WRITE, (wp & 0xFFF));
				while (rd32(REG_CMD_READ) || rd32(REG_CMD_WRITE))
				{
					if (!s_EmulatorRunning) return;
				}
				wp = 0;
				rp = 0;
				fullness = ((wp & 0xFFF) - rp) & 0xFFF;
				freespace = ((4096 - 4) - fullness);

				int *cpWrite = FT800EMU::Memory.getDisplayListCoprocessorWrites();
				for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
				{
					if (cpWrite[i] >= 0)
					{
						// printf("D %i\n", i);
						s_DisplayListCoprocessorCommandWrite[i]
							= coprocessorWrites[cpWrite[i]];
					}
				}
				for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
				FT800EMU::Memory.clearDisplayListCoprocessorWrites();

				swrbegin(RAM_CMD + (wp & 0xFFF));
				swr32(CMD_DLSTART);
				wp += 4;
				freespace -= 4;
				swrend();
				wr32(REG_CMD_WRITE, (wp & 0xFFF));
				while (rd32(REG_CMD_READ) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) return;
				}
				swrbegin(RAM_CMD + (wp & 0xFFF));
				printf("Finished CMD_LOGO\n");
			}
			else if (cmdList[i] == CMD_CALIBRATE)
			{
				printf("Waiting for CMD_CALIBRATE...\n");
				swrend();
				wr32(REG_CMD_WRITE, (wp & 0xFFF));
				while (rd32(REG_CMD_READ) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) return;
				}
				swrbegin(RAM_CMD + (wp & 0xFFF));
				swr32(CMD_DLSTART);
				wp += 4;
				freespace -= 4;
				swrend();
				wr32(REG_CMD_WRITE, (wp & 0xFFF));
				while (rd32(REG_CMD_READ) != (wp & 0xFFF))
				{
					if (!s_EmulatorRunning) return;
				}
				swrbegin(RAM_CMD + (wp & 0xFFF));
				printf("Finished CMD_CALIBRATE\n");
			}
		}

		if (validCmd)
		{
			swr32(DISPLAY());
			swr32(CMD_SWAP);
			wp += 8;
			swrend();
			wr32(REG_CMD_WRITE, (wp & 0xFFF));

			// Finish all processing
			int rpl = rd32(REG_CMD_READ);
			while ((wp & 0xFFF) != rpl)
			{
				rpl = rd32(REG_CMD_READ);
				if (!s_EmulatorRunning) return;
			}
			int *cpWrite = FT800EMU::Memory.getDisplayListCoprocessorWrites();
			for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
			{
				if (cpWrite[i] >= 0)
				{
					// printf("C %i, %i, %i\n", i, cpWrite[i], coprocessorWrites[cpWrite[i]]);
					s_DisplayListCoprocessorCommandWrite[i]
						= coprocessorWrites[cpWrite[i]];
				}
			}
			for (int i = 0; i < 1024; ++i) coprocessorWrites[i] = -1;
			FT800EMU::Memory.clearDisplayListCoprocessorWrites();

			for (int i = FT800EMU_DL_SIZE - 1; i >= 0; --i)
			{
				if (s_DisplayListCoprocessorCommandWrite[i] >= 0)
				{
					s_UtilizationDisplayListCmd = i;
					break;
				}
			}

			// Test
			/*for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
			{
				if (s_DisplayListCoprocessorCommandWrite[i] >= 0)
				{
					std::string res;
					DlParser::toString(res, rd32(RAM_DL + (i * 4)));
					printf("DL %i was written by CMD %i: %s\n", i, s_DisplayListCoprocessorCommandWrite[i], res.c_str());
				}
			}*/

			swrbegin(RAM_CMD + (wp & 0xFFF));
			swr32(CMD_DLSTART);
			wp += 4;
			swrend();
			wr32(REG_CMD_WRITE, (wp & 0xFFF));

			coprocessorSwapped = true;
		}
		else
		{
			// Swap frame directly if nothing was written to the coprocessor
			s_UtilizationDisplayListCmd = 0;

			swrend();
			wr32(REG_CMD_WRITE, (wp & 0xFFF));

			wr32(REG_DLSWAP, DLSWAP_FRAME);
		}

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
}

void keyboard()
{

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
	m_RegistersDock(NULL), m_Macro(NULL), m_HSize(NULL), m_VSize(NULL),
	m_ControlsDock(NULL), m_StepEnabled(NULL), m_StepCount(NULL),
	m_TraceEnabled(NULL), m_TraceX(NULL), m_TraceY(NULL),
	m_FileMenu(NULL), m_EditMenu(NULL), m_ViewportMenu(NULL), m_WidgetsMenu(NULL),
#ifdef FT800EMU_PYTHON
	m_ScriptsMenu(NULL),
#endif
	m_HelpMenu(NULL),
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_NewAct(NULL), m_OpenAct(NULL), m_SaveAct(NULL), m_SaveAsAct(NULL),
	m_ImportAct(NULL), m_ExportAct(NULL),
	m_AboutAct(NULL), m_AboutQtAct(NULL), m_QuitAct(NULL), // m_PrintDebugAct(NULL),
	m_UndoAct(NULL), m_RedoAct(NULL), //, m_SaveScreenshotAct(NULL)
	m_TemporaryDir(NULL)
{
	setObjectName("MainWindow");

	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
	setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

	m_InitialWorkingDir = QDir::currentPath();
	m_UndoStack = new QUndoStack(this);

	QWidget *centralWidget = new QWidget(this);
	QVBoxLayout *cvLayout = new QVBoxLayout(this);
	QHBoxLayout *chLayout = new QHBoxLayout(this);
	chLayout->addStretch();
	m_EmulatorViewport = new InteractiveViewport(this);
	chLayout->addWidget(m_EmulatorViewport);
	chLayout->addStretch();
	cvLayout->addStretch();
	cvLayout->addLayout(chLayout);
	cvLayout->addStretch();
	centralWidget->setLayout(cvLayout);
    setCentralWidget(centralWidget);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();

	incbLanguageCode();

	s_DlEditor = m_DlEditor;
	s_CmdEditor = m_CmdEditor;
	s_Macro = m_Macro;

	FT800EMU::EmulatorParameters params;
	params.Setup = setup;
	params.Loop = loop;
	params.Flags =
		// FT800EMU::EmulatorEnableKeyboard
		/*|*/ FT800EMU::EmulatorEnableMouse
		//| FT800EMU::EmulatorEnableDebugShortkeys
		| FT800EMU::EmulatorEnableCoprocessor
		| FT800EMU::EmulatorEnableGraphicsMultithread
		| FT800EMU::EmulatorEnableDynamicDegrade
		;
	params.Keyboard = keyboard;
	s_EmulatorRunning = true;
	m_EmulatorViewport->run(params);

	FT800EMU::GraphicsProcessor.setDebugLimiter(2048 * 64);

	actNew();
}

MainWindow::~MainWindow()
{
	s_EmulatorRunning = false;
	m_EmulatorViewport->stop();

	s_DlEditor = NULL;
	s_CmdEditor = NULL;
	s_Macro = NULL;

	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = NULL;
}

void MainWindow::refreshScriptsMenu()
{
#ifdef FT800EMU_PYTHON
	//printf("Refresh scripts menu\n");
	QDir currentDir = m_InitialWorkingDir; //QDir::currentPath(); //currentDir(QCoreApplication::applicationDirPath());
	QStringList filters;
	filters.push_back("*.py");
	QStringList scriptFiles = currentDir.entryList(filters);
	std::map<QString, RunScript *> scriptActs;
	scriptActs.swap(m_ScriptActs);
	for (int i = 0; i < scriptFiles.size(); ++i)
	{
		//char *fileName = scriptFiles[i].toLatin1().data();
		//printf("Script: %s\n", fileName);
		if (scriptActs.find(scriptFiles[i]) == scriptActs.end())
		{
			RunScript *rs = new RunScript();
			QAction *act = new QAction(this);
			act->setText(scriptFiles[i]);
			act->setStatusTip(tr("Run Python script"));
			m_ScriptsMenu->addAction(act);
			rs->Action = act;
			rs->Script = scriptFiles[i];
			rs->Window = this,
			connect(act, SIGNAL(triggered()), rs, SLOT(runScript()));
			m_ScriptActs[scriptFiles[i]] = rs;
		}
		else
		{
			m_ScriptActs[scriptFiles[i]] = scriptActs[scriptFiles[i]];
			scriptActs.erase(scriptFiles[i]);
		}
	}
	for (std::map<QString, RunScript *>::iterator it = scriptActs.begin(), end = scriptActs.end(); it != end; ++it)
	{
		delete it->second;
	}
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
	QString scriptN = script.left(script.size() - 3);
	QByteArray scriptNa = scriptN.toLatin1();
	char *scriptName = scriptNa.data();
	statusBar()->showMessage(tr("Executed Python script '%1'").arg(scriptName));

	PyObject *pyScript = PyString_FromString(scriptName); // FIXME Unicode
	PyObject *pyModule = PyImport_Import(pyScript);
	Py_DECREF(pyScript); pyScript = NULL;

	bool error = true;

	if (pyModule != NULL)
	{
		/*PyObject *pyFunc = PyObject_GetAttrString(pyModule, "multiply");
		if (pyFunc && PyCallable_Check(pyFunc))
		{
			PyObject *pyValue;
			PyObject *pyArgs = PyTuple_New(2);
			pyValue = PyInt_FromLong(5);
			PyTuple_SetItem(pyArgs, 0, pyValue);
			pyValue = PyInt_FromLong(10);
			PyTuple_SetItem(pyArgs, 1, pyValue);
			pyValue = PyObject_CallObject(pyFunc, pyArgs);
			Py_DECREF(pyArgs);
			if (pyValue)
			{
				printf("Ok\n");
			}
			else
			{
				printf("Not ok\n");
			}
		}
		else
		{
			printf("Missing function\n");
		}

		Py_XDECREF(pyFunc); pyFunc = NULL;*/

		PyObject *pyImageConvClass = PyObject_GetAttrString(pyModule, "Image_Conv");
		if (pyImageConvClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			PyObject *pyImageConv = PyObject_CallObject(pyImageConvClass, pyArgs);
			Py_DECREF(pyImageConvClass); pyImageConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (pyImageConv)
			{
				PyObject *pyRunFunc = PyObject_GetAttrString(pyImageConv, "run");
				if (pyRunFunc)
				{
					PyObject *pyValue;
					PyObject *pyArgs = PyTuple_New(1);
					PyObject *pyTuple = PyTuple_New(6);
					pyValue = PyString_FromString("-i");
					PyTuple_SetItem(pyTuple, 0, pyValue);
					pyValue = PyString_FromString("/home/kaetemi/Downloads/blocks.jpg"); // FIXME Unicode
					PyTuple_SetItem(pyTuple, 1, pyValue);
					pyValue = PyString_FromString("-o");
					PyTuple_SetItem(pyTuple, 2, pyValue);
					pyValue = PyString_FromString("blocks_argb1555");
					PyTuple_SetItem(pyTuple, 3, pyValue);
					pyValue = PyString_FromString("-f");
					PyTuple_SetItem(pyTuple, 4, pyValue);
					pyValue = PyString_FromString("0");
					PyTuple_SetItem(pyTuple, 5, pyValue);
					PyTuple_SetItem(pyArgs, 0, pyTuple);
					PyObject *pyResult = PyObject_CallObject(pyRunFunc, pyArgs);
					Py_DECREF(pyRunFunc); pyRunFunc = NULL;
					Py_DECREF(pyArgs); pyArgs = NULL;
					if (pyResult)
					{
						printf("Ok\n");
						error = false;
					}
				}
			}
		}

		Py_DECREF(pyModule); pyModule = NULL;
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyErr_Print();
		printf("---\n");
	}
#endif /* FT800EMU_PYTHON */
}

void MainWindow::frameEmu()
{

}

void MainWindow::frameQt()
{
	int utilizationDisplayList = std::max(s_UtilizationDisplayListCmd, m_DlEditor->codeEditor()->document()->blockCount());
	m_UtilizationDisplayList->setValue(utilizationDisplayList);
}

void MainWindow::createActions()
{
	m_NewAct = new QAction(this);
	connect(m_NewAct, SIGNAL(triggered()), this, SLOT(actNew()));
	m_OpenAct = new QAction(this);
	connect(m_OpenAct, SIGNAL(triggered()), this, SLOT(actOpen()));

	m_SaveAct = new QAction(this);
	connect(m_SaveAct, SIGNAL(triggered()), this, SLOT(actSave()));
	m_SaveAsAct = new QAction(this);
	connect(m_SaveAsAct, SIGNAL(triggered()), this, SLOT(actSaveAs()));

	m_ImportAct = new QAction(this);
	connect(m_ImportAct, SIGNAL(triggered()), this, SLOT(actImport()));
	m_ExportAct = new QAction(this);
	connect(m_ExportAct, SIGNAL(triggered()), this, SLOT(actExport()));

	m_QuitAct = new QAction(this);
	m_QuitAct->setShortcuts(QKeySequence::Quit);
	connect(m_QuitAct, SIGNAL(triggered()), this, SLOT(close()));

	m_AboutAct = new QAction(this);
	connect(m_AboutAct, SIGNAL(triggered()), this, SLOT(about()));
	m_AboutQtAct = new QAction(this);
	connect(m_AboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

	//m_PrintDebugAct = new QAction(this);
	//connect(m_PrintDebugAct, SIGNAL(triggered()), this, SLOT(printDebug()));

	m_UndoAct = m_UndoStack->createUndoAction(this);
	m_UndoAct->setShortcuts(QKeySequence::Undo);
	m_RedoAct = m_UndoStack->createRedoAction(this);
	m_RedoAct->setShortcuts(QKeySequence::Redo);

	m_DummyAct = new QAction(this);
	connect(m_DummyAct, SIGNAL(triggered()), this, SLOT(dummyCommand()));

	// m_SaveScreenshotAct = m_EmulatorViewport->createSaveScreenshotAction(this);
}

void MainWindow::translateActions()
{
	m_NewAct->setText(tr("New"));
	m_NewAct->setStatusTip(tr("Create a new project"));
	m_OpenAct->setText(tr("Open"));
	m_OpenAct->setStatusTip(tr("Open an existing project"));
	m_SaveAct->setText(tr("Save"));
	m_SaveAct->setStatusTip(tr("Save the current project"));
	m_SaveAsAct->setText(tr("Save As"));
	m_SaveAsAct->setStatusTip(tr("Save the current project to a new file"));
	m_ImportAct->setText(tr("Import"));
	m_ImportAct->setStatusTip(tr("Import file to a new project"));
	m_ExportAct->setText(tr("Export"));
	m_ExportAct->setStatusTip(tr("Export project to file"));
	m_QuitAct->setText(tr("Quit"));
	m_QuitAct->setStatusTip(tr("Exit the application"));
	m_AboutAct->setText(tr("About"));
	m_AboutAct->setStatusTip(tr("Show information about the application"));
	m_AboutQtAct->setText(tr("About Qt"));
	m_AboutQtAct->setStatusTip(tr("Show information about the Qt libraries"));
	// m_PrintDebugAct->setText(tr("ActionPrintDebug"));
	// m_PrintDebugAct->setStatusTip(tr("ActionPrintDebugStatusTip"));
	m_UndoAct->setText(tr("Undo"));
	m_UndoAct->setStatusTip(tr("Reverses the last action"));
	m_RedoAct->setText(tr("Redo"));
	m_RedoAct->setStatusTip(tr("Reapply the action"));
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
	m_FileMenu->addAction(m_ImportAct);
	m_FileMenu->addAction(m_ExportAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_QuitAct);

	m_EditMenu = menuBar()->addMenu(QString::null);
	m_EditMenu->addAction(m_UndoAct);
	m_EditMenu->addAction(m_RedoAct);
	//m_EditMenu->addAction(m_DummyAct);

	//m_ViewportMenu = menuBar()->addMenu(QString::null);
	// m_ViewportMenu->addAction(m_SaveScreenshotAct);

	m_WidgetsMenu = menuBar()->addMenu(QString::null);

#ifdef FT800EMU_PYTHON
	m_ScriptsMenu = menuBar()->addMenu(QString::null);
	connect(m_ScriptsMenu, SIGNAL(aboutToShow()), this, SLOT(refreshScriptsMenu()));
#endif /* FT800EMU_PYTHON */

	menuBar()->addSeparator();

	m_HelpMenu = menuBar()->addMenu(QString::null);
	m_HelpMenu->addAction(m_AboutAct);
	m_HelpMenu->addAction(m_AboutQtAct);
}

void MainWindow::translateMenus()
{
	m_FileMenu->setTitle(tr("File"));
	m_EditMenu->setTitle(tr("Edit"));
	//m_ViewportMenu->setTitle(tr("Viewport"));
	m_WidgetsMenu->setTitle(tr("View"));
#ifdef FT800EMU_PYTHON
	m_ScriptsMenu->setTitle(tr("Scripts"));
#endif /* FT800EMU_PYTHON */
	m_HelpMenu->setTitle(tr("Help"));
}

void MainWindow::createToolBars()
{
	//m_FileToolBar = addToolBar(QString::null);
	//m_FileToolBar->addAction(m_QuitAct);
	// m_FileToolBar->addAction(m_PrintDebugAct);

	//m_EditToolBar = addToolBar(QString::null);
	//m_EditToolBar->addAction(m_AboutAct);
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

	// Utilization
	{
		m_UtilizationDock = new QDockWidget(this);
		m_UtilizationDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_UtilizationDock->setObjectName("Utilization");
		QScrollArea *scrollArea = new QScrollArea(this);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		scrollArea->setMinimumWidth(240);
		QWidget *widget = new QWidget(this);
		QVBoxLayout *layout = new QVBoxLayout(widget);

		// Display List
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Display List"));
			QVBoxLayout *groupLayout = new QVBoxLayout(widget);

			m_UtilizationDisplayList = new QProgressBar(this);
			m_UtilizationDisplayList->setMinimum(0);
			m_UtilizationDisplayList->setMaximum(FT800EMU_DL_SIZE);
			groupLayout->addWidget(m_UtilizationDisplayList);

			//QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 ); }";
			//QString safe= "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #78d,stop: 0.4999 #46a,stop: 0.5 #45a,stop: 1 #238 ); }";
			QPalette p = m_UtilizationDisplayList->palette();
			/*p.setColor(QPalette::Active, QColorGroup::Highlight, Qt::red);
			p.setColor(QPalette::Inactive, QColorGroup::Highlight, Qt::red);
			p.setColor(QPalette::Disabled, QColorGroup::Highlight, Qt::red);*/
			p.setColor(QPalette::Highlight, Qt::red);
			m_UtilizationDisplayList->setPalette(p);
			//m_UtilizationDisplayList->setStyleSheet(danger);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		layout->addStretch();
		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_UtilizationDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_UtilizationDock);
		m_WidgetsMenu->addAction(m_UtilizationDock->toggleViewAction());
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
		QVBoxLayout *layout = new QVBoxLayout(widget);

		// Step
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Display List Steps"));
			QHBoxLayout *groupLayout = new QHBoxLayout(widget);

			m_StepEnabled = new QCheckBox(this);
			m_StepEnabled->setChecked(false);
			connect(m_StepEnabled, SIGNAL(toggled(bool)), this, SLOT(stepEnabled(bool)));
			groupLayout->addWidget(m_StepEnabled);
			m_StepCount = new QSpinBox(this);
			m_StepCount->setMinimum(1);
			m_StepCount->setMaximum(2048 * 64);
			m_StepCount->setEnabled(false);
			connect(m_StepCount, SIGNAL(valueChanged(int)), this, SLOT(stepChanged(int)));
			groupLayout->addWidget(m_StepCount);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		// Trace
		{
			QGroupBox *group = new QGroupBox(widget);
			group->setTitle(tr("Trace"));
			QHBoxLayout *groupLayout = new QHBoxLayout(widget);

			m_TraceEnabled = new QCheckBox(this);
			m_TraceEnabled->setChecked(false);
			connect(m_TraceEnabled, SIGNAL(toggled(bool)), this, SLOT(traceEnabledChanged(bool)));
			groupLayout->addWidget(m_TraceEnabled);
			m_TraceX = new QSpinBox(this);
			m_TraceX->setMinimum(0);
			m_TraceX->setMaximum(511);
			m_TraceX->setEnabled(false);
			groupLayout->addWidget(m_TraceX);
			m_TraceY = new QSpinBox(this);
			m_TraceY->setMinimum(0);
			m_TraceY->setMaximum(511);
			m_TraceY->setEnabled(false);
			groupLayout->addWidget(m_TraceY);

			group->setLayout(groupLayout);
			layout->addWidget(group);
		}

		layout->addStretch();
		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_ControlsDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ControlsDock);
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
		QVBoxLayout *layout = new QVBoxLayout(widget);

		// Size
		{
			QGroupBox *sizeGroup = new QGroupBox(widget);
			sizeGroup->setTitle(tr("Size"));
			QVBoxLayout *sizeLayout = new QVBoxLayout(widget);

			m_HSize = new QSpinBox(widget);
			m_HSize->setMinimum(1);
			m_HSize->setMaximum(512);
			connect(m_HSize, SIGNAL(valueChanged(int)), this, SLOT(hsizeChanged(int)));
			QHBoxLayout *hsizeLayout = new QHBoxLayout(widget);
			QLabel *hsizeLabel = new QLabel(widget);
			hsizeLabel->setText(tr("Horizontal"));
			hsizeLayout->addWidget(hsizeLabel);
			hsizeLayout->addWidget(m_HSize);
			sizeLayout->addLayout(hsizeLayout);

			m_VSize = new QSpinBox(widget);
			m_VSize->setMinimum(1);
			m_VSize->setMaximum(512);
			connect(m_VSize, SIGNAL(valueChanged(int)), this, SLOT(vsizeChanged(int)));
			QHBoxLayout *vsizeLayout = new QHBoxLayout(widget);
			QLabel *vsizeLabel = new QLabel(widget);
			vsizeLabel->setText(tr("Vertical"));
			vsizeLayout->addWidget(vsizeLabel);
			vsizeLayout->addWidget(m_VSize);
			sizeLayout->addLayout(vsizeLayout);

			sizeGroup->setLayout(sizeLayout);
			layout->addWidget(sizeGroup);
		}

		// Macro
		{
			QGroupBox *macroGroup = new QGroupBox(widget);
			macroGroup->setTitle(tr("Macro"));
			QVBoxLayout *macroLayout = new QVBoxLayout(widget);

			m_Macro = new DlEditor(this);
			m_Macro->setPropertiesEditor(m_PropertiesEditor);
			m_Macro->setUndoStack(m_UndoStack);
			m_Macro->setModeMacro();
			//QHBoxLayout *macroLayout = new QHBoxLayout(widget);
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
		scrollArea->setWidget(m_ContentManager);
		m_ContentManagerDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ContentManagerDock);
		m_WidgetsMenu->addAction(m_ContentManagerDock->toggleViewAction());
	}

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

	tabifyDockWidget(m_ControlsDock, m_RegistersDock);
	tabifyDockWidget(m_RegistersDock, m_ContentManagerDock);
	tabifyDockWidget(m_ContentManagerDock, m_ToolboxDock);

#if FT800_DEVICE_MANAGER
	tabifyDockWidget(m_DeviceManagerDock, m_UtilizationDock);
#endif /* FT800_DEVICE_MANAGER */
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
}

void MainWindow::translateDockWindows()
{
	m_InspectorDock->setWindowTitle(tr("Inspector"));
	m_DlEditorDock->setWindowTitle(tr("Display List"));
	m_CmdEditorDock->setWindowTitle(tr("Coprocessor"));
#if FT800_DEVICE_MANAGER
	m_DeviceManagerDock->setWindowTitle(tr("Devices"));
#endif /* FT800_DEVICE_MANAGER */
	m_UtilizationDock->setWindowTitle(tr("Utilization"));
	m_PropertiesEditorDock->setWindowTitle(tr("Properties"));
	m_ToolboxDock->setWindowTitle(tr("Toolbox"));
	m_ContentManagerDock->setWindowTitle(tr("Content"));
	m_RegistersDock->setWindowTitle(tr("Registers"));
	m_ControlsDock->setWindowTitle(tr("Controls"));
}

void MainWindow::incbLanguageCode()
{
	setWindowTitle(tr("FT800 Editor"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
}

void MainWindow::editorTabChanged(int i)
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
		if (std::find(m_HookedTabs.begin(), m_HookedTabs.end(), tabBar) == m_HookedTabs.end())
		{
			connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(editorTabChanged(int)));
			connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(editorTabChanged(int))); // this is not working FIXME
			m_HookedTabs.push_back(tabBar);
		}
		for (int j = 0; j < tabBar->count(); ++j)
		{
			QDockWidget *dw = reinterpret_cast<QDockWidget *>(qvariant_cast<quintptr>(tabBar->tabData(j)));
			if (dw == m_CmdEditorDock)
			{
				cmdExist = true;
				if (tabBar->currentIndex() != j)
					cmdTop = false;
			}
			else if (dw == m_DlEditorDock)
			{
				dlExist = true;
				if (tabBar->currentIndex() != j)
					dlTop = false;
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
		m_EmulatorViewport->unsetEditorLine();
		m_Toolbox->unsetEditorLine();
		setFocus(Qt::OtherFocusReason);
	}
}

void MainWindow::tabChanged(int i)
{
	// Defocus editor
	// setFocus(Qt::OtherFocusReason);
}

void MainWindow::focusDlEditor()
{
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

void MainWindow::hsizeChanged(int hsize)
{
	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new HSizeCommand(hsize, m_HSize));
}

void MainWindow::vsizeChanged(int vsize)
{
	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new VSizeCommand(vsize, m_VSize));
}

void MainWindow::stepEnabled(bool enabled)
{
	m_StepCount->setEnabled(enabled);
	if (enabled)
	{
		FT800EMU::GraphicsProcessor.setDebugLimiter(m_StepCount->value());
		FT800EMU::Memory.poke();
	}
	else
	{
		FT800EMU::GraphicsProcessor.setDebugLimiter(2048 * 64);
		FT800EMU::Memory.poke();
	}
}

void MainWindow::stepChanged(int step)
{
	if (m_StepEnabled->isChecked())
	{
		FT800EMU::GraphicsProcessor.setDebugLimiter(step);
		FT800EMU::Memory.poke();
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

void MainWindow::clearEditor()
{
	m_HSize->setValue(FT800EMU_WINDOW_WIDTH_DEFAULT);
	m_VSize->setValue(FT800EMU_WINDOW_HEIGHT_DEFAULT);
	m_StepEnabled->setChecked(false);
	m_StepCount->setValue(1);
	setTraceEnabled(false);
	setTraceX(0);
	setTraceY(0);
	m_DlEditor->clear();
	m_CmdEditor->clear();
	m_Macro->clear();
	m_ContentManager->clear();
}

void MainWindow::clearUndoStack()
{
	m_UndoStack->clear();
	m_DlEditor->clearUndoStack();
	m_CmdEditor->clearUndoStack();
	m_Macro->clearUndoStack();
}

void MainWindow::actNew()
{
	printf("** New **\n");

	// reset filename
	m_CurrentFile = QString();

	// reset editors to their default state
	clearEditor();

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusCmdEditor();
	m_PropertiesEditor->setInfo(tr("Start typing in the <b>Coprocessor</b> editor."));
	m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	m_Toolbox->setEditorLine(m_CmdEditor, 0);

	// set working directory to temporary directory // FIXME: tempPath()
	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = new QTemporaryDir("ft800editor-");
	QDir::setCurrent(m_TemporaryDir->path());
	printf("Current path: %s\n", QDir::currentPath().toUtf8().data());
}

void documentFromJsonArray(QPlainTextEdit *textEditor, const QJsonArray &array)
{
	bool firstLine = true;
	for (int i = 0; i < array.size(); ++i)
	{
		if (firstLine) firstLine = false;
		else textEditor->textCursor().insertText("\n");
		textEditor->textCursor().insertText(array[i].toString());
	}
}

QString MainWindow::getFileDialogPath()
{
	return m_TemporaryDir ? m_InitialWorkingDir : QDir::currentPath();
}

void MainWindow::actOpen()
{
	printf("*** Open ***\n");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), getFileDialogPath(),
		tr("FT800 Editor Project, *.ft800proj (*.ft800proj)"));
	if (fileName.isNull())
		return;

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
		QJsonObject registers = root["registers"].toObject();
		m_HSize->setValue(((QJsonValue)registers["hSize"]).toVariant().toInt());
		m_VSize->setValue(((QJsonValue)registers["vSize"]).toVariant().toInt());
		documentFromJsonArray(m_Macro->codeEditor(), registers["macro"].toArray());
		documentFromJsonArray(m_DlEditor->codeEditor(), root["displayList"].toArray());
		documentFromJsonArray(m_CmdEditor->codeEditor(), root["coprocessor"].toArray());
		statusBar()->showMessage(tr("Opened FT800 Editor project"));
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
		QDir::setCurrent(QDir::tempPath());
		m_TemporaryDir = new QTemporaryDir("ft800editor-");
		QDir::setCurrent(m_TemporaryDir->path());
	}

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusCmdEditor();
	m_PropertiesEditor->setInfo(tr("Start typing in the <b>Coprocessor</b> editor."));
	m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	m_Toolbox->setEditorLine(m_CmdEditor, 0);
	printf("Current path: %s\n", QDir::currentPath().toUtf8().data());
}

QJsonArray documentToJsonArray(const QTextDocument *textDocument)
{
	QJsonArray result;
	for (int i = 0; i < textDocument->blockCount(); ++i)
	{
		result.push_back(textDocument->findBlockByNumber(i).text());
	}
	return result;
}

void MainWindow::actSave()
{
	if (m_TemporaryDir) { actSaveAs(); return; }

	QFile file(m_CurrentFile);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	QJsonObject root;
	QJsonObject registers;
	registers["hSize"] = s_HSize;
	registers["vSize"] = s_VSize;
	registers["macro"] = documentToJsonArray(m_Macro->codeEditor()->document());
	root["registers"] = registers;
	root["displayList"] = documentToJsonArray(m_DlEditor->codeEditor()->document());
	root["coprocessor"] = documentToJsonArray(m_CmdEditor->codeEditor()->document());
	QJsonDocument doc(root);

	QByteArray data = doc.toJson();
	out.writeRawData(data, data.size());
}

void MainWindow::actSaveAs()
{
	QString filterft800proj = tr("FT800 Editor Project, *.ft800proj (*.ft800proj)");
	QString filter = filterft800proj;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project"), getFileDialogPath(), filter, &filter);
	if (fileName.isNull())
		return;

	if (filter == filterft800proj)
	{
		if (!fileName.endsWith(".ft800proj"))
			fileName = fileName + ".ft800proj";
	}

	// Copy asset files, abort if already exists (check first)
	QDir dir(fileName);
	dir.cdUp();
	QString dstPath = dir.path();
	QString srcPath = QDir::currentPath();
	if (dstPath != srcPath)
	{
		// printf("Path: %s\n", dirPath.toUtf8().data());
		// (copy assets from srcPath to dstPath)
	}

	if (m_TemporaryDir)
	{
		// Delete temporary directory
		QDir::setCurrent(QDir::tempPath());
		delete m_TemporaryDir; m_TemporaryDir = NULL;
	}

	// Set the folder to be the project folder
	QDir::setCurrent(dstPath);

	// Save the project itself
	m_CurrentFile = fileName;
	actSave();
}

void MainWindow::actImport()
{
	printf("*** Import ***\n");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), getFileDialogPath(),
		tr("Memory dump, *.vc1dump (*.vc1dump)"));
	if (fileName.isNull())
		return;

	m_CurrentFile = QString();

	// reset editors to their default state
	clearEditor();

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
		if (header[0] != 100)
		{
			QString message;
			message.sprintf(tr("Invalid header version: %i").toUtf8().constData(), header[0]);
			QMessageBox::critical(this, tr("Import .vc1dump"), message);
		}
		else
		{
			if (s != sizeof(uint32_t) * headersz)
			{
				QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete header"));
			}
			else
			{
				m_HSize->setValue(header[1]);
				m_VSize->setValue(header[2]);
				m_Macro->lockDisplayList();
				m_Macro->getDisplayList()[0] = header[3];
				m_Macro->getDisplayList()[1] = header[4];
				m_Macro->reloadDisplayList(false);
				m_Macro->unlockDisplayList();
				char *ram = static_cast<char *>(static_cast<void *>(FT800EMU::Memory.getRam()));
				s = in.readRawData(&ram[RAM_G], 262144); // FIXME_GUI GLOBAL MEMORY
				if (s != 262144) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_G"));
				else
				{
					s = in.readRawData(&ram[RAM_PAL], 1024); // FIXME_GUI PALETTE
					if (s != 1024) QMessageBox::critical(this, tr("Import .vc1dump"), tr("Incomplete RAM_PAL"));
					else
					{
						m_DlEditor->lockDisplayList();
						s = in.readRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FT800EMU_DL_SIZE * sizeof(uint32_t));
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
							focusDlEditor();
						}
					}
				}
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

	// set working directory to temporary directory // FIXME: tempPath()
	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = new QTemporaryDir("ft800editor-");
	QDir::setCurrent(m_TemporaryDir->path());
	printf("Current path: %s\n", QDir::currentPath().toUtf8().data());
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
		char *ram = static_cast<char *>(static_cast<void *>(FT800EMU::Memory.getRam()));
		int s = out.writeRawData(static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz);
		if (s != sizeof(uint32_t) * headersz) goto ExportWriteError;
		s = out.writeRawData(&ram[RAM_G], 262144); // FIXME_GUI GLOBAL MEMORY
		if (s != 262144) goto ExportWriteError;
		s = out.writeRawData(&ram[RAM_PAL], 1024); // FIXME_GUI PALETTE
		if (s != 1024) goto ExportWriteError;
		m_DlEditor->lockDisplayList();
		// s = out.writeRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FT800EMU_DL_SIZE * sizeof(uint32_t));
		s = out.writeRawData(static_cast<const char *>(static_cast<const void *>(FT800EMU::Memory.getDisplayList())), FT800EMU_DL_SIZE * sizeof(uint32_t));
		m_DlEditor->unlockDisplayList();
		if (s != FT800EMU_DL_SIZE * sizeof(uint32_t)) goto ExportWriteError;
		statusBar()->showMessage(tr("Exported project to .vc1dump file"));
	}

	m_PropertiesEditor->setInfo(tr("Exported project to .vc1dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);

	return;
ExportWriteError:
	QMessageBox::critical(this, tr("Export"), tr("Failed to write file"));
}

void MainWindow::dummyCommand()
{
	printf("!!!!!!!! dummy action !!!!!!!!!!!!!!!!\n");
	m_UndoStack->push(new QUndoCommand());
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About FT800 Editor"), tr("Copyright (C) 2013-2014  Future Technology Devices International Ltd"));
}

void MainWindow::aboutQt()
{
	QMessageBox::about(this, tr("About Qt"), tr(
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
		"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."));
}

} /* namespace FT800EMUQT */

/* end of file */
