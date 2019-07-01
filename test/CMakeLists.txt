configure_file(gtest.CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(
	COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
	RESULT_VARIABLE result)
if(result)
	message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(
	COMMAND "${CMAKE_COMMAND}" --build .
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
	RESULT_VARIABLE result)
if(result)
	message(FATAL_ERROR "CMake build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
	"${CMAKE_CURRENT_BINARY_DIR}/googletest-build")

add_executable(metric-test test_metric.cpp)
target_compile_features(metric-test PUBLIC cxx_std_11)
target_include_directories(metric-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(metric-test PRIVATE metric gtest gtest_main)

add_test(NAME metric-tests COMMAND metric-test)
