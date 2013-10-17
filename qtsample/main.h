/**
 * main.h
 * $Id$
 * \file main.cpp
 * \brief main.cpp
 * \date 2013-10-17 18:54GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */
 
#include <QThread>

class EmulatorThread : public QThread
{
	Q_OBJECT

protected:
	void run();
	
signals:
	void repaint();
	
};
