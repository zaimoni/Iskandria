cmake_minimum_required(VERSION 3.15)
project(Iskandria C CXX)
set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED $CACHE{ZSTL_CMAKE_SUFFIX})
	string(FIND ${CMAKE_INSTALL_PREFIX} "/" TMP_INDEX REVERSE)
	string(SUBSTRING ${CMAKE_INSTALL_PREFIX} ${TMP_INDEX} -1 TMP_CMAKE_SUFFIX)
	set(ZSTL_CMAKE_SUFFIX ${TMP_CMAKE_SUFFIX} CACHE STRING "platform suffix")
endif()

if (NOT DEFINED $CACHE{ZSTL_COMPILER})
	string(FIND ${CMAKE_C_COMPILER} "/" TMP_INDEX REVERSE)
	math(EXPR TMP_INDEX "${TMP_INDEX}+1")
	string(SUBSTRING ${CMAKE_C_COMPILER} ${TMP_INDEX} -1 TMP_COMPILER_STEM)
	string(FIND ${TMP_COMPILER_STEM} "." TMP_INDEX REVERSE)
	string(SUBSTRING ${TMP_COMPILER_STEM} 0 ${TMP_INDEX} TMP_COMPILER_STEM)
	set(ZSTL_COMPILER ${TMP_COMPILER_STEM} CACHE STRING "compiler")
endif()

string(APPEND SFML_DIR $CACHE{ZSTL_CMAKE_SUFFIX})

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)

add_subdirectory(Zaimoni.STL)

if ($CACHE{ZSTL_USLEEP_CODE})
	add_compile_definitions(ZSTL_USLEEP_CODE=$CACHE{ZSTL_USLEEP_CODE})
endif()


# might actually need this?
include_directories(${CMAKE_BINARY_DIR})

add_executable(Iskandria main.cpp test_drivers.cpp telephoto_grid.cpp gridtile.test.cpp text_menu.cpp cssbox.cpp display_manager.cpp voxelspace.cpp input_manager.cpp constants.cpp game_manager.cpp world_manager.cpp minkowski.cpp craft.cpp agent.cpp angle.cpp taylor.cpp)
target_compile_definitions(Iskandria PRIVATE ZAIMONI_ASSERT_STD_LOGIC)
target_link_libraries(Iskandria z_log_adapter z_logging z_stdio_c z_clock z_json sfml-graphics sfml-window sfml-system winmm)
add_dependencies(Iskandria AutoDetect)

# EXCLUDE_FROM_ALL means "can't test"
add_executable(angle.test angle.cpp taylor.cpp)
add_executable(cssbox.test cssbox.cpp)
add_executable(cssSFML.test css_SFML.cpp cssbox.cpp display_manager.cpp voxelspace.cpp)
add_executable(conic.test conic.cpp)
add_executable(constants constants.cpp)
add_executable(cyclic_fn.test cyclic_fn.cpp)
add_executable(dicounter.test dicounter.cpp)
add_executable(gridtile.test gridtile.test.cpp)
add_executable(heat_engine.test heat_engine.cpp mass.cpp constants.cpp)
add_executable(int_range.test int_range.cpp)
add_executable(interval_shim.test interval_shim.cpp)
add_executable(interval.test interval.test.cpp)
add_executable(kepler_orbit.test kepler_orbit.cpp angle.cpp taylor.cpp symbolic_fp.cpp power_fp.cpp arithmetic.cpp conic.cpp mass.cpp constants.cpp)
add_executable(lossy.test lossy.cpp)
add_executable(mass.test mass.cpp constants.cpp)
add_executable(matrix.test matrix.cpp)
add_executable(minkowski.test minkowski.cpp angle.cpp taylor.cpp)
add_executable(taylor.test taylor.cpp)

target_compile_definitions(angle.test PRIVATE TEST_APP2)
target_link_libraries(angle.test z_log_adapter)
add_dependencies(angle.test AutoDetect)

target_compile_definitions(cssbox.test PRIVATE TEST_APP2)
target_link_libraries(cssbox.test z_log_adapter)
add_dependencies(cssbox.test AutoDetect)

target_compile_definitions(cssSFML.test PRIVATE TEST_APP3)
target_link_libraries(cssSFML.test sfml-graphics sfml-window sfml-system z_log_adapter)
add_dependencies(cssSFML.test AutoDetect)

target_compile_definitions(conic.test PRIVATE TEST_APP2)
target_link_libraries(conic.test z_log_adapter)
add_dependencies(conic.test AutoDetect)

target_compile_definitions(constants PRIVATE TEST_APP)
target_link_libraries(constants z_log_adapter)
add_dependencies(constants AutoDetect)

target_link_libraries(cyclic_fn.test z_log_adapter)
add_dependencies(cyclic_fn.test AutoDetect)

target_link_libraries(dicounter.test z_log_adapter)

target_compile_definitions(gridtile.test PRIVATE TEST_APP)
target_link_libraries(gridtile.test z_log_adapter z_stdio_c z_json)
add_dependencies(gridtile.test AutoDetect)

target_compile_definitions(heat_engine.test PRIVATE TEST_APP3)
target_link_libraries(heat_engine.test z_log_adapter)
add_dependencies(heat_engine.test AutoDetect)

target_link_libraries(int_range.test z_log_adapter)
target_link_libraries(interval.test z_log_adapter)
add_dependencies(interval.test AutoDetect)

target_link_libraries(interval_shim.test z_log_adapter)
add_dependencies(interval_shim.test AutoDetect)

target_compile_definitions(kepler_orbit.test PRIVATE TEST_APP3)
target_link_libraries(kepler_orbit.test z_log_adapter)
add_dependencies(kepler_orbit.test AutoDetect)

target_link_libraries(lossy.test z_log_adapter)
add_dependencies(lossy.test AutoDetect)

target_compile_definitions(mass.test PRIVATE TEST_APP2)
target_link_libraries(mass.test z_log_adapter)
add_dependencies(mass.test AutoDetect)

target_link_libraries(matrix.test z_log_adapter)
add_dependencies(matrix.test AutoDetect)

target_compile_definitions(minkowski.test PRIVATE TEST_APP3)
target_link_libraries(minkowski.test z_log_adapter)
add_dependencies(minkowski.test AutoDetect)

target_compile_definitions(taylor.test PRIVATE TEST_APP)
target_link_libraries(taylor.test z_log_adapter)
add_dependencies(taylor.test AutoDetect)

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
add_test(NAME matrix COMMAND matrix.test)
add_test(NAME kepler_orbits COMMAND kepler_orbit.test)
