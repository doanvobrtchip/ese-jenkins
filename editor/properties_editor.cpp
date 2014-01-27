/**
 * properties_editor.cpp
 * $Id$
 * \file properties_editor.cpp
 * \brief properties_editor.cpp
 * \date 2013-11-10 09:43GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "properties_editor.h"

// STL includes
#include "stdio.h"

// Qt includes
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

// Emulator includes
#include <vc.h>

// Project includes

using namespace std;

namespace FT800EMUQT {

PropertiesEditor::PropertiesEditor(QWidget *parent) : QWidget(parent), m_CurrentEditWidget(NULL), m_OwnCurrentEditWidget(false)
{
	QVBoxLayout *infoLayout = new QVBoxLayout(this);
	m_InfoGroupBox = new QGroupBox(this);
	m_InfoGroupBox->setLayout(infoLayout);
	m_InfoLabel = new QLabel(this);
	m_InfoLabel->setTextFormat(Qt::RichText);
	m_InfoLabel->setWordWrap(true);
	infoLayout->addWidget(m_InfoLabel);
	m_InfoGroupBox->hide();

	m_EditLayout = new QVBoxLayout(this);
	m_EditGroupBox = new QGroupBox(this);
	m_EditGroupBox->setLayout(m_EditLayout);
	m_EditGroupBox->hide();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(m_EditGroupBox);
	layout->addWidget(m_InfoGroupBox);
	layout->addStretch();
	setLayout(layout);

	translate();
}

PropertiesEditor::~PropertiesEditor()
{

}

void PropertiesEditor::setInfo(QString message)
{
	printf("set info: %s\n", message.toLatin1().data());
	m_InfoLabel->setText(message);
	if (message.isEmpty() || message.isNull())
	{
		m_InfoGroupBox->hide();
	}
	else
	{
		m_InfoGroupBox->show();
	}
}

void PropertiesEditor::setEditWidget(QWidget *widget, bool own, QWidget *setter)
{
	if (m_CurrentEditWidget)
	{
		m_EditLayout->removeWidget(m_CurrentEditWidget);
		if (m_OwnCurrentEditWidget)
		{
			delete m_CurrentEditWidget;
		}
		m_CurrentEditWidget = NULL;
		m_OwnCurrentEditWidget = false;
	}

	if (widget)
	{
		m_CurrentEditWidget = widget;
		m_OwnCurrentEditWidget = own;
		m_EditLayout->addWidget(m_CurrentEditWidget);
		m_EditGroupBox->show();
	}
	else
	{
		m_EditGroupBox->hide();
	}

	m_CurrentEditWidgetSetter = setter;
}

void PropertiesEditor::translate()
{
	m_InfoGroupBox->setTitle(tr("Information"));
	m_EditGroupBox->setTitle(tr("Edit"));
}

} /* namespace FT800EMUQT */

/* end of file */
