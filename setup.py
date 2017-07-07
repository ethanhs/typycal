from setuptools import setup
from setuptools.extension import Extension
import sys

if sys.version_info < (3, 6):
    print("This _must_ be used on Python 3.6+", file=sys.stderr)
    sys.exit(1)

if sys.platform != 'win32':
    cpp11 = ['--std=c++11']
else:
    cpp11 = []

ext = Extension(
    "typycal",
    ["typycal/typycal.cc"],
    language='c++',
    extra_compile_args=cpp11,
)


setup(
    name='typycal',
    version='0.0.1',
    author='Ethan Smith',
    author_email='ethan@ethanhs.me',
    packages=['typycal'],
    ext_modules=[ext],
    classifiers=[
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 3.6',
    ],
    keywords='typing type inference frame eval dynamic',
)
