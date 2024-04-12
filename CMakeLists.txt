cmake_minimum_required (VERSION 3.8)

project(libfileio CXX)

option(FILEIO_ENABLE_POSIX "Enable build of posix file interface" ON)

set(libfileio_SOURCES src/fileio.cpp)
set(libfileio_HEADERS src/fileio.h)

if (FILEIO_ENABLE_POSIX)
	list(APPEND libfileio_SOURCES "src/posixfile.cpp")
	list(APPEND libfileio_HEADERS "src/posixfile.h")
endif()

if (WIN32)
	list(APPEND libfileio_SOURCES "src/location_win32.cpp")
else()
	list(APPEND libfileio_SOURCES "src/location_linux.cpp")
endif()

add_library(fileio STATIC ${libfileio_SOURCES})
add_library(fileio-static STATIC ${libfileio_SOURCES})

target_compile_features(fileio PRIVATE cxx_std_17)
target_compile_features(fileio-static PRIVATE cxx_std_17)
target_include_directories(fileio PUBLIC ${libfileio_SOURCE_DIR}/src)
target_include_directories(fileio-static PUBLIC ${libfileio_SOURCE_DIR}/src)

SET_TARGET_PROPERTIES(fileio-static PROPERTIES OUTPUT_NAME "fileio")
SET_TARGET_PROPERTIES(fileio PROPERTIES OUTPUT_NAME "fileio")
SET_TARGET_PROPERTIES(fileio-static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(fileio PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(fileio PROPERTIES PUBLIC_HEADER "${libfileio_HEADERS}")
SET_TARGET_PROPERTIES(fileio PROPERTIES IMPORTED_IMPLIB "libfileio.lib")

install( TARGETS fileio fileio-static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)

add_executable(fileio_tests tests/fileio_test.cpp)
target_link_libraries(fileio_tests fileio)
target_compile_features(fileio_tests PRIVATE cxx_std_17)

add_executable(fileio_location_tests tests/fileio_location_test.cpp)
target_link_libraries(fileio_location_tests fileio)
target_compile_features(fileio_location_tests PRIVATE cxx_std_17)
