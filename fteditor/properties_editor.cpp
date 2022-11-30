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

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "properties_editor.h"

// STL includes
#include "stdio.h"

// Qt includes
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

// Emulator includes

// Project includes

namespace FTEDITOR {

PropertiesEditor::PropertiesEditor(QWidget *parent) : QWidget(parent), m_OwnCurrentEditWidget(false), m_SurpressSet(false)
{
	QVBoxLayout *infoLayout = new QVBoxLayout();
	m_InfoGroupBox = new QGroupBox(this);
	m_InfoGroupBox->setLayout(infoLayout);
	m_InfoLabel = new QLabel(this);
    m_InfoLabel->setMinimumWidth(150);
	m_InfoLabel->setTextFormat(Qt::RichText);
	m_InfoLabel->setWordWrap(true);
	infoLayout->addWidget(m_InfoLabel);
	m_InfoGroupBox->hide();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_InfoGroupBox);
	layout->addStretch();
	setLayout(layout);
	m_GlobalLayout = layout;

	m_EditLayout = new QVBoxLayout();
	m_LayoutInserted = false;

	translate();
}

PropertiesEditor::~PropertiesEditor()
{

}

void PropertiesEditor::setInfo(QString message, bool isError)
{
	if (m_SurpressSet) return;

	m_InfoLabel->setText(message);
	if (message.isEmpty())
	{
		m_InfoGroupBox->hide();
	}
	else
	{
		m_InfoGroupBox->show();
		if (isError)
		{
			emit errorSet(message);
		}
		else
		{
			emit infoSet(message);
		}
	}
}

void PropertiesEditor::setError(QString message)
{
	setInfo(message, true);
}

void PropertiesEditor::setEditWidget(QWidget *widget, bool own, QWidget *setter)
{
	if (m_SurpressSet) return;

	std::vector<QWidget *> widgets;
	if (widget) widgets.push_back(widget);
	setEditWidgets(widgets, own, setter);
}

void PropertiesEditor::setEditWidgets(const std::vector<QWidget *> &widgets, bool own, QWidget *setter)
{
	if (m_SurpressSet) return;

	for (std::vector<QWidget *>::iterator it(m_CurrentEditWidgets.begin()), end(m_CurrentEditWidgets.end()); it != end; ++it)
	{
		m_EditLayout->removeWidget((*it));
		if (m_OwnCurrentEditWidget)
		{
			delete (*it);
		}
		else
		{
			(*it)->setHidden(true);
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
			(*it)->setHidden(false);
		}
		if (!m_LayoutInserted)
		{
			m_GlobalLayout->insertLayout(0, m_EditLayout);
			m_LayoutInserted = true;
		}
	}
	else if (m_LayoutInserted)
	{
		m_GlobalLayout->removeItem(m_EditLayout);
		m_LayoutInserted = false;
	}

	m_CurrentEditWidgetSetter = setter;
	emit setterChanged(setter);
}

void PropertiesEditor::translate()
{
	m_InfoGroupBox->setTitle(tr("Information"));
}

} /* namespace FTEDITOR */

/* end of file */
