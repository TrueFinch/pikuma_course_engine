cmake_minimum_required(VERSION 3.30)
project(gameEngine)

set(CMAKE_CXX_STANDARD 20)

set(ENGINE_LIB engine)

add_subdirectory("engine" engine)

if (BUILD_DEMO)
	message("BUILD_DEMO=TRUE")
	set(DEMO_TARGET demo)
	add_subdirectory("demo")
endif ()
