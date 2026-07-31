//
// Created by Vladimir on 31.07.2026.
//

#include <catch2/catch_test_macros.hpp>

#include "engine/utilsModule/Assert.h"

TEST_CASE("PCE_ASSERT throws in test environment", "[Assert]") {
	SECTION("Failing assertion throws pce::AssertionException") {
		REQUIRE_THROWS_AS(PCE_ASSERT(false, "Test failure"), pce::AssertionException);
	}

	SECTION("Passing assertion does not throw") {
		REQUIRE_NOTHROW(PCE_ASSERT(true, "Test success"));
	}
}
