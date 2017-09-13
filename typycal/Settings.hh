#include "Python.h"
#include <string>

std::string lib_path;
Py_ssize_t libpath_size;

bool ends_with(std::string const & str, std::string const & end)
{
    if (end.size() > str.size()) return false;
    return std::equal(end.rbegin(), end.rend(), str.rbegin());
}

void init_log(PyObject* path) {
	Py_ssize_t len;
	const char* file = PyUnicode_AsUTF8AndSize(path, &len);
	if (len == 0) {
		output = std::ofstream("output.log");
	} 
	else {
		output = std::ofstream(file);
	}
}

void set_lib_path() {
	PyObject* sysconfig = PyImport_ImportModule("sysconfig");
	PyObject* getpath = PyObject_GetAttrString(sysconfig, "get_path");
	if (getpath != NULL) {
		if (PyCallable_Check(getpath)) {
			PyObject* path = PyObject_CallFunction(getpath, "s", "stdlib");
			lib_path = PyUnicode_AsUTF8AndSize(path, &libpath_size);
		}
	}
}

bool check_file(std::string const &path) {
	//std::cout << path << std::endl;
	return (path == "c:\\users\\ethanhs\\documents\\mypy\\mypy\\main.py"s) ? true : false;
	// ignore files that start with the lib prefix. Ignores a _lot_ of noise. We compare case insensitive on Windows,
	// as Windows is illogical.
#ifdef _WIN32
	if (path.compare(0, 11, "C:\\Python36") == 0) {
#else
	if (path.compare(0, libpath_size, lib_path) == 0) {
#endif // _WIN32
		return false;
	}
	return true;
}

bool check_name(std::string const &name) {
	if (name == "<listcomp>"s || name == "<genexpr>"s || name == "<setcomp>"s || name == "<dictcomp>"s) {
		return false;
	}
	else {
		return true;
	}
}

class TypeManager
{
		
};