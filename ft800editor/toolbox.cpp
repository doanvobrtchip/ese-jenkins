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

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

// Emulator includes

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "interactive_properties.h"
#include "properties_editor.h"
#include "constant_mapping.h"
#include "constant_common.h"

using namespace std;

namespace FT800EMUQT {

Toolbox::Toolbox(MainWindow *parent) : QWidget(parent), m_MainWindow(parent),
	m_LineEditor(NULL), m_LineNumber(0)
{
	QVBoxLayout *layout = new QVBoxLayout();
	m_Tools = new QTreeWidget(this);
	m_Tools->setDragEnabled(true);
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
		item->setData(1, Qt::UserRole, QVariant((uint)2));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color RGB"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_COLOR_RGB));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color Alpha"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_COLOR_A));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Stencil"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_STENCIL));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Tag"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_CLEAR_TAG));
	}

	m_Primitives = new QTreeWidgetItem(m_Tools);
	m_Primitives->setText(0, tr("Primitives"));
	m_Primitives->setIcon(0, QIcon(":/icons/layer.png"));
	{
		QTreeWidgetItem *primBitmaps = new QTreeWidgetItem(m_Primitives);
		primBitmaps->setText(0, tr("Bitmaps"));
		primBitmaps->setData(1, Qt::UserRole, QVariant((uint)1));
		primBitmaps->setData(2, Qt::UserRole, QVariant((uint)BITMAPS));
		QTreeWidgetItem *primPoints = new QTreeWidgetItem(m_Primitives);
		primPoints->setText(0, tr("Points"));
		primPoints->setData(1, Qt::UserRole, QVariant((uint)1));
		primPoints->setData(2, Qt::UserRole, QVariant((uint)POINTS));
		QTreeWidgetItem *primLines = new QTreeWidgetItem(m_Primitives);
		primLines->setText(0, tr("Lines"));
		primLines->setData(1, Qt::UserRole, QVariant((uint)1));
		primLines->setData(2, Qt::UserRole, QVariant((uint)LINES));
		QTreeWidgetItem *primLineStrip = new QTreeWidgetItem(m_Primitives);
		primLineStrip->setText(0, tr("Line Strip"));
		primLineStrip->setData(1, Qt::UserRole, QVariant((uint)1));
		primLineStrip->setData(2, Qt::UserRole, QVariant((uint)LINE_STRIP));
		QTreeWidgetItem *primEdgeStripR = new QTreeWidgetItem(m_Primitives);
		primEdgeStripR->setText(0, tr("Edge Strip R"));
		primEdgeStripR->setData(1, Qt::UserRole, QVariant((uint)1));
		primEdgeStripR->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_R));
		QTreeWidgetItem *primEdgeStripL = new QTreeWidgetItem(m_Primitives);
		primEdgeStripL->setText(0, tr("Edge Strip L"));
		primEdgeStripL->setData(1, Qt::UserRole, QVariant((uint)1));
		primEdgeStripL->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_L));
		QTreeWidgetItem *primEdgeStripA = new QTreeWidgetItem(m_Primitives);
		primEdgeStripA->setText(0, tr("Edge Strip A"));
		primEdgeStripA->setData(1, Qt::UserRole, QVariant((uint)1));
		primEdgeStripA->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_A));
		QTreeWidgetItem *primEdgeStripB = new QTreeWidgetItem(m_Primitives);
		primEdgeStripB->setText(0, tr("Edge Strip B"));
		primEdgeStripB->setData(1, Qt::UserRole, QVariant((uint)1));
		primEdgeStripB->setData(2, Qt::UserRole, QVariant((uint)EDGE_STRIP_B));
		QTreeWidgetItem *primRects = new QTreeWidgetItem(m_Primitives);
		primRects->setText(0, tr("Rects"));
		primRects->setData(1, Qt::UserRole, QVariant((uint)1));
		primRects->setData(2, Qt::UserRole, QVariant((uint)RECTS));
	}

	m_Widgets = new QTreeWidgetItem(m_Tools);
	m_Widgets->setText(0, tr("Widgets"));
	m_Widgets->setIcon(0, QIcon(":/icons/ui-buttons.png"));
	{
		QTreeWidgetItem *primText = new QTreeWidgetItem(m_Widgets);
		primText->setText(0, tr("Text"));
		primText->setData(1, Qt::UserRole, QVariant((uint)2));
		primText->setData(2, Qt::UserRole, QVariant((uint)CMD_TEXT));
		QTreeWidgetItem *primButton = new QTreeWidgetItem(m_Widgets);
		primButton->setText(0, tr("Button"));
		primButton->setData(1, Qt::UserRole, QVariant((uint)2));
		primButton->setData(2, Qt::UserRole, QVariant((uint)CMD_BUTTON));
		QTreeWidgetItem *primKeys = new QTreeWidgetItem(m_Widgets);
		primKeys->setText(0, tr("Keys"));
		primKeys->setData(1, Qt::UserRole, QVariant((uint)2));
		primKeys->setData(2, Qt::UserRole, QVariant((uint)CMD_KEYS));
		QTreeWidgetItem *primProgress = new QTreeWidgetItem(m_Widgets);
		primProgress->setText(0, tr("Progress"));
		primProgress->setData(1, Qt::UserRole, QVariant((uint)2));
		primProgress->setData(2, Qt::UserRole, QVariant((uint)CMD_PROGRESS));
		QTreeWidgetItem *primSlider = new QTreeWidgetItem(m_Widgets);
		primSlider->setText(0, tr("Slider"));
		primSlider->setData(1, Qt::UserRole, QVariant((uint)2));
		primSlider->setData(2, Qt::UserRole, QVariant((uint)CMD_SLIDER));
		QTreeWidgetItem *primScrollbar = new QTreeWidgetItem(m_Widgets);
		primScrollbar->setText(0, tr("Scrollbar"));
		primScrollbar->setData(1, Qt::UserRole, QVariant((uint)2));
		primScrollbar->setData(2, Qt::UserRole, QVariant((uint)CMD_SCROLLBAR));
		QTreeWidgetItem *primToggle = new QTreeWidgetItem(m_Widgets);
		primToggle->setText(0, tr("Toggle"));
		primToggle->setData(1, Qt::UserRole, QVariant((uint)2));
		primToggle->setData(2, Qt::UserRole, QVariant((uint)CMD_TOGGLE));
		QTreeWidgetItem *primGauge = new QTreeWidgetItem(m_Widgets);
		primGauge->setText(0, tr("Gauge"));
		primGauge->setData(1, Qt::UserRole, QVariant((uint)2));
		primGauge->setData(2, Qt::UserRole, QVariant((uint)CMD_GAUGE));
		QTreeWidgetItem *primClock = new QTreeWidgetItem(m_Widgets);
		primClock->setText(0, tr("Clock"));
		primClock->setData(1, Qt::UserRole, QVariant((uint)2));
		primClock->setData(2, Qt::UserRole, QVariant((uint)CMD_CLOCK));
		QTreeWidgetItem *primDial = new QTreeWidgetItem(m_Widgets);
		primDial->setText(0, tr("Dial"));
		primDial->setData(1, Qt::UserRole, QVariant((uint)2));
		primDial->setData(2, Qt::UserRole, QVariant((uint)CMD_DIAL));
		QTreeWidgetItem *primNumber = new QTreeWidgetItem(m_Widgets);
		primNumber->setText(0, tr("Number"));
		primNumber->setData(1, Qt::UserRole, QVariant((uint)2));
		primNumber->setData(2, Qt::UserRole, QVariant((uint)CMD_NUMBER));
		QTreeWidgetItem *primSpinner = new QTreeWidgetItem(m_Widgets);
		primSpinner->setText(0, tr("Spinner"));
		primSpinner->setData(1, Qt::UserRole, QVariant((uint)2));
		primSpinner->setData(2, Qt::UserRole, QVariant((uint)CMD_SPINNER));
		QTreeWidgetItem *primScreensaver = new QTreeWidgetItem(m_Widgets);
		primScreensaver->setText(0, tr("Screensaver"));
		primScreensaver->setData(1, Qt::UserRole, QVariant((uint)2));
		primScreensaver->setData(2, Qt::UserRole, QVariant((uint)CMD_SCREENSAVER));
		QTreeWidgetItem *primGradient = new QTreeWidgetItem(m_Widgets);
		primGradient->setText(0, tr("Gradient"));
		primGradient->setData(1, Qt::UserRole, QVariant((uint)2));
		primGradient->setData(2, Qt::UserRole, QVariant((uint)CMD_GRADIENT));
	}

	m_Utilities = new QTreeWidgetItem(m_Tools);
	m_Utilities->setText(0, tr("Utilities"));
	m_Utilities->setIcon(0, QIcon(":/icons/beaker-empty.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Tracker"));
		item->setData(1, Qt::UserRole, QVariant((uint)2));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TRACK));
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Sketch"));
		item->setData(1, Qt::UserRole, QVariant((uint)2));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SKETCH));
