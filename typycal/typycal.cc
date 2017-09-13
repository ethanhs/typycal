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

/*[clinic input]
module _typycal
[clinic start generated code]*/

// The main frame hook, generate the name of the type of objecstate, and serializes it to the store for later analysis.
PyObject* typycal_evalframe(PyFrameObject* frame, int exc) {
    if (exc) {
        return _PyEval_EvalFrameDefault(frame, exc);
    } else {
        std::string file_name(PyUnicode_AsUTF8(frame->f_code->co_filename));
		if (check_file(file_name)) {
			std::string f_name(PyUnicode_AsUTF8(frame->f_code->co_name));
			if (check_name(f_name)) {
				PyObject* locals = PyObject_GetAttrString((PyObject*)frame, "f_locals");
				if (locals == NULL) {
					printf("%s:%s locals is null, typycal cannot test this frame.\n", file_name.c_str(), f_name.c_str());
				}
				else if (PyMapping_Check(frame->f_locals)) {
					int argc = frame->f_code->co_argcount;
					PyObject* vals = PyMapping_Items(locals);
					PyObject* res = _PyEval_EvalFrameDefault(frame, exc);
					analyze_types(file_name, f_name, vals, argc, res);
					Py_DECREF(vals);
					Py_DECREF(locals);
					return res;
				}
				else {
					printf("Failed to infer type of frame %s in file %s", f_name.c_str(), file_name.c_str());
				}
				Py_XDECREF(locals);
			}
		}

    }
    return _PyEval_EvalFrameDefault(frame, exc);
}

/*[clinic input]
_typycal.unhook

Unhook and stop listening to frames. Execution should resume at the same speed.
[clinic start generated code]*/
static PyObject *typycal_unhook(PyObject *module) {
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = _PyEval_EvalFrameDefault;
    if (func == typycal_evalframe) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


/*[clinic input]
_typycal.hook

    incremental: bool = True


Hook into Python and start listening for frames. This *will* slow execution
[clinic start generated code]*/
static PyObject *typycal_hook(PyObject *module, PyObject* args, PyObject* kwargs) {
	
	//PyArg_ParseTupleAndKeywords
	Py_INCREF(args);
	init_log(args);
	PyThreadState* state = PyThreadState_Get();
	_PyFrameEvalFunction func = state->interp->eval_frame;
	state->interp->eval_frame = typycal_evalframe;
	if (func == typycal_evalframe) {
		Py_DECREF(args);
		Py_RETURN_FALSE;
	}
	Py_DECREF(args);
	Py_RETURN_TRUE;
}

/*[clinic input]
_typycal.hooked

Check if the typycal is hooked into the interpreter. Returns True if so, False otherwise.
[clinic start generated code]*/
static PyObject *typycal_hooked(PyObject* module) {
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
	METH_VARARGS | METH_KEYWORDS,
	"Enable hooking, the signature:"
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
	"_typycal",   /* name of module */
	"Typycal, runtime type inference based on Python 3.6 frame evaluation API", /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
			  or -1 if the module keeps state in global variables. */
	TypycalMethods
};

PyMODINIT_FUNC PyInit__typycal(void)
{
	PyObject* module = PyModule_Create(&typycalmodule);
	TypeManager* logger = new TypeManager();
	PyObject* m_logger = PyCapsule_New(&logger, "logger", nullptr);
	if (!m_logger)
	{
		PyErr_SetString(PyExc_RuntimeError, "Failed to instantiate the logger.");
		return nullptr;
	}
	int ret = PyModule_AddObject(module, "logger", m_logger);
	if (ret != 0)
	{
		PyErr_SetString(PyExc_RuntimeError, "Failed to add the logger to the typycal module.");
		return nullptr;
	}
	return module;
}

}