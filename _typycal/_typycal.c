#include "Python.h"
#include <frameobject.h>
#include <stdio.h>
#include <stdbool.h>

#include "TypeLogger.h"

#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 6
#error "This library only supports Python 3.6+"
#endif

#ifdef __cplusplus
extern "C" {
#endif



// The main frame hook, generate the name of the type of objecstate, and serializes it to the store for later analysis.
PyObject* typycal_evalframe(PyFrameObject* frame, int exc) {
    if (exc) {
        return _PyEval_EvalFrameDefault(frame, exc);
    } else {
        PyObject* file = frame->f_code->co_filename;
        Py_INCREF(file);
        PyObject* frame_name = frame->f_code->co_name;
        Py_INCREF(frame_name);
        PyObject* locals = PyObject_GetAttrString((PyObject*)frame, "f_locals");
        if (locals == NULL) {
            puts("locals is null, typycal cannot test this frame.");
        } else {
            if (PyMapping_Check(frame->f_locals)) {
                int argc = frame->f_code->co_argcount;
                PyObject* vals = PyMapping_Items(locals);
                PyObject* res = _PyEval_EvalFrameDefault(frame, exc);
                analyze_types(file, frame_name, vals, argc, res);
                return res;
            } else {
                puts("Failed to infer type of frame");
            }
        }
        Py_DECREF(file);
        Py_DECREF(frame_name);
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



static PyObject *typycal_hook(PyObject *self) {
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = typycal_evalframe;
	if (func == typycal_evalframe) {
		Py_RETURN_FALSE;
	}
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
    METH_NOARGS,
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
    {NULL, NULL, 0, NULL} /* sentinal node, do not delete */
};

static struct PyModuleDef typycalmodule = {
	PyModuleDef_HEAD_INIT,
	"_typycal",
	"Typycal, runtime type inference based on Python 3.6 frame evaluation API",
	-1,
	TypycalMethods
};

PyMODINIT_FUNC PyInit__typycal(void)
{
	return PyModule_Create(&typycalmodule);
}


#ifdef __cplusplus
}
#endif