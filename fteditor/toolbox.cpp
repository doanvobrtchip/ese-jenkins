/**
 * toolbox.cpp
 * $Id$
 * \file toolbox.cpp
 * \brief toolbox.cpp
 * \date 2013-12-22 01:01GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "toolbox.h"

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QPainter>

// Emulator includes

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "interactive_properties.h"
#include "properties_editor.h"
#include "constant_mapping.h"
#include "constant_common.h"

void HighlightDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!m_reFilter.isEmpty() && index.column() == 0)
    {
        //const QColor highlight = QColor(176, 196, 222);
		const QColor highlight = QColor(164, 255, 164);
        QString text = index.data().toString();
        QFontMetrics fm = option.fontMetrics;

        QRegularExpression re(m_reFilter, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator rei = re.globalMatch(text);

        while (rei.hasNext())
        {
            QRegularExpressionMatch rem = rei.next();
            for (int i = 0; i <= rem.lastCapturedIndex(); ++i)
            {
                int t1 = rem.capturedStart(i);
                int t2 = rem.capturedLength(i);

                int x = fm.boundingRect(option.rect, Qt::AlignLeft, text.left(t1)).width();
                int w = fm.boundingRect(option.rect, Qt::AlignLeft, text.mid(t1, t2)).width();

                QRect r = option.rect;
                r.translate(x + 3, 0);
                r.setWidth(w);

                painter->fillRect(r, highlight);
            }
        }

        QStyledItemDelegate::paint(painter, option, index);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }

}

namespace FTEDITOR {

Toolbox::Toolbox(MainWindow *parent)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_LineEditor(NULL)
    , m_LineNumber(0)
{
	QVBoxLayout *layout = new QVBoxLayout();

	m_FilterText = new QLineEdit(this);
	m_FilterText->setPlaceholderText("Filter");
	m_FilterText->setClearButtonEnabled(true);
	connect(m_FilterText, &QLineEdit::textEdited, this, &Toolbox::processFilter);
	layout->addWidget(m_FilterText);

	m_Tools = new QTreeWidget(this);
	m_Tools->setDragEnabled(true);
	m_highlightDelegate = new HighlightDelegate(this);
	m_Tools->setItemDelegate(m_highlightDelegate);
	layout->addWidget(m_Tools);
	setLayout(layout);
	connect(m_Tools, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentSelectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	m_Tools->header()->close();

	m_Background = new QTreeWidgetItem(m_Tools);
	m_Background->setText(0, tr("Background"));
	m_Background->setIcon(0, QIcon(":/icons/table-paint-can.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color RGB"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_COLOR_RGB));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color Alpha"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_COLOR_A));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Stencil"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_STENCIL));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Tag"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_TAG));
	}

	m_Primitives = new QTreeWidgetItem(m_Tools);
	m_Primitives->setText(0, tr("Primitives"));
	m_Primitives->setIcon(0, QIcon(":/icons/layer.png"));
	{
		QTreeWidgetItem *primBitmaps = new QTreeWidgetItem(m_Primitives);
		primBitmaps->setText(0, tr("Bitmaps"));
		primBitmaps->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primBitmaps->setData(2, Qt::UserRole, QVariant((uint)BITMAPS));
		QTreeWidgetItem *primPoints = new QTreeWidgetItem(m_Primitives);
		primPoints->setText(0, tr("Points"));
		primPoints->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primPoints->setData(2, Qt::UserRole, QVariant((uint)POINTS));
		QTreeWidgetItem *primLines = new QTreeWidgetItem(m_Primitives);
		primLines->setText(0, tr("Lines"));
		primLines->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primLines->setData(2, Qt::UserRole, QVariant((uint)LINES));
		QTreeWidgetItem *primLineStrip = new QTreeWidgetItem(m_Primitives);
		primLineStrip->setText(0, tr("Line Strip"));
		primLineStrip->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primLineStrip->setData(2, Qt::UserRole, QVariant((uint)LINE_STRIP));
		QTreeWidgetItem *primEdgeStripR = new QTreeWidgetItem(m_Primitives);
		primEdgeStripR->setText(0, tr("Edge Strip R"));
		primEdgeStripR->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primEdgeStripR->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_R));
		QTreeWidgetItem *primEdgeStripL = new QTreeWidgetItem(m_Primitives);
		primEdgeStripL->setText(0, tr("Edge Strip L"));
		primEdgeStripL->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primEdgeStripL->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_L));
		QTreeWidgetItem *primEdgeStripA = new QTreeWidgetItem(m_Primitives);
		primEdgeStripA->setText(0, tr("Edge Strip A"));
		primEdgeStripA->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primEdgeStripA->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_A));
		QTreeWidgetItem *primEdgeStripB = new QTreeWidgetItem(m_Primitives);
		primEdgeStripB->setText(0, tr("Edge Strip B"));
		primEdgeStripB->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primEdgeStripB->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_B));
		QTreeWidgetItem *primRects = new QTreeWidgetItem(m_Primitives);
		primRects->setText(0, tr("Rects"));
		primRects->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_PRIMITIVE));
		primRects->setData(2, Qt::UserRole, QVariant((uint)RECTS));
	}

	m_Widgets = new QTreeWidgetItem(m_Tools);
	m_Widgets->setText(0, tr("Widgets"));
	m_Widgets->setIcon(0, QIcon(":/icons/ui-buttons.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Text"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TEXT));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Button"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_BUTTON));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Keys"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_KEYS));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Progress"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_PROGRESS));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Slider"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SLIDER));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Scrollbar"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SCROLLBAR));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Toggle"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TOGGLE));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Gauge"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GAUGE));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Clock"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_CLOCK));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Dial"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_DIAL));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Number"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_NUMBER));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Spinner"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SPINNER));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Screensaver"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SCREENSAVER));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Gradient"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GRADIENT));
		item = new QTreeWidgetItem(m_Widgets);
		item->setText(0, tr("Gradient A"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GRADIENTA));
		m_CoprocessorBT815Plus.push_back(item);
	}

	m_Utilities = new QTreeWidgetItem(m_Tools);
	m_Utilities->setText(0, tr("Utilities"));
	m_Utilities->setIcon(0, QIcon(":/icons/beaker-empty.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Logo"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_LOGO));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Tracker"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TRACK));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Sketch"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SKETCH));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Capacitive Sketch"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_CSKETCH));
		m_CoprocessorFT801Only.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Display List Start"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_DLSTART));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Swap"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SWAP));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Interrupt"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_INTERRUPT));
		/*item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Video Start"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_VIDEOSTART));
		m_CoprocessorTools.push_back(item);*/
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Snapshot"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SNAPSHOT));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Snapshot 2"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SNAPSHOT2));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Load Image"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_LOADIMAGE));
		
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Get Ptr"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GETPTR));

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Get Props"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GETPROPS));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Play Video"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_PLAYVIDEO));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Append"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_APPEND));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Memory Set"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_MEMSET));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Memory Zero"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_MEMZERO));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Inflate"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_INFLATE));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Inflate 2"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_INFLATE2));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Video Start"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_VIDEOSTART));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Video Start F"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_VIDEOSTARTF));
		m_CoprocessorBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Video Frame"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_VIDEOFRAME));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Reset Font"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_RESETFONTS));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Flash Source"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FLASHSOURCE));
		m_CoprocessorBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Start"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMSTART));
		m_CoprocessorBT815Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Start RAM"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMSTARTRAM));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Run Animation"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_RUNANIM));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Stop"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMSTOP));
		m_CoprocessorBT817Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation XY"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMXY));
		m_CoprocessorBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Draw"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMDRAW));
		m_CoprocessorBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Frame"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMFRAME));
		m_CoprocessorBT815Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Animation Frame RAM"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ANIMFRAMERAM));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("API Level"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_APILEVEL));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Calibrate Sub"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_CALIBRATESUB));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Test Card"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TESTCARD));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Font Cache"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FONTCACHE));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Font Cache Query"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FONTCACHEQUERY));
		m_CoprocessorBT817Plus.push_back(item);

		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Get Image"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GETIMAGE));
		m_CoprocessorBT817Plus.push_back(item);
	}

	m_Graphics = new QTreeWidgetItem(m_Tools);
	m_Graphics->setText(0, tr("Graphics State"));
	m_Graphics->setIcon(0, QIcon(":/icons/categories.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Save Context"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_SAVE_CONTEXT));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Restore Context"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_RESTORE_CONTEXT));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color RGB"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_RGB));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color A"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_A));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_MASK));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Foreground Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FGCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Background Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_BGCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Gradient Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GRADCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Line Width"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_LINE_WIDTH));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Point Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_POINT_SIZE));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Blend Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BLEND_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Scissor Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_SCISSOR_SIZE));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Scissor XY"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_SCISSOR_XY));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Alpha Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_ALPHA_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_MASK));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Op"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_OP));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Tag"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_TAG));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Tag Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_TAG_MASK));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Vertex Format"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_VERTEX_FORMAT));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Vertex Translate X"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_VERTEX_TRANSLATE_X));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Vertex Translate Y"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_VERTEX_TRANSLATE_Y));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Cold Start"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_COLDSTART));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Media FIFO"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_MEDIAFIFO));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Set Scratch"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETSCRATCH));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Set Base"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETBASE));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Text Fill Width"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FILLWIDTH));
		m_CoprocessorBT815Plus.push_back(item);
	}

	m_Bitmaps = new QTreeWidgetItem(m_Tools);
	m_Bitmaps->setText(0, tr("Bitmap State"));
	m_Bitmaps->setIcon(0, QIcon(":/icons/map-resize.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Handle"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_HANDLE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Source"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SOURCE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Layout"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_LAYOUT));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Layout H"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_LAYOUT_H));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SIZE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Size H"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SIZE_H));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Extend Format"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_EXT_FORMAT));
		m_DisplayListBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Set Bitmap"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETBITMAP));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Cell"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CELL));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Palette Source"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_PALETTE_SOURCE));
		m_DisplayListFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Swizzle"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SWIZZLE));
		m_DisplayListBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform A"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_A));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform B"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_B));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform C"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_C));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform D"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_D));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform E"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_E));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform F"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_DL_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_F));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Load Identity"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_LOADIDENTITY));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Translate"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TRANSLATE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Scale"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SCALE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Rotate"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ROTATE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Rotate Around"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ROTATEAROUND));
		m_CoprocessorBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Set Current"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETMATRIX));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Transform"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_BITMAP_TRANSFORM));
		m_CoprocessorBT815Plus.push_back(item);
		/*item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Get Current"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GETMATRIX));
		m_CoprocessorTools.push_back(item);*/
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Set Font"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETFONT));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Set Font 2"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETFONT2));
		m_CoprocessorFT810Plus.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("ROM Font"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_CMD_STATE));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ROMFONT));
		m_CoprocessorFT810Plus.push_back(item);
	}

	m_Drawing = new QTreeWidgetItem(m_Tools);
	m_Drawing->setText(0, tr("Drawing Actions"));
	m_Drawing->setIcon(0, QIcon(":/icons/pencil-field.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Drawing);
		item->setText(0, tr("Begin"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BEGIN));
		item = new QTreeWidgetItem(m_Drawing);
		item->setText(0, tr("End"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_END));
		item = new QTreeWidgetItem(m_Drawing);
		item->setText(0, tr("Vertex Float"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_VERTEX));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_VERTEX2F));
		item = new QTreeWidgetItem(m_Drawing);
		item->setText(0, tr("Vertex Integer"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_VERTEX));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_VERTEX2II));
	}
	//m_CoprocessorFT810Plus.push_back(item);
	//m_DisplayListFT810Plus.push_back(item);
	//m_CoprocessorTools

	m_Execution = new QTreeWidgetItem(m_Tools);
	m_Execution->setText(0, tr("Execution Control"));
	m_Execution->setIcon(0, QIcon(":/icons/task--arrow.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Macro"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_MACRO));
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Display"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_DISPLAY));
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Jump"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_JUMP));
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Call"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CALL));
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Return"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_RETURN));
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("No-op"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_NOP));
		m_DisplayListBT815Plus.push_back(item);
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Wait"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_WAIT));
		m_CoprocessorBT817Plus.push_back(item);
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("New List"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_NEWLIST));
		m_CoprocessorBT817Plus.push_back(item);
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("End List"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ENDLIST));
		m_CoprocessorBT817Plus.push_back(item);
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Call List"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_CALLLIST));
		m_CoprocessorBT817Plus.push_back(item);
		item = new QTreeWidgetItem(m_Execution);
		item->setText(0, tr("Command Return"));
		item->setData(1, Qt::UserRole, QVariant((uint)FTEDITOR_SELECTION_FUNCTION));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_RETURN));
		m_CoprocessorBT817Plus.push_back(item);
	}

	// m_Advanced = new QTreeWidgetItem(m_Tools);
	// m_Advanced->setText(0, tr("Advanced")); // Context & Macro commands?

	QTimer::singleShot(0, this, [=]() {
		bindCurrentDevice();
	});
}

