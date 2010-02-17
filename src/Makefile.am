# set the include path found by configure
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libfileio.la
include_HEADERS = fileio.h
libfileio_la_SOURCES = fileio.cpp
libfileio_la_LDFLAGS = -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
