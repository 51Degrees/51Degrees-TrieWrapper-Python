'''
51Degrees Mobile Detector (C Trie Wrapper)
==========================================

51Degrees Mobile Detector is a Python wrapper of the C trie-based mobile
detection solution by 51Degrees.mobi. Check out http://51degrees.mobi for
a detailed description, extra documentation and other useful information.

:copyright: (c) 2013 by 51Degrees.mobi, see README.rst for more details.
:license: MPL2, see LICENSE.txt for more details.
'''

from __future__ import absolute_import
import os
import subprocess
import shutil
import tempfile
from setuptools import setup, find_packages, Extension
from distutils import ccompiler


def has_snprintf():
    '''Checks C function snprintf() is available in the platform.

    '''
    cc = ccompiler.new_compiler()
    tmpdir = tempfile.mkdtemp(prefix='51degrees-mobile-detector-trie-wrapper-install-')
    try:
        try:
            source = os.path.join(tmpdir, 'snprintf.c')
            with open(source, 'w') as f:
                f.write(
                    '#include <stdio.h>\n'
                    'int main() {\n'
                    '  char buffer[8];\n'
                    '  snprintf(buffer, 8, "Hey!");\n'
                    '  return 0;\n'
                    '}')
            objects = cc.compile([source], output_dir=tmpdir)
            cc.link_executable(objects, os.path.join(tmpdir, 'a.out'))
        except:
            return False
        return True
    finally:
        shutil.rmtree(tmpdir)

define_macros = []
if has_snprintf():
    define_macros.append(('HAVE_SNPRINTF', None))

setup(
    name='51degrees-mobile-detector-trie-wrapper',
    version='1.0',
    author='51Degrees.mobi',
    author_email='info@51degrees.mobi',
    packages=find_packages(),
    include_package_data=True,
    ext_modules=[
        Extension('_fiftyone_degrees_mobile_detector_trie_wrapper',
            sources=[
                'wrapper.c',
                os.path.join('lib', '51Degrees.mobi.c'),
                os.path.join('lib', 'snprintf', 'snprintf.c'),
            ],
            define_macros=define_macros,
            extra_compile_args=[
                '-w',
            ],
        ),
    ],
    url='http://51degrees.mobi',
    description='51Degrees Mobile Detector (C Trie Wrapper).',
    long_description=__doc__,
    license='MPL2',
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries',
        'License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Operating System :: POSIX',
        'Operating System :: MacOS :: MacOS X',
    ],
    install_requires=[
        'distribute',
        '51degrees-mobile-detector',
    ],
)