void Toolbox::processFilter(const QString & filter)
{
	QRegularExpression re(filter, QRegularExpression::CaseInsensitiveOption);

	if (!re.isValid()) {
		return;
	}

	m_highlightDelegate->setRegularExpressionFilter(filter);

	QTreeWidgetItem * sub = nullptr;
	QTreeWidgetItem * node = nullptr;
	bool hasChidVisible = false;
	for(int i = 0; i < m_Tools->topLevelItemCount(); i++)
	{
		sub = m_Tools->topLevelItem(i);
		hasChidVisible = false;

		for (int j = 0; j < sub->childCount() ; j++) {
			node = sub->child(j);
			QString nodeText = node->text(0);

			bool hiddenByDevice = node->data(1, 0).toBool();

			if (re.match(nodeText).hasMatch()) {
				node->setHidden(false || hiddenByDevice);
			} else {
				sub->child(j)->setHidden(true);
			}
			hasChidVisible |= !node->isHidden();
		}
		sub->setExpanded(hasChidVisible);
		sub->setHidden(!hasChidVisible);
	}

	if (filter.isEmpty()) {
		for(int i = 0; i < m_Tools->topLevelItemCount(); i++)
		{
			QTreeWidgetItem * sub = m_Tools->topLevelItem(i);
			sub->setExpanded(false);
		}
	}
}

