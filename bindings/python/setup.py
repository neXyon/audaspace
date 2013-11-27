from distutils.core import setup, Extension

audaspace = Extension('aud',
                    include_dirs = ['../../include', '/usr/include/SDL', '/usr/include/jack'],
                    libraries = ['audaspace'],
                    library_dirs = ['../../../build'],
                    extra_compile_args = ['-std=c++11'],
                    sources = ['PyAPI.cpp'])

setup (name = 'audaspace',
       version = '0.1',
       description = 'Audaspace is a high level audio library.',
       author = 'Jörg Müller',
       author_email = 'nexyon@gmail.com',
       url = 'https://github.com/neXyon/audaspace',
       ext_modules = [audaspace])

