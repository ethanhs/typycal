# Typycal

Typycal is an experimental project to generate PEP 484 and 526 compliant stub files based on runtime types. The hope is to ease the
creation of stub files for projects and allow more projects to take advantage of typing in Python.

# Basic example

    >>> from _typycal import *
    >>> hook()
    True
    >>> def foo(a,b,c):
    ...     return "The answer to a + b + c is", a + b + c
    ...
    >>> foo(1,2,3)
    {"frame" : "foo", "file" : "<stdin>", "args" : {"a" : "int", "b" : "int", "c" : "int"}, "ret" : "Tuple[str, int]"}
    ('The answer to a + b + c is', 6)
    
# Building
    
    python setup.py build_ext
    cd build/lib...
    python
    ...

# Status
This project is an experiment in how fast and with what accuracy an interpreter hook can collect type information. I have the
interpeter hook running well, but there is no permanent logging, stub generation, or anything beyond an example.