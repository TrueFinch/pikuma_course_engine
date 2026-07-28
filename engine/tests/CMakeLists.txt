cmake_minimum_required(VERSION 3.30)

add_executable(engine_tests
		ecs_tests.cpp
)

target_link_libraries(engine_tests
		PRIVATE
		${ENGINE_LIB}
		Catch2::Catch2WithMain
)

target_compile_features(engine_tests PRIVATE cxx_std_20)

include(Catch)
catch_discover_tests(engine_tests)
