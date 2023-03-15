/*
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "main_window.h"
#include "version.h"

// STL includes
#include <stdio.h>
#include <algorithm>
#include <math.h>

// Qt includes
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTreeView>
// #include <QDirModel>
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
#include <QFileDialog>
#include <QSettings>
#include <QMimeData>
#include <QTextEdit>
#include <QSizePolicy>

// Emulator includes
#include <bt8xxemu_inttypes.h>
#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>
#ifdef WIN32
#undef min
#undef max
#endif

// Project includes
#include "customize/SaveOptionDialog.h"
#include "utils/ReadWriteUtil.h"
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

namespace FTEDITOR
{

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

typedef int32_t ramaddr;

extern volatile int g_HSize;
extern volatile int g_VSize;
extern volatile int g_Rotate;

extern volatile bool g_EmulatorRunning;

extern ContentManager *g_ContentManager;

extern DlEditor *g_DlEditor;
extern DlEditor *g_CmdEditor;
extern DlEditor *g_Macro;

extern int g_UtilizationDisplayListCmd;
extern volatile bool g_WaitingCoprocessorAnimation;

extern int *g_DisplayListCoprocessorCommandRead;

extern int g_StepCmdLimit;

extern volatile bool g_CoprocessorFaultOccured;
extern volatile bool g_CoprocessorFrameSuccess;
extern volatile bool g_CoprocessorContentSuccess;
extern char g_CoprocessorDiagnostic[128 + 4];
extern bool g_StreamingData;

extern bool g_WarnMissingClear;
extern bool g_WarnMissingClearActive;

extern bool g_WarnMissingTestcardDLStart;
extern bool g_WarnMissingTestcardDLStartActive;

extern volatile bool g_ShowCoprocessorBusy;

QString g_ApplicationDataDir;

extern int *g_CoCmdReadIndicesRead;
extern uint32_t (*g_CoCmdReadValuesRead)[DL_PARSER_MAX_READOUT];
static volatile int s_CoCmdChangeNbEmu;
static int s_CoCmdChangeNbQt;
static int s_CoCmdReadChanged[FTEDITOR_DL_SIZE];

void cleanupMediaFifo();
void emuMain(BT8XXEMU_Emulator *sender, void *context);
void closeDummy(BT8XXEMU_Emulator *sender, void *context);
void resetemu();

static const int s_StandardResolutionNb[FTEDITOR_DEVICE_NB] = {
	3, // FT800
	3, // FT801
	5, // FT810
	5, // FT811
	5, // FT812
	5, // FT813
	5, // BT880
	5, // BT881
	5, // BT882
	5, // BT883
	5, // BT815
	5, // BT816
	7, // BT817
	7, // BT818
};

static const char *s_StandardResolutions[] = {
	"QVGA (320x240)",
	"WQVGA (480x272)",
	"HVGA Portrait (320x480)",
	"WVGA (800x480)",
	"SVGA (800x600)",
	"WSVGA (1024x600)",
	"WXGA (1280x800)"
};

static const int s_StandardWidths[] = {
	320,
	480,
	320,
	800,
	800,
	1024,
	1280
};

static const int s_StandardHeights[] = {
	240,
	272,
	480,
	480,
	600,
	600,
	800
};

bool MainWindow::waitingCoprocessorAnimation()
{
	return g_WaitingCoprocessorAnimation;
}

void MainWindow::updateProgressBars()
{
    m_UtilizationBitmapHandleStatus->setValue(inspector()->countHandleUsage());

    int utilizationDisplayList = std::max(g_UtilizationDisplayListCmd, m_DlEditor->codeEditor()->document()->blockCount());
    m_UtilizationDisplayList->setValue(utilizationDisplayList);
    m_UtilizationDisplayListStatus->setValue(utilizationDisplayList);

    m_UtilizationGlobalStatus->setValue(g_RamGlobalUsage);
}

int *MainWindow::getDlCmd()
{
	// FIXME: Not very thread-safe, but not too critical
	return g_DisplayListCoprocessorCommandRead;
}

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_MinFlashType(-1)
    , m_AddRecentProjectFlag(false)
    , m_UndoStack(NULL)
	, m_Settings(QStringLiteral("Bridgetek"), QStringLiteral("EVE Screen Editor"))
    , m_EmulatorViewport(NULL)
    , m_DlEditor(NULL)
    , m_DlEditorDock(NULL)
    , m_CmdEditor(NULL)
    , m_CmdEditorDock(NULL)
    , m_PropertiesEditor(NULL)
    , m_PropertiesEditorScroll(NULL)
    , m_PropertiesEditorDock(NULL)
    , m_OutputDock(NULL)
    , m_ToolboxDock(NULL)
    , m_Toolbox(NULL)
    , m_ContentManagerDock(NULL)
    , m_ContentManager(NULL)
    , m_RegistersDock(NULL)
    , m_Macro(NULL)
    , m_HSize(NULL)
    , m_VSize(NULL)
    , m_Rotate(NULL)
    , m_ControlsDock(NULL)
    , m_StepEnabled(NULL)
    , m_StepCount(NULL)
    , m_StepCmdEnabled(NULL)
    , m_StepCmdCount(NULL)
    , m_TraceEnabled(NULL)
    , m_TraceX(NULL)
    , m_TraceY(NULL)
    , m_FileMenu(NULL)
    , m_EditMenu(NULL)
    , m_ToolsMenu(NULL)
    , m_WidgetsMenu(NULL)
    ,
#ifdef FT800EMU_PYTHON
    m_ScriptsMenu(NULL)
    ,
#endif
    m_HelpMenu(NULL)
    , m_FileToolBar(NULL)
    , m_EditToolBar(NULL)
    , m_NewAct(NULL)
    , m_OpenAct(NULL)
    , m_SaveAct(NULL)
    , m_SaveAsAct(NULL)
    , m_CloseProjectAct(NULL)
    , m_ImportAct(NULL)
    , m_ExportAct(NULL)
    , m_ProjectFolderAct(NULL)
    , m_ResetEmulatorAct(NULL)
    , m_SaveScreenshotAct(NULL)
    , m_ImportDisplayListAct(NULL)
	, m_SaveDisplayListAct(NULL)
	, m_SaveCoproCmdAct(NULL)
    , m_DisplayListFromIntegers(NULL)
    , m_ManualAct(NULL)
    , m_AboutAct(NULL)
    , m_AboutQtAct(NULL)
    , m_QuitAct(NULL)
    , // m_PrintDebugAct(NULL),
    m_UndoAct(NULL)
    , m_RedoAct(NULL)
    , m_RecentSeparator(NULL)
    , //, m_SaveScreenshotAct(NULL)
    m_CursorPosition(NULL)
    , m_PixelColor(NULL)
    , m_CoprocessorBusy(NULL)
    , m_TemporaryDir(NULL)
	, m_LastProjectDir(QString())
{
	setObjectName("MainWindow");
	setWindowIcon(QIcon(":/icons/eve-puzzle-16.png"));

	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
	setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

	setAcceptDrops(true);

	m_InitialWorkingDir = QDir::currentPath();
	if (QDir(m_InitialWorkingDir).cd("firmware"))
	{
		m_ApplicationDataDir = m_InitialWorkingDir;
	}
	else if (QDir(QCoreApplication::applicationDirPath()).cd("firmware"))
	{
		m_ApplicationDataDir = QCoreApplication::applicationDirPath();
	}
	else
	{
		QDir dir(m_InitialWorkingDir);
		if (dir.cdUp() && dir.cdUp() && dir.cd("fteditor") && dir.cd("firmware") && dir.cdUp())
		{
			m_ApplicationDataDir = dir.absolutePath();
		}
		else
		{
			QMessageBox::critical(this, tr("EVE Screen Editor"), tr("Cannot find flash firmware. The editor may not function correctly."));
		}
	}

	g_ApplicationDataDir = m_ApplicationDataDir;

	m_UndoStack = new QUndoStack(this);
	connect(m_UndoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoCleanChanged(bool)));

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	
	loadRecentProject();

    m_EmulatorViewport = new InteractiveViewport(this);
    emit readyToSetup(m_EmulatorViewport);

	QWidget *centralWidget = new QWidget(this);
	QGridLayout *layout = new QGridLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_EmulatorViewport->verticalRuler(), 1, 0);
	layout->addWidget(m_EmulatorViewport->horizontalRuler(), 0, 1, Qt::AlignBottom);
	layout->addWidget(m_EmulatorViewport, 1, 1);
	layout->addWidget(m_EmulatorViewport->verticalScrollbar(), 1, 2);
	layout->addWidget(m_EmulatorViewport->horizontalScrollbar(), 2, 1);
	layout->setRowStretch(0, 0);
	layout->setRowStretch(1, 50);
	centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

	// Error message box pop-up over viewport
	QVBoxLayout *outerWarning = new QVBoxLayout();
	QFrame *frame = new QFrame(this);
	frame->setFrameStyle(QFrame::StyledPanel);
	frame->setStyleSheet("background-color: " + palette().color(QPalette::ToolTipBase).name() + ";");
	QHBoxLayout *warningLayout = new QHBoxLayout();
	QLabel *warningIcon = new QLabel();
	warningIcon->setPixmap(QIcon(":/icons/exclamation-red.png").pixmap(16, 16));
	warningLayout->addWidget(warningIcon);
	QLabel *warningText = new QLabel(tr("ERROR\nERROR\nERROR"));
	warningLayout->addWidget(warningText);
	warningLayout->setStretchFactor(warningText, 1);
	frame->setLayout(warningLayout);
	warningLayout->setAlignment(warningIcon, Qt::AlignTop | Qt::AlignLeft);
	warningLayout->setAlignment(warningText, Qt::AlignTop | Qt::AlignLeft);
	outerWarning->addWidget(frame);
	outerWarning->addStretch();
	layout->addLayout(outerWarning, 1, 1);
	m_ErrorFrame = frame;
	m_ErrorLabel = warningText;
	frame->setVisible(false);

	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	createDockWindows();

	m_InteractiveProperties = new InteractiveProperties(this);
	m_InteractiveProperties->setVisible(false);

	incbLanguageCode();

	g_DlEditor = m_DlEditor;
	g_CmdEditor = m_CmdEditor;
	g_Macro = m_Macro;

	startEmulatorInternal();

	bindCurrentDevice();

	actNew(true);
}

MainWindow::~MainWindow()
{
	stopEmulatorInternal();

	m_ContentManager->lockContent();
	g_DlEditor = NULL;
	g_CmdEditor = NULL;
	g_Macro = NULL;
	g_ContentManager = NULL;
	m_ContentManager->unlockContent();
	//s_BitmapSetup = NULL;

	QDir::setCurrent(QDir::tempPath());
	delete m_TemporaryDir;
	m_TemporaryDir = NULL;
}

#ifdef FT800EMU_PYTHON
QString pythonError()
{
	printf("---\nPython ERROR: \n");
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyObject *errStr = PyObject_Repr(pvalue);
	const char *pStrErrorMessage = PyUnicode_AsUTF8(errStr);
	QString error = QString::fromUtf8(pStrErrorMessage);
#ifdef WIN32
	wprintf(L"%s\n", pStrErrorMessage ? error.toStdWString().c_str() : L"<NULL>");
#else
	printf("%s\n", pStrErrorMessage ? error.toLocal8Bit().data() : "<NULL>");
#endif
	Py_DECREF(errStr);
	printf("---\n");
	return error;
}

static QString scriptDisplayName(const QString &script)
{
	QByteArray scriptNa = script.toUtf8();
	char *scriptName = scriptNa.data();

	PyObject *pyUserScript = PyUnicode_FromString(scriptName);
	PyObject *pyUserModule = PyImport_Import(pyUserScript);
	Py_DECREF(pyUserScript);
	pyUserScript = NULL;

	if (!pyUserModule)
	{
		pythonError();
		return script;
	}

	PyObject *pyUserFunc = PyObject_GetAttrString(pyUserModule, "displayName");

	if (!pyUserFunc)
	{
		pythonError();
		return script;
	}

	if (!PyCallable_Check(pyUserFunc))
	{
		Py_DECREF(pyUserFunc);
		pyUserFunc = NULL;
		pythonError();
		return script;
	}

	PyObject *pyValue;
	PyObject *pyArgs = PyTuple_New(0);
	pyValue = PyObject_CallObject(pyUserFunc, pyArgs);
	Py_DECREF(pyArgs);
	pyArgs = NULL;

	Py_DECREF(pyUserFunc);
	pyUserFunc = NULL;
	Py_DECREF(pyUserModule);
	pyUserModule = NULL;

	if (pyValue)
	{
		const char *resCStr = PyUnicode_AsUTF8(pyValue);
		QString res = QString::fromUtf8(resCStr);
		Py_DECREF(pyValue);
		pyValue = NULL; // !
		return res;
	}

	pythonError();
	return script;
}
#endif

#ifdef FT800EMU_PYTHON

const char *scriptFolder = "export_scripts";

const char *scriptDeviceFolder[FTEDITOR_DEVICE_NB] = {
	"ft80x",
	"ft80x",
	"ft81x",
	"ft81x",
	"ft81x",
	"ft81x",
	"bt88x",
	"bt88x",
	"bt88x",
	"bt88x",
	"bt81x",
	"bt81x",
	"bt81x",
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
	return m_ApplicationDataDir + "/"
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

		if (FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT815 || FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT816)
		{
			if (!scriptFile.contains("815") && !scriptFile.contains("816"))
				continue;
		}
		else if (FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT817 || FTEDITOR_CURRENT_DEVICE == FTEDITOR_BT818)
		{
			if (!scriptFile.contains("817") && !scriptFile.contains("818"))
				continue;
		}

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
	if (outputName.isEmpty())
		outputName = "untitled";
	QByteArray outN = outputName.toUtf8();

	////////////////////////////////////////////////////////////////////
	// Initialize JSON

	bool error = true;

	PyObject *pyJsonScript = PyUnicode_FromString("json");
	PyObject *pyJsonModule = PyImport_Import(pyJsonScript);
	Py_DECREF(pyJsonScript);
	pyJsonScript = NULL;
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
		QString error = pythonError();
		if (error.isEmpty())
			error = "&lt;NULL&gt;";
		m_PropertiesEditor->setError("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}

	////////////////////////////////////////////////////////////////////
	// Create python object from JSON
	error = true;
	QByteArray jsonDoc = toJson(true);
	PyObject *pyJsonDoc = PyUnicode_FromString(jsonDoc.data());
	PyObject *pyArgs = PyTuple_New(1);
	PyTuple_SetItem(pyArgs, 0, pyJsonDoc);
	PyObject *pyDocument = PyObject_CallObject(pyJsonLoadS, pyArgs);
	if (pyDocument)
		error = false;
	Py_DECREF(pyArgs);
	pyArgs = NULL;
	Py_DECREF(pyJsonLoadS);
	pyJsonLoadS = NULL;
	Py_DECREF(pyJsonModule);
	pyJsonModule = NULL;

	if (error)
	{
		QString error = pythonError();
		if (error.isEmpty())
			error = "&lt;NULL&gt;";
		m_PropertiesEditor->setError("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}

	/*PyObject *pyDocumentString = PyObject_Repr(pyDocument);
	printf("Demo: %s\n", PyUnicode_AsUTF8(pyDocumentString));
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
		Py_DECREF(pyUserScript);
		pyUserScript = NULL;

		if (pyUserModule != NULL)
		{
			PyObject *pyUserFunc = PyObject_GetAttrString(pyUserModule, "run");
			if (pyUserFunc && PyCallable_Check(pyUserFunc))
			{
				PyObject *pyValue;
				PyObject *pyArgs = 0;

				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					pyArgs = PyTuple_New(4);
				else
					pyArgs = PyTuple_New(3);

				pyValue = PyUnicode_FromString(outN.data());

				PyTuple_SetItem(pyArgs, 0, pyValue);
				PyTuple_SetItem(pyArgs, 1, pyDocument);
				pyDocument = NULL;
				char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
				pyValue = PyByteArray_FromStringAndSize(ram, addressSpace(FTEDITOR_CURRENT_DEVICE));
				PyTuple_SetItem(pyArgs, 2, pyValue);

				// screen resolution argument
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				{
					QString resol = QString("%1x%2").arg(m_HSize->text()).arg(m_VSize->text());
					pyValue = PyUnicode_FromString(resol.toUtf8().data());
					PyTuple_SetItem(pyArgs, 3, pyValue);
				}

				pyValue = PyObject_CallObject(pyUserFunc, pyArgs);

				Py_DECREF(pyArgs);
				pyArgs = NULL;
				if (pyValue)
				{
					printf("Ok\n");
					PyObject *resStr = PyObject_Repr(pyValue);
					const char *resCStr = PyUnicode_AsUTF8(resStr);
					QString res = QString::fromUtf8(resCStr);
					Py_DECREF(pyValue);
					pyValue = NULL;
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

			Py_XDECREF(pyUserFunc);
			pyUserFunc = NULL;
			Py_XDECREF(pyUserModule);
			pyUserModule = NULL;
		}
		Py_XDECREF(pyUserModuleOld);
		pyUserModuleOld = NULL;
	}

	Py_XDECREF(pyDocument);
	pyDocument = NULL;

	if (error)
	{
		QString error = pythonError();
		if (error.isEmpty())
			error = "&lt;NULL&gt;";
		m_PropertiesEditor->setError("<b>Error</b>: <i>(Python)</i> " + error);
		m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	}
#endif /* FT800EMU_PYTHON */
}

