/**
 * main_window.h
 * $Id$
 * \file main_window.h
 * \brief main_window.h
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_MAIN_WINDOW_H
#define FT800EMUQT_MAIN_WINDOW_H

// STL includes
#include <vector>
#include <map>
#include <string>

// Qt includes
#include <QMainWindow>
#include <QString>
#include <QByteArray>

// Emulator includes

// Project includes
#include "device_manager.h" // for the #define

#define FT_VCDUMP_VISIBLE false

class QTemporaryDir;
class QTreeView;
class QDirModel;
class QUndoStack;
class QScrollArea;
class QSpinBox;
class QCheckBox;
class QTabBar;
class QProgressBar;

namespace FT800EMUQT {

class InteractiveViewport;
class DlEditor;
class PropertiesEditor;
class Toolbox;
class DeviceManager;
class Inspector;
class MainWindow;
class ContentManager;
// class BitmapSetup;
class InteractiveProperties;
class EmulatorNavigator;

class RunScript : public QObject
{
	Q_OBJECT

public:
	RunScript() : Action(NULL), Window(NULL)
	{

	}

	virtual ~RunScript();

	QAction *Action;
	QString Script;
	MainWindow *Window;

public slots:
	void runScript();

};

/**
 * MainWindow
 * \brief MainWindow
 * \date 2010-02-05 13:01GMT
 * \author Jan Boon (Kaetemi)
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

	inline DlEditor *dlEditor() { return m_DlEditor; }
	inline DlEditor *cmdEditor() { return m_CmdEditor; }
	inline InteractiveViewport *viewport() { return m_EmulatorViewport; }
	inline Toolbox *toolbox() { return m_Toolbox; }
	inline Inspector *inspector() { return m_Inspector; }
	inline QUndoStack *undoStack() { return m_UndoStack; }
	inline PropertiesEditor *propertiesEditor() { return m_PropertiesEditor; }
	inline ContentManager *contentManager() { return m_ContentManager; }
	// inline BitmapSetup *bitmapSetup() { return m_BitmapSetup; }
	inline InteractiveProperties *interactiveProperties() { return m_InteractiveProperties; }

	bool waitingCoprocessorAnimation();

	void focusDlEditor();
	void focusCmdEditor();
	void focusProperties();

	void setTraceEnabled(bool enabled);
	void setTraceX(int x);
	void setTraceY(int y);

	bool traceEnabled();
	int traceX();
	int traceY();

	// Tracking coprocessor command used to write display list i
	int *getDlCmd();

	void runScript(const QString &script);

	void frameEmu();
	void frameQt();

	QString getFileDialogPath();

	QByteArray toJson(bool exportScript = false);

	void editorTabChangedGo(bool load);

	void actNew(bool addClear);

private slots:
	// void applyEmulatorConfig();

	void manual();
	void about();
	void aboutQt();
	void dummyCommand();

	void actNew();
	void actOpen();
	void actSave();
	void actSaveAs();
	void actImport();
	void actExport();

	void undoCleanChanged(bool clean);

	void hsizeChanged(int hsize);
	void vsizeChanged(int vsize);

	void stepEnabled(bool enabled);
	void stepChanged(int step);

	void stepCmdEnabled(bool enabled);
	void stepCmdChanged(int step);

	void traceEnabledChanged(bool enabled);

	void editorTabChanged(int i);
	void tabChanged(int i);

	void refreshScriptsMenu();
	void updateWindowTitle();

private:
	void updateInitialization(bool visible);

	void clearEditor();
	void clearUndoStack();

	void createActions();
	void translateActions();
	void createMenus();
	void translateMenus();
	void createToolBars();
	void translateToolBars();
	void createStatusBar();
	void createDockWindows();
	void translateDockWindows();

	void incbLanguageCode();

	bool maybeSave();

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	MainWindow(const MainWindow &);
	MainWindow &operator=(const MainWindow &);

private:

	QString m_InitialWorkingDir;
	QUndoStack *m_UndoStack;

	InteractiveViewport *m_EmulatorViewport;

	DlEditor *m_DlEditor;
	QDockWidget *m_DlEditorDock;
	DlEditor *m_CmdEditor;
	QDockWidget *m_CmdEditorDock;

	QDockWidget *m_NavigatorDock;
	EmulatorNavigator *m_EmulatorNavigator;

	PropertiesEditor *m_PropertiesEditor;
	QScrollArea *m_PropertiesEditorScroll;
	QDockWidget *m_PropertiesEditorDock;

	InteractiveProperties *m_InteractiveProperties;

	QDockWidget *m_ContentManagerDock;
	ContentManager *m_ContentManager;

	//QDockWidget *m_BitmapSetupDock;
	//BitmapSetup *m_BitmapSetup;

	QDockWidget *m_ToolboxDock;
	Toolbox *m_Toolbox;

	QDockWidget *m_InspectorDock;
	Inspector *m_Inspector;

#if FT800_DEVICE_MANAGER
	QDockWidget *m_DeviceManagerDock;
	DeviceManager *m_DeviceManager;
#endif /* FT800_DEVICE_MANAGER */

	QDockWidget *m_UtilizationDock;
	QProgressBar *m_UtilizationDisplayList;
	QProgressBar *m_UtilizationBitmapHandleStatus;
	QProgressBar *m_UtilizationDisplayListStatus;
	QProgressBar *m_UtilizationGlobalStatus;

	QDockWidget *m_RegistersDock;
	DlEditor *m_Macro;
	QSpinBox *m_HSize;
	QSpinBox *m_VSize;

	std::vector<QTabBar *> m_HookedTabs;

	// Controls
	QDockWidget *m_ControlsDock;
	QCheckBox *m_StepEnabled;
	QSpinBox *m_StepCount;
	QCheckBox *m_StepCmdEnabled;
	QSpinBox *m_StepCmdCount;
	QCheckBox *m_TraceEnabled;
	QSpinBox *m_TraceX;
	QSpinBox *m_TraceY;

	QMenu *m_FileMenu;
	QMenu *m_EditMenu;
	QMenu *m_ViewportMenu;
	QMenu *m_WidgetsMenu;
#ifdef FT800EMU_PYTHON
	QMenu *m_ScriptsMenu;
#endif
	QMenu *m_HelpMenu;

	QToolBar *m_FileToolBar;
	QToolBar *m_EditToolBar;

	bool m_CleanUndoStack;

	QAction *m_NewAct;
	QAction *m_OpenAct;
	QAction *m_SaveAct;
	QAction *m_SaveAsAct;
	QAction *m_ImportAct;
	QAction *m_ExportAct;
	QAction *m_ManualAct;
	QAction *m_AboutAct;
	QAction *m_AboutQtAct;
	QAction *m_QuitAct;
	// QAction *m_PrintDebugAct;
	QAction *m_UndoAct;
	QAction *m_RedoAct;
	QAction *m_DummyAct;
	// QAction *m_SaveScreenshotAct;
	std::map<QString, RunScript *> m_ScriptActs;

	QString m_CurrentFile;
	QTemporaryDir *m_TemporaryDir;

}; /* class MainWindow */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_MAIN_WINDOW_H */

/* end of file */
