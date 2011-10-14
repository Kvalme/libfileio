cmake_minimum_required (VERSION 2.6)

project(libfileio CXX)

include_directories(src)

set(libfileio_SOURCES src/fileio.cpp)
set(libfileio_HEADERS src/fileio.h)

add_library(fileio SHARED ${libfileio_SOURCES})
add_library(fileio-static STATIC ${libfileio_SOURCES})

SET_TARGET_PROPERTIES(fileio-static PROPERTIES OUTPUT_NAME "fileio")
SET_TARGET_PROPERTIES(fileio-static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(fileio PROPERTIES PUBLIC_HEADER ${libfileio_HEADERS})

install( TARGETS fileio fileio-static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)
