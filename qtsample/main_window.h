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

#ifndef FTQT_MAIN_WINDOW_H
#define FTQT_MAIN_WINDOW_H

// STL includes

// Qt includes
#include <QMainWindow>

// Emulator includes

// Project includes

class QTreeView;
class QDirModel;
class QUndoStack;
class QScrollArea;

namespace FTQT {
	class CommandLog;
	class EmulatorViewport;
	// class EmulatorConfig;

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
	
private:
	void updateInitialization(bool visible);

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
	
	CommandLog *m_CommandLog;
	QDockWidget *m_CommandLogDock;

	EmulatorViewport *m_EmulatorViewport;

	/*CEmulatorConfig *m_EmulatorConfig;
	QScrollArea *m_EmulatorConfigScroll;
	QDockWidget *m_EmulatorConfigDock;*/

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

	QAction *m_AboutAct;
	QAction *m_QuitAct;
	// QAction *m_PrintDebugAct;
	QAction *m_UndoAct;
	QAction *m_RedoAct;
	// QAction *m_SaveScreenshotAct;

}; /* class MainWindow */

} /* namespace FTQT */

#endif /* #ifndef FTQT_MAIN_WINDOW_H */

/* end of file */