void Toolbox::currentSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	uint32_t selectionType = getSelectionType();
	if (selectionType == 1)
	{
		uint32_t selection = getSelectionId();
		switch (selection)
		{
		case BITMAPS:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>BITMAPS</b><br>"
			    "Rectangular pixel arrays, in various color format."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		case POINTS:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>POINTS</b><br>"
			    "Anti-aliased points, point radius is 1-256 pixels."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		case LINES:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>LINES</b><br>"
			    "Anti-aliased lines, with width of 1-256 pixels (width is from center of the line to boundary)."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		case LINE_STRIP:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>LINE_STRIP</b><br>"
			    "Anti-aliased lines, connected head-to-tail."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		case EDGE_STRIP_R:
		case EDGE_STRIP_L:
		case EDGE_STRIP_A:
		case EDGE_STRIP_B:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>EDGE STRIP A, B, L, R</b><br>"
			    "Edge strips."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		case RECTS:
			m_MainWindow->propertiesEditor()->setInfo(tr(
			    "<b>RECTS</b><br>"
			    "Round-cornered rectangles, curvature of the corners can be adjusted using LINE_WIDTH."));
			m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, NULL);
			break;
		}
	}
	else if (selectionType == 5)
	{
		uint32_t selection = getSelectionId();
		int idLeft = (int)selection;
		int idRight = 0;
		m_MainWindow->interactiveProperties()->setProperties(idLeft, idRight, NULL);
	}
	else if (selectionType)
	{
		uint32_t selection = getSelectionId();
		int idLeft;
		int idRight;
		if ((selection & 0xFFFFFF00) == 0xFFFFFF00) // Coprocessor
		{
			idLeft = 0xFFFFFF00;
			idRight = selection & 0xFF;
		}
		else
		{
			idLeft = 0;
			idRight = selection;
		}
		m_MainWindow->interactiveProperties()->setProperties(idLeft, idRight, NULL);
	}
}

