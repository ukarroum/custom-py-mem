from distutils.core import setup, Extension

module = Extension('memalloc', sources=['memallocmodule.c'])

setup(name='MemAlloc', ext_modules = [module])
