//
// Created by Vladimir Glushkov on 26.07.2026.
//

#include <catch2/catch_test_macros.hpp>
#include "engine/ecsModule/ECS.h"

TEST_CASE("Demo - Basic Sanity Check", "[demo]") {
	pce::Registry registry;
	auto entity = registry.CreateEntity();

	SECTION("Entity created via Registry") {
		REQUIRE(entity.GetIndex() == 0);
	}
}
