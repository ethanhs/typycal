from __future__ import absolute_import
"""
This module and exported functions handle the main frame evaluation hooks and interpreter introspection.
"""

from cpython.mapping cimport *
from collections import deque
import sys

include_files = {'<stdin>': None}




cpdef unhook_frames():
    """
    Unhook our custom frame evaluator. Called before system exit
    """
    cdef PyThreadState *state = PyThreadState_Get()
    state.interp.eval_frame = _PyEval_EvalFrameDefault

cdef tuple resolve_name(object frame, int nargs, object args):
    """
    Get the fully resolved name, eg (file, class, function) or (file, function)
    """
    cdef str f_file = frame.f_code.co_filename
    cdef f_name = frame.f_code.co_name
    if nargs > 0:
        cls = args[0].__class__
        if cls and isinstance(cls, type) and hasattr(cls, f_name):
            f_cls = args[0].__class__.__name__
            return (f_file, f_cls, f_name)
    return (f_file, f_name)

cdef object getargs(object frame):
    """
    Get a list of arguments
    """
    nargs = frame.f_code.co_argcount
    locals = <dict> frame.f_locals
    if nargs > 0:
        out = None
        try:
            out = deque()
            for key in locals.keys():
                if nargs > 0:
                    out.appendleft(locals[key])
                else:
                    break
        except Exception as e:
            print("NumArgs: %s, Locals: %s, Error: %s" % (nargs, locals, e))
        return nargs, out
    else:
        return 0, None

cdef PyObject* frame_evaluator(PyFrameObject *c_frame, int exc):
    """
    The main hook into the interpreter, this captures the input and output types of the frame, and tells handlers to
    merge it into stubs.
    """
    # ignore frames that are in exception state
    if exc:
        unhook_frames()
        return _PyEval_EvalFrameDefault(c_frame, exc)
    frame = <object> c_frame
    cdef str f_name = frame.f_code.co_name
    cdef str f_file = frame.f_code.co_filename
    if not f_file in include_files or f_name == '<module>':
        return _PyEval_EvalFrameDefault(c_frame, exc)

    # the docs in CPython say this can be any mapping, but it should be a dict for our purposes.
    check = PyDict_Check(frame.f_locals)
    if not check:
        print('Failed to analyze {}'.format(f_name))
        return _PyEval_EvalFrameDefault(c_frame, exc)

    # if we don't need to bail out, continue with normal analysis

    (nargs, args) = getargs(frame)
    if nargs > 0:
        print("Frame args: " + ', '.join([str(arg) for arg in args]))
    else:
        print("No args to frame")
    resolved_name = resolve_name(frame, nargs, args)
    print("Frame path: " + '->'.join(resolved_name))

    # evaluate the frame. The last value on the stack should be the return value.
    ret = _PyEval_EvalFrameDefault(c_frame, exc)
    print("Returns: " + str(<object> c_frame.f_valuestack[0]))
    return ret


cpdef hook_frames(str path):
    cdef PyThreadState *state = PyThreadState_Get()
    state.interp.eval_frame = frame_evaluator