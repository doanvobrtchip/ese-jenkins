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
#include <stdio.h>

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
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>

// Emulator includes
#define NOMINMAX
#include <ft800emu_inttypes.h>
#include <ft800emu_keyboard_keys.h>
#include <ft800emu_emulator.h>
#include <ft800emu_keyboard.h>
#include <ft800emu_system.h>
#include <ft800emu_memory.h>
#include <ft800emu_spi_i2c.h>
#include <ft800emu_graphics_processor.h>
#include <ft800emu_graphics_driver.h>
#include <vc.h>

// Project includes
#include "dl_editor.h"
// #include "command_log.h"
#include "emulator_viewport.h"
// #include "emulator_config.h"
#include "properties_editor.h"

namespace FT800EMUQT {

#define FT800EMU_XBU_FILE "../reference/xbu/BIRDS.XBU"

int s_HSize = FT800EMU_WINDOW_WIDTH_DEFAULT;
int s_VSize = FT800EMU_WINDOW_HEIGHT_DEFAULT;

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
static DlEditor *s_Macro = NULL;
// static FILE *s_F = NULL;

void setup()
{
	wr32(REG_HSIZE, 480);
	wr32(REG_VSIZE, 272);
	wr32(REG_PCLK, 5);
}

static bool displayListSwapped = false;
void loop()
{
	// wait
	if (displayListSwapped)
	{
		// dl swap wait
		while (rd32(REG_DLSWAP) != DLSWAP_DONE) { /* do nothing */ }
		displayListSwapped = false;
	}
	else
	{
		FT800EMU::System.delay(10);
	}
	
	// switch to next resolution
	wr32(REG_HSIZE, s_HSize);
	wr32(REG_VSIZE, s_VSize);
	// switch to next macro list
	s_Macro->lockDisplayList();
	if (s_Macro->isDisplayListModified())
	{
		wr32(REG_MACRO_0, s_Macro->getDisplayList()[0]);
		wr32(REG_MACRO_1, s_Macro->getDisplayList()[1]);
	}
	s_Macro->unlockDisplayList();
	// switch to next display list
	s_DlEditor->lockDisplayList();
	if (s_DlEditor->isDisplayListModified())
	{
		uint32_t *displayList = s_DlEditor->getDisplayList();
		swrbegin(RAM_DL);
		for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
			swr32(displayList[i]);
		swrend();
		wr32(REG_DLSWAP, DLSWAP_FRAME);
		displayListSwapped = true;
	}
	s_DlEditor->unlockDisplayList();
}

void keyboard()
{

}

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL), 
	m_EmulatorViewport(NULL), 
	m_DlEditor(NULL), m_DlEditorDock(NULL), 
	m_PropertiesEditor(NULL), m_PropertiesEditorScroll(NULL), m_PropertiesEditorDock(NULL), 
	m_RegistersDock(NULL), m_Macro(NULL), m_HSize(NULL), m_VSize(NULL), 
	m_ControlsDock(NULL), m_StepEnabled(NULL), m_StepCount(NULL), 
	m_FileMenu(NULL), m_EditMenu(NULL), m_ViewportMenu(NULL), m_WidgetsMenu(NULL), m_HelpMenu(NULL), 
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_NewAct(NULL), m_OpenAct(NULL), m_SaveAct(NULL), m_SaveAsAct(NULL), 
	m_ImportAct(NULL), m_ExportAct(NULL), 
	m_AboutAct(NULL), m_AboutQtAct(NULL), m_QuitAct(NULL), // m_PrintDebugAct(NULL), 
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
	
	s_DlEditor = m_DlEditor;
	s_Macro = m_Macro;
	
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
	
	FT800EMU::GraphicsProcessor.setDebugLimiter(2048 * 64);
	
	actNew();
}

MainWindow::~MainWindow()
{
	s_DlEditor = NULL;
}

