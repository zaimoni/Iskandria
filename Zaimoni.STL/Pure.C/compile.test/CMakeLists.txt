if (NOT DEFINED $CACHE{ZSTL_HAVE_NANOSLEEP})
	try_compile(HAVE_NANOSLEEP ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Zaimoni.STL/Pure.C/compile.test/exists_nanosleep.c)
	set(ZSTL_HAVE_NANOSLEEP ${HAVE_NANOSLEEP} CACHE STRING "*NIX nanosleep() found")
endif()

if (NOT DEFINED $CACHE{ZSTL_HAVE_MS_IO_H})
	try_compile(HAVE_MS_IO_H ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Zaimoni.STL/Pure.C/compile.test/exists_MS_io_h.c)
	set(ZSTL_HAVE_MS_IO_H ${HAVE_MS_IO_H} CACHE STRING "MS #include <io.h> works")
endif()

if (NOT DEFINED $CACHE{ZSTL_HAVE_TIMESPEC})
	try_compile(HAVE_TIMESPEC ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Zaimoni.STL/Pure.C/compile.test/exists_timespec.c)
	set(ZSTL_HAVE_TIMESPEC ${HAVE_TIMESPEC} CACHE STRING "#include <time.h> provides timespec")
endif()

add_executable(exists_MS_io_h EXCLUDE_FROM_ALL exists_MS_io_h.c)
add_executable(exists_timespec EXCLUDE_FROM_ALL exists_timespec.c)
add_executable(exists_nanosleep EXCLUDE_FROM_ALL exists_nanosleep.c)
add_executable(realloc_0 EXCLUDE_FROM_ALL realloc_0.c)