#ifndef FT810EMU_MODE // This is deprecated in FT810
		item = new QTreeWidgetItem(m_Utilities);
		item->setText(0, tr("Capacitive Sketch"));
		item->setData(1, Qt::UserRole, QVariant((uint)2));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_CSKETCH));
#endif
	}

	m_Graphics = new QTreeWidgetItem(m_Tools);
	m_Graphics->setText(0, tr("Graphics State"));
	m_Graphics->setIcon(0, QIcon(":/icons/categories.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color RGB"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_RGB));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color A"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_A));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Color Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_COLOR_MASK));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Foreground Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_FGCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Background Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_BGCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Gradient Color"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GRADCOLOR));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Line Width"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_LINE_WIDTH));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Point Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_POINT_SIZE));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Blend Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BLEND_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Scissor Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_SCISSOR_SIZE));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Scissor XY"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_SCISSOR_XY));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Alpha Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_ALPHA_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Func"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_FUNC));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_MASK));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Stencil Op"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_STENCIL_OP));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Tag"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_TAG));
		item = new QTreeWidgetItem(m_Graphics);
		item->setText(0, tr("Tag Mask"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_TAG_MASK));
	}

	m_Bitmaps = new QTreeWidgetItem(m_Tools);
	m_Bitmaps->setText(0, tr("Bitmap State"));
	m_Bitmaps->setIcon(0, QIcon(":/icons/map-resize.png"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Handle"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_HANDLE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Source"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SOURCE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Layout"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_LAYOUT));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Bitmap Size"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_SIZE));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform A"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_A));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform B"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_B));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform C"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_C));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform D"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_D));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform E"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_E));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Transform F"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FTEDITOR_DL_BITMAP_TRANSFORM_F));
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Load Identity"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_LOADIDENTITY));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Translate"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_TRANSLATE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Scale"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SCALE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Rotate"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_ROTATE));
		m_CoprocessorTools.push_back(item);
		item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Set Current"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_SETMATRIX));
		m_CoprocessorTools.push_back(item);
		/*item = new QTreeWidgetItem(m_Bitmaps);
		item->setText(0, tr("Matrix Get Current"));
		item->setData(1, Qt::UserRole, QVariant((uint)4));
		item->setData(2, Qt::UserRole, QVariant((uint)CMD_GETMATRIX));
		m_CoprocessorTools.push_back(item);*/
	}

	// m_Advanced = new QTreeWidgetItem(m_Tools);
	// m_Advanced->setText(0, tr("Advanced")); // Context & Macro commands?
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
			for (int i = 0; i < m_CoprocessorTools.size(); ++i)
			{
				m_CoprocessorTools[i]->setHidden(!editor->isCoprocessor());
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

} /* namespace FT800EMUQT */

/* end of file */
