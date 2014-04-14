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
#include "main_window.h"
#include "emulator_viewport.h"
#include "ft800emu_emulator.h"
#include "asset_converter.h"

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

#ifdef NL_OS_WINDOWS
	HRESULT hr;
	hr = hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool coInitOk = (hr == S_OK) || (hr == S_FALSE);
#endif
#ifdef FT800EMU_PYTHON
	printf("With Python support\n");
	Py_Initialize();
	PyObject* sysPath = PySys_GetObject((char*)"path");
	{
		PyObject* curPath = PyString_FromString(QDir::currentPath().toUtf8().data()); // FIXME Unicode ?
		PyList_Append(sysPath, curPath);
		Py_DECREF(curPath);
	}
	FT800EMUQT::AssetConverter::init();
#endif /* FT800EMU_PYTHON */
	QApplication app(argc, const_cast<char **>(argv));
	app.setStyleSheet("QStatusBar::item { border: 0px solid black }; ");
	QMap<QString, QSize> customSizeHints = parseCustomSizeHints(argc, argv);
#ifdef FT800EMU_PYTHON
	{
		PyObject* curPath = PyString_FromString(QCoreApplication::applicationDirPath().toUtf8().data()); // FIXME Unicode ?
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
#ifdef NL_OS_WINDOWS
	if (coInitOk) CoUninitialize();
#endif
	return result;
}
