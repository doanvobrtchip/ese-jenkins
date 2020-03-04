/**
 * command_log.h
 * $Id$
 * \file command_log.h
 * \brief command_log.h
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_COMMAND_LOG_H
#define FTEDITOR_COMMAND_LOG_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes

// Qt includes
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>

// Emulator includes

// Project includes

namespace FTEDITOR {

/**
 * CommandLog
 * \brief CommandLog
 * \date 2010-02-05 20:27GMT
 * \author Jan Boon (Kaetemi)
 */
class CommandLog : public QWidget
{
	Q_OBJECT
	
public:
	CommandLog(QWidget *parent);
	virtual ~CommandLog();

private slots:
	void returnPressed();

private:
	QTextEdit *m_DisplayerOutput;
	QLineEdit *m_CommandInput;

private:
	CommandLog(const CommandLog &);
	CommandLog &operator=(const CommandLog &);
	
}; /* class CommandLog */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_COMMAND_LOG_H */

/* end of file */
