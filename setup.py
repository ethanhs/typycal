from setuptools import setup
from setuptools.extension import Extension
import sys

if sys.version_info < (3, 6):
    print("This _must_ be used on Python 3.6+", file=sys.stderr)
    sys.exit(1)


ext = Extension(
    "typycal",
    ["typycal/typycal.cc"],
    language='c++',
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
        'Programming Language :: Python :: Implementation :: CPython',
    ],
    keywords='typing type inference frame eval dynamic',
)

