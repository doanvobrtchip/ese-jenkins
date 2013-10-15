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
#include "command_log.h"
#include "emulator_viewport.h"
// #include "emulator_config.h"

namespace FTQT {

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL), 
	m_EmulatorViewport(NULL), 
	m_CommandLog(NULL), m_CommandLogDock(NULL), 
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
	// CommandLog (Console)
	{
		m_CommandLogDock = new QDockWidget(this);
		m_CommandLogDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_CommandLog = new CommandLog(m_CommandLogDock);
		m_CommandLogDock->setWidget(m_CommandLog);
		addDockWidget(Qt::BottomDockWidgetArea, m_CommandLogDock);
		m_WidgetsMenu->addAction(m_CommandLogDock->toggleViewAction());
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
	m_CommandLogDock->setWindowTitle(tr("WidgetCommandLog"));
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

void MainWindow::about()
{
	QMessageBox::about(this, tr("AboutTitle"), tr("AboutText"));
}

} /* namespace FTQT */

/* end of file */
