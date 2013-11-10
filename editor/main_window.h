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

// Qt includes
#include <QMainWindow>
#include <QString>

// Emulator includes

// Project includes

class QTreeView;
class QDirModel;
class QUndoStack;
class QScrollArea;

namespace FT800EMUQT {
	// class CommandLog;
	class EmulatorViewport;
	class DlEditor;
	// class EmulatorConfig;
	class PropertiesEditor;

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

private slots:
	// void applyEmulatorConfig();

	void about();
	void aboutQt();
	void dummyCommand();
	
	void actNew();
	void actOpen();
	void actSave();
	void actSaveAs();
	void actImport();
	void actExport();
	
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

	void recalculateMinimumWidth();

	void incbLanguageCode();

private:
	MainWindow(const MainWindow &);
	MainWindow &operator=(const MainWindow &);

private:

	QUndoStack *m_UndoStack;

	EmulatorViewport *m_EmulatorViewport;
	
	DlEditor *m_DlEditor;
	QDockWidget *m_DlEditorDock;
	
	PropertiesEditor *m_PropertiesEditor;
	QScrollArea *m_PropertiesEditorScroll;
	QDockWidget *m_PropertiesEditorDock;

	QTreeView *m_AssetTreeView;
	QDirModel *m_AssetTreeModel;
	QDockWidget *m_AssetTreeDock;
	
	QMenu *m_FileMenu;
	QMenu *m_EditMenu;
	QMenu *m_ViewportMenu;
	QMenu *m_WidgetsMenu;
	QMenu *m_HelpMenu;

	QToolBar *m_FileToolBar;
	QToolBar *m_EditToolBar;

	QAction *m_NewAct;
	QAction *m_OpenAct;
	QAction *m_SaveAct;
	QAction *m_SaveAsAct;
	QAction *m_ImportAct;
	QAction *m_ExportAct;
	QAction *m_AboutAct;
	QAction *m_AboutQtAct;
	QAction *m_QuitAct;
	// QAction *m_PrintDebugAct;
	QAction *m_UndoAct;
	QAction *m_RedoAct;
	QAction *m_DummyAct;
	// QAction *m_SaveScreenshotAct;
	
	QString m_CurrentFile;

}; /* class MainWindow */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_MAIN_WINDOW_H */

/* end of file */
