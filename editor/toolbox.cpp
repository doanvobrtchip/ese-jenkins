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
#include <vc.h>

// Project includes
#include "main_window.h"
#include "dl_editor.h"

using namespace std;

namespace FT800EMUQT {

Toolbox::Toolbox(MainWindow *parent) : QWidget(parent), m_MainWindow(parent),
	m_LineEditor(NULL), m_LineNumber(0)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	m_Tools = new QTreeWidget(this);
	m_Tools->setDragEnabled(true);
	layout->addWidget(m_Tools);
	setLayout(layout);

	m_Tools->header()->close();

	m_Background = new QTreeWidgetItem(m_Tools);
	m_Background->setText(0, tr("Background"));
	{
		QTreeWidgetItem *item;
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear"));
		item->setData(1, Qt::UserRole, QVariant((uint)2));
		item->setData(2, Qt::UserRole, QVariant((uint)FT800EMU_DL_CLEAR));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color RGB"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FT800EMU_DL_CLEAR_COLOR_RGB));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Color Alpha"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FT800EMU_DL_CLEAR_COLOR_A));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Stencil"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FT800EMU_DL_CLEAR_STENCIL));
		item = new QTreeWidgetItem(m_Background);
		item->setText(0, tr("Clear Tag"));
		item->setData(1, Qt::UserRole, QVariant((uint)3));
		item->setData(2, Qt::UserRole, QVariant((uint)FT800EMU_DL_CLEAR_TAG));
	}

	m_Primitives = new QTreeWidgetItem(m_Tools);
	m_Primitives->setText(0, tr("Primitives"));
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
	}

	m_Graphics = new QTreeWidgetItem(m_Tools);
	m_Graphics->setText(0, tr("Graphics"));

	m_Bitmaps = new QTreeWidgetItem(m_Tools);
	m_Bitmaps->setText(0, tr("Bitmaps"));

	m_Advanced = new QTreeWidgetItem(m_Tools);
	m_Advanced->setText(0, tr("Advanced"));
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
		//m_Tools->clear();
		if (editor)
		{
			m_Widgets->setHidden(!editor->isCoprocessor());
		}
	}
}

void Toolbox::unsetEditorLine()
{
	m_LineEditor = NULL;
	//m_Tools->clear();
}

} /* namespace FT800EMUQT */

/* end of file */
