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

PropertiesEditor::PropertiesEditor(QWidget *parent) : QWidget(parent), m_OwnCurrentEditWidget(false)
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
	m_EditWidget = new QWidget(this);
	m_EditWidget->setLayout(m_EditLayout);
	m_EditWidget->hide();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(m_EditWidget);
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
	// printf("set info: %s\n", message.toLatin1().data());
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
	std::vector<QWidget *> widgets;
	if (widget) widgets.push_back(widget);
	setEditWidgets(widgets, own, setter);
}

void PropertiesEditor::setEditWidgets(const std::vector<QWidget *> &widgets, bool own, QWidget *setter)
{
	for (std::vector<QWidget *>::iterator it(m_CurrentEditWidgets.begin()), end(m_CurrentEditWidgets.end()); it != end; ++it)
	{
		m_EditLayout->removeWidget((*it));
		if (m_OwnCurrentEditWidget)
		{
			delete (*it);
		}
	}
	m_CurrentEditWidgets.clear();
	m_OwnCurrentEditWidget = false;

	if (widgets.size())
	{
		m_CurrentEditWidgets = widgets;
		m_OwnCurrentEditWidget = own;
		for (std::vector<QWidget *>::iterator it(m_CurrentEditWidgets.begin()), end(m_CurrentEditWidgets.end()); it != end; ++it)
		{
			m_EditLayout->addWidget((*it));
		}
		m_EditWidget->show();
	}
	else
	{
		m_EditWidget->hide();
	}

	m_CurrentEditWidgetSetter = setter;
}

void PropertiesEditor::translate()
{
	m_InfoGroupBox->setTitle(tr("Information"));
}

} /* namespace FT800EMUQT */

/* end of file */
