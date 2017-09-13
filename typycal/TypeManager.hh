#include "Python.h"
#include <string>
#include <set>
#include <iostream>
#include <fstream>

using namespace std::string_literals;

std::ofstream output;

std::string get_type(PyObject*  ob);

// PEP 484 generic sequence repr
std::string sequence_type(PyObject* ob) {
	Py_ssize_t len = PySequence_Fast_GET_SIZE(ob);
	if (len > 3) {
		return "[Any]"s;
	}
	else {
		// used to avoid infinite recursion in cyclical sequences
		if (Py_EnterRecursiveCall(" in sequence_type"))
			return "[Any]"s;
		std::string s = "["s;
		for (Py_ssize_t i = 0; i < len; i++) {
			PyObject* item = PySequence_Fast_GET_ITEM(ob, i);
			s += get_type(item);
			if (i != len - 1) {
				s += ", "s;
			}
		}
		s += "]"s;
		Py_LeaveRecursiveCall();
		return s;
	}
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
		const char* name = PyUnicode_AsUTF8(dundername);
		if (name != NULL) {
			s += std::string(name);
		}
		Py_DECREF(dundername);
		s += "]"s;
		return s;
	}
	else {
		return "Callable"s;
	}
}

/* Return the PEP 484 compatible type for known types otherwise, fall back to the type name.
* Analysis of if it is a defined class or make it Any (hard to analyze) done in the secondary analysis
*/
std::string get_type(PyObject* ob) {

	if (ob == Py_None) {
		return "None"s;
	}
	else if (PyList_CheckExact(ob)) {
		return list_type(ob);
	}
	else if (PyTuple_CheckExact(ob)) {
		return tuple_type(ob);
	}
	else if (PyLong_CheckExact(ob)) {
		return "int"s;
	}
	else if (PyFloat_CheckExact(ob)) {
		return "float"s;
	}
	else if (PyBool_Check(ob)) {
		return "bool"s;
	}
	else if (PyCallable_Check(ob)) {
		return callable_type(ob);
	}
	else {
		// return name of type. Many of these will be Any'd, but some are self parameters for methods.
		return ob->ob_type->tp_name;
	}
}


void analyze_types(const std::string &file, const std::string &frame, PyObject* args, int argc, PyObject* ret) {
	output << "{ file: \"" << file << "\", " << "name : \"" << frame << "\", ";
	for (int i = argc - 1; i >= 0; i--) {
		PyObject* arg = PySequence_Fast_GET_ITEM(args, i);
		if (arg != NULL) {
			PyObject* arg_name = PySequence_Fast_GET_ITEM(arg, 0);
			const char* name = PyUnicode_AsUTF8(arg_name);
			PyObject* ob = PySequence_Fast_GET_ITEM(arg, 1);
			output << "\"" << name << "\" : \"" << get_type(ob) << "\", ";
		};
	}
	if (ret != NULL) {
		output << "ret : \"" << get_type(ret) << "\"";
	}
	output << "}" << std::endl;
}