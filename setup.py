from setuptools import setup
from setuptools.extension import Extension



setup(
    name='typycal',
    version='0.0.1',
    author='Ethan Smith',
    author_email='ethan@ethanhs.me',
    packages=['typycal'],
    ext_modules=[Extension("typycal", ["typycal/typycal.c"], )],
    classifiers=[
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Cython',
    ],
    keywords='typing type inference frame eval dynamic',
)
