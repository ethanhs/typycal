from setuptools import setup
from Cython.Build import cythonize
from setuptools.extension import Extension



setup(
    ext_modules = cythonize("typycal/frame_evaluator.pyx")
)