Toolbox::~Toolbox()
{
}

uint32_t Toolbox::getSelectionType()
{
	QTreeWidgetItem *c = m_Tools->currentItem();
	if (c)
	{
		return c->data(1, Qt::UserRole).toUInt();
	}
	return 0;
}

uint32_t Toolbox::getSelectionId()
{
	QTreeWidgetItem *c = m_Tools->currentItem();
	if (c)
	{
		return c->data(2, Qt::UserRole).toUInt();
	}
	return 0;
}

void Toolbox::bindCurrentDevice()
{
	if (!m_LineEditor)
	{
		return;
	}
	for (size_t i = 0; i < m_CoprocessorFT801Only.size(); ++i)
	{
		m_CoprocessorFT801Only[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801 && m_LineEditor->isCoprocessor()));
		m_CoprocessorFT801Only[i]->setData(1, 0, m_CoprocessorFT801Only[i]->isHidden());
	}
	for (size_t i = 0; i < m_CoprocessorFT810Plus.size(); ++i)
	{
		m_CoprocessorFT810Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && m_LineEditor->isCoprocessor()));
		m_CoprocessorFT810Plus[i]->setData(1, 0, m_CoprocessorFT810Plus[i]->isHidden());
	}
	for (size_t i = 0; i < m_CoprocessorBT815Plus.size(); ++i)
	{
		m_CoprocessorBT815Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815 && m_LineEditor->isCoprocessor()));
		m_CoprocessorBT815Plus[i]->setData(1, 0, m_CoprocessorBT815Plus[i]->isHidden());
	}
	for (size_t i = 0; i < m_CoprocessorBT817Plus.size(); ++i)
	{
		m_CoprocessorBT817Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT817 && m_LineEditor->isCoprocessor()));
		m_CoprocessorBT817Plus[i]->setData(1, 0, m_CoprocessorBT817Plus[i]->isHidden());
	}
	for (size_t i = 0; i < m_DisplayListFT810Plus.size(); ++i)
	{
		m_DisplayListFT810Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810));
		m_DisplayListFT810Plus[i]->setData(1, 0, m_DisplayListFT810Plus[i]->isHidden());
	}
	for (size_t i = 0; i < m_DisplayListBT815Plus.size(); ++i)
	{
		m_DisplayListBT815Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815));
		m_DisplayListBT815Plus[i]->setData(1, 0, m_DisplayListBT815Plus[i]->isHidden());
	}

	processFilter(m_FilterText->text());
}

