try:
	from setuptools import setup, Extension
except ImportError:
	from distutils.core import setup
	from distutils.extension import Extension
import os
import sysconfig

cmdclass = {}

try:
	from Cython.Build import build_ext
except ImportError: # without Cython
	module_src = 'ailist.c'
else: # with Cython
	module_src = 'ailist.pyx'
	cmdclass['build_ext'] = build_ext

import sys, platform

sys.path.append('python')

include_dirs = ["."]

setup(
	name = 'ailist',
	version = '0.1',
	url = 'https://github.com/databio/AIList',
	description = 'Augmented Interval List for genomic interval overlap',
	author = 'Jianglin Feng',
	author_email = 'jf.xtable@gmail.com',
	license = 'GPL2',
	keywords = 'interval',
    ext_modules = [Extension('ailist',
		sources = [module_src, 'AIList.c'],
		depends = ['AIList.h', 'khash.h', 'kseq.h', 'ailist.pyx'], 
		include_dirs = include_dirs)],
	classifiers = [
		'Development Status :: Beta',
		'License :: OSI Approved :: GPL2 License',
		'Operating System :: POSIX',
		'Programming Language :: C',
		'Programming Language :: Cython',
		'Programming Language :: Python :: 2.7',
		'Programming Language :: Python :: 3',
		'Intended Audience :: Science/Research',
		'Topic :: Scientific/Engineering :: Bio-Informatics'],
	cmdclass = cmdclass)
