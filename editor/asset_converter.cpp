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

// Qt includes

// Emulator includes

// Project includes

namespace FT800EMUQT {

namespace {

#ifdef FT800EMU_PYTHON
PyObject *a_ImageConvModule = NULL;
PyObject *a_ImageConvRun = NULL;
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
			PyObject *pyImageConv = PyObject_CallObject(pyImageConvClass, pyArgs);
			Py_DECREF(pyImageConvClass); pyImageConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (pyImageConv)
			{
				a_ImageConvRun = PyObject_GetAttrString(pyImageConv, "run");
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
		PyErr_Print();
		printf("---\n");
	}

	// error = true;

	// ...
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::release()
{
#ifdef FT800EMU_PYTHON
	Py_DECREF(a_ImageConvRun); a_ImageConvRun = NULL;
	Py_DECREF(a_ImageConvModule); a_ImageConvModule = NULL;
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::convertImage(const QString &inFile, const QString &outName, int format)
{
#ifdef FT800EMU_PYTHON
	bool error = true;

	QByteArray inFileU8 = inFile.toUtf8(); // FIXME Unicode ?
	QByteArray outNameU8 = outName.toUtf8();

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
	pyValue = PyString_FromString("0");
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
		PyErr_Print();
		printf("---\n");
	}

#endif /* FT800EMU_PYTHON */
}

} /* namespace FT800EMUQT */

/* end of file */
