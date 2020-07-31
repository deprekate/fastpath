from distutils.core import setup, Extension
setup(name="FastpathZ",
	  version="1.0",
      ext_modules=[
         Extension("fastpath", ["fpz.c"]),
         ])
