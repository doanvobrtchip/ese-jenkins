/**
 * asset_converter.cpp
 * $Id$
 * \file asset_converter.cpp
 * \brief asset_converter.cpp
 * \date 2014-01-30 14:26GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "asset_converter.h"

// STL includes
#include <stdio.h>
#include <sstream>

// Qt includes

// Emulator includes

// Project includes

namespace FT800EMUQT {

namespace {

#ifdef FT800EMU_PYTHON
PyObject *a_ImageConvModule = NULL;
PyObject *a_ImageConvObject = NULL;
PyObject *a_ImageConvRun = NULL;
QString a_ImageConvError;
PyObject *a_RawConvModule = NULL;
PyObject *a_RawConvObject = NULL;
PyObject *a_RawConvRun = NULL;
QString a_RawConvError;
#endif /* FT800EMU_PYTHON */

}

void AssetConverter::init()
{
#ifdef FT800EMU_PYTHON
	bool error = true;

	PyObject *pyImageConvScript = PyString_FromString("img_cvt");
	PyObject *a_ImageConvModule = PyImport_Import(pyImageConvScript);
	Py_DECREF(pyImageConvScript); pyImageConvScript = NULL;

	if (a_ImageConvModule)
	{
		PyObject *pyImageConvClass = PyObject_GetAttrString(a_ImageConvModule, "Image_Conv");
		if (pyImageConvClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			a_ImageConvObject = PyObject_CallObject(pyImageConvClass, pyArgs);
			Py_DECREF(pyImageConvClass); pyImageConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (a_ImageConvObject)
			{
				a_ImageConvRun = PyObject_GetAttrString(a_ImageConvObject, "run");
				if (a_ImageConvRun)
				{
					printf("Image Converter available\n");
					error = false;
				}
			}
		}
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		char *pStrErrorMessage = PyString_AsString(pvalue);
		a_ImageConvError = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		printf("---\n");
	}

	error = true;

	PyObject *pyRawConvScript = PyString_FromString("raw_cvt");
	PyObject *a_RawConvModule = PyImport_Import(pyRawConvScript);
	Py_DECREF(pyRawConvScript); pyRawConvScript = NULL;

	if (a_RawConvModule)
	{
		PyObject *pyRawConvClass = PyObject_GetAttrString(a_RawConvModule, "Raw_Conv");
		if (pyRawConvClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			a_RawConvObject = PyObject_CallObject(pyRawConvClass, pyArgs);
			Py_DECREF(pyRawConvClass); pyRawConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (a_RawConvObject)
			{
				a_RawConvRun = PyObject_GetAttrString(a_RawConvObject, "run");
				if (a_RawConvRun)
				{
					printf("Raw Converter available\n");
					error = false;
				}
			}
		}
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		char *pStrErrorMessage = PyString_AsString(pvalue);
		a_RawConvError = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		printf("---\n");
	}

	// error = true;

	// ...
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::release()
{
#ifdef FT800EMU_PYTHON
	Py_XDECREF(a_ImageConvRun); a_ImageConvRun = NULL;
	Py_XDECREF(a_ImageConvObject); a_ImageConvObject = NULL;
	Py_XDECREF(a_ImageConvModule); a_ImageConvModule = NULL;
	Py_XDECREF(a_RawConvRun); a_RawConvRun = NULL;
	Py_XDECREF(a_RawConvObject); a_RawConvObject = NULL;
	Py_XDECREF(a_RawConvModule); a_RawConvModule = NULL;
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::convertImage(QString &buildError, const QString &inFile, const QString &outName, int format)
{
#ifdef FT800EMU_PYTHON
	if (a_ImageConvRun)
	{
		bool error = true;

		QByteArray inFileU8 = inFile.toLocal8Bit(); // FIXME Unicode ?
		QByteArray outNameU8 = outName.toLocal8Bit();
		std::stringstream sFormat;
		sFormat << format;

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(6);
		pyValue = PyString_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyString_FromString(inFileU8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyString_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyString_FromString(outNameU8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyString_FromString("-f");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyString_FromString(sFormat.str().c_str());
		PyTuple_SetItem(pyTuple, 5, pyValue);
		PyTuple_SetItem(pyArgs, 0, pyTuple);
		PyObject *pyResult = PyObject_CallObject(a_ImageConvRun, pyArgs);
		Py_DECREF(pyArgs); pyArgs = NULL;
		if (pyResult)
		{
			printf("Image converted\n");
			error = false;
		}
		if (error)
		{
			printf("---\nPython ERROR: \n");
			PyObject *ptype, *pvalue, *ptraceback;
			PyErr_Fetch(&ptype, &pvalue, &ptraceback);
			char *pStrErrorMessage = PyString_AsString(pvalue);
			if (pStrErrorMessage)
			{
				buildError = QString::fromLocal8Bit(pStrErrorMessage);
			}
			else
			{
				buildError = "<i>(Python)</i> Unknown Error";
			}
			QByteArray er = buildError.toLocal8Bit();
			printf("%s\n", er.data());
			printf("---\n");

			// Reinitialize Python converters
			release();
			init();
		}
	}
	else
	{
		buildError = "<i>(Python)</i> " + a_ImageConvError;
	}
#else
	buildError = "Python not available";
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::convertRaw(QString &buildError, const QString &inFile, const QString &outName, int begin, int length)
{
#ifdef FT800EMU_PYTHON
	if (a_RawConvRun)
	{
		bool error = true;

		QByteArray inFileU8 = inFile.toLocal8Bit(); // FIXME Unicode ?
		QByteArray outNameU8 = outName.toLocal8Bit();
		std::stringstream sBegin;
		sBegin << begin;
		std::stringstream sLength;
		sLength << length;

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(8);
		pyValue = PyString_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyString_FromString(inFileU8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyString_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyString_FromString(outNameU8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyString_FromString("-s");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyString_FromString(sBegin.str().c_str());
		PyTuple_SetItem(pyTuple, 5, pyValue);
		pyValue = PyString_FromString("-l");
		PyTuple_SetItem(pyTuple, 6, pyValue);
		pyValue = PyString_FromString(sLength.str().c_str());
		PyTuple_SetItem(pyTuple, 7, pyValue);
		PyTuple_SetItem(pyArgs, 0, pyTuple);
		PyObject *pyResult = PyObject_CallObject(a_RawConvRun, pyArgs);
		Py_DECREF(pyArgs); pyArgs = NULL;
		if (pyResult)
		{
			printf("Raw converted\n");
			error = false;
		}
		if (error)
		{
			printf("---\nPython ERROR: \n");
			PyObject *ptype, *pvalue, *ptraceback;
			PyErr_Fetch(&ptype, &pvalue, &ptraceback);
			char *pStrErrorMessage = PyString_AsString(pvalue);
			if (pStrErrorMessage)
			{
				buildError = QString::fromLocal8Bit(pStrErrorMessage);
			}
			else
			{
				buildError = "<i>(Python)</i> Unknown Error";
			}
			QByteArray er = buildError.toLocal8Bit();
			printf("%s\n", er.data());
			printf("---\n");

			// Reinitialize Python converters
			release();
			init();
		}
	}
	else
	{
		buildError = "<i>(Python)</i> " + a_RawConvError;
	}
#else
	buildError = "Python not available";
#endif /* FT800EMU_PYTHON */
}

} /* namespace FT800EMUQT */

/* end of file */