void MainWindow::frameEmu()
{
	// Read CoCmd directly into DlParsed, no need to lock since this is just a one-way data copy, and it'll be updated again anyway
	int coCmdChangeNbEmu = s_CoCmdChangeNbEmu + 1;
	const DlParsed *cmdParsed = m_CmdEditor->getDisplayListParsed();
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		int line = g_CoCmdReadIndicesRead[i];
		if (line < 0)
			break; // no more
		if (memcmp(cmdParsed[line].ReadOut, g_CoCmdReadValuesRead[i], sizeof(cmdParsed->ReadOut)))
		{
			m_CmdEditor->setReadOut(line, g_CoCmdReadValuesRead[i]);
			s_CoCmdReadChanged[line] = coCmdChangeNbEmu;
			// DEBUG:
			/*
			printf("Line %i, readout ", line);
			for (int j = 0; j < DL_PARSER_MAX_READOUT; ++j)
			{
				printf("%i, ", (int)g_CoCmdReadValuesRead[i][j]);
			}
			printf("\n");
			*/
		}
	}
	s_CoCmdChangeNbEmu = coCmdChangeNbEmu;
}

void MainWindow::popupTimeout()
{
	m_ErrorLabel->setText("<b>Co-processor engine timeout</b><br><br>"
		"The co-processor is taking longer than expected to process this command");
	m_ErrorFrame->setVisible(true);
	g_CoprocessorFrameSuccess = false;
}

