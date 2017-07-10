#include "Python.h"
#include <string>
#include <iostream>
#include <set>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std::string_literals;

// we keep track of hashed signatures to not duplicate writing them
std::set<json> signatures;

std::ofstream output("output.log"s);

std::string get_type(PyObject* ob);



std::string sequence_type(PyObject* ob) {
    int len = PySequence_Length(ob);
    std::string s = "["s;
    for (int i = 0; i < len; i++) {
            s += get_type(PySequence_GetItem(ob, i));
            if (i != len - 1) {
                s += ", "s;
        }
    }
    s += "]"s;
    return s;
}

// PEP 484 repr of list
std::string list_type(PyObject* ob) {
    std::string s = "List"s;
    s += sequence_type(ob);
    return s;
}

// PEP 484 repr of tuple
std::string tuple_type(PyObject* ob) {
    std::string s = "Tuple"s;
    s += sequence_type(ob);
    return s;
}

// since we are at runtime, give name as arg, to be analyzed later.
std::string callable_type(PyObject* ob) {
    int hasname = PyObject_HasAttrString(ob, "__name__");
    if (hasname == 1) {
        std::string s = "Callable["s;
        PyObject* dundername = PyObject_GetAttrString(ob, "__name__");
        char* name = PyUnicode_AsUTF8(dundername);
        s += name;
        s += "]"s;
        return s;
    } else {
        return "Callable"s;
    }
}

/* Return the PEP 484 compatible type for known types otherwise, fall back to the type name.
 * Analysis of if it is a defined class or make it Any (hard to analyze) done in the secondary analysis
 */
std::string get_type(PyObject* ob) {
    PyTypeObject* type = ob->ob_type;
    const char* name = type->tp_name;
    if (strcmp(name, "NoneType") == 0) {
        return "None"s;
    } else if(strcmp(name, "int") == 0) {
        return "int"s;
	} else if (strcmp(name, "list") == 0) {
		return list_type(ob);
	} else if (strcmp(name, "tuple") == 0) {
		return tuple_type(ob);
    } else if(strcmp(name, "function") == 0 || strcmp(name, "builtin_function_or_method") == 0) {
        return callable_type(ob);
    } else {
        return std::string(name);
    }
}


void analyze_types(const char* file, const char* frame, PyObject* args, int argc, PyObject* ret) {
    json out;
    out["file"s] = std::string(file);
    out["name"s] = std::string(frame);
    for (int i = argc - 1; i >= 0; i--) {
        PyObject* arg = PySequence_GetItem(args, i);
        if (arg != NULL) {
            std::string name(PyUnicode_AsUTF8(PySequence_GetItem(arg, 0)));
            PyObject* ob = PySequence_GetItem(arg, 1);
            out["args"s][name] = get_type(ob);
        };
    }
    if (ret != NULL) {
		if (PyBool_Check(ret))
		{
			out["ret"s] = "bool"s;
		} else {
			out["ret"s] = get_type(ret);
		}
		if (signatures.find(out) == signatures.end()) {
			signatures.emplace(out);
			output << out.dump() << std::endl;
		}
	}
}
