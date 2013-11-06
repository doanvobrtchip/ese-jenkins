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

#include "main_window.h"

// STL includes

// Qt includes
#include <QtGui>
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

// Emulator includes

// Project includes
#include "dl_editor.h"
// #include "command_log.h"
#include "emulator_viewport.h"
// #include "emulator_config.h"

#include <ft800emu_keyboard_keys.h>
#include <ft800emu_emulator.h>
#include <ft800emu_keyboard.h>
#include <ft800emu_system.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <stdio.h>
#include <vc.h>

namespace FT800EMUQT {

#define FT800EMU_XBU_FILE "../reference/xbu/BIRDS.XBU"

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

static FILE *s_F = NULL;

void setup()
{
	s_F = fopen(FT800EMU_XBU_FILE, "rb");
	if (!s_F) printf("Failed to open XBU file\n");
	else
	{
		printf("Load XBU\n");

		wr32(REG_HSIZE, 480);
		wr32(REG_VSIZE, 272);
		wr32(REG_PCLK, 5);
		// if (fclose(s_F)) printf("Error closing vc1dump file\n");

		
		/*swrbegin(RAM_CMD);
		swr32(CMD_DLSTART);
		swr32(CMD_SWAP);
		swr32(CMD_DLSTART);
		swr32(CLEAR_COLOR_RGB(0, 32, 64));
		swr32(CLEAR(1, 1, 1));
		swr32(TAG(1));
		swr32(CMD_BUTTON);
		swr16(10);
		swr16(10);
		swr16(160);
		swr16(24);
		swr16(26);
		swr16(0);
		swr8('B');
		swr8('a');
		swr8('r');
		swr8(0);
		swr32(CMD_SWAP);
		swrend();

		wr32(REG_CMD_WRITE, (6 * 4) + (4 * 4) + 4 + 4);*/
	}
}

static int wp = 0;
static int wpr = 0;

static bool okwrite = false;

static int lastround = 0;

void loop()
{
	if (!s_F)
	{
		FT800EMU::System.delay(10);
	}
	else
	{
		int regwp = rd32(REG_CMD_WRITE);
		int rp = rd32(REG_CMD_READ);
		int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
		int freespace = ((4096 - 4) - fullness);

		// printf("rp: %i, wp: %i, wpr: %i, regwp: %i\n", rp, wp, wpr, regwp);
		if (rp == -1)
		{
			printf("rp < 0, error\n");
			printf("Close XBU\n");
			if (fclose(s_F)) printf("Error closing vc1dump file\n");
			s_F = NULL;
		}
		else
		{
			if (freespace)
			// if (freespace >= 2048)
			{
				int freespacediv = freespace >> 2;
				
				swrbegin(RAM_CMD + (wp & 0xFFF));
				for (int i = 0; i < freespacediv; ++i)
				{
					uint32_t buffer;
					size_t nb = fread(&buffer, 4, 1, s_F);
					if (nb == 1)
					{
						swr32(buffer);
						wp += 4;

						if (buffer == CMD_SWAP)
						{
							wpr = wp;
							swrend();
							wr32(REG_CMD_WRITE, (wpr & 0xFFF));
							swrbegin(RAM_CMD + (wp & 0xFFF));
						}
					}
					else
					{
						printf("Close XBU, nb = %i\n", nb);
						if (fclose(s_F)) printf("Error closing vc1dump file\n");
						s_F = NULL;
						break;
					}
				}
				swrend();

				if (s_F)
				{
					int wprn = (wp - 128);
					if (wprn > wpr)
					{
						wpr = wprn;
						wr32(REG_CMD_WRITE, (wpr & 0xFFF));
					}
				}
				else
				{
					wpr = wp;
					wr32(REG_CMD_WRITE, (wpr & 0xFFF));
				}
			}
			else
			{
				// FT800EMU::System.delay(1000);
			}
			int newround = wpr / 4096;
			if (lastround != newround)
			{
				// printf("new round\n");
				lastround = newround;
			}
		}
	}
}

void keyboard()
{

}

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL), 
	m_EmulatorViewport(NULL), 
	m_DlEditor(NULL), m_DlEditorDock(NULL), 
	// m_EmulatorConfig(NULL), m_EmulatorConfigScroll(NULL), m_EmulatorConfigDock(NULL), 
	m_FileMenu(NULL), m_EditMenu(NULL), m_ViewportMenu(NULL), m_WidgetsMenu(NULL), m_HelpMenu(NULL), 
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_AboutAct(NULL), m_QuitAct(NULL), // m_PrintDebugAct(NULL), 
	m_UndoAct(NULL), m_RedoAct(NULL) //, m_SaveScreenshotAct(NULL)
{
	setObjectName("MainWindow");
	
	m_UndoStack = new QUndoStack(this);
	
	m_EmulatorViewport = new EmulatorViewport(this);
    setCentralWidget(m_EmulatorViewport);
	
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();
	
	incbLanguageCode();

	recalculateMinimumWidth();
	
	// connect(m_EmulatorConfig, SIGNAL(applyEmulatorConfig()), this, SLOT(applyEmulatorConfig()));
	
	FT800EMU::EmulatorParameters params;
	params.Setup = setup;
	params.Loop = loop;
	params.Flags = 
		FT800EMU::EmulatorEnableKeyboard 
		| FT800EMU::EmulatorEnableMouse 
		| FT800EMU::EmulatorEnableDebugShortkeys
		| FT800EMU::EmulatorEnableCoprocessor 
		| FT800EMU::EmulatorEnableGraphicsMultithread
		//| FT800EMU::EmulatorEnableDynamicDegrade
		;
	params.Keyboard = keyboard;
	m_EmulatorViewport->run(params);
}

