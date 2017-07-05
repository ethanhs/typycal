from cpython.dict cimport *

cdef extern from "Python.h":
    ctypedef struct PyObject
    ctypedef struct PyCodeObject:
        int co_argcount  # number of arguments
        int co_kwonlyargcount  # number of kwargs
        int co_nlocals  # number of locals
        int co_stacksize  # size of stack for frame
        int co_flags  # flags for code (generator, etc)
        int co_firstlineno  # source line number
        PyObject* co_code  # bytes of code
        PyObject* co_consts  # constants in the code
        PyObject* co_names
        PyObject* co_varnames
        PyObject* co_freevars
        PyObject* co_cellvars
        unsigned char* co_cell2arg
        PyObject* co_filename  # filename of source
        PyObject* co_name  # frame name
        PyObject* co_lnotab
        void* co_zombieframe
        PyObject* co_weakreflist
        void* co_extra  # magic playground for fun

cdef extern from "frameobject.h":
    ctypedef struct PyDictObject
    ctypedef struct PyFrameObject:
        PyFrameObject* f_back  # parent frame
        PyCodeObject* f_code  # code object
        PyDictObject* f_builtins  # frame builtins symbol table
        PyDictObject* f_globals   # frame globals symbol table
        PyDictObject* f_locals  # frame locals symbol table
        PyObject** f_valuestack  # stack of values top is last value
        PyObject** f_stacktop
        PyObject* f_trace
        PyObject* f_exc_type
        PyObject* f_exc_value
        PyObject* f_exc_traceback
        PyObject* f_gen

        int f_lasti  # last instruction (bytecode) called
        int f_lineno  # frame line number
        int f_iblock
        char f_executing
        PyObject* f_localsplus[1]

cdef extern from "code.h":
    ctypedef void freefunc(void* )
    int _PyCode_GetExtra(PyObject* code, Py_ssize_t index, void* *extra)
    int _PyCode_SetExtra(PyObject* code, Py_ssize_t index, void* extra)

cdef extern from "pystate.h":
    ctypedef PyObject* _PyFrameEvalFunction(PyFrameObject* frame, int exc)

    ctypedef struct PyInterpreterState:
        PyInterpreterState* next
        PyInterpreterState* tstate_head

        PyObject* modules

        PyObject* modules_by_index
        PyObject* sysdict
        PyObject* builtins
        PyObject* importlib

        PyObject* codec_search_path
        PyObject* codec_search_cache
        PyObject* codec_error_registry
        int codecs_initialized
        int fscodec_initialized

        int dlopenflags

        PyObject* builtins_copy
        PyObject* import_func
        # Initialized to PyEval_EvalFrameDefault().
        _PyFrameEvalFunction eval_frame

    ctypedef struct PyThreadState:
        PyThreadState* prev
        PyThreadState* next
        PyInterpreterState* interp
        int use_tracing
        PyObject *exc_type
        PyObject *exc_value
        PyObject *exc_traceback

    PyThreadState* PyThreadState_Get()

cdef extern from "ceval.h":
    int _PyEval_RequestCodeExtraIndex(freefunc)
    PyObject* _PyEval_EvalFrameDefault(PyFrameObject* frame, int exc)