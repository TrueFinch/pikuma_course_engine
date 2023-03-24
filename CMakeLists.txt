cmake_minimum_required(VERSION 3.23)
project(picuma_course_engine)

set(CMAKE_CXX_STANDARD 17)
set(DEMO_TARGET demo)
set(ENGINE_LIB engine)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fuse-ld=lld")

if (!BUILD_WIN)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wextra -pedantic")
endif()

add_subdirectory("engine" engine)

if (BUILD_DEMO)
	message("BUILD_DEMO=TRUE")
	add_subdirectory("demo")
	if (BUILD_WIN)
		message("Build win")
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${DEMO_TARGET})
	endif()
endif()
