cmake_minimum_required(VERSION 3.15)

add_executable(int_probe EXCLUDE_FROM_ALL int_probe.c)

add_custom_command(OUTPUT auto_int.h
	COMMAND int_probe > auto_int.h
	VERBATIM)

add_subdirectory(compile.test)
add_subdirectory(stdio.log)

add_custom_command(OUTPUT comptest.h
	configure_file(comptest.in comptest.h COPYONLY)
	COMMAND exists_MS_io_h >> comptest.h
	COMMAND exists_timespec >> comptest.h
	COMMAND exists_nanosleep >> comptest.h
	COMMAND realloc_0 >> comptest.h
	VERBATIM)

add_custom_target(AutoDetect ALL
                  DEPENDS auto_int.h
                  DEPENDS comptest.h
                  VERBATIM)

add_library(z_format_util STATIC format_util.c)
add_library(z_logging STATIC logging.c)
add_library(z_stdio_c STATIC stdio_c.c)

target_include_directories(z_stdio_c PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(z_stdio_c AutoDetect)
