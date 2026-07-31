cmake_minimum_required(VERSION 3.30)

add_executable(demo_tests
	demo_tests.cpp
)

target_link_libraries(demo_tests
	PRIVATE
	${ENGINE_LIB}
	Catch2::Catch2WithMain
)

target_compile_features(demo_tests PRIVATE cxx_std_20)
target_compile_definitions(demo_tests PRIVATE PCE_TESTING)


include(Catch)
catch_discover_tests(demo_tests)

