#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD
	PyObject *nodes;
	PyObject *names;
	PyObject *edges;
	PyObject *dist;
	long int n;
	long int e;
} FastpathObject;

/*
static PyObject * 
Custom_iter(PyObject *self)
{
  Py_INCREF(self);
  return self;
}

static PyObject * 
Custom_iternext(PyObject *self)
{
  FastpathObject *p = (FastpathObject *)self;
  if (p->i < p->m) {
    PyObject *tmp = Py_BuildValue("l", p->i);
    (p->i)++;
    return tmp;
  } else {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}
*/

static void
Custom_dealloc(FastpathObject *self)
{
    Py_XDECREF(self->nodes);
    Py_XDECREF(self->names);
    Py_XDECREF(self->edges);
    Py_XDECREF(self->dist);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyObject *
Custom_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    FastpathObject *self;
    self = (FastpathObject *) type->tp_alloc(type, 0);

    if (self != NULL) {
		self->nodes = PyDict_New();
		self->names = PyDict_New();
        if (self->nodes == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        if (self->names == NULL) {
            Py_DECREF(self);
            return NULL;
        }
		self->n = 0;
		self->e = 0;
    }
    return (PyObject *) self;
}

static PyObject *
Custom_init(FastpathObject *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

static PyObject *
Add_Edge(FastpathObject *self, PyObject *args)
{
	char *src, *dst, *wgt;

	if (!PyArg_ParseTuple(args, "sss", &src, &dst, &wgt))  return 0;
	
	PyDict_SetItem(self->names, PyUnicode_FromString("foo"), PyUnicode_FromString(src));

	return PyUnicode_FromString("1");
}

static PyObject *
Get_Edge(FastpathObject *self, PyObject *args)
{
	return PyDict_GetItem(self->names, PyUnicode_FromString("foo"));
	//return PyUnicode_FromString("bar");
}


static PyMemberDef Custom_members[] = {
    {"nodes", T_OBJECT_EX, offsetof(FastpathObject, nodes), 0, "nodes"},
    {"names", T_OBJECT_EX, offsetof(FastpathObject, names), 0, "names"},
    {NULL}  /* Sentinel */
};


static PyMethodDef Custom_methods[] = {
    //{"name", (PyCFunction) Custom_name, METH_NOARGS, "Return the name, combining the first and last name"},
	//{"myiter",  (PyCFunction) iterable, METH_VARARGS, "Iterate from i=0 while i<m."},
	{"get_edge",  (PyCFunction) Get_Edge, METH_VARARGS, "Iterate from i=0 while i<m."},
	{"add_edge", (PyCFunction) Add_Edge, METH_VARARGS, "Iterate from i=0 while i<m."},
    {NULL}  /* Sentinel */
};

static PyTypeObject CustomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "custom2.Custom",
    .tp_doc       = "Custom objects",
    .tp_basicsize = sizeof(FastpathObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new       = Custom_new,
    .tp_init      = (initproc) Custom_init,
    .tp_dealloc   = (destructor) Custom_dealloc,
    .tp_members   = Custom_members,
    .tp_methods   = Custom_methods
	//.tp_iter      = Custom_iter,
	//.tp_iternext  = Custom_iternext
};

static PyModuleDef custommodule = {
    PyModuleDef_HEAD_INIT,
    .m_name   = "custom2",
    .m_doc    = "Example module that creates an extension type.",
    .m_size   = -1,
};

PyMODINIT_FUNC
PyInit_custom2(void)
{
    PyObject *m;

    if (PyType_Ready(&CustomType) < 0)
        return NULL;

    m = PyModule_Create(&custommodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CustomType);
    if (PyModule_AddObject(m, "Custom", (PyObject *) &CustomType) < 0) {
        Py_DECREF(&CustomType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
