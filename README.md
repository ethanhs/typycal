# Typycal

Typycal is a project to generate PEP 484 and 526 compliant stub files based on runtime types. The hope is to ease the
creation of stub files for projects and allow more projects to take advantage of typing in Python.


# Basic example

    >>> import typycal
    >>> typycal.hook('')
    >>> def foo(a, b, c):
    ...     return a + b, "%s" % c
    ...
    >>> foo(1,2,3)
    <stdin>:foo:int int int  -> tuple
    (3, '3')
    >>>
    
# Building
    
    python setup.py build_ext
    cd build/lib...
    python
    ...

# Status
This project is in the planning stages. You can compile the typycal extension, and experiment with it,
but the generation of stubs is _not_ started. Also it is likely the method of resolving arguments and such is not ideal,
and the frame evaluation hook may cause Python to crash. The goal is to make the library as robust as possible.
