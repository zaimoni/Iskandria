cmake_minimum_required(VERSION 3.15)
project(Iskandria C CXX)
set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED $CACHE{ZSTL_CMAKE_SUFFIX})
	string(FIND ${CMAKE_INSTALL_PREFIX} "/" TMP_INDEX REVERSE)
	string(SUBSTRING ${CMAKE_INSTALL_PREFIX} ${TMP_INDEX} -1 TMP_CMAKE_SUFFIX)
	set(ZSTL_CMAKE_SUFFIX ${TMP_CMAKE_SUFFIX} CACHE STRING "platform suffix")
endif()

string(APPEND SFML_DIR $CACHE{ZSTL_CMAKE_SUFFIX})

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)

add_subdirectory(Zaimoni.STL)

add_executable(Iskandria main.cpp interval_shim.cpp text_menu.cpp cssbox.cpp display_manager.cpp input_manager.cpp constants.cpp game_manager.cpp world_manager.cpp minkowski.cpp craft.cpp agent.cpp)
target_link_libraries(Iskandria z_log_adapter z_logging z_format_util z_stdio_c z_stdio_log z_clock  sfml-graphics sfml-window sfml-system winmm)

add_executable(angle.test angle.cpp taylor.cpp interval_shim.cpp)
add_executable(cssbox.test cssbox.cpp)
add_executable(cssSFML.test css_SFML.cpp cssbox.cpp display_manager.cpp)
add_executable(conic.test conic.cpp interval_shim.cpp)
add_executable(constants constants.cpp interval_shim.cpp)
add_executable(cyclic_fn.test cyclic_fn.cpp)
add_executable(dicounter.test dicounter.cpp)
add_executable(gridtile.test gridtile.test.cpp)
add_executable(heat_engine.test heat_engine.cpp mass.cpp constants.cpp interval_shim.cpp)
add_executable(int_range.test int_range.cpp)
add_executable(interval.test interval.test.cpp)
add_executable(kepler_orbit.test EXCLUDE_FROM_ALL kepler_orbit.cpp arithmetic.cpp conic.cpp mass.cpp constants.cpp interval_shim.cpp)
add_executable(lossy.test lossy.cpp)
add_executable(mass.test mass.cpp constants.cpp interval_shim.cpp)
add_executable(matrix.test matrix.cpp)
add_executable(minkowski.test minkowski.cpp angle.cpp taylor.cpp interval_shim.cpp)
add_executable(taylor.test taylor.cpp)

target_compile_definitions(angle.test PRIVATE TEST_APP2)
target_link_libraries(angle.test z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(cssbox.test PRIVATE TEST_APP2)
target_link_libraries(cssbox.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(cssSFML.test PRIVATE TEST_APP3)
target_link_libraries(cssSFML.test sfml-graphics sfml-window sfml-system z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(conic.test PRIVATE TEST_APP2)
target_link_libraries(conic.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(constants PRIVATE TEST_APP)
target_link_libraries(constants z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_link_libraries(cyclic_fn.test z_log_adapter z_stdio_log z_format_util)
target_link_libraries(dicounter.test z_log_adapter z_stdio_log z_format_util)
target_link_libraries(gridtile.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(heat_engine.test PRIVATE TEST_APP3)
target_link_libraries(heat_engine.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_link_libraries(int_range.test z_log_adapter z_stdio_log z_format_util)
target_link_libraries(interval.test z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(kepler_orbit.test PRIVATE TEST_APP3)
target_link_libraries(kepler_orbit.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_link_libraries(lossy.test z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(mass.test PRIVATE TEST_APP2)
target_link_libraries(mass.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_link_libraries(matrix.test z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(minkowski.test PRIVATE TEST_APP3)
target_link_libraries(minkowski.test z_stdio_c z_log_adapter z_stdio_log z_format_util)

target_compile_definitions(taylor.test PRIVATE TEST_APP)
target_link_libraries(taylor.test z_log_adapter z_stdio_log z_format_util)

enable_testing()

add_test(NAME css_box COMMAND cssbox.test)
add_test(NAME cyclic_function COMMAND cyclic_fn.test)
add_test(NAME dicounter COMMAND dicounter.test)
add_test(NAME gridtiles COMMAND gridtile.test)
add_test(NAME integer_range COMMAND int_range.test)
add_test(NAME interval_arithmetic COMMAND interval.test)
add_test(NAME lossy_arithmetic COMMAND lossy.test)
# add_test(NAME matrix COMMAND matrix.test)
add_test(NAME taylor_series COMMAND taylor.test)

add_test(NAME angle COMMAND angle.test)
add_test(NAME conics COMMAND conic.test)
add_test(NAME fundamental_constants COMMAND constants)

add_test(NAME minkowski_space COMMAND minkowski.test)

add_test(NAME mass_equivalence COMMAND mass.test)

add_test(NAME heat_engines COMMAND heat_engine.test)
add_test(NAME matrix COMMAND matrix.test)	# hangs on MingW64
# add_test(NAME kepler_orbits COMMAND kepler_orbit.test)	# doesn't build on CLang
