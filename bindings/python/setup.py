from distutils.core import setup, Extension

# python setup.py build
# LD_LIBRARY_PATH=../../../build python
# import sys
# sys.path.append('/data/Work/Computer/Programming/Audaspace/audaspace/bindings/python/build/lib.linux-x86_64-3.3')

audaspace = Extension('aud',
                    include_dirs = ['@PYTHON_SOURCE_DIRECTORY@/../../include'],
                    libraries = ['audaspace'],
                    library_dirs = ['.'],
                    extra_compile_args = ['-std=c++11'],
                    sources = ['@PYTHON_SOURCE_DIRECTORY@/' + file for file in ['PyAPI.cpp', 'PyDevice.cpp', 'PyHandle.cpp', 'PySound.cpp']])

setup (name = 'audaspace',
       version = '0.1',
       description = 'Audaspace is a high level audio library.',
       author = 'Jörg Müller',
       author_email = 'nexyon@gmail.com',
       url = 'https://github.com/neXyon/audaspace',
       ext_modules = [audaspace])

