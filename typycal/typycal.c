#ifdef __cplusplus
extern "C" {
#endif
#include "Python.h"
#include <frameobject.h>
#include <stdio.h>
#include <stdbool.h>



#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 6
#error "This library only supports Python 3.6+"
#endif

void write_type(PyObject* ob);

#define DEBUG

FILE* out;

void writeout(const char* str) {
    #ifndef DEBUG
    fprintf(out, str);
    #else
    printf("%s", str);
    #endif
}


void write_list(PyObject* list) {
    writeout("List");
    Py_ssize_t len = PySequence_Length(list);
    if (len > 0) {
        writeout("[");
        write_type(PySequence_GetItem(list, 0));
        for(int i = 1; i < len;i++) {
            writeout(", ");
            write_type(PySequence_GetItem(list, i));
        }
        writeout("]");
    }
}


void write_func(PyObject* ob) {
    writeout("Callable");
}


// returns PEP 3107/484/526 compatible types from PyObject*. If no type can be inferred, it falls back to Any.
void write_type(PyObject* ob) {
    PyTypeObject* type = ob->ob_type;
    if (type != NULL) {
        const char* name = type->tp_name;
        if (strcmp(name, "int") == 0) {
            writeout("int");
        } else if (strcmp(name, "str") == 0) {
            writeout("str");
        } else if (strcmp(name, "float") == 0) {
            writeout("float");
        } else if (strcmp(name, "NoneType") == 0) {
            writeout("None");
        } else if (strcmp(name, "list") == 0) {
            write_list(ob);
        } else if (strcmp(name, "function") == 0 || strcmp(name, "builtin_function_or_method") == 0) {
            write_func(ob);
        } else {
            writeout(name);
        }
    } else {
    writeout("Any");
    }
}


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
                writeout(file_name);
                writeout(":");
                writeout(f_name);
                writeout(":");
                int argc = frame->f_code->co_argcount;
                PyObject* vals = PyMapping_Values(locals);
                if (vals != NULL) {
                    for (int i = argc - 1; i >= 0; i--) {
                        PyObject* val = PySequence_GetItem(vals, i);
                        if (val != NULL) {
                            write_type(val);
                            printf(" ");
                        }
                    }

                } else {
                    puts("ERR: Values null\n");
                }
                PyObject* res = _PyEval_EvalFrameDefault(frame, exc);
                PyObject* ret = frame->f_valuestack[0];
                writeout(" -> ");
                if (ret == Py_None) {
                    writeout("None");
                } else {
                    write_type(ret);
                }
                writeout("\n");
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

#ifdef __cplusplus
}
#endif
