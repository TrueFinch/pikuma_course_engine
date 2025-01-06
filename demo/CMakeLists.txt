cmake_minimum_required(VERSION 3.30)
project(${DEMO_TARGET} LANGUAGES CXX)

# header files
set(DEMO_HEADERS

)

# source files
set(DEMO_SOURCES
		src/main.cpp
)

add_executable(${DEMO_TARGET} ${DEMO_HEADERS} ${DEMO_SOURCES})

set_target_properties(${DEMO_TARGET} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(${DEMO_TARGET} PRIVATE ${ENGINE_LIB})
target_compile_features(${DEMO_TARGET} PRIVATE cxx_std_20)
