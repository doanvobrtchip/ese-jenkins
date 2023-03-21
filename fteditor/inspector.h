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
class QSplitter;

namespace FTEDITOR {

#define FTED_NUM_HANDLES 16

class MainWindow;
class RamDL;
class RamG;
class RamReg;
class RamCMD;

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
	void addSplitter(QWidget *widget);
	
	RamG *ramG() const;
	RamDL *ramDL() const;
	RamReg *ramReg() const;
	RamCMD *ramCMD() const;
	
private:
	MainWindow *m_MainWindow;

	bool m_HandleUsage[FTED_NUM_HANDLES];
	
	QSplitter *m_Splitter;
	RamG *m_RamG;
	RamDL *m_RamDL;
	RamReg *m_RamReg;
	RamCMD *m_RamCMD;
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
