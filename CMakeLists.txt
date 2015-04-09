cmake_minimum_required (VERSION 2.6)

project(libfileio CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(src)

option(FILEIO_ENABLE_POSIX "Enable build of posix file interface" ON)

set(libfileio_SOURCES src/fileio.cpp)
set(libfileio_HEADERS src/fileio.h)

if (FILEIO_ENABLE_POSIX)
	list(APPEND libfileio_SOURCES "src/posixfile.cpp")
	list(APPEND libfileio_HEADERS "src/posixfile.h")
endif()

add_library(fileio SHARED ${libfileio_SOURCES})
add_library(fileio-static STATIC ${libfileio_SOURCES})

SET_TARGET_PROPERTIES(fileio-static PROPERTIES OUTPUT_NAME "fileio")
SET_TARGET_PROPERTIES(fileio-static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(fileio PROPERTIES PUBLIC_HEADER "${libfileio_HEADERS}")

install( TARGETS fileio fileio-static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)

add_executable(fileio_tests tests/fileio_test.cpp)
target_link_libraries(fileio_tests fileio)
