cmake_minimum_required(VERSION 3.30)
project(${DEMO_TARGET} LANGUAGES CXX)

find_package(SDL2 CONFIG REQUIRED)

# header files
set(DEMO_HEADERS
		src/ecsModule/systems/MovementSystem.h

)

# source files
set(DEMO_SOURCES
		src/main.cpp
		src/ecsModule/systems/MovementSystem.cpp
)

add_executable(${DEMO_TARGET} ${DEMO_HEADERS} ${DEMO_SOURCES})

set_target_properties(${DEMO_TARGET} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(${DEMO_TARGET} PRIVATE ${ENGINE_LIB} $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>)
target_compile_features(${DEMO_TARGET} PRIVATE cxx_std_20)

if (BUILD_DEMO_TESTS)
	add_subdirectory(tests)
endif ()
