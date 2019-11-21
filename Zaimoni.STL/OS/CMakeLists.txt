cmake_minimum_required(VERSION 3.15)

add_library(z_log_adapter STATIC log_adapter_inc_inform.cpp log_adapter_inform.cpp log_adapter_log.cpp)

if (NOT DEFINED $CACHE{ZSTL_WINDOWS_H} OR $CACHE{ZSTL_WINDOWS_H})
	add_subdirectory(Win32)
endif()
