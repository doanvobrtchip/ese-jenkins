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

// Project includes
#include "dl_editor.h"

class QTreeWidget;
class QTreeWidgetItem;
class QEvent;
class QKeyEvent;
class QMenu;
class QAction;

namespace FTEDITOR {

#define FTED_NUM_HANDLES 16

class MainWindow;

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

	bool eventFilter(QObject* watched, QEvent* event);

	QString getDisplayListContent(bool isBigEndian = false);  // 0: little; 1: big

private slots:
    void onCopy();
    void onPrepareContextMenu(const QPoint &pos);

private:
	void initDisplayReg();
	void releaseDisplayReg();

	void copy(const QTreeWidget *widget);

	MainWindow *m_MainWindow;

	QTreeWidget *m_DisplayList;
	QTreeWidgetItem *m_DisplayListItems[FTEDITOR_DL_SIZE];
	uint32_t m_DisplayListCopy[FTEDITOR_DL_SIZE];
	bool m_DisplayListUpdate[FTEDITOR_DL_SIZE];

	QTreeWidget *m_Registers;
	std::vector<QTreeWidgetItem *> m_RegisterItems;
	std::vector<uint32_t> m_RegisterCopy;

	bool m_HandleUsage[FTED_NUM_HANDLES];

    QMenu   *m_ContextMenu;
    QAction *m_CopyAct;

private:
	Inspector(const Inspector &);
	Inspector &operator=(const Inspector &);

}; /* class Inspector */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INSPECTOR_H */

/* end of file */
