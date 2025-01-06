cmake_minimum_required(VERSION 3.30)
project(Engine LANGUAGES CXX)

# Build options
option(ENGINE_BUILD_TESTS "Build tests for Engine" OFF)

add_library(${ENGINE_LIB} SHARED)
target_compile_features(${ENGINE_LIB} PUBLIC cxx_std_20)

# header files
set(ENGINE_HEADERS
		include/engine/coreModule/Game.h
)

# source files
set(ENGINE_SOURCES
		src/coreModule/Game.cpp
)

target_sources(${ENGINE_LIB}
		PRIVATE
		${ENGINE_HEADERS}
		${ENGINE_SOURCES}
)

# dependencies
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(${ENGINE_LIB}
		PUBLIC ${PNG_INCLUDE_DIR}
		PUBLIC ${fmt_INCLUDE_DIR}
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		src
)

target_link_libraries(${ENGINE_LIB}
		PUBLIC
		$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>

		PUBLIC
		$<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>

		PUBLIC PNG::PNG
		PUBLIC glm::glm

		PUBLIC spdlog::spdlog_header_only
		PUBLIC fmt::fmt-header-only
)

#TODO create install script later


if (BUILD_DEMO)
	# Copying shared library to demo target directory
	add_custom_command(
			TARGET ${ENGINE_LIB} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:${ENGINE_LIB}>
			$<TARGET_FILE_DIR:demo>
			COMMENT "Copying shared library to demo target directory"
	)
endif ()