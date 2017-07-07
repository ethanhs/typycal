#include "Python.h"
#include <frameobject.h>
#include <stdio.h>
#include <stdbool.h>

#include "TypeManager.hh"

#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 6
#error "This library only supports Python 3.6+"
#endif

extern "C" {



bool whitelisted(const char* file) {
    if (strcmp(file, "<stdin>") == 0) {
        return true;
    } else {
        return false;
    }

}


// The main frame hook, gestate the name of the type of objecstate, and serializes it to the store for later analysis.
PyObject* typycal_evalframe(PyFrameObject* frame, int exc) {
    if (exc) {
        return _PyEval_EvalFrameDefault(frame, exc);
    } else {
        const char* file_name = PyUnicode_AsUTF8AndSize(frame->f_code->co_filename, NULL);
        const char* f_name = PyUnicode_AsUTF8AndSize(frame->f_code->co_name, NULL);
        bool check = whitelisted(file_name);
        if (strcmp(f_name, "<module>") != 0 && check) {
            PyObject* locals = PyObject_GetAttrString((PyObject*)frame, "f_locals");
            if (locals == NULL) {
                puts("ERR: locals is null, typycal cannot test this frame.\n");
            }
            if (PyDict_CheckExact(frame->f_locals)) {
                int argc = frame->f_code->co_argcount;
                PyObject* vals = PyMapping_Items(locals);
                PyObject* res = _PyEval_EvalFrameDefault(frame, exc);
                PyObject* ret = frame->f_valuestack[0];
                analyze_types(vals, argc, ret);
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

	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = typycal_evalframe;
    if (func == typycal_evalframe) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyObject *typycal_enabled(PyObject* self, PyObject* args) {
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