void Toolbox::setEditorLine(DlEditor *editor, int line)
{
	m_LineNumber = line;
	if (editor != m_LineEditor)
	{
		m_LineEditor = editor;
		// m_Tools->clear();
		if (editor)
		{
			m_Background->setHidden(false);
			m_Primitives->setHidden(false);
			m_Widgets->setHidden(!editor->isCoprocessor());
			m_Utilities->setHidden(!editor->isCoprocessor());
			m_Graphics->setHidden(false);
			m_Bitmaps->setHidden(false);
			// m_Advanced->setHidden(false);
			m_Drawing->setHidden(false);
			m_Execution->setHidden(false);
			for (size_t i = 0; i < m_CoprocessorTools.size(); ++i)
			{
				m_CoprocessorTools[i]->setHidden(!editor->isCoprocessor());
			}
			for (size_t i = 0; i < m_CoprocessorFT801Only.size(); ++i)
			{
				m_CoprocessorFT801Only[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE == FTEDITOR_FT801 && editor->isCoprocessor()));
			}
			for (size_t i = 0; i < m_CoprocessorFT810Plus.size(); ++i)
			{
				m_CoprocessorFT810Plus[i]->setHidden(!(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810 && editor->isCoprocessor()));
			}
		}
		else
		{
			m_Background->setHidden(true);
			m_Primitives->setHidden(true);
			m_Widgets->setHidden(true);
			m_Utilities->setHidden(true);
			m_Graphics->setHidden(true);
			m_Bitmaps->setHidden(true);
			// m_Advanced->setHidden(true);
			m_Drawing->setHidden(true);
			m_Execution->setHidden(true);
		}
	}
}

void Toolbox::unsetEditorLine()
{
	m_LineEditor = NULL;
	// m_Tools->clear();
	m_Background->setHidden(true);
	m_Primitives->setHidden(true);
	m_Widgets->setHidden(true);
	m_Utilities->setHidden(true);
	m_Graphics->setHidden(true);
	m_Bitmaps->setHidden(true);
	// m_Advanced->setHidden(true);
}

} /* namespace FTEDITOR */

/* end of file */
