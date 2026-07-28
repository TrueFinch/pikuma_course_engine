cmake_minimum_required(VERSION 3.30)
project(gameEngine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_DEMO "Build the demo application" OFF)
option(BUILD_ENGINE_TESTS "Build tests for Engine" OFF)
option(BUILD_DEMO_TESTS "Build tests for Demo" OFF)

if (BUILD_ENGINE_TESTS OR BUILD_DEMO_TESTS)
	find_package(Catch2 CONFIG REQUIRED)
	enable_testing()
endif ()

set(ENGINE_LIB engine)

add_compile_definitions($<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)

add_subdirectory("engine" engine)

if (BUILD_DEMO)
	message("BUILD_DEMO=TRUE")
	set(DEMO_TARGET demo)
	add_subdirectory("demo")
endif ()
