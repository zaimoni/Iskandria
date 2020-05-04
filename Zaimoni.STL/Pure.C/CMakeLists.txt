cmake_minimum_required(VERSION 3.15)

# generate headers
add_executable(int_probe int_probe.c)

add_custom_command(OUTPUT auto_int.h
	COMMAND int_probe > auto_int.h
	VERBATIM)

add_subdirectory(compile.test)
add_subdirectory(stdio.log)

if(${CMAKE_CURRENT_SOURCE_DIR}/comptest.in IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/comptest.h)
	configure_file(comptest.in comptest.h COPYONLY)
endif()

add_custom_command(OUTPUT comptest.h
	COMMAND realloc_0 >> comptest.h
	DEPENDS realloc_0
	VERBATIM)

if ($CACHE{ZSTL_HAVE_MS_IO_H})
add_custom_command(OUTPUT comptest.h
	COMMAND exists_MS_io_h >> comptest.h
	DEPENDS exists_MS_io_h
	VERBATIM APPEND)
endif()

if ($CACHE{ZSTL_HAVE_TIMESPEC})
add_custom_command(OUTPUT comptest.h
	COMMAND exists_timespec >> comptest.h
	DEPENDS exists_timespec
	VERBATIM APPEND)
endif()

if ($CACHE{ZSTL_HAVE_NANOSLEEP})
add_custom_command(OUTPUT comptest.h
	COMMAND exists_nanosleep >> comptest.h
	DEPENDS exists_nanosleep
	VERBATIM APPEND)
endif()

add_custom_target(AutoDetect ALL
                  DEPENDS auto_int.h
                  DEPENDS comptest.h
                  VERBATIM)

add_library(z_format_util STATIC format_util.c)
add_library(z_logging STATIC logging.c)
add_library(z_stdio_c STATIC stdio_c.c)

target_include_directories(z_stdio_c PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(z_stdio_c AutoDetect)