void MainWindow::frameQt()
{
	if (/*!g_StreamingData && */g_CoprocessorFaultOccured) // && (m_PropertiesEditor->getEditWidgetSetter() == m_DlEditor || m_PropertiesEditor->getEditWidgetSetter() == m_CmdEditor || m_PropertiesEditor->getEditWidgetSetter() == NULL))
	{
		QString info;
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
		{
			if (g_CoprocessorDiagnostic[0])
			{
				info = "<b>Co-processor engine fault</b><br><br>";
				info += QString::fromLatin1(g_CoprocessorDiagnostic);
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
		// m_PropertiesEditor->setInfo(info);
		// m_PropertiesEditor->setEditWidget(NULL, false, m_PropertiesEditorDock); // m_PropertiesEditorDock is a dummy
		// focusProperties();
		m_ErrorLabel->setText(info);
		m_ErrorFrame->setVisible(true);
		g_CoprocessorFrameSuccess = false;
	}
	if ((g_CoprocessorFrameSuccess || g_WaitingCoprocessorAnimation) && (/* g_CoprocessorContentSuccess || */!m_ContentManager->getContentCount()))
	{
		m_ErrorFrame->setVisible(false);
	}

	// printf("msc: %s\n", g_WarnMissingClear ? "warn" : "ok");
	if (g_WarnMissingClear != g_WarnMissingClearActive)
	{
		if (g_WarnMissingClear)
		{
			statusBar()->showMessage(tr("WARNING: Missing CLEAR instruction in display list"));
		}
		else
		{
			statusBar()->showMessage("");
		}
		g_WarnMissingClearActive = g_WarnMissingClear;
	}

	if (g_WarnMissingTestcardDLStart != g_WarnMissingTestcardDLStartActive)
	{
		if (g_WarnMissingTestcardDLStart)
		{
			statusBar()->showMessage(tr("WARNING: Commands following CMD_TESTCARD must be preceeded by CMD_DLSTART"));
		}
		else
		{
			statusBar()->showMessage("");
		}
		g_WarnMissingTestcardDLStartActive = g_WarnMissingTestcardDLStart;
	}

	// m_CursorPosition
	if (m_EmulatorViewport->mouseOver())
	{
		m_CursorPosition->setText(QString("XY: %1, %2").arg(m_EmulatorViewport->mouseX()).arg(m_EmulatorViewport->mouseY()));

		QColor c = m_EmulatorViewport->fetchColorAsync();
		QString strColor("");

		if (c.isValid())
			strColor = QString("ARGB: %1, %2, %3, %4").arg(c.alpha(), 0, 10).arg(c.red(), 0, 10).arg(c.green(), 0, 10).arg(c.blue(), 0, 10);
		m_PixelColor->setText(strColor.toUpper());
	}
	else
	{
		m_CursorPosition->setText("");
		m_PixelColor->setText("");
	}

	// Busy loader
	updateLoadingIcon();

	// Trigger changes to readout cocmd on qt thread
	int coCmdChangeNbEmu = s_CoCmdChangeNbEmu;
	int coCmdChangeNbQt = s_CoCmdChangeNbQt;
	bool anyReadOutChanged = false;
	for (int i = 0; i < FTEDITOR_DL_SIZE && i < m_CmdEditor->getLineCount(); ++i)
	{
		if (s_CoCmdReadChanged[i] >= coCmdChangeNbQt)
		{
			// TODO: Line-specific refresh?
			anyReadOutChanged = true;
			// DEBUG: printf("Line %i, readout refresh\n", i);
			break;
		}
	}
	s_CoCmdChangeNbQt = coCmdChangeNbEmu;
	if (anyReadOutChanged)
	{
		// Always refresh for now
		m_InteractiveProperties->modifiedEditorLine();
	}
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

	m_CloseProjectAct = new QAction(this);
	m_CloseProjectAct->setShortcuts(QKeySequence::Close);
	connect(m_CloseProjectAct, SIGNAL(triggered()), this, SLOT(actCloseProject()));

#ifndef NDEBUG
	// Enable VC dump for any user that has run a Debug build.
	if (!m_Settings.contains(QStringLiteral("VCDumpEnabled")))
		m_Settings.setValue(QStringLiteral("VCDumpEnabled"), 1);
#endif

	m_ImportAct = new QAction(this);
	connect(m_ImportAct, SIGNAL(triggered()), this, SLOT(actImport()));
	m_ImportAct->setVisible(m_Settings.value(QStringLiteral("VCDumpEnabled")).toInt() != 0);

	m_ExportAct = new QAction(this);
	connect(m_ExportAct, SIGNAL(triggered()), this, SLOT(actExport()));
	m_ExportAct->setVisible(m_Settings.value(QStringLiteral("VCDumpEnabled")).toInt() != 0);

	m_ProjectFolderAct = new QAction(this);
	m_ProjectFolderAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
	connect(m_ProjectFolderAct, SIGNAL(triggered()), this, SLOT(actProjectFolder()));

	m_ResetEmulatorAct = new QAction(this);
	m_ResetEmulatorAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
	connect(m_ResetEmulatorAct, SIGNAL(triggered()), this, SLOT(actResetEmulator()));

	m_ImportDisplayListAct = new QAction(this);
	m_ImportDisplayListAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
	connect(m_ImportDisplayListAct, SIGNAL(triggered()), this, SLOT(actImportDisplayList()));

	m_SaveScreenshotAct = new QAction(this);
	connect(m_SaveScreenshotAct, SIGNAL(triggered()), this, SLOT(actSaveScreenshot()));

	m_SaveDisplayListAct = new QAction(this);
	connect(m_SaveDisplayListAct, SIGNAL(triggered()), this, SLOT(handleSaveDL()));

	m_SaveCoproCmdAct = new QAction(this);
	connect(m_SaveCoproCmdAct, SIGNAL(triggered()), this, SLOT(handleSaveCoproCmd()));

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
	m_RedoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));

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
	m_SaveAsAct->setStatusTip(tr("Save the current project to a new file"));
	m_SaveAsAct->setIcon(QIcon(":/icons/disk--pencil.png"));
	m_CloseProjectAct->setText(tr("Close Project"));
	m_CloseProjectAct->setStatusTip(tr("Close the current project"));
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

	m_SaveDisplayListAct->setText(tr("Save Display List"));
	m_SaveDisplayListAct->setToolTip(tr("Save Display List to a file"));
	m_SaveCoproCmdAct->setText(tr("Save Coprocessor Command"));
	m_SaveCoproCmdAct->setToolTip(tr("Save Coprocessor Command to a file"));

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
	m_UndoAct->setIcon(QIcon(":/icons/arrow-return-180-left.png"));
	// m_UndoAct->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
	m_RedoAct->setText(tr("Redo"));
	m_RedoAct->setStatusTip(tr("Reapply the action"));
	m_RedoAct->setIcon(QIcon(":/icons/arrow-circle-315.png"));
	// m_RedoAct->setIcon(QIcon(":/icons/arrow-return-180.png"));
	m_DummyAct->setText(tr("Dummy"));
	m_DummyAct->setStatusTip(tr("Does nothing"));
	// m_SaveScreenshotAct->setText(tr("ActionSaveScreenshot"));
	// m_SaveScreenshotAct->setStatusTip(tr("ActionSaveScreenshotStatusTip"));
}

void MainWindow::createMenus()
{
	m_FileMenu = menuBar()->addMenu(QString());
	m_FileMenu->addAction(m_NewAct);
	m_FileMenu->addAction(m_OpenAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_SaveAct);
	m_FileMenu->addAction(m_SaveAsAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_CloseProjectAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_ProjectFolderAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_ImportAct);
	m_FileMenu->addAction(m_ExportAct);
	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_SaveScreenshotAct);

	m_FileMenu->addAction(m_SaveDisplayListAct);
	m_FileMenu->addAction(m_SaveCoproCmdAct);

	m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_QuitAct);

	m_EditMenu = menuBar()->addMenu(QString());
	m_EditMenu->addAction(m_UndoAct);
	m_EditMenu->addAction(m_RedoAct);
	//m_EditMenu->addAction(m_DummyAct);

	m_ToolsMenu = menuBar()->addMenu(QString());
	m_ToolsMenu->addAction(m_ResetEmulatorAct);
	// m_ToolsMenu->addAction(m_SaveScreenshotAct);
	m_ToolsMenu->addAction(m_ImportDisplayListAct);

	

#if _DEBUG
	m_ToolsMenu->addAction(m_DisplayListFromIntegers);
#endif

	m_WidgetsMenu = menuBar()->addMenu(QString());

#ifdef FT800EMU_PYTHON
	m_ScriptsMenu = menuBar()->addMenu(QString());
	connect(m_ScriptsMenu, SIGNAL(aboutToShow()), this, SLOT(refreshScriptsMenu()));
