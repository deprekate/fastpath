import os
#from distutils.core import setup, Extension
from setuptools import setup, Extension

os.environ["CC"] = "gcc"
#compile_args = ["-v"]
#link_args    = [""]

fastpath_module = Extension('fastpath',
                    language='gcc',
                    #extra_compile_args=compile_args,
                    #extra_link_args=link_args,
                    include_dirs=[
                             '.',
                             '...',
                             os.path.join(os.getcwd(), 'include'),
                    ],
                    library_dirs = [os.getcwd(),],
                    sources = ['src/fastpath-py.c'])

with open("README.md", "r") as fh:
    long_desc = fh.read()


setup (
    name = 'fastpath',
    version = '0.4',
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
    ext_modules = [fastpath_module]
)
