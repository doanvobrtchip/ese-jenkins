/**
 * command_log.cpp
 * $Id$
 * \file command_log.cpp
 * \brief command_log.cpp
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "command_log.h"

// STL includes

// Qt includes
#include <QVBoxLayout>

// Emulator includes

// Project includes

namespace FTEDITOR {

CommandLog::CommandLog(QWidget *parent) : QWidget(parent)
{
	m_DisplayerOutput = new QTextEdit();
	m_DisplayerOutput->setReadOnly(true);
	m_DisplayerOutput->setFocusPolicy(Qt::NoFocus);
	m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_DisplayerOutput);
	layout->addWidget(m_CommandInput);
	setLayout(layout);

	connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
}

CommandLog::~CommandLog()
{

}

// m_DisplayerOutput->append(str.c_str());

void CommandLog::returnPressed()
{
	QString text = m_CommandInput->text();
	if (text.isEmpty())
		return;

	// std::string cmd = text.toAscii().data();
	// execute(cmd);

	m_CommandInput->clear();
}

} /* namespace FTEDITOR */

/* end of file */
