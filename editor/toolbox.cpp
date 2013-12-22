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
#include <QListWidget>

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
	m_Tools = new QListWidget(this);
	m_Tools->setDragEnabled(true);
	layout->addWidget(m_Tools);
	setLayout(layout);
}

Toolbox::~Toolbox()
{

}

uint32_t Toolbox::getSelection()
{
	QListWidgetItem *c = m_Tools->currentItem();
	if (c)
	{
		return c->data(Qt::UserRole).toUInt();
	}
	return 0;
}

void Toolbox::setEditorLine(DlEditor *editor, int line)
{
	m_LineNumber = line;
	if (editor != m_LineEditor)
	{
		m_LineEditor = editor;
		m_Tools->clear();
		if (editor)
		{
			QListWidgetItem *primBitmaps = new QListWidgetItem(m_Tools);
			primBitmaps->setText(tr("Bitmaps"));
			primBitmaps->setData(Qt::UserRole, QVariant((uint)BITMAPS));
			QListWidgetItem *primPoints = new QListWidgetItem(m_Tools);
			primPoints->setText(tr("Points"));
			primPoints->setData(Qt::UserRole, QVariant((uint)POINTS));
			QListWidgetItem *primLines = new QListWidgetItem(m_Tools);
			primLines->setText(tr("Lines"));
			primLines->setData(Qt::UserRole, QVariant((uint)LINES));
			QListWidgetItem *primLineStrip = new QListWidgetItem(m_Tools);
			primLineStrip->setText(tr("Line Strip"));
			primLineStrip->setData(Qt::UserRole, QVariant((uint)LINE_STRIP));
			QListWidgetItem *primEdgeStripR = new QListWidgetItem(m_Tools);
			primEdgeStripR->setText(tr("Edge Strip R"));
			primEdgeStripR->setData(Qt::UserRole, QVariant((uint)EDGE_STRIP_R));
			QListWidgetItem *primEdgeStripL = new QListWidgetItem(m_Tools);
			primEdgeStripL->setText(tr("Edge Strip L"));
			primEdgeStripL->setData(Qt::UserRole, QVariant((uint)EDGE_STRIP_L));
			QListWidgetItem *primEdgeStripA = new QListWidgetItem(m_Tools);
			primEdgeStripA->setText(tr("Edge Strip A"));
			primEdgeStripA->setData(Qt::UserRole, QVariant((uint)EDGE_STRIP_A));
			QListWidgetItem *primEdgeStripB = new QListWidgetItem(m_Tools);
			primEdgeStripB->setText(tr("Edge Strip B"));
			primEdgeStripB->setData(Qt::UserRole, QVariant((uint)EDGE_STRIP_B));
			QListWidgetItem *primRects = new QListWidgetItem(m_Tools);
			primRects->setText(tr("Rects"));
			primRects->setData(Qt::UserRole, QVariant((uint)RECTS));

			if (editor->isCoprocessor())
			{
				QListWidgetItem *primText = new QListWidgetItem(m_Tools);
				primText->setText(tr("Text"));
				primText->setData(Qt::UserRole, QVariant((uint)CMD_TEXT));
				QListWidgetItem *primButton = new QListWidgetItem(m_Tools);
				primButton->setText(tr("Button"));
				primButton->setData(Qt::UserRole, QVariant((uint)CMD_BUTTON));
				QListWidgetItem *primKeys = new QListWidgetItem(m_Tools);
				primKeys->setText(tr("Keys"));
				primKeys->setData(Qt::UserRole, QVariant((uint)CMD_KEYS));
				QListWidgetItem *primProgress = new QListWidgetItem(m_Tools);
				primProgress->setText(tr("Progress"));
				primProgress->setData(Qt::UserRole, QVariant((uint)CMD_PROGRESS));
				QListWidgetItem *primSlider = new QListWidgetItem(m_Tools);
				primSlider->setText(tr("Slider"));
				primSlider->setData(Qt::UserRole, QVariant((uint)CMD_SLIDER));
				QListWidgetItem *primScrollbar = new QListWidgetItem(m_Tools);
				primScrollbar->setText(tr("Scrollbar"));
				primScrollbar->setData(Qt::UserRole, QVariant((uint)CMD_SCROLLBAR));
				QListWidgetItem *primToggle = new QListWidgetItem(m_Tools);
				primToggle->setText(tr("Toggle"));
				primToggle->setData(Qt::UserRole, QVariant((uint)CMD_TOGGLE));
				QListWidgetItem *primGauge = new QListWidgetItem(m_Tools);
				primGauge->setText(tr("Gauge"));
				primGauge->setData(Qt::UserRole, QVariant((uint)CMD_GAUGE));
				QListWidgetItem *primClock = new QListWidgetItem(m_Tools);
				primClock->setText(tr("Clock"));
				primClock->setData(Qt::UserRole, QVariant((uint)CMD_CLOCK));
				QListWidgetItem *primDial = new QListWidgetItem(m_Tools);
				primDial->setText(tr("Dial"));
				primDial->setData(Qt::UserRole, QVariant((uint)CMD_DIAL));
				QListWidgetItem *primNumber = new QListWidgetItem(m_Tools);
				primNumber->setText(tr("Number"));
				primNumber->setData(Qt::UserRole, QVariant((uint)CMD_NUMBER));
				QListWidgetItem *primSpinner = new QListWidgetItem(m_Tools);
				primSpinner->setText(tr("Spinner"));
				primSpinner->setData(Qt::UserRole, QVariant((uint)CMD_SPINNER));
			}
		}
	}
}

void Toolbox::unsetEditorLine()
{
	m_LineEditor = NULL;
	m_Tools->clear();
}

} /* namespace FT800EMUQT */

/* end of file */
