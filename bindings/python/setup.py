# -*- coding: utf-8 -*-

from setuptools import setup, Extension

audaspace = Extension('aud',
                    include_dirs = ['@PYTHON_SOURCE_DIRECTORY@/../../include'],
                    libraries = ['audaspace'],
                    library_dirs = ['.'],
                    language = 'c++',
                    extra_compile_args = ['-std=c++11'],
                    sources = ['@PYTHON_SOURCE_DIRECTORY@/' + file for file in ['PyAPI.cpp', 'PyDevice.cpp', 'PyHandle.cpp', 'PySound.cpp', 'PySequenceEntry.cpp', 'PySequence.cpp']])

setup (name = 'audaspace',
       version = '0.1',
       description = 'Audaspace is a high level audio library.',
       author = 'Jörg Müller',
       author_email = 'nexyon@gmail.com',
       url = 'https://github.com/neXyon/audaspace',
       ext_modules = [audaspace],
       headers = ['@PYTHON_SOURCE_DIRECTORY@/' + file for file in ['PyAPI.h', 'PyDevice.h', 'PyHandle.h', 'PySound.h', 'PySequenceEntry.h', 'PySequence.h']])

