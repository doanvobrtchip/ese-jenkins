/**
 * main.cpp
 * $Id$
 * \file main.cpp
 * \brief main.cpp
 * \date 2013-10-15 13:18GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <stdio.h>
#include <QApplication>
#include <QDebug>
#include "main_window.h"

void usage()
{
	/* from Qt sample */

	qWarning() << "Usage: mainwindow [-SizeHint<color> <width>x<height>] ...";
	exit(1);
}

QMap<QString, QSize> parseCustomSizeHints(int argc, char **argv)
{
	/* from Qt sample */

	QMap<QString, QSize> result;

	for (int i = 1; i < argc; ++i) {
		QString arg = QString::fromLocal8Bit(argv[i]);

		if (arg.startsWith(QLatin1String("-SizeHint"))) {
			QString name = arg.mid(9);
			if (name.isEmpty())
				usage();
			if (++i == argc)
				usage();
			QString sizeStr = QString::fromLocal8Bit(argv[i]);
			int idx = sizeStr.indexOf(QLatin1Char('x'));
			if (idx == -1)
				usage();
			bool ok;
			int w = sizeStr.left(idx).toInt(&ok);
			if (!ok)
				usage();
			int h = sizeStr.mid(idx + 1).toInt(&ok);
			if (!ok)
				usage();
			result[name] = QSize(w, h);
		}
	}

	return result;
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char* argv[])
{
#ifdef NL_OS_WINDOWS
	HRESULT hr;
	hr = hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool coInitOk = (hr == S_OK) || (hr == S_FALSE);
#endif
	QApplication app(argc, const_cast<char **>(argv));
	QMap<QString, QSize> customSizeHints = parseCustomSizeHints(argc, argv);
	FTQT::MainWindow mainWin(customSizeHints);
	mainWin.resize(800, 600);
	mainWin.show(); // calls isVisible(true)
	int result = app.exec();
#ifdef NL_OS_WINDOWS
	if (coInitOk) CoUninitialize();
#endif
	return result;
}