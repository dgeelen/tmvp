bin_PROGRAMS = mpngtocaas
mpngtocaas_SOURCES = mpngtocaas.cpp readpng.cpp aaify.cpp aadiff.cpp lzw.cpp \
				colorize.cpp palette.cpp octree.cpp mediancut.cpp imagizer.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
mpngtocaas_LDFLAGS = $(all_libraries) 
noinst_HEADERS = readpng.h mpngtocaas.h aaify.h aadiff.h lzw.h colorize.h \
			palette.h octree.h mediancut.h imagizer.h