#endif /* FT800EMU_PYTHON */

	menuBar()->addSeparator();

	m_HelpMenu = menuBar()->addMenu(QString());
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
			QVBoxLayout *groupLayout = new QVBoxLayout();

			QHBoxLayout *hBoxLayout = new QHBoxLayout();

			m_ProjectDevice = new QComboBox(this);
			for (int i = 0; i < FTEDITOR_DEVICE_NB; ++i)
				m_ProjectDevice->addItem(deviceToString(i));
			m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE);
			hBoxLayout->addWidget(m_ProjectDevice);
			connect(m_ProjectDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(projectDeviceChanged(int)));

			m_ProjectDisplay = new QComboBox(this);
			for (int i = 0; i < s_StandardResolutionNb[FTEDITOR_CURRENT_DEVICE]; ++i)
				m_ProjectDisplay->addItem(s_StandardResolutions[i]);
			m_ProjectDisplay->addItem("");
			hBoxLayout->addWidget(m_ProjectDisplay);
			connect(m_ProjectDisplay, SIGNAL(currentIndexChanged(int)), this, SLOT(projectDisplayChanged(int)));

			groupLayout->addLayout(hBoxLayout);

			QVBoxLayout *vBoxLayout = new QVBoxLayout();
			vBoxLayout->setContentsMargins(0, 0, 0, 0);
			hBoxLayout = new QHBoxLayout();
			m_ProjectFlashLayout = new QWidget(this);
			m_ProjectFlashLayout->setContentsMargins(0, 0, 0, 0);
			hBoxLayout->setContentsMargins(0, 0, 0, 0);

			m_ProjectFlash = new QComboBox(this);
			for (int i = 0; i < FTEDITOR_FLASH_NB; ++i)
				m_ProjectFlash->addItem(flashToString(i));
			m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_FLASH);
			hBoxLayout->addWidget(m_ProjectFlash, 1);
			connect(m_ProjectFlash, SIGNAL(currentIndexChanged(int)), this, SLOT(projectFlashChanged(int)));

			m_ProjectFlashImport = new QPushButton(this);
			// m_ProjectFlashImport->setText(tr("Import"));
			m_ProjectFlashImport->setIcon(QIcon(":/icons/folder-horizontal-open.png"));
			m_ProjectFlashImport->setToolTip(tr("Import Mapped Flash Image"));
			hBoxLayout->addWidget(m_ProjectFlashImport);

			// m_ProjectFlashLayout->stretch

			vBoxLayout->addLayout(hBoxLayout);
			m_ProjectFlashLayout->setLayout(vBoxLayout);
			groupLayout->addWidget(m_ProjectFlashLayout);

			m_ProjectFlashFilename = new QLabel(this);
			QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
			sizePolicy.setHorizontalStretch(0);
			sizePolicy.setVerticalStretch(0);
			sizePolicy.setHeightForWidth(m_ProjectFlashFilename->sizePolicy().hasHeightForWidth());
			m_ProjectFlashFilename->setSizePolicy(sizePolicy);
			m_ProjectFlashFilename->installEventFilter(this);
			vBoxLayout->addWidget(m_ProjectFlashFilename);

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

	// pixel color (RGBA)
	{
		m_PixelColor = new QLabel(statusBar());
		QFontMetrics fm = m_PixelColor->fontMetrics();
		int mw = fm.horizontalAdvance("ARGB: 255, 255, 255, 255   ");
		m_PixelColor->setFixedWidth(mw);
		m_PixelColor->setText("");
		statusBar()->addPermanentWidget(m_PixelColor);
	}

	// Cursor position
	{
		m_CursorPosition = new QLabel(statusBar());
		QFontMetrics fm = m_CursorPosition->fontMetrics();
		int mw = fm.horizontalAdvance("XY: 9999, 999   ");
		m_CursorPosition->setFixedWidth(mw);
		m_CursorPosition->setText("");
		statusBar()->addPermanentWidget(m_CursorPosition);
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

		QStyle *progressStyle = QStyleFactory::create("Fusion"); // FIXME: This might be a memory leak
		QPalette progressPalette = palette();
		progressPalette.setColor(QPalette::Link, QColor(96, 192, 48));
		progressPalette.setColor(QPalette::Highlight, QColor(96, 192, 48));
		
		m_UtilizationBitmapHandleStatus = new QProgressBar(statusBar());
		m_UtilizationBitmapHandleStatus->setStyle(progressStyle);
		m_UtilizationBitmapHandleStatus->setPalette(progressPalette);
		m_UtilizationBitmapHandleStatus->setMinimum(0);
		m_UtilizationBitmapHandleStatus->setMaximum(FTED_NUM_HANDLES);
		m_UtilizationBitmapHandleStatus->setMinimumSize(60, 8);
		m_UtilizationBitmapHandleStatus->setMaximumSize(100, 19); // FIXME
		m_UtilizationBitmapHandleStatus->setValue(0);
		m_UtilizationBitmapHandleStatus->installEventFilter(this);
		statusBar()->addPermanentWidget(m_UtilizationBitmapHandleStatus);
		statusBar()->addPermanentWidget(new QLabel(statusBar()));

		QLabel *dlLabel = new QLabel(statusBar());
		dlLabel->setText(tr("RAM_DL: "));
		statusBar()->addPermanentWidget(dlLabel);

		m_UtilizationDisplayListStatus = new QProgressBar(statusBar());
		m_UtilizationDisplayListStatus->setStyle(progressStyle);
		m_UtilizationDisplayListStatus->setPalette(progressPalette);
		m_UtilizationDisplayListStatus->setMinimum(0);
		m_UtilizationDisplayListStatus->setMaximum(displayListSize(FTEDITOR_CURRENT_DEVICE));
		m_UtilizationDisplayListStatus->setMinimumSize(60, 8);
		m_UtilizationDisplayListStatus->setMaximumSize(100, 19); // FIXME
		m_UtilizationDisplayListStatus->setValue(0);
		m_UtilizationDisplayListStatus->installEventFilter(this);

		statusBar()->addPermanentWidget(m_UtilizationDisplayListStatus);
		statusBar()->addPermanentWidget(new QLabel(statusBar()));

		QLabel *dlLabelG = new QLabel(statusBar());
		dlLabelG->setText(tr("RAM_G: "));
		statusBar()->addPermanentWidget(dlLabelG);

		m_UtilizationGlobalStatus = new QProgressBar(statusBar());
		m_UtilizationGlobalStatus->setStyle(progressStyle);
		m_UtilizationGlobalStatus->setPalette(progressPalette);
		m_UtilizationGlobalStatus->setMinimum(0);
		m_UtilizationGlobalStatus->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END));
		m_UtilizationGlobalStatus->setMinimumSize(60, 8);
		m_UtilizationGlobalStatus->setMaximumSize(100, 19); // FIXME
		m_UtilizationGlobalStatus->setValue(0);
		m_UtilizationGlobalStatus->installEventFilter(this);
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
			m_UtilizationDisplayList->setValue(0);
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
		connect(m_PropertiesEditor, &PropertiesEditor::errorSet, this, &MainWindow::propertyErrorSet);
		m_PropertiesEditorScroll->setWidget(m_PropertiesEditor);
		m_PropertiesEditorDock->setWidget(m_PropertiesEditorScroll);
		addDockWidget(Qt::RightDockWidgetArea, m_PropertiesEditorDock);
		m_WidgetsMenu->addAction(m_PropertiesEditorDock->toggleViewAction());
	}

	// Output Window
	{
		m_OutputTextEdit = new QTextEdit(this);
		m_OutputTextEdit->setReadOnly(true);
		m_OutputTextEdit->setAcceptRichText(true);
		m_OutputTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_OutputTextEdit->setMinimumWidth(240);

		m_OutputDock = new QDockWidget(this);
		m_OutputDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_OutputDock->setObjectName("OutputWindow");
		m_OutputDock->setWidget(m_OutputTextEdit);

		addDockWidget(Qt::RightDockWidgetArea, m_OutputDock);
		m_WidgetsMenu->addAction(m_OutputDock->toggleViewAction());
	}

	// Inspector
	{
		m_InspectorDock = new QDockWidget(this);
		m_InspectorDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_InspectorDock->setObjectName("Inspector");
		m_Inspector = new Inspector(this);
		emit readyToSetup(m_Inspector);
		m_InspectorDock->setWidget(m_Inspector);
		emit readyToSetup(m_InspectorDock);
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
		connect(m_DlEditorDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
			if (visible)
			{
				m_DlEditor->codeEditor()->setFocus();
			}
		});
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
		connect(m_CmdEditorDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
			if (visible)
			{
				m_CmdEditor->codeEditor()->setFocus();
			}
		});
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
		emit readyToSetup(m_ContentManager);
		g_ContentManager = m_ContentManager;
		scrollArea->setWidget(m_ContentManager);
		m_ContentManagerDock->setWidget(scrollArea);
		addDockWidget(Qt::LeftDockWidgetArea, m_ContentManagerDock);
		m_WidgetsMenu->addAction(m_ContentManagerDock->toggleViewAction());

		connect(m_ProjectFlashImport, SIGNAL(clicked()), m_ContentManager, SLOT(importFlashMapped()));
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

	// Ruler
	m_WidgetsMenu->addAction(m_EmulatorViewport->ActionRuler());
	
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
	tabifyDockWidget(m_ContentManagerDock, m_ToolboxDock);

#if FT800_DEVICE_MANAGER
	tabifyDockWidget(m_DeviceManagerDock, m_ControlsDock);
#endif /* FT800_DEVICE_MANAGER */
	tabifyDockWidget(m_ControlsDock, m_UtilizationDock);
	tabifyDockWidget(m_UtilizationDock, m_PropertiesEditorDock);
	tabifyDockWidget(m_UtilizationDock, m_OutputDock);

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
	connect(this, SIGNAL(utilizationDisplayListCmdChanged(int)), this, SLOT(updateProgressBars()));
	connect(m_Inspector, SIGNAL(countHandleBitmapChanged(int)), this, SLOT(updateProgressBars()));
	connect(m_ContentManager, SIGNAL(ramGlobalUsageChanged(int)), this, SLOT(updateProgressBars()));
	connect(m_ContentManager, SIGNAL(busyNow(QObject *)),this, SLOT(appendBusyList(QObject *)));
	connect(m_ContentManager, SIGNAL(freeNow(QObject *)),this, SLOT(removeBusyList(QObject *)));
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
	m_OutputDock->setWindowTitle(tr("Output"));
	m_ToolboxDock->setWindowTitle(tr("Toolbox"));
	m_ContentManagerDock->setWindowTitle(tr("Content"));
	m_RegistersDock->setWindowTitle(tr("Registers"));
	m_ControlsDock->setWindowTitle(tr("Controls"));
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
			else if (dw == m_OutputDock)
			{
				tabBar->setTabIcon(j, processIcon(tabBar, QIcon(":/icons/information-white.png")));
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
	if (!cmdExist)
		cmdTop = false;
	if (!dlExist)
		dlTop = false;
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
	HSizeCommand(int hsize, QSpinBox *spinbox)
	    : QUndoCommand()
	    , m_NewHSize(hsize)
	    , m_OldHSize(g_HSize)
	    , m_SpinBox(spinbox)
	{
	}
	virtual ~HSizeCommand() {}
	virtual void undo()
	{
		g_HSize = m_OldHSize;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_HSize);
		s_UndoRedoWorking = false;
	}
	virtual void redo()
	{
		g_HSize = m_NewHSize;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_HSize);
		s_UndoRedoWorking = false;
	}
	virtual int id() const
	{
		printf("id get\n");
		return 41517686;
	}
	virtual bool mergeWith(const QUndoCommand *command)
	{
		m_NewHSize = static_cast<const HSizeCommand *>(command)->m_NewHSize;
		return true;
	}

private:
	int m_NewHSize;
	int m_OldHSize;
	QSpinBox *m_SpinBox;
};

