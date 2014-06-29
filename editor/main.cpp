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

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include <stdio.h>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QString>
#include <QStyleFactory>
#include "main_window.h"
#include "emulator_viewport.h"
#include "ft800emu_emulator.h"
#include "asset_converter.h"
#ifdef WIN32
#	include <Objbase.h>
#endif
#ifdef FT800EMU_SDL2
#	include <SDL.h>
#endif

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
	Q_INIT_RESOURCE(icons);

#ifndef WIN32
	// Workaround to default -style=gtk+ on recent Cinnamon versions
	char *currentDesktop = getenv("XDG_CURRENT_DESKTOP");
	if (currentDesktop)
	{
		printf("XDG_CURRENT_DESKTOP: %s\n",  currentDesktop);
		if (!strcmp(currentDesktop, "X-Cinnamon"))
		{
			printf("Theme override for X-Cinnamon\n");
			setenv("XDG_CURRENT_DESKTOP", "gnome", 1);
		}
	}
#endif

#ifdef FT800EMU_PYTHON
	printf("With Python support\n");
	Py_Initialize();
	PyObject* sysPath = PySys_GetObject((char*)"path");
	{
		QByteArray cpUtf8 = QDir::currentPath().toUtf8();
		PyObject* curPath = PyUnicode_FromString(cpUtf8.data());
		PyList_Append(sysPath, curPath);
		Py_DECREF(curPath);
	}
	FT800EMUQT::AssetConverter::init();
#endif /* FT800EMU_PYTHON */
	QApplication app(argc, const_cast<char **>(argv));

	app.setStyleSheet("QStatusBar::item { border: 0px solid black }; ");

	/*
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QPalette palette = app.palette();
	palette.setColor(QPalette::Window, QColor(64, 64, 64));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(48, 48, 48));
	palette.setColor(QPalette::AlternateBase, QColor(64, 64, 64));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(64, 64, 64));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(64, 96, 128));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	app.setPalette(palette);
	*/

	QMap<QString, QSize> customSizeHints = parseCustomSizeHints(argc, argv);
#ifdef FT800EMU_PYTHON
	{
		QByteArray apUtf8 = QCoreApplication::applicationDirPath().toUtf8();
		PyObject* curPath = PyUnicode_FromString(apUtf8.data());
		PyList_Append(sysPath, curPath);
		Py_DECREF(curPath);
	}
#endif /* FT800EMU_PYTHON */
	FT800EMUQT::MainWindow mainWin(customSizeHints);
	mainWin.resize(800, 600);
	mainWin.show(); // calls isVisible(true)
	int result = app.exec();
#ifdef FT800EMU_PYTHON
	FT800EMUQT::AssetConverter::release();
    Py_Finalize();
#endif /* FT800EMU_PYTHON */
	return result;
}
