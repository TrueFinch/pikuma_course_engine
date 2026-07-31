//
// Created by Vladimir on 26.07.2026.
//

#include <catch2/catch_test_macros.hpp>
#include <unordered_map>
#include <unordered_set>

#include "engine/ecsModule/ECS.h"

TEST_CASE("Entity Basic Operations & Semantics", "[Entity]") {
	pce::EntityManager manager;

	SECTION("Creation and index/generation getters") {
		auto e1 = manager.CreateEntity();

		REQUIRE(e1.GetIndex() == 0);
		REQUIRE(e1.GetGeneration() == 0);
	}

	SECTION("Copy and Move semantics") {
		auto e1 = manager.CreateEntity();

		// Copy Construction
		pce::Entity e1_copy = e1;
		REQUIRE(e1_copy == e1);
		REQUIRE(manager.IsAlive(e1_copy));

		// Copy Assignment
		pce::Entity e1_assign = manager.CreateEntity();
		e1_assign = e1;
		REQUIRE(e1_assign == e1);

		// Move Construction
		pce::Entity e1_moved = std::move(e1_copy);
		REQUIRE(e1_moved == e1);
		REQUIRE(manager.IsAlive(e1_moved));
	}

	SECTION("Comparison operators") {
		auto e1 = manager.CreateEntity();
		auto e2 = manager.CreateEntity();
		auto e1_copy = e1;

		REQUIRE(e1 == e1_copy);
		REQUIRE(e1 != e2);
		REQUIRE(e1 < e2);
	}

	SECTION("std::hash consistency") {
		auto e1 = manager.CreateEntity();
		auto e2 = manager.CreateEntity();

		std::hash<pce::Entity> hasher;

		// Детерминированность хэша для одного и того же объекта
		REQUIRE(hasher(e1) == hasher(e1));

		// Разные сущности с высокой вероятностью имеют разный хэш
		REQUIRE(hasher(e1) != hasher(e2));
	}
}

TEST_CASE("EntityManager Lifetime & Reuse Order", "[EntityManager]") {
	pce::EntityManager manager;

	SECTION("Entity is not alive after destroying") {
		auto e = manager.CreateEntity();
		manager.DestroyEntity(e);
		REQUIRE_FALSE(manager.IsAlive(e));
	}

	SECTION("Multiple reuses and generation overflow checks") {
		auto e = manager.CreateEntity();
		const auto initialIndex = e.GetIndex();

		// Цикл из 100 пересозданий на одном и том же индексе
		for (int i = 0; i < 100; ++i) {
			REQUIRE(e.GetIndex() == initialIndex);
			REQUIRE(e.GetGeneration() == static_cast<pce::Entity::ValueType>(i));

			manager.DestroyEntity(e);
			REQUIRE_FALSE(manager.IsAlive(e));

			e = manager.CreateEntity();
		}

		REQUIRE(e.GetIndex() == initialIndex);
		REQUIRE(e.GetGeneration() == 100);
		REQUIRE(manager.IsAlive(e));
	}

	SECTION("LIFO (Stack-like) order of free list reuse") {
		auto e0 = manager.CreateEntity(); // Index 0
		auto e1 = manager.CreateEntity(); // Index 1
		auto e2 = manager.CreateEntity(); // Index 2

		// Destroy in order: e1, затем e0
		manager.DestroyEntity(e1); // m_freeList: [1]
		manager.DestroyEntity(e0); // m_freeList: [1, 0]

		// Because of 'pop_back()' first get from 'freeList' index e0 (Index 0), then e1 (Index 1)
		auto new0 = manager.CreateEntity();
		auto new1 = manager.CreateEntity();

		REQUIRE(new0.GetIndex() == 0);
		REQUIRE(new0.GetGeneration() == 1); // Generation increased

		REQUIRE(new1.GetIndex() == 1);
		REQUIRE(new1.GetGeneration() == 1); // Generation increased

		// Old entities is dead, new
		REQUIRE_FALSE(manager.IsAlive(e0));
		REQUIRE_FALSE(manager.IsAlive(e1));
		REQUIRE(manager.IsAlive(new0));
		REQUIRE(manager.IsAlive(new1));
		REQUIRE(manager.IsAlive(e2)); // e2 stays unused during test
	}

	SECTION("Double destroy entity throws AssertionException") {
		auto e1 = manager.CreateEntity();
		manager.DestroyEntity(e1);
		REQUIRE_THROWS_AS(manager.DestroyEntity(e1), pce::AssertionException);
		REQUIRE_FALSE(manager.IsAlive(e1));
	}
}

TEST_CASE("EntityManager Signatures", "[EntityManager][Signature]") {
	pce::EntityManager manager;

	SECTION("Signatures of different entities are strictly independent") {
		auto e1 = manager.CreateEntity();
		auto e2 = manager.CreateEntity();

		pce::details::Signature sig1;
		sig1.set(3);
		sig1.set(7);

		manager.SetSignature(e1, sig1);

		// Changing e1 should not affect e2
		REQUIRE(manager.GetSignature(e1) == sig1);
		REQUIRE(manager.GetSignature(e2).none());
	}

	SECTION("Modification through mutable reference from GetSignature()") {
		auto e = manager.CreateEntity();

		// Get non-const reference and modify signature
		pce::details::Signature& sigRef = manager.GetSignature(e);
		sigRef.set(12);

		const auto& managerConst = manager;
		REQUIRE(managerConst.GetSignature(e).test(12));
	}

	SECTION("Signature resets on entity destruction and allocation") {
		auto e = manager.CreateEntity();

		pce::details::Signature sig;
		// fill edge bits
		sig.set(0);
		sig.set(31);
		manager.SetSignature(e, sig);

		manager.DestroyEntity(e);

		// When re-allocating, the signature memory must be completely cleared
		auto reusedEntity = manager.CreateEntity();
		REQUIRE(reusedEntity.GetIndex() == e.GetIndex());
		REQUIRE(manager.GetSignature(reusedEntity).none());
	}
}