class VSizeCommand : public QUndoCommand
{
public:
	VSizeCommand(int hsize, QSpinBox *spinbox)
	    : QUndoCommand()
	    , m_NewVSize(hsize)
	    , m_OldVSize(g_VSize)
	    , m_SpinBox(spinbox)
	{
	}
	virtual ~VSizeCommand() {}
	virtual void undo()
	{
		g_VSize = m_OldVSize;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_VSize);
		s_UndoRedoWorking = false;
	}
	virtual void redo()
	{
		g_VSize = m_NewVSize;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_VSize);
		s_UndoRedoWorking = false;
	}
	virtual int id() const { return 78984351; }
	virtual bool mergeWith(const QUndoCommand *command)
	{
		m_NewVSize = static_cast<const VSizeCommand *>(command)->m_NewVSize;
		return true;
	}

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
	RotateCommand(int rotate, QSpinBox *spinbox)
	    : QUndoCommand()
	    , m_NewRotate(rotate)
	    , m_OldRotate(g_Rotate)
	    , m_SpinBox(spinbox)
	{
	}
	virtual ~RotateCommand() {}
	virtual void undo()
	{
		g_Rotate = m_OldRotate;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_Rotate);
		s_UndoRedoWorking = false;
	}
	virtual void redo()
	{
		g_Rotate = m_NewRotate;
		s_UndoRedoWorking = true;
		m_SpinBox->setValue(g_Rotate);
		s_UndoRedoWorking = false;
	}
	virtual int id() const { return 78994352; }
	virtual bool mergeWith(const QUndoCommand *command)
	{
		m_NewRotate = static_cast<const RotateCommand *>(command)->m_NewRotate;
		return true;
	}

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
	m_ContentManager->clear(true);
	//m_BitmapSetup->clear();
	m_ProjectDock->setVisible(true);
	m_OutputDock->setVisible(true);
	m_OutputTextEdit->clear();
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
	QString titleSuffix =  QString("EVE Screen Editor v%1 [Build Time: %2 - %3]")
		.arg(STR_PRODUCTVERSION)
		.arg(__DATE__)
		.arg(__TIME__);
	if (!m_CloseProjectAct->isEnabled())
	{
		setWindowTitle(titleSuffix);
	}
	else
	{
		QString title = QString("%1%2 - %3 - ")
			.arg(QString(m_CleanUndoStack ? "" : "*"))
			.arg(m_CurrentFile.isEmpty() ? tr("New Project") : QFileInfo(m_CurrentFile).completeBaseName())
			.arg(QDir::currentPath());
		setWindowTitle(title + titleSuffix);
	}
}

void MainWindow::undoCleanChanged(bool clean)
{
	m_CleanUndoStack = clean;
	updateWindowTitle();
}

bool MainWindow::maybeSave()
{
	bool res = true;

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
		}
		else if (ret == QMessageBox::Cancel)
		{
			res = false;
		}
	}

	if (res && m_AddRecentProjectFlag)
	{
		m_AddRecentProjectFlag = false;
		addRecentProject(m_CurrentFile);
	}

	return res;
}

void MainWindow::loadRecentProject()
{
	// insert recent project actions
	QAction *pRecentProjAction = 0;
	m_RecentActionList.clear();
	for (int i = 0; i < 5; i++)
	{
		pRecentProjAction = new QAction("", this);
		connect(pRecentProjAction, &QAction::triggered, this, &MainWindow::openRecentProject);
		pRecentProjAction->setVisible(false);
		pRecentProjAction->setShortcut(QKeySequence(QString("Alt+%1").arg(i + 1)));
		m_RecentActionList << pRecentProjAction;
		m_FileMenu->insertAction(m_QuitAct, pRecentProjAction);
	}

	// insert recent project separator
	if (m_RecentSeparator == NULL)
		m_RecentSeparator = m_FileMenu->insertSeparator(m_QuitAct);

	m_RecentSeparator->setVisible(false);

	QStringList pathList;
	for (int i = 0; i < 10; ++i)
	{
		QString str = m_Settings.value(QStringLiteral("RecentProject") + QString::number(i)).toString();
		if (!str.isEmpty())
			pathList.push_back(str);
	}

	// add recent project path to File Menu
	for (int i = pathList.size() - 1; i >= 0; --i)
	{
		if (pathList.at(i).isEmpty())
			continue;
		addRecentProject(pathList.at(i));
	}
}

void MainWindow::addRecentProject(QString recentPath)
{
	m_RecentPathList.prepend(recentPath);
	while (m_RecentPathList.size() > 5)
	{
		m_RecentPathList.removeLast();
	}

	for (int i = 0; i < m_RecentPathList.size() && i < m_RecentActionList.size(); ++i)
	{
		m_RecentActionList[i]->setText(QString("%1: %2").arg(i + 1).arg(m_RecentPathList.at(i)));
		m_RecentActionList[i]->setData(m_RecentPathList.at(i));
		m_RecentActionList[i]->setVisible(true);
	}

	if (m_RecentSeparator)
	{
		m_RecentSeparator->setVisible(m_RecentPathList.size() > 0);
	}
}

void MainWindow::removeRecentProject(QString removePath)
{
	int i = 0;

	if (!m_RecentPathList.removeOne(removePath))
		return;

	for (i = 0; i < m_RecentPathList.size() && i < m_RecentActionList.size(); ++i)
	{
		m_RecentActionList[i]->setText(QString("%1: %2").arg(i + 1).arg(m_RecentPathList.at(i)));
		m_RecentActionList[i]->setData(m_RecentPathList.at(i));
		m_RecentActionList[i]->setVisible(true);
	}

	for (; i < m_RecentActionList.size(); ++i)
	{
		m_RecentActionList[i]->setVisible(false);
	}

	if (m_RecentSeparator)
	{
		m_RecentSeparator->setVisible(m_RecentPathList.size() > 0);
	}
}

void MainWindow::saveRecentProject()
{
	for (int i = 0; i < 10; ++i)
	{
		if (i < m_RecentPathList.size())
		{
			m_Settings.setValue(QStringLiteral("RecentProject") + QString::number(i), m_RecentPathList[i]);
		}
		else
		{
			m_Settings.remove(QStringLiteral("RecentProject") + QString::number(i));
		}
	}
	m_Settings.sync();
}

bool MainWindow::checkAndPromptFlashPath(const QString &filePath)
{
	if (filePath.isEmpty())
	{
		return false;
	}

	// check .map and .bin files
	QString binPath(filePath);
	binPath = binPath.replace(filePath.length() - 3, 3, "bin");
	if (false == QFileInfo::exists(filePath) || false == QFileInfo::exists(binPath))
	{
		// prompt user
		QMessageBox::information(this, tr("Flash files do not exist"), tr("Flash files do not exist!"));
	}
	else
	{
		// check flash configuration

		quint64 binSize = QFileInfo(binPath).size();

		// Iterate through flash, starting from the selected one, until one is found that is large enough
		// Assume flash are sorted from small to large, and assume similar flash are in the same sequence
		// No need to pick a smaller flash in case the user already selected a good flash size

		int flashIntf = g_CurrentFlash;
		for (; flashIntf < FTEDITOR_FLASH_NB; ++flashIntf)
		{
			if (flashSizeBytes(flashIntf) >= binSize) // There is enough space in this flash type
				break;
		}

		if (flashIntf == FTEDITOR_FLASH_NB)
		{
			// This is technically not an issue for FTEDITOR to handle,
			// internally this will restrict the loaded content to the available size.
			// It is permissible to let the user continue and explore the flash with the missing content
			int ans = QMessageBox::critical(this, tr("Flash image is too big"), 
				tr("Flash image is too big.\nUnable to load completely."), QMessageBox::Abort, QMessageBox::Ignore);
			return ans == QMessageBox::Ignore;
		}
		else if (flashIntf != g_CurrentFlash)
		{
			// Push the change of flash size onto the undo stack
			// We ask the user permission when changing the flash size,
			// since it's not reliable application behaviour
			// to change options without the user's knowledge
			int ans = QMessageBox::question(this, "Increase flash size",
			    "The selected flash image is larger than the current flash size.\n"
			    "Would you like to increase it?",
			    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			    QMessageBox::Cancel);
			if (ans == QMessageBox::Yes)
			{
				m_UndoStack->beginMacro(tr("Increase flash size"));
				m_ProjectFlash->setCurrentIndex(flashIntf);
				m_UndoStack->endMacro();
			}
			// It is permissible to let the user continue and explore the flash with the missing content
			return ans != QMessageBox::Cancel;
		}
	}

	return true;
}

void MainWindow::toggleDockWindow(bool isShow)
{
	m_InspectorDock->setVisible(isShow);
	m_DlEditorDock->setVisible(false);
	m_CmdEditorDock->setVisible(isShow);
	m_ProjectDock->setVisible(isShow);
#if FT800_DEVICE_MANAGER
	m_DeviceManagerDock->setVisible(isShow);
#endif /* FT800_DEVICE_MANAGER */
	m_UtilizationDock->setVisible(isShow);
	m_NavigatorDock->setVisible(isShow);
	m_PropertiesEditorDock->setVisible(isShow);
	m_OutputDock->setVisible(isShow);
	m_ToolboxDock->setVisible(isShow);
	m_ContentManagerDock->setVisible(isShow);
	m_RegistersDock->setVisible(isShow);
	m_ControlsDock->setVisible(isShow);
}

void MainWindow::toggleUI(bool hasProject)
{
	toggleDockWindow(hasProject);

	// toggle Edit, Tools, View and Export menu
	m_EditMenu->setEnabled(hasProject);
	m_ToolsMenu->setEnabled(hasProject);
	m_WidgetsMenu->setEnabled(hasProject);
#ifdef FT800EMU_PYTHON
	m_ScriptsMenu->setEnabled(hasProject);
#endif /* FT800EMU_PYTHON */

	// toggle Save, Save As, Browse Project Folder menu item
	m_SaveAct->setEnabled(hasProject);
	m_SaveAsAct->setEnabled(hasProject);
	m_CloseProjectAct->setEnabled(hasProject);
	m_ProjectFolderAct->setEnabled(hasProject);
	m_SaveScreenshotAct->setEnabled(hasProject);

	// toogle emulator viewport
	m_EmulatorViewport->setVisible(hasProject);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
		event->accept();
	else
		event->ignore();

	saveRecentProject();
}

