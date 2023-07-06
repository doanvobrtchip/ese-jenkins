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

#ifndef FTEDITOR_MAIN_WINDOW_H
#define FTEDITOR_MAIN_WINDOW_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>
#include <map>
#include <string>

// Qt includes
#include <QMainWindow>
#include <QString>
#include <QByteArray>
#include <QSettings>

// Emulator includes

// Project includes
#include "device_manager.h" // for the #define

class QTemporaryDir;
class QTreeView;
class QDirModel;
class QUndoStack;
class QScrollArea;
class QSpinBox;
class QCheckBox;
class QTabBar;
class QProgressBar;
class QComboBox;
class QLabel;
class QMenu;
class QGroupBox;
class QHBoxLayout;
class QTextEdit;

namespace FTEDITOR {

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
	RunScript()
	    : Action(NULL)
	    , Window(NULL)
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

	const char *PROPERTY_FLASH_FILE_NAME = "originalFlashFileName";

public:
	MainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
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
	inline QComboBox *projectFlash() { return m_ProjectFlash; }
	inline QDockWidget *inspectorDock() { return m_InspectorDock; }

	inline const QString &applicationDataDir() { return m_ApplicationDataDir; }

	bool waitingCoprocessorAnimation();

	void focusDlEditor(bool forceOnly = false);
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

	void userChangeResolution(int hsize, int vsize);

	void openFile(const QString &fileName);

	void setFlashFileNameToLabel(const QString &fileName);

	const bool isProjectSaved(void);

	bool eventFilter(QObject *watched, QEvent *event);
	void requestSave() { actSave(); }
	bool checkAndPromptFlashPath(const QString &filePath);

	void toggleDockWindow(bool isShow);
	void toggleUI(bool hasProject);

	QString getProjectContent(void) const;

	QString getDisplaySize();

	void appendTextToOutputDock(const QString &text);
	void updateLoadingIcon();
	
	const QList<QAction *> &RecentActionList() const;
	
private slots:
	// void applyEmulatorConfig();

	void manual();
	void about();
	void aboutQt();
	void dummyCommand();
	
	void actNew();
	void actSave();
	void actCloseProject();
	void actImport();
	void actExport();

	void actProjectFolder();
	void actSaveScreenshot();
	void actImportDisplayList();
	void handleSaveDL();
	void handleSaveCoproCmd();
	void actDisplayListFromIntegers();

	void undoCleanChanged(bool clean);

	void hsizeChanged(int hsize);
	void vsizeChanged(int vsize);
	void rotateChanged(int rotate);

	void stepEnabled(bool enabled);
	void stepChanged(int step);

	void stepCmdEnabled(bool enabled);
	void stepCmdChanged(int step);

	void traceEnabledChanged(bool enabled);

	void editorTabChanged(int i);
	void tabChanged(int i);

	void refreshScriptsMenu();
	void updateWindowTitle();

	void projectDeviceChanged(int i);
	void projectDisplayChanged(int i);
	void projectFlashChanged(int i);

	void openRecentProject();
	
public slots:
	void popupTimeout();
	void actResetEmulator();
	void propertyErrorSet(QString info);
	void updateProgressBars();
	void appendBusyList(QObject *obj);
	void removeBusyList(QObject *obj);
	void openProject(QString projectPath);
	bool actSaveAs();
	bool actOpen(QString projectPath = QString());

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

	void updateProjectDisplay(int hsize, int vsize);

	bool maybeSave();

	void saveCoproCmdToTextFile(QString fileName, bool isBigEndian);
	void saveDisplayListToTextFile(QString fileName, bool isBigEndian);
	void saveDLToBinaryFile(QString fileName, bool isBigEndian);
	void saveCoproToBinaryFile(QString fileName, bool isBigEndian);

#ifdef FT800EMU_PYTHON
	QString scriptModule(QString folderName);
	QString scriptDir(QString folderName);
#endif

	void bindCurrentDevice();

	void stopEmulatorInternal();
	void startEmulatorInternal();
	void changeEmulatorInternal(int deviceIntf, int flashIntf);

	void loadRecentProject();
	void addRecentProject(QString recentPath);
	void removeRecentProject(QString removePath);
	void saveRecentProject();

	bool writeDumpData(QDataStream *ds, const char *data, int size);

