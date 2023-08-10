/*!
 * @file InteractionModule.cpp
 * @date 7/17/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include <Python.h>

#include <filesystem>
#include <fstream>

int translate(char *string) { return 1; }

static PyObject *method_transfer(PyObject *self, PyObject *args) {
  if (!std::filesystem::exists("output")) {
    return PyLong_FromLong(-2);
  }
  char *str = NULL;
  if (!PyArg_ParseTuple(args, "s", &str)) {
    return PyLong_FromLong(-1);
  }
  int parsed = translate(str);
  std::ofstream out("output");
  out << str;
  out.close();
  return PyLong_FromLong(parsed);
}

static PyObject *method_init(PyObject *self, PyObject *args) {
  std::ofstream outputFile("output");
  outputFile.close();
  Py_RETURN_NONE;
}

static PyObject *method_deinit(PyObject *self, PyObject *args) {
  remove("output");
  Py_RETURN_NONE;
}

static PyMethodDef InteractionMethods[] = {
    {"transfer", method_transfer, METH_VARARGS,
     "Python interface for transfering"},
    {"init", method_init, METH_VARARGS, "Python interface for initializing"},
    {"deinit", method_deinit, METH_VARARGS,
     "Python interface for deinitializing"},
    // Terminate the array with an object containing nulls.
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef interactModule = {
    PyModuleDef_HEAD_INIT, "interact",
    "Python interface for interacting with the C library function", -1,
    InteractionMethods};

PyMODINIT_FUNC PyInit_interact(void) {
  return PyModule_Create(&interactModule);
}
