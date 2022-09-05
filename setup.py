from distutils.core import setup, Extension

module = Extension(
    'cusmem',
    sources=['cusmemmodule.cpp']
)

setup(
    name='PackageName',
    version='1.0',
    description='This is a demo package',
    ext_modules=[module]
)
                    