	bool importDumpFT80X(QDataStream &ds);
	bool importDumpFT81X(QDataStream &ds);
	bool importDumpBT81X(QDataStream &ds);

	QString readLastProjectDir();
	void writeLastProjectDir(QString dirPath);

	void showExactNumberOfResourceWhenMouseHover(QObject *watched, const bool isShowExact);

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	
protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	MainWindow(const MainWindow &);
	MainWindow &operator=(const MainWindow &);

private:
	int m_MinFlashType;
	bool m_AddRecentProjectFlag;
	QAction *m_RecentSeparator;
	QList<QAction *> m_RecentActionList;
	QStringList m_RecentPathList;
	QString m_InitialWorkingDir;
	QString m_ApplicationDataDir;
	QUndoStack *m_UndoStack;

	QSettings m_Settings;

	InteractiveViewport *m_EmulatorViewport;

	DlEditor *m_DlEditor;
	QDockWidget *m_DlEditorDock;
	DlEditor *m_CmdEditor;
	QDockWidget *m_CmdEditorDock;

	QWidget *m_ErrorFrame;
	QLabel *m_ErrorLabel;

	QLabel *m_CursorPosition;
	QLabel *m_PixelColor;
	QLabel *m_CoprocessorBusy;

	QDockWidget *m_NavigatorDock;
	EmulatorNavigator *m_EmulatorNavigator;

	PropertiesEditor *m_PropertiesEditor;
	QScrollArea *m_PropertiesEditorScroll;
	QDockWidget *m_PropertiesEditorDock;

	QTextEdit *m_OutputTextEdit = nullptr;
	QDockWidget *m_OutputDock = nullptr;

	InteractiveProperties *m_InteractiveProperties;

	QDockWidget *m_ContentManagerDock;
	ContentManager *m_ContentManager;

	//QDockWidget *m_BitmapSetupDock;
	//BitmapSetup *m_BitmapSetup;

	QDockWidget *m_ToolboxDock;
	Toolbox *m_Toolbox;

	QDockWidget *m_InspectorDock;
	Inspector *m_Inspector;

  QDockWidget *m_RulerDock;

#if FT800_DEVICE_MANAGER
	QDockWidget *m_DeviceManagerDock;
	DeviceManager *m_DeviceManager;
#endif /* FT800_DEVICE_MANAGER */

	QDockWidget *m_UtilizationDock;
	QProgressBar *m_UtilizationDisplayList;
	QProgressBar *m_UtilizationBitmapHandleStatus;
	QProgressBar *m_UtilizationDisplayListStatus;
	QProgressBar *m_UtilizationGlobalStatus;

	QDockWidget *m_ProjectDock;
	QComboBox *m_ProjectDevice;
	QComboBox *m_ProjectDisplay;
	QWidget *m_ProjectFlashLayout;
	QComboBox *m_ProjectFlash;
	QPushButton *m_ProjectFlashImport;
	QLabel *m_ProjectFlashFilename;

	QDockWidget *m_RegistersDock;
	DlEditor *m_Macro;
	QSpinBox *m_HSize;
	QSpinBox *m_VSize;
	QSpinBox *m_Rotate;

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
	QMenu *m_ToolsMenu;
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
	QAction *m_WelcomeAct;
	QAction *m_CloseProjectAct;
	QAction *m_ImportAct;
	QAction *m_ExportAct;
	QAction *m_ProjectFolderAct;
	QAction *m_ResetEmulatorAct;
	QAction *m_SaveScreenshotAct;
	QAction *m_ImportDisplayListAct;

	QAction *m_SaveDisplayListAct;
	QAction *m_SaveCoproCmdAct;

	QAction *m_DisplayListFromIntegers;
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
	std::map<QString, QMenu *> m_ScriptFolderMenus;

	QString m_CurrentFile;
	QTemporaryDir *m_TemporaryDir;
	QString m_LastProjectDir;

	friend class ProjectDeviceCommand;
	friend class ProjectFlashCommand;

	QLabel *infoLabel;
	QList<QObject *> busyList;
	
signals:
	void utilizationDisplayListCmdChanged(int value);
	void ramGChanged(uint8_t *value);
	void readyToSetup(QObject *obj);
	void deviceChanged();
	void windowActivate();
}; /* class MainWindow */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_MAIN_WINDOW_H */

/* end of file */
