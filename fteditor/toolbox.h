/**
 * toolbox.h
 * $Id$
 * \file toolbox.h
 * \brief toolbox.h
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_TOOLBOX_H
#define FTEDITOR_TOOLBOX_H

// STL includes
#include <vector>

// Qt includes
#include <QWidget>

// Emulator includes
#include <ft8xxemu_inttypes.h>

// Project includes

class QTreeWidget;
class QTreeWidgetItem;

namespace FTEDITOR {

class MainWindow;
class DlEditor;

/**
 * Toolbox
 * \brief Toolbox
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */
class Toolbox : public QWidget
{
	Q_OBJECT

public:
	Toolbox(MainWindow *parent);
	virtual ~Toolbox();

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void unsetEditorLine();

	QTreeWidget *treeWidget() { return m_Tools; }

	void bindCurrentDevice();

	uint32_t getSelectionType();
	uint32_t getSelectionId();

private slots:
	void currentSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
	MainWindow *m_MainWindow;
	QTreeWidget *m_Tools;

	QTreeWidgetItem *m_Background;
	QTreeWidgetItem *m_Primitives;
	QTreeWidgetItem *m_Widgets;
	QTreeWidgetItem *m_Utilities;
	QTreeWidgetItem *m_Graphics;
	QTreeWidgetItem *m_Bitmaps;
	// QTreeWidgetItem *m_Advanced;
	std::vector<QTreeWidgetItem *> m_CoprocessorTools;
	std::vector<QTreeWidgetItem *> m_CoprocessorFT801Only;
	std::vector<QTreeWidgetItem *> m_CoprocessorFT810Plus;
	std::vector<QTreeWidgetItem *> m_DisplayListFT810Plus;

	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

private:
	Toolbox(const Toolbox &);
	Toolbox &operator=(const Toolbox &);

}; /* class Toolbox */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_TOOLBOX_H */

/* end of file */