MainWindow::~MainWindow()
{
	// delete m_EmulatorConfig; m_EmulatorConfig = NULL;
}

void MainWindow::createActions()
{
	m_QuitAct = new QAction(this);
	m_QuitAct->setShortcuts(QKeySequence::Quit);	
	connect(m_QuitAct, SIGNAL(triggered()), this, SLOT(close()));
	
	m_AboutAct = new QAction(this);
	connect(m_AboutAct, SIGNAL(triggered()), this, SLOT(about()));

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
	m_QuitAct->setText(tr("ActionQuit"));
	m_QuitAct->setStatusTip(tr("ActionQuitStatusTip"));
	m_AboutAct->setText(tr("ActionAbout"));
	m_AboutAct->setStatusTip(tr("ActionAboutStatusTip"));
	// m_PrintDebugAct->setText(tr("ActionPrintDebug"));
	// m_PrintDebugAct->setStatusTip(tr("ActionPrintDebugStatusTip"));
	m_UndoAct->setText(tr("ActionUndo"));
	m_UndoAct->setStatusTip(tr("ActionUndoStatusTip"));
	m_RedoAct->setText(tr("ActionRedo"));
	m_RedoAct->setStatusTip(tr("ActionRedoStatusTip"));
	m_DummyAct->setText(tr("ActionDummy"));
	m_RedoAct->setStatusTip(tr("ActionDummyStatusTip"));
	// m_SaveScreenshotAct->setText(tr("ActionSaveScreenshot"));
	// m_SaveScreenshotAct->setStatusTip(tr("ActionSaveScreenshotStatusTip"));
}

void MainWindow::createMenus()
{
	m_FileMenu = menuBar()->addMenu(QString::null);
	//m_FileMenu->addAction(saveAct);
	//m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_QuitAct);

	m_EditMenu = menuBar()->addMenu(QString::null);
	m_EditMenu->addAction(m_UndoAct);
	m_EditMenu->addAction(m_RedoAct);
	m_EditMenu->addAction(m_DummyAct);

	m_ViewportMenu = menuBar()->addMenu(QString::null);
	// m_ViewportMenu->addAction(m_SaveScreenshotAct);
	
	m_WidgetsMenu = menuBar()->addMenu(QString::null);
	
	menuBar()->addSeparator();
	
	m_HelpMenu = menuBar()->addMenu(QString::null);
	m_HelpMenu->addAction(m_AboutAct);
}

