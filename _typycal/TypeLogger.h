#include "Python.h"
#include <wchar.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



// forward reference for recursive type inference (used by sequence_type)
PyObject* get_type(PyObject* ob);

PyObject* sequence_type(PyObject* ob) {
    Py_ssize_t len = PySequence_Length(ob);
    PyObject* items = PyUnicode_FromString("");
    // we currently only check sequences of length less than or equal to 3
    if (len > 3) {
        return items;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject* item = PySequence_GetItem(ob, i);
        PyObject* type = get_type(item);
        Py_DECREF(item);
        if (i == len - 1) {
            items = PyUnicode_Concat(items, type);
        }
        else {
            PyObject* formatted = PyUnicode_FromFormat("%U, ", type);
            items = PyUnicode_Concat(items, formatted);
        }
    }
    PyObject* ret = PyUnicode_FromFormat("[%U]", items);
    Py_DECREF(items);
    return ret;
}

/* PEP 484 rendering of the List type. If it is too long it becomes "List", otherwise it is of the form
 * "List[T1,T2,T3]"
 */
PyObject* list_type(PyObject* ob) {
    PyObject* type = PyUnicode_FromWideChar(L"List", 4);
    PyObject* list = sequence_type(ob);
    PyObject* ret = PyUnicode_Concat(type, list);
    Py_DECREF(list);
    Py_DECREF(type);
    return ret;
}

/* PEP 484 rendering of the Tuple type. If it is too long it becomes "Tuple", otherwise it is of the form
 * "Tuple[T1,T2,T3]"
 */
 PyObject* tuple_type(PyObject* ob) {
    PyObject* type = PyUnicode_FromWideChar(L"Tuple", 5);
    PyObject* tuple = sequence_type(ob);
    PyObject* ret = PyUnicode_Concat(type, tuple);
    Py_DECREF(tuple);
    Py_DECREF(type);
    return ret;
}

PyObject* callable_type(PyObject* ob) {
    int hasname = PyObject_HasAttrString(ob, "__name__");
    if (hasname == 1) {
        PyObject* dundername = PyObject_GetAttrString(ob, "__name__");
        return PyUnicode_FromFormat("Callable[%U]", dundername);
    } else {
        return PyUnicode_FromWideChar(L"Callable", 8);
    }
}


/* Return the PEP 484 compatible type for known types otherwise, fall back to the type name.
 * Analysis of if it is a defined class or make it Any (hard to analyze) is done in the secondary analysis
 */
PyObject* get_type(PyObject* ob) {
    if (ob == Py_None) {
        return PyUnicode_FromWideChar(L"None", 4);
    } else if (PyBool_Check(ob)) {
        return PyUnicode_FromWideChar(L"bool", 4);
    } else if (PyLong_CheckExact(ob)) {
        return PyUnicode_FromWideChar(L"int", 3);
    } else if (PyUnicode_CheckExact(ob)) {
        return PyUnicode_FromWideChar(L"str", 3);
	} else if (PyList_CheckExact(ob)) {
		return list_type(ob);
	} else if (PyTuple_CheckExact(ob)) {
		return tuple_type(ob);
    } else if(PyFunction_Check(ob)) {
        return callable_type(ob);
    } else {
        return PyUnicode_FromString(ob->ob_type->tp_name);
    }
}

/* Serialize the type information to a JSON like format where types are rendered in a PEP 484 format
 * get_type does the brunt of the work, this function just makes each argument and return type look
 * pretty :)
 */
void analyze_types(PyObject* file, PyObject* frame_name, PyObject* args, int argc, PyObject* ret) {
    char* fname = PyUnicode_AsUTF8(frame_name);
    char* path = PyUnicode_AsUTF8(file);
    if (strcmp(fname, "<module>")) {
        printf("{\"frame\" : \"%s\", \"file\" : \"%s\", ", fname, path);
        if (argc != 0) {
            printf( "\"args\" : {");
        }
        for (int i = argc - 1; i >= 0; i--) {
            PyObject* arg = PySequence_GetItem(args, i);
            if (arg != NULL) {
                Py_INCREF(arg);
                PyObject* item = PySequence_GetItem(arg, 0);
                PyObject* ob = PySequence_GetItem(arg, 1);
                PyObject* type = get_type(ob);
                char* name = PyUnicode_AsUTF8(item);
                char* typ = PyUnicode_AsUTF8(type);
                if (i == 0) {
                    printf("\"%s\" : \"%s\"}, ", name, typ);

                } else {
                    printf("\"%s\" : \"%s\", ", name, typ);
                }
                Py_DECREF(arg);
                Py_DECREF(item);
                Py_DECREF(ob);
                Py_DECREF(type);
            } else {
            printf("<> : \"err\"");
            }
        }
        if (ret != NULL) {
            PyObject* type = get_type(ret);
            Py_DECREF(type);
            char* typ = PyUnicode_AsUTF8(type);
            printf("\"ret\" : \"%s\"}\n", typ);
        } else {
            printf("\"ret\" : \"None\"}\n");
        }
    }
}

#ifdef __cplusplus
}
#endif
