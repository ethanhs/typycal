#include "Python.h"
#include <fstream>
#include <iostream>
#include <set>

std::set<std::string> files;

bool whitelisted(const char* file) {
	std::string f(file);
	std::cout << f << std::endl;
    if (files.find(f) != files.end() || strcmp(file, "<stdin>") == 0) {
        return true;
    } else {
		std::cout << "Skipped" << std::endl;
        return false;
    }
}

void print_files() {
	for (auto & file : files) {
		std::cout << file << std::endl;
	}
}
