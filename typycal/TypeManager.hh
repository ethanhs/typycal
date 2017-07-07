#include "Python.h"
#include <string>
#include <map>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

void analyze_types(PyObject* args, int argc, PyObject* ret) {

    for (int i = argc - 1; i >= 0; i--) {
        PyObject* arg = PySequence_GetItem(args, i);
        if (arg != NULL) {
            PyObject* name = PySequence_GetItem(arg, 0);
            PyTypeObject* type = PySequence_GetItem(arg, 1)->ob_type;
        }
    }

}