void MainWindow::actCloseProject()
{
#define FTEDITOR_INITIAL_HELP tr("Start typing in the <b>Coprocessor</b> editor, or drag and drop items from the <b>Toolbox</b> onto the display viewport.")

	if (!maybeSave())
		return;

	// reset editors to their default state
	clearEditor();

	// reset filename
	m_CurrentFile = QString();

	// clear undo stacks
	clearUndoStack();

	// be helpful
	focusCmdEditor();
	m_PropertiesEditor->setInfo(FTEDITOR_INITIAL_HELP);
	m_PropertiesEditor->setEditWidget(NULL, false, NULL);
	m_Toolbox->setEditorLine(m_CmdEditor, 0);
	m_CmdEditor->selectLine(0);

	QDir::setCurrent(QDir::tempPath());
#ifdef FTEDITOR_TEMP_DIR
	delete m_TemporaryDir;
	m_TemporaryDir = NULL;
#endif

	// reset flash file name
	setFlashFileNameToLabel("");

	// hide all dock windows
	toggleUI(false);

	updateWindowTitle();
}

void MainWindow::actNew()
{
	toggleUI(true);
	actNew(true);
}

void MainWindow::actNew(bool addClear)
{
	if (!maybeSave())
		return;

	printf("** New **\n");

	// reset editors to their default state
	clearEditor();

	// reset filename
	m_CurrentFile = QString();

	// add clear
	int editLine;
	if (addClear)
	{
		DlParsed pa;
		pa.ValidId = true;
		pa.IdLeft = 0;
		pa.IdRight = FTEDITOR_DL_CLEAR;
		pa.ExpectedStringParameter = false;
		pa.VarArgCount = 0;
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
	m_TemporaryDir = new QTemporaryDir("ESE");
	QDir::setCurrent(m_TemporaryDir->path());
#else
	QDir::setCurrent(m_InitialWorkingDir);
#endif
	updateWindowTitle();
	printf("Current path: %s\n", QDir::currentPath().toLocal8Bit().data());

	// reset flash file name
	setFlashFileNameToLabel("");

	// WORKAROUND: Issue #100
	actResetEmulator();
	m_MinFlashType = -1;
}

void documentFromJsonArray(QPlainTextEdit *textEditor, const QJsonArray &arr)
{
	bool firstLine = true;
	for (int i = 0; i < arr.size(); ++i)
	{
		if (firstLine)
			firstLine = false;
		else
			textEditor->textCursor().insertText("\n");
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
			pa.ExpectedStringParameter = false;
			pa.VarArgCount = 0;

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
				case CMD_GRADCOLOR:
				{
					DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa, editor->getLineText(i), editor->isCoprocessor(), true);
					if (pa.ExpectedParameterCount == 3) // Old RGB, upgrade
					{
						uint32_t rgb = pa.Parameter[0].U << 16
						    | pa.Parameter[1].U << 8
						    | pa.Parameter[2].U;
						pa.Parameter[0].U = rgb;
						pa.ExpectedParameterCount = 1;
						editor->replaceLine(i, pa);
					}
				}
				break;
				case CMD_GRADIENT:
				{
					DlParser::parse(FTEDITOR_CURRENT_DEVICE, pa, editor->getLineText(i), editor->isCoprocessor(), true);
					if (pa.ExpectedParameterCount == 10) // Old RGB, upgrade
					{
						uint32_t rgb0 = pa.Parameter[2].U << 16
						    | pa.Parameter[3].U << 8
						    | pa.Parameter[4].U;
						pa.Parameter[2].U = rgb0;
						pa.Parameter[3].U = pa.Parameter[5].U;
						pa.Parameter[4].U = pa.Parameter[6].U;
						uint32_t rgb1 = pa.Parameter[7].U << 16
						    | pa.Parameter[8].U << 8
						    | pa.Parameter[9].U;
						pa.Parameter[5].U = rgb1;
						pa.ExpectedParameterCount = 6;
						editor->replaceLine(i, pa);
					}
				}
				break;
				}
				break;
			}
		}
	}
}

QString MainWindow::getFileDialogPath()
{
	if (!m_LastProjectDir.isEmpty())
		return m_LastProjectDir;

	m_LastProjectDir = readLastProjectDir();

	if (m_LastProjectDir.isEmpty())
		m_LastProjectDir = qApp->applicationDirPath();
	
	return m_LastProjectDir;
}

void MainWindow::actOpen(QString projectPath)
{
	if (!maybeSave())
		return;

	if (projectPath.isEmpty())
	{
		projectPath = QFileDialog::getOpenFileName(this, tr("Open Project"), getFileDialogPath(),
		    tr("EVE Screen Editor Project (*.ese  *.ft800proj  *.ft8xxproj)"));
		if (projectPath.isEmpty())
			return;
	}

	m_MinFlashType = -1;
	openFile(projectPath);
	actResetEmulator();
}

void MainWindow::openFile(const QString &fileName)
{
	toggleUI(true);

	m_CurrentFile = fileName;

	// Remove temporary paths
	if (m_TemporaryDir)
	{
		QDir::setCurrent(QDir::tempPath());
		delete m_TemporaryDir;
		m_TemporaryDir = NULL;
	}

	// Set current project path
	QDir dir(fileName);
	dir.cdUp();
	QString dstPath = dir.path();
	QDir::setCurrent(dstPath);
	m_LastProjectDir = QDir::currentPath();
	writeLastProjectDir(m_LastProjectDir);

	// Reset editors to their default state
	clearEditor();


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
		QJsonArray content = root["content"].toArray();
		m_ContentManager->suppressOverlapCheck();

		bool checkFlashPath = false;
		for (int i = 0; i < content.size(); ++i)
		{
			ContentInfo *ci = new ContentInfo("");
			QJsonObject cio = content[i].toObject();
			ci->fromJson(cio, false);
			m_ContentManager->add(ci);

			if (!checkFlashPath && ci->Converter == ContentInfo::FlashMap)
			{
				checkFlashPath = true;
				checkAndPromptFlashPath(ci->SourcePath);
			}
		}

		documentFromJsonArray(m_DlEditor->codeEditor(), root["displayList"].toArray());
		documentFromJsonArray(m_CmdEditor->codeEditor(), root["coprocessor"].toArray());

		if (root.contains("bitmaps") || root.contains("handles"))
		{
			// Compatibility loading BITMAP_SETUP
			QJsonArray bitmaps = root[root.contains("bitmaps") ? "bitmaps" : "handles"].toArray();
			bitmapSetupfromJson(this, m_CmdEditor, bitmaps);
			// m_BitmapSetup->fromJson(bitmaps);
		}
		m_ContentManager->resumeOverlapCheck();
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
		m_TemporaryDir = new QTemporaryDir("ese-");
		QDir::setCurrent(m_TemporaryDir->path());
#else
		QDir::setCurrent(m_InitialWorkingDir);
#endif
		m_LastProjectDir = QDir::currentPath();
		writeLastProjectDir(m_LastProjectDir);
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

	m_AddRecentProjectFlag = true;
	removeRecentProject(fileName);
}

void MainWindow::setFlashFileNameToLabel(const QString &fileName)
{
	QString flashName(fileName);
	QString text;
	if (flashName.isEmpty())
	{
		text = tr("<i>No flash file is currently loaded.</i>");
		m_ProjectFlashFilename->setTextFormat(Qt::RichText);
	}
	else
	{
		text = m_ProjectFlashFilename->fontMetrics().elidedText(flashName, Qt::ElideMiddle, m_ProjectFlashFilename->width());
		m_ProjectFlashFilename->setTextFormat(Qt::PlainText);
	}
	m_ProjectFlashFilename->setProperty(PROPERTY_FLASH_FILE_NAME, flashName);
	m_ProjectFlashFilename->setText(text);
}

const bool MainWindow::isProjectSaved(void)
{
	return (false == m_CurrentFile.isEmpty());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_ProjectFlashFilename && event->type() == QEvent::Resize)
	{
		QString flashName = m_ProjectFlashFilename->property(PROPERTY_FLASH_FILE_NAME).toString();
		if (!flashName.isEmpty())
			setFlashFileNameToLabel(flashName);
	}
	else if ((watched == m_UtilizationBitmapHandleStatus ||
			  watched == m_UtilizationDisplayListStatus ||
		      watched == m_UtilizationGlobalStatus) &&
		     (event->type() == QEvent::HoverMove || 
			  event->type() == QEvent::HoverLeave))
	{
		bool isShowExact = (event->type() == QEvent::HoverMove);
		showExactNumberOfResourceWhenMouseHover(watched, isShowExact);
		return false;
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showExactNumberOfResourceWhenMouseHover(QObject *watched, const bool isShowExact)
{
	QProgressBar *pb = dynamic_cast<QProgressBar *>(watched);
	if (!pb)
		return;

	if (isShowExact)
		pb->setFormat("%v/%m");
	else
		pb->resetFormat();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls()) {
		foreach (QUrl url, event->mimeData()->urls())
		{
			if (url.toString().endsWith(".ese"))
			{
				event->setDropAction(Qt::LinkAction);
				event->accept();
				return;
			}
			else if (QDir d(url.toLocalFile()); d.exists())
			{
				// find only one file which ended by ".ese"
				if (QStringList s = d.entryList({ "*.ese" }, QDir::Files); s.count() == 1) {
					event->setDropAction(Qt::LinkAction);
					event->accept();
					return;
				}
			}
		}
	}
	QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();

	foreach(QUrl url, urls)
	{
		if (url.toString().endsWith(".ese")) {
			event->acceptProposedAction();
			actOpen(url.toLocalFile());
			return;
		}
		else if (QDir d(url.toLocalFile()); d.exists())
		{
			// find only one file which ended by ".ese"
			if (QStringList s = d.entryList({ "*.ese" }, QDir::Files); s.count() == 1)
			{
				event->acceptProposedAction();
				actOpen(d.absoluteFilePath(s[0]));
				return;
			}
		}
	}
	QMainWindow::dropEvent(event);
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
			if (!parsed.ValidId)
				line = "";
			else
				line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
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
	registers["hSize"] = g_HSize;
	registers["vSize"] = g_VSize;
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
	if (m_CurrentFile.isEmpty())
	{
		actSaveAs();
		return;
	}

	QFile file(m_CurrentFile);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	QByteArray data = toJson();

	out.writeRawData(data, data.size());

	m_UndoStack->setClean();

	m_AddRecentProjectFlag = true;
}

