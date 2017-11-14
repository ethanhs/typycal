_typycal
========

_typycal is the C extension that hooks into the CPython interpreter and collects type information, then logs it for further analysis by the command line program (`typycal`).

The basic logic can be explained by this Python code as an analogue to the C code (the real code is in _typycal.c):

```python
def typycal_evalframe(frame: frameobject, exc: int):
    """
    frame is the current frame to be executed
    exc indicates whether an exception has been thrown calling the frame.
    """
    if exc:
        return _PyEval_EvalFrameDefault(frame, exc)  # execute the frame as normal, this function is part of Python's private C API
    else:
        code = frame.f_code  # this is the bytecode of the frame (stuff to be executed), and some other useful info
        name = code.co_name  # name of the current frame (the function name)
        file_name = code.co_filename  # file path of the currently executing function
        if whitelisted(file_name, name):  # ignore generators/comprehensions
            locals = frame.f_locals  # a dict of locals. names are keys, objects are values
            argc = code.co_argcount  # number of arguments passed to the function
            ret = _PyEval_EvalFrameDefault(frame, exc)  # run the frame, store the return value for analysis
            analyze_types(file_name, name, locals, argc, ret)
            return ret
        else:
            return _PyEval_EvalFrameDefault(frame, exc)  # don't want to analyze this frame, so execute it as normal

def hook():
    thread_state = thread_state_get() # Needed to tell Python to run our frame evaluation function, instead of the default
    thread_state.interp.eval_frame = typycal_evalframe  # assign our function to be called when a frame needs to be evaluated

def unhook():
    thread_state = thread_state_get()
    thread_state.interp.eval_frame = _PyEval_EvalFrameDefault(frame, exc)

```

`analyze_types` then gathers information about the frame and serializes the type information for later.