void MainWindow::translateMenus()
{
	m_FileMenu->setTitle(tr("MenuFile"));
	m_EditMenu->setTitle(tr("MenuEdit"));
	m_ViewportMenu->setTitle(tr("MenuViewport"));
	m_WidgetsMenu->setTitle(tr("MenuWidgets"));
	m_HelpMenu->setTitle(tr("MenuHelp"));
}

void MainWindow::createToolBars()
{
	m_FileToolBar = addToolBar(QString::null);
	m_FileToolBar->addAction(m_QuitAct);
	// m_FileToolBar->addAction(m_PrintDebugAct);

	m_EditToolBar = addToolBar(QString::null);
	m_EditToolBar->addAction(m_AboutAct);
}

void MainWindow::translateToolBars()
{
	m_FileToolBar->setWindowTitle(tr("BarFile"));
	m_EditToolBar->setWindowTitle(tr("BarEdit"));
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("StatusReady"));
}

void MainWindow::createDockWindows()
{
	// DlEditor (Console)
	{
		m_DlEditorDock = new QDockWidget(this);
		m_DlEditorDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_DlEditor = new DlEditor(m_DlEditorDock);
		m_DlEditor->setUndoStack(m_UndoStack);
		m_DlEditorDock->setWidget(m_DlEditor);
		addDockWidget(Qt::BottomDockWidgetArea, m_DlEditorDock);
		m_WidgetsMenu->addAction(m_DlEditorDock->toggleViewAction());
	}

	// EmulatorConfig (Emulator Configuration)
	/*{
		m_EmulatorConfigDock = new QDockWidget(this);
		m_EmulatorConfigDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_EmulatorConfigScroll = new QScrollArea();
		m_EmulatorConfigScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_EmulatorConfigScroll->setWidgetResizable(true);
		m_EmulatorConfig = new CEmulatorConfig(m_EmulatorConfigDock, &m_Configuration, &m_Internationalization, m_UndoStack);
		m_EmulatorConfigScroll->setWidget(m_EmulatorConfig);
		m_EmulatorConfigDock->setWidget(m_EmulatorConfigScroll);
		addDockWidget(Qt::RightDockWidgetArea, m_EmulatorConfigDock);
		m_WidgetsMenu->addAction(m_EmulatorConfigDock->toggleViewAction());
	}*/

	// AssetTree (Assets)
	{
		m_AssetTreeDock = new QDockWidget(this);
		m_AssetTreeDock->setAllowedAreas(Qt::AllDockWidgetAreas);
		m_AssetTreeView = new QTreeView(m_AssetTreeDock);
		m_AssetTreeModel = new QDirModel();
		m_AssetTreeView->setModel(m_AssetTreeModel);
		m_AssetTreeView->setSortingEnabled(true);
		m_AssetTreeDock->setWidget(m_AssetTreeView);
		addDockWidget(Qt::LeftDockWidgetArea, m_AssetTreeDock);
		m_WidgetsMenu->addAction(m_AssetTreeDock->toggleViewAction());
	}
}

void MainWindow::translateDockWindows()
{
	m_DlEditorDock->setWindowTitle(tr("WidgetDlEditor"));
	//m_EmulatorConfigDock->setWindowTitle(tr("WidgetEmulatorConfig"));
	m_AssetTreeDock->setWindowTitle(tr("WidgetAssetTree"));
}

void MainWindow::recalculateMinimumWidth()
{
	//if (m_EmulatorConfigScroll) 
	//	m_EmulatorConfigScroll->setMinimumWidth(m_EmulatorConfig->minimumSizeHint().width() + m_EmulatorConfigScroll->minimumSizeHint().width());
}

void MainWindow::incbLanguageCode()
{
	setWindowTitle(tr("WindowTitle"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
	recalculateMinimumWidth();
}

void MainWindow::dummyCommand()
{
	printf("!!!!!!!! dummy action !!!!!!!!!!!!!!!!\n");
	m_UndoStack->push(new QUndoCommand());
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("AboutTitle"), tr("AboutText"));
}

} /* namespace FT800EMUQT */

/* end of file */
