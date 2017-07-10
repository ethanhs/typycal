#include "Python.h"
#include <frameobject.h>
#include <stdio.h>
#include <stdbool.h>

#include "TypeManager.hh"
#include "Settings.hh"
#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 6
#error "This library only supports Python 3.6+"
#endif

extern "C" {

//TODO: distutils.sysconfig.get_python_lib(stdlib=True)

// The main frame hook, generate the name of the type of objecstate, and serializes it to the store for later analysis.
PyObject* typycal_evalframe(PyFrameObject* frame, int exc) {
    PyObject* err = PyErr_Occurred();
    bool error = false;
    if (err != NULL) {
        error = (PyObject_IsTrue(err) == 1) ? true : false;
    }
    if (exc || error) {
        return _PyEval_EvalFrameDefault(frame, exc);
    } else {
        const char* file_name = PyUnicode_AsUTF8(frame->f_code->co_filename);
        const char* f_name = PyUnicode_AsUTF8(frame->f_code->co_name);
        if (strcmp(f_name, "<module>") != 0 && whitelisted(std::string(file_name))) {
            PyObject* locals = PyObject_GetAttrString((PyObject*)frame, "f_locals");
            if (locals == NULL) {
                printf("ERR:%s:%s locals is null, typycal cannot test this frame.\n", file_name, f_name);
            }
            if (PyDict_CheckExact(frame->f_locals)) {
                int argc = frame->f_code->co_argcount;
                PyObject* vals = PyMapping_Items(locals);
                PyObject* res = _PyEval_EvalFrameDefault(frame, exc);
                analyze_types(file_name, f_name, vals, argc, res);
                return res;
            } else {
                printf("Failed to infer type of frame %s", f_name);
            }
        }

    }
    return _PyEval_EvalFrameDefault(frame, exc);
}

static PyObject *typycal_unhook(PyObject *self) {
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = _PyEval_EvalFrameDefault;
    if (func == typycal_evalframe) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}



static PyObject *typycal_hook(PyObject *self, PyObject* path) {
	Py_INCREF(path);
	set_lib_path();
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = typycal_evalframe;
	if (func == typycal_evalframe) {
		Py_DECREF(path);
		Py_RETURN_FALSE;
	}
	Py_DECREF(path);
    Py_RETURN_TRUE;
}

static PyObject *typycal_hooked(PyObject* self) {
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
    if (func == typycal_evalframe) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


static PyMethodDef TypycalMethods[] = {
    {
    "hook",
    (PyCFunction)typycal_hook,
    METH_O,
    "Enable hooking. Pass the str path to the config file."
    },
    {
    "unhook",
    (PyCFunction)typycal_unhook,
    METH_NOARGS,
    "Remove the hook from the interpreter."
    },
    {
    "hooked",
    (PyCFunction)typycal_hooked,
    METH_NOARGS,
    "Return True if hooked, False otherwise."
    },
    {NULL, NULL, 0, NULL} /* sentinal node */
};

static struct PyModuleDef typycalmodule = {
	PyModuleDef_HEAD_INIT,
	"typycal",   /* name of module */
	"Typycal, runtime type inference based on Python 3.6 frame evaluation API", /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
			  or -1 if the module keeps state in global variables. */
	TypycalMethods
};

PyMODINIT_FUNC PyInit_typycal(void)
{
	return PyModule_Create(&typycalmodule);
}

}