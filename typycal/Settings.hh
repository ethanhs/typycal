#include "Python.h"
#include <string>
const char* lib_path;
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
	} else {
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

bool whitelisted(std::string path, std::string name) {
    if (name == "<module>"s) {
	    return false;
	} else if (name == "<dictcomp>"s || name == "<setcomp>"s || name == "<genexpr>"s || name == "<listcomp>"s) {
		return false;
	} else if (path == "<stdin>"s) {
		return true;
	} else if (!ends_with(path, ".py")) {
		return false;
	} else {
		// ignore files that start with the lib prefix. Ignores a _lot_ of noise. We compare case insensitive on Windows,
		// as Windows is illogical.
#ifdef _WIN32
		if (_strnicmp(lib_path, path.c_str(), libpath_size) == 0) {
#else
		if (path.compare(0, libpath_size, lib_path) == 0) {
#endif // _WIN32
			return false;
		}
		return true;
	}
}