void MainWindow::actSaveAs()
{
	const QString fileExtend(".ese");

	QString filter = tr("ESE Project (*.ese)").arg(fileExtend);
	QString dirPath = getFileDialogPath();
	QString fileName;

	while (true)
	{
		fileName = QFileDialog::getSaveFileName(this, tr("Save Project"), dirPath, filter, &filter);

		if (fileName.isEmpty())
			return;

	    QDir dir(fileName);
		dir.cdUp();

		dirPath = dir.absolutePath();

		if (QFileInfo(fileName).baseName().length() > 20) {
			QMessageBox::warning(this, tr("Save Project"), tr("Project name must not exceed 20 characters!"));
			continue;
		}

		// check if dir empty
		if (dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0)
		{
			QMessageBox::warning(this, tr("Save Project"), tr("Please select an empty folder!"));
		}
		else
		{
			break;
		}
	}

	if (!fileName.endsWith(fileExtend))
		fileName = fileName + fileExtend;

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

		// Copy resource files
		QDir destDir(dstPath + '/' + ContentManager::ResourceDir);
		if (!destDir.exists())
		{
			destDir.mkpath(".");
		}

		QDir sourceDir(srcPath + '/' + ContentManager::ResourceDir);
		QStringList files = sourceDir.entryList(QDir::Files);
		for (int i = 0; i < files.count(); i++)
		{
			QString srcName = sourceDir.absolutePath() + '/' + files[i];
			QString destName = destDir.absolutePath() + '/' + files[i];
			QFile::copy(srcName, destName);
		}

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
		delete m_TemporaryDir;
		m_TemporaryDir = NULL;
	}

	// Set the folder to be the project folder
	QDir::setCurrent(dstPath);
	m_LastProjectDir = dstPath;

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

#define DUMP_VERSION_FT80X			(100)
#define DUMP_VERSION_FT81X		    (110)
#define DUMP_VERSION_BT81X		    (110)
#define DUMP_256K				    (256 * 1024)
#define DUMP_1K						(1024)
#define DUMP_8K						(8 * 1024)
#define DUMP_1024K					(1024 * 1024)

void MainWindow::actImport()
{
	if (!maybeSave())
		return;

	toggleUI(true);

	printf("*** Import ***\n");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), getFileDialogPath(),
	    tr("Memory dump (*.eve_dump)"));
	if (fileName.isNull())
		return;

	// reset editors to their default state
	clearEditor();

	m_CurrentFile = QString();

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

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return;

	QDataStream in(&file);
	bool loadOk = false;
		
	const size_t headersz = 6;
	uint32_t header[headersz];
	int s = in.readRawData(static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz);
	if (s != sizeof(uint32_t) * headersz)
	{
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete header"));
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

		ContentInfo *ramG = m_ContentManager->add(fileName);
		m_ContentManager->changeConverter(ramG, ContentInfo::Raw);
		m_ContentManager->changeMemoryAddress(ramG, 0);
		m_ContentManager->changeMemoryLoaded(ramG, true);
		m_ContentManager->changeRawStart(ramG, sizeof(uint32_t) * headersz);

		switch (header[0])
		{
		case DUMP_VERSION_FT80X:
			m_ProjectDevice->setCurrentIndex(FTEDITOR_FT800);
			m_ContentManager->changeRawLength(ramG, DUMP_256K);
			loadOk = importDumpFT80X(in);
			break;
		case DUMP_VERSION_FT81X:
			m_ProjectDevice->setCurrentIndex(FTEDITOR_FT810);
			m_ContentManager->changeRawLength(ramG, DUMP_1024K);
			loadOk = importDumpFT81X(in);
			break;
		default:
			QString message = QString("Invalid header version: %1").arg(header[0]);
			QMessageBox::critical(this, tr("Import .eve_dump"), message);
			break;
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
	m_PropertiesEditor->setInfo(tr("Imported project from .eve_dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);
	m_Toolbox->setEditorLine(m_DlEditor, 0);
}

bool MainWindow::importDumpFT80X(QDataStream & in)
{
	char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
	

	int s = in.readRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], DUMP_256K);
	if (s != DUMP_256K)
	{		
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete RAM_G"));
		return false;
	}

	ramaddr ramPal = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL);
	if (ramPal < 0)
		ramPal = DUMP_256K;
	s = in.readRawData(&ram[ramPal], DUMP_1K); // FIXME_GUI PALETTE
	if (s != DUMP_1K)
	{
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete RAM_PAL"));
		return false;
	}

	m_DlEditor->lockDisplayList();
	s = in.readRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), DUMP_8K);
	m_DlEditor->reloadDisplayList(false);
	m_DlEditor->unlockDisplayList();
	if (s != DUMP_8K)
	{
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete RAM_DL"));
		return false;
	}

	return true;
}

bool MainWindow::importDumpFT81X(QDataStream & in)
{
	char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));
	
	int s = in.readRawData(&ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], DUMP_1024K);
	if (s != DUMP_1024K)
	{	
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete RAM_G"));
		return false;
	}

	m_DlEditor->lockDisplayList();
	s = in.readRawData(static_cast<char *>(static_cast<void *>(m_DlEditor->getDisplayList())), DUMP_8K);
	m_DlEditor->reloadDisplayList(false);
	m_DlEditor->unlockDisplayList();
	if (s != DUMP_8K)
	{
		QMessageBox::critical(this, tr("Import .eve_dump"), tr("Incomplete RAM_DL"));
		return false;
	}
		
	return true;
}

bool MainWindow::importDumpBT81X(QDataStream & in)
{
	return importDumpFT81X(in); 
}

QString MainWindow::readLastProjectDir()
{
	return m_Settings.value("LastProjectDir").toString();
}

void MainWindow::writeLastProjectDir(QString dirPath)
{
	m_Settings.setValue("LastProjectDir", dirPath);
	m_Settings.sync();
}

void MainWindow::actExport()
{
	QString filterDump = tr("Memory dump (*.eve_dump)");
	QString filter = filterDump;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"), getFileDialogPath(), filter, &filter);
	if (fileName.isNull())
		return;

	if (filter == filterDump)
	{
		if (!fileName.endsWith(".eve_dump"))
			fileName = fileName + ".eve_dump";
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
		return;

	QDataStream out(&file);

	const size_t headersz = 6;
	uint32_t header[headersz];
	header[0] = 100;
	header[1] = g_HSize;
	header[2] = g_VSize;
	m_Macro->lockDisplayList();
	header[3] = m_Macro->getDisplayList()[0];
	header[4] = m_Macro->getDisplayList()[1];
	m_Macro->unlockDisplayList();
	header[5] = 0;

	// get ram content
	char *ram = static_cast<char *>(static_cast<void *>(BT8XXEMU_getRam(g_Emulator)));

	if (FTEDITOR_CURRENT_DEVICE <= FTEDITOR_FT801)
	{
		header[0] = DUMP_VERSION_FT80X;
		
		if (!writeDumpData(&out, static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz)) return;
		
		// <256K of main RAM_G>
		if (!writeDumpData(&out, &ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], DUMP_256K)) return;
		
		// <1K of main RAM_PAL>
		if (!writeDumpData(&out, &ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_PAL)], DUMP_1K)) return;
		
		// <8K of DL RAM> setup display list
		m_DlEditor->lockDisplayList();
		if (!writeDumpData(&out, static_cast<const char *>(static_cast<const void *>(BT8XXEMU_getDisplayList(g_Emulator))), DUMP_8K))
			return;
		m_DlEditor->unlockDisplayList();
	}
	else if (FTEDITOR_CURRENT_DEVICE < FTEDITOR_BT815)
	{
		header[0] = DUMP_VERSION_FT81X;
		if (!writeDumpData(&out, static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz)) return;

		// <1024K of main RAM>
		if (!writeDumpData(&out, &ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], DUMP_1024K)) return;

		// <8K of DL RAM>
		m_DlEditor->lockDisplayList();
		if (!writeDumpData(&out, static_cast<const char *>(static_cast<const void *>(BT8XXEMU_getDisplayList(g_Emulator))), DUMP_8K))
			return;
		m_DlEditor->unlockDisplayList();
	}
	else
	{
		header[0] = DUMP_VERSION_BT81X;
		if (!writeDumpData(&out, static_cast<char *>(static_cast<void *>(header)), sizeof(uint32_t) * headersz)) return;

		// <1024K of main RAM>
		if (!writeDumpData(&out, &ram[addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G)], DUMP_1024K)) return;

		// <8K of DL RAM>
		m_DlEditor->lockDisplayList();
		if (!writeDumpData(&out, static_cast<const char *>(static_cast<const void *>(BT8XXEMU_getDisplayList(g_Emulator))), DUMP_8K))
			return;
		m_DlEditor->unlockDisplayList();
	}

	file.close();

	statusBar()->showMessage(tr("Exported project to .eve_dump file"));
	m_PropertiesEditor->setInfo(tr("Exported project to .eve_dump file."));
	m_PropertiesEditor->setEditWidget(NULL, false, this);

	return;
}

bool MainWindow::writeDumpData(QDataStream * ds, const char* data, int size)
{
	int res = ds->writeRawData(data, size);
	if (res != size)
	{
		ds->device()->close();
		QMessageBox::critical(this, tr("Export"), tr("Failed to write file"));
		return false;
	}
	return true;
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

void MainWindow::bindCurrentDevice()
{
	m_Inspector->bindCurrentDevice();
	m_DlEditor->bindCurrentDevice();
	m_CmdEditor->bindCurrentDevice();
	m_Macro->bindCurrentDevice();
	m_Toolbox->bindCurrentDevice();
	m_ContentManager->bindCurrentDevice();
	m_InteractiveProperties->bindCurrentDevice();
}

void MainWindow::stopEmulatorInternal()
{
	printf("Stop the emulator\n");
	g_EmulatorRunning = false;
	m_EmulatorViewport->stop();
	cleanupMediaFifo();
}

void MainWindow::startEmulatorInternal()
{
	printf("Start the emulator\n");
	BT8XXEMU_EmulatorParameters params;
	memset(&params, 0, sizeof(BT8XXEMU_EmulatorParameters));
	params.Main = emuMain;
	params.Flags = BT8XXEMU_EmulatorEnableMouse
	    | BT8XXEMU_EmulatorEnableAudio
	    | BT8XXEMU_EmulatorEnableCoprocessor
	    | BT8XXEMU_EmulatorEnableGraphicsMultithread
	    | BT8XXEMU_EmulatorEnableMainPerformance;
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT817)
		params.Flags |= BT8XXEMU_EmulatorEnableHSFPreview;
	params.Mode = deviceToEnum(FTEDITOR_CURRENT_DEVICE);
	params.Close = closeDummy;
	g_EmulatorRunning = true;
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
	if (changeDevice)
		FTEDITOR_CURRENT_DEVICE = deviceIntf;
	if (changeFlash)
		FTEDITOR_CURRENT_FLASH = flashIntf;

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
	bindCurrentDevice();

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
		m_ProjectFlashLayout->setVisible(flashSupport(FTEDITOR_CURRENT_DEVICE));
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
	ProjectDeviceCommand(int deviceIntf, MainWindow *mainWindow)
	    : QUndoCommand()
	    , m_NewProjectDevice(deviceIntf)
	    , m_OldProjectDevice(FTEDITOR_CURRENT_DEVICE)
	    , m_MainWindow(mainWindow)
	{
	}
	virtual ~ProjectDeviceCommand() {}
	virtual void undo()
	{
		m_MainWindow->changeEmulatorInternal(m_OldProjectDevice, FTEDITOR_CURRENT_FLASH);
		s_UndoRedoWorking = true;
		m_MainWindow->m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE);
		s_UndoRedoWorking = false;
	}
	virtual void redo()
	{
		m_MainWindow->changeEmulatorInternal(m_NewProjectDevice, FTEDITOR_CURRENT_FLASH);
		s_UndoRedoWorking = true;
		m_MainWindow->m_ProjectDevice->setCurrentIndex(FTEDITOR_CURRENT_DEVICE);
		s_UndoRedoWorking = false;
	}
	virtual int id() const { return 98919600; }
	virtual bool mergeWith(const QUndoCommand *command)
	{
		m_NewProjectDevice = static_cast<const ProjectDeviceCommand *>(command)->m_NewProjectDevice;
		return true;
	}

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
	ProjectFlashCommand(int flashIntf, MainWindow *mainWindow)
	    : QUndoCommand()
	    , m_NewProjectFlash(flashIntf)
	    , m_OldProjectFlash(FTEDITOR_CURRENT_FLASH)
	    , m_MainWindow(mainWindow)
	{
	}
	virtual ~ProjectFlashCommand() {}
	virtual void undo()
	{
		m_MainWindow->changeEmulatorInternal(FTEDITOR_CURRENT_DEVICE, m_OldProjectFlash);
		s_UndoRedoWorking = true;
		m_MainWindow->m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_FLASH);
		s_UndoRedoWorking = false;
	}
	virtual void redo()
	{
		m_MainWindow->changeEmulatorInternal(FTEDITOR_CURRENT_DEVICE, m_NewProjectFlash);
		s_UndoRedoWorking = true;
		m_MainWindow->m_ProjectFlash->setCurrentIndex(FTEDITOR_CURRENT_FLASH);
		s_UndoRedoWorking = false;
	}
	virtual int id() const { return 98919601; }
	virtual bool mergeWith(const QUndoCommand *command)
	{
		m_NewProjectFlash = static_cast<const ProjectFlashCommand *>(command)->m_NewProjectFlash;
		return true;
	}

private:
	int m_NewProjectFlash;
	int m_OldProjectFlash;
	MainWindow *m_MainWindow;
};

void MainWindow::projectFlashChanged(int flashIntf)
{
	/*

	Permit this case, since ignoring the change callback breaks the consistency between UI state and internal state!
	From technical POV there is no issue in having a flash that's too small, the extra content will simply not be loaded
	The emulator is also masking the flash addresses, so that address overrun does not occur

    if (flashIntf <= m_MinFlashType)
        return;

	*/

	if (s_UndoRedoWorking)
		return;

	m_UndoStack->push(new ProjectFlashCommand(flashIntf, this));
}

void MainWindow::openRecentProject()
{
	QAction *pAction = (QAction *)sender();
	QString projectPath = pAction->data().toString();

	if (!QFile::exists(projectPath))
	{
		removeRecentProject(projectPath);
		QMessageBox::critical(this, tr(""), QString(tr("%1 cannot be opened.")).arg(projectPath));
		return;
	}

	if (!maybeSave())
		return;

	openFile(projectPath);
}

void FTEDITOR::MainWindow::propertyErrorSet(QString info)
{
	appendTextToOutputDock(info);
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

void MainWindow::handleSaveDL() {
	SaveOptionDialog saveOptDlg(SaveOptionDialog::DisplayList, this);
	connect(
		&saveOptDlg, &SaveOptionDialog::handleAccept, this,
		[this](QString fileName, int outputType, int byteOrder,
			bool autoOpen) {
				if (outputType == SaveOptionDialog::Binary) {
					saveDLToBinaryFile(
						fileName,
						byteOrder == SaveOptionDialog::LittleEndian ? false
						: true);
				} else {
					saveDisplayListToTextFile(
						fileName,
						byteOrder == SaveOptionDialog::LittleEndian ? false
						: true);
				}
	if (autoOpen)
		QDesktopServices::openUrl(
			QUrl(fileName, QUrl::TolerantMode));
		});
	saveOptDlg.exec();
}

void MainWindow::handleSaveCoproCmd() {
	SaveOptionDialog saveOptDlg(SaveOptionDialog::CoprocessorCommand,
		this);
	connect(
		&saveOptDlg, &SaveOptionDialog::handleAccept, this,
		[this](QString fileName, int outputType, int byteOrder,
			bool autoOpen) {
				if (outputType == SaveOptionDialog::Binary) {
					saveCoproToBinaryFile(
						fileName,
						byteOrder == SaveOptionDialog::LittleEndian ? false
						: true);
				} else {
					saveCoproCmdToTextFile(
						fileName,
						byteOrder == SaveOptionDialog::LittleEndian ? false
						: true);
				}
	if (autoOpen)
		QDesktopServices::openUrl(
			QUrl(fileName, QUrl::TolerantMode));
		});
	saveOptDlg.exec();
}

void MainWindow::saveDisplayListToTextFile(QString fileName, bool isBigEndian)
{
	auto data = m_Inspector->getDLContent(isBigEndian);
	ReadWriteUtil::writeText(fileName, data);
}

void MainWindow::saveCoproCmdToTextFile(QString fileName, bool isBigEndian)
{
	auto data = m_CmdEditor->getCoproCmdText(isBigEndian);
	ReadWriteUtil::writeText(fileName, data);
}

void MainWindow::saveDLToBinaryFile(QString fileName, bool isBigEndian)
{
	auto data = m_Inspector->getDLBinary(isBigEndian);
	ReadWriteUtil::writeBinary(fileName, data);
}

void MainWindow::saveCoproToBinaryFile(QString fileName, bool isBigEndian)
{
	auto data = m_CmdEditor->getCoproCmdBinary(isBigEndian);
	ReadWriteUtil::writeBinary(fileName, data);
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
	QString ug_path = QString("/Manual/BRT_AN_037_EVE Screen Editor %1 User Guide.pdf").arg(STR_VERSION);
	QDesktopServices::openUrl(QUrl::fromLocalFile(m_ApplicationDataDir + ug_path));
}

void MainWindow::about()
{
	QStringList versionLines = QString::fromLatin1(BT8XXEMU_version()).split('\n');
	QString emulatorVersion = versionLines.length() ? versionLines[0].trimmed() : QString();
	   	 
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(QString(tr("About EVE Screen Editor v%1")).arg(STR_PRODUCTVERSION));
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setText(tr(
	    "Copyright (C) 2013-2015  Future Technology Devices International Ltd<br>"
	    "<br>"
	    "Copyright (C) 2016-2022  Bridgetek Pte Ltd<br>"
	    "<br>"
		"%1<br>"
		"<br>"
	    "Support and updates:<br>"
	    "<a href='http://www.ftdichip.com/Support/Utilities.htm'>http://www.ftdichip.com/Support/Utilities.htm</a><br>"
	    "<br>"
	    "<a href='http://brtchip.com/utilities/#evescreeneditor'>http://brtchip.com/utilities/#evescreeneditor</a>").arg(emulatorVersion));
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
		"Python\n"
		"Copyright (C) 2001-2023\n"
		"https://www.python.org/"
		"\n\n"
		"Fugue Icons\n"
		"(C) 2013 Yusuke Kamiyamane. All rights reserved.\n"
		"These icons are licensed under a Creative Commons"
		"Attribution 3.0 License.\n"
		"http://creativecommons.org/licenses/by/3.0"
		"\n\n"
		"virinext/QHexView\n"
		"virinext/QHexView is licensed under the MIT License\n"
		"Copyright (C) 2015\n"));
}

QString MainWindow::getProjectContent() const
{
	QFile file(m_CurrentFile);
	file.open(QIODevice::ReadOnly);
	QTextStream ts(&file);
	QString projectContent = ts.readAll();
	file.close();

	return projectContent;
}

QString MainWindow::getDisplaySize()
{
	return QString("%1x%2").arg(m_HSize->text()).arg(m_VSize->text());
}

void FTEDITOR::MainWindow::appendTextToOutputDock(const QString &text)
{
	if (m_OutputTextEdit->toPlainText().contains(text))
		return;

	m_OutputTextEdit->append(text);
}

void MainWindow::updateLoadingIcon()
{
	m_CoprocessorBusy->setVisible((g_ShowCoprocessorBusy && !g_WaitingCoprocessorAnimation)
	    || !busyList.isEmpty());
}

void MainWindow::appendBusyList(QObject *obj)
{
    if (busyList.contains(obj))
        return;
    busyList.append(obj);
    updateLoadingIcon();
}

void MainWindow::removeBusyList(QObject *obj)
{
    busyList.removeOne(obj);
    updateLoadingIcon();
}

} /* namespace FTEDITOR */

/* end of file */
