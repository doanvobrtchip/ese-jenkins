/*!
 * @file PyUtil.cpp
 * @date 7/20/2023
 * @author Liem Do <liem.do@brtchip.com>
 */

#ifdef FT800EMU_PYTHON
#include "PyUtil.h"
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

QString PyUtil::pyError()
{
	printf("---\nPython ERROR: \n");
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyObject *errStr = PyObject_Repr(pvalue);
	const char *pStrErrorMessage = PyUnicode_AsUTF8(errStr);
	QString error = QString::fromUtf8(pStrErrorMessage);
#ifdef WIN32
	wprintf(L"%s\n", pStrErrorMessage ? error.toStdWString().c_str() : L"<NULL>");
#else
	printf("%s\n", pStrErrorMessage ? error.toLocal8Bit().data() : "<NULL>");
#endif
	Py_DECREF(errStr);
	printf("---\n");
	return error;
}
#endif /* FT800EMU_PYTHON */
