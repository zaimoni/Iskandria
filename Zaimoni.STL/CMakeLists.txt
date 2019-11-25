cmake_minimum_required(VERSION 3.15)

add_subdirectory(Pure.C)
add_subdirectory(OS)

# Iskandria uses C stdio logging
target_link_libraries(z_log_adapter INTERFACE z_format_util INTERFACE z_stdio_log)