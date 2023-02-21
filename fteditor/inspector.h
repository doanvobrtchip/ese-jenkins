/**
/**
 * inspector.h
 * $Id$
 * \file inspector.h
 * \brief inspector.h
 * \date 2014-01-29 16:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_INSPECTOR_H
#define FTEDITOR_INSPECTOR_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>

// Qt includes
#include <QWidget>

// Emulator includes
#include "bt8xxemu_inttypes.h"

class QTreeWidget;
class QTreeWidgetItem;
class QEvent;
class QKeyEvent;
class QMenu;
class QAction;

namespace FTEDITOR {

#define FTED_NUM_HANDLES 16

class MainWindow;
class RamDLDockWidget;
class RamDLInspector;
class RamGDockWidget;
class RamGInspector;
class RamRegDockWidget;
class RamRegInspector;

/**
 * Inspector
 * \brief Inspector
 * \date 2014-01-29 16:53GMT
 * \author Jan Boon (Kaetemi)
 */
class Inspector : public QWidget
{
	Q_OBJECT

public:
	Inspector(MainWindow *parent);
	virtual ~Inspector();

	void bindCurrentDevice();
	void unbindCurrentDevice();

	void frameEmu();
	void frameQt();

	int countHandleUsage();
	void setCountHandleUsage(int value);

	QByteArray getDLBinary(bool isBigEndian);
	QString getDLContent(bool isBigEndian = false);

	MainWindow *mainWindow() { return m_MainWindow; }
	RamGInspector *ramGInspector() const;

	RamGDockWidget *ramGDockWidget() const;
	void initRamGDockWidget();

	RamRegDockWidget *ramRegDockWidget() const;
	void initRamRegDockWidget();

	RamDLDockWidget *ramDLDockWidget() const;
	void initRamDLDockWidget();

private:
	MainWindow *m_MainWindow;

	bool m_HandleUsage[FTED_NUM_HANDLES];

	RamDLInspector *m_RamDL;
	RamDLDockWidget *m_RamDLDockWidget;

	RamRegInspector *m_RamReg;
	RamRegDockWidget *m_RamRegDockWidget;

	RamGInspector *m_RamG;
	RamGDockWidget *m_RamGDockWidget;

	int m_countHandleBitmap;

private:
	Inspector(const Inspector &);
	Inspector &operator=(const Inspector &);

signals:
	void countHandleBitmapChanged(int value);
	void updateView(int dlCMDCount);
	void updateData();
	void initDisplayReg();
	void releaseDisplayReg();

}; /* class Inspector */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INSPECTOR_H */

/* end of file */