void MainWindow::createActions()
{
	m_NewAct = new QAction(this);
	connect(m_NewAct, SIGNAL(triggered()), this, SLOT(actNew()));
	m_OpenAct = new QAction(this);
	connect(m_OpenAct, SIGNAL(triggered()), this, SLOT(actOpen()));
	m_OpenAct->setEnabled(false);
	
	m_SaveAct = new QAction(this);
	connect(m_SaveAct, SIGNAL(triggered()), this, SLOT(actSave()));
	m_SaveAct->setEnabled(false);
	m_SaveAsAct = new QAction(this);
	connect(m_SaveAsAct, SIGNAL(triggered()), this, SLOT(actSaveAs()));
	m_SaveAsAct->setEnabled(false);
	
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
	m_EditMenu->addAction(m_DummyAct);

	//m_ViewportMenu = menuBar()->addMenu(QString::null);
	// m_ViewportMenu->addAction(m_SaveScreenshotAct);
	
	m_WidgetsMenu = menuBar()->addMenu(QString::null);
	
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
	m_WidgetsMenu->setTitle(tr("Widgets"));
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
	// PropertiesEditor
	{
		m_PropertiesEditorDock = new QDockWidget(this);
		m_PropertiesEditorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
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
	
	// DlEditor (Display List)
	{
		m_DlEditorDock = new QDockWidget(this);
		m_DlEditorDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_DlEditor = new DlEditor(m_DlEditorDock);
		m_DlEditor->setPropertiesEditor(m_PropertiesEditor);
		m_DlEditor->setUndoStack(m_UndoStack);
		connect(m_EmulatorViewport, SIGNAL(frame()), m_DlEditor, SLOT(frame()));
		m_DlEditorDock->setWidget(m_DlEditor);
		addDockWidget(Qt::BottomDockWidgetArea, m_DlEditorDock);
		m_WidgetsMenu->addAction(m_DlEditorDock->toggleViewAction());
	}
	
	// Registers
	{
		m_RegistersDock = new QDockWidget(this);
		m_RegistersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
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
			
			m_Macro = new DlEditor(widget);
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
	
	// Controls
	{
		m_ControlsDock = new QDockWidget(this);
		m_ControlsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
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
		
		layout->addStretch();
		widget->setLayout(layout);
		scrollArea->setWidget(widget);
		m_ControlsDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ControlsDock);
		m_WidgetsMenu->addAction(m_ControlsDock->toggleViewAction());
	}
	
	tabifyDockWidget(m_ControlsDock, m_RegistersDock);
}

void MainWindow::translateDockWindows()
{
	m_DlEditorDock->setWindowTitle(tr("Display List"));
	m_PropertiesEditorDock->setWindowTitle(tr("Properties"));
	m_RegistersDock->setWindowTitle(tr("Registers"));
	m_ControlsDock->setWindowTitle(tr("Controls"));
}

void MainWindow::recalculateMinimumWidth()
{
	//if (m_EmulatorConfigScroll) 
	//	m_EmulatorConfigScroll->setMinimumWidth(m_EmulatorConfig->minimumSizeHint().width() + m_EmulatorConfigScroll->minimumSizeHint().width());
}

void MainWindow::incbLanguageCode()
{
	setWindowTitle(tr("FT800 Editor"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
	recalculateMinimumWidth();
}

static bool s_UndoRedoWorking = false;

class HSizeCommand : public QUndoCommand
{
public:
	HSizeCommand(int hsize, QSpinBox *spinbox) : QUndoCommand(), m_NewHSize(hsize), m_OldHSize(s_HSize), m_SpinBox(spinbox) { }
	virtual ~HSizeCommand() { }
	virtual void undo() { s_HSize = m_OldHSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_HSize); s_UndoRedoWorking = false; }
	virtual void redo() { s_HSize = m_NewHSize; s_UndoRedoWorking = true; m_SpinBox->setValue(s_HSize); s_UndoRedoWorking = false; }
	virtual int id() const { return 41517686; }
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
	}
	else
	{
		FT800EMU::GraphicsProcessor.setDebugLimiter(2048 * 64);
	}
}

void MainWindow::stepChanged(int step)
{
	if (m_StepEnabled->isChecked())
	{
		FT800EMU::GraphicsProcessor.setDebugLimiter(step);
	}
}

void MainWindow::clearEditor()
{
	m_HSize->setValue(FT800EMU_WINDOW_WIDTH_DEFAULT);
	m_VSize->setValue(FT800EMU_WINDOW_HEIGHT_DEFAULT);
	m_StepEnabled->setChecked(false);
	m_StepCount->setValue(1);
	m_DlEditor->clear();
	m_Macro->clear();
}

void MainWindow::clearUndoStack()
{
	m_UndoStack->clear();
	m_DlEditor->clearUndoStack();
	m_Macro->clearUndoStack();
}

void MainWindow::actNew()
{
	// reset filename
	m_CurrentFile = QString();
	
	// reset editors to their default state
	clearEditor();
	
	// clear undo stacks
	clearUndoStack();
	
	// be helpful
	m_PropertiesEditor->setInfo(tr("Start typing in the <b>Display List</b> editor."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);
}

void MainWindow::actOpen()
{
	QMessageBox::critical(this, tr("Not implemented"), tr("Not implemented"));
}

void MainWindow::actSave()
{
	QMessageBox::critical(this, tr("Not implemented"), tr("Not implemented"));
}

void MainWindow::actSaveAs()
{
	QMessageBox::critical(this, tr("Not implemented"), tr("Not implemented"));
}

void MainWindow::actImport()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), "",
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
						}
					}
				}
			}
		}
	}
	
	if (!loadOk)
	{
		m_CurrentFile = QString();
		clearEditor();
	}
	
	// clear undo stacks
	clearUndoStack();
	
	m_PropertiesEditor->setInfo(tr("Imported project from .vc1dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);
}

void MainWindow::actExport()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"), "",
		tr("Memory dump, *.vc1dump (*.vc1dump)"));
	if (fileName.isNull())
		return;
	
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
		s = out.writeRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), FT800EMU_DL_SIZE * sizeof(uint32_t));
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
	QMessageBox::about(this, tr("About FT800 Editor"), tr("Copyright (C) 2013  Future Technology Devices International Ltd"));
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
