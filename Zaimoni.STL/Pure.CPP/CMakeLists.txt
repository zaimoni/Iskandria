add_library(z_stacktrace STATIC stacktrace.cpp)
add_library(z_json STATIC json.cpp)

target_compile_definitions(z_stacktrace INTERFACE ZAIMONI_USING_STACKTRACE)
