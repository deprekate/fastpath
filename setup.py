import os
#from distutils.core import setup, Extension
from setuptools import setup, Extension

os.environ["CC"] = "gcc"
compile_args = ["-Wno-unused-variable"]
#link_args	= [""]

def extension(package_name):
	if package_name == 'fastpath':
		sources = ['src/fastpath-py.c']
	else:
		sources = ['src/fastpathz-py.c', 'src/mini-gmp.c']
	ext = Extension(package_name,
				language='gcc',
				extra_compile_args=compile_args,
				#extra_link_args=link_args,
				include_dirs=[
						 '.',
						 '...',
						 os.path.join(os.getcwd(), 'include'),
				],
				library_dirs = [os.getcwd(),],
				sources = sources)
	return ext


with open("README.md", "r") as fh:
	long_desc = fh.read()

def get_version():
	with open("VERSION", 'r') as f:
		v = f.readline().strip()
		return v

#for package_name in ['fastpath', 'fastpathz']:
setup (
	name = 'fastpath',
	version = get_version(),
	author = "Katelyn McNair",
	author_email = "deprekate@gmail.com",
	description = 'A package for finding the best path through a network graph',
	long_description = long_desc,
	long_description_content_type="text/markdown",
	url =  "https://github.com/deprekate/fastpath",
	classifiers=[
		"Programming Language :: Python :: 3",
		"License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
		"Operating System :: OS Independent",
	],
	python_requires='>3.5.2',
	ext_modules = [extension('fastpath'), extension('fastpathz')]
)
