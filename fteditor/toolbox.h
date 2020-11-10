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

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>

// Qt includes
#include <QWidget>
#include <QStyledItemDelegate>

// Emulator includes
#include <bt8xxemu_inttypes.h>

// Project includes

class QTreeWidget;
class QTreeWidgetItem;

class QLineEdit;

// Primitive, functions, and vertices drop on top of the command list,
// Display list and command state drop before the widget that's under the cursor

#define FTEDITOR_SELECTION_PRIMITIVE (1)
#define FTEDITOR_SELECTION_FUNCTION (2)
#define FTEDITOR_SELECTION_DL_STATE (3)
#define FTEDITOR_SELECTION_CMD_STATE (4)
#define FTEDITOR_SELECTION_VERTEX (5)


class HighlightDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    HighlightDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void setRegularExpressionFilter(const QString &reFilter) { m_reFilter = reFilter; }

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

private:
    QString m_reFilter = QString("");
};

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
	void processFilter(const QString& filter);

private:
	MainWindow *m_MainWindow;

	QLineEdit   * m_FilterText;
	HighlightDelegate * m_highlightDelegate;

	QTreeWidget *m_Tools;

	QTreeWidgetItem *m_Background;
	QTreeWidgetItem *m_Primitives;
	QTreeWidgetItem *m_Widgets;
	QTreeWidgetItem *m_Utilities;
	QTreeWidgetItem *m_Graphics;
	QTreeWidgetItem *m_Bitmaps;
	QTreeWidgetItem *m_Drawing;
	QTreeWidgetItem *m_Execution;
	// QTreeWidgetItem *m_Advanced;
	std::vector<QTreeWidgetItem *> m_CoprocessorTools;
	std::vector<QTreeWidgetItem *> m_CoprocessorFT801Only;
	std::vector<QTreeWidgetItem *> m_CoprocessorFT810Plus;
	std::vector<QTreeWidgetItem *> m_CoprocessorBT815Plus;
	std::vector<QTreeWidgetItem *> m_CoprocessorBT817Plus;

	std::vector<QTreeWidgetItem *> m_DisplayListFT810Plus;
	std::vector<QTreeWidgetItem *> m_DisplayListBT815Plus;	

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
