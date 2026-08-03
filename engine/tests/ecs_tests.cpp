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

		// Hash is deterministic for the same object
		REQUIRE(hasher(e1) == hasher(e1));

		// Different entities have different hashes with high probability
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

		// Loop of 100 re-creations on the same index
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

		// Destroy in order: e1, then e0
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

	SECTION("Unable to set/get signature of the dead entity") {
		// create entity and signature
		auto e1 = manager.CreateEntity();
		pce::details::Signature sig;
		// set signature to entity
		manager.SetSignature(e1, sig);
		// destroy entity and reset signature
		manager.DestroyEntity(e1);
		// SetSignature and GetSignature throw in test build (assert in debug)
		REQUIRE_THROWS_AS(manager.SetSignature(e1, sig), pce::AssertionException);
		REQUIRE_THROWS_AS(manager.GetSignature(e1), pce::AssertionException);

		const auto& managerConst = manager;
		REQUIRE_THROWS_AS(managerConst.GetSignature(e1), pce::AssertionException);
	}
}

struct TestComponent1 final: pce::BaseComponent<TestComponent1> {
	[[maybe_unused]] static constexpr std::string_view COMPONENT_NAME = "TestComponent1";
	uint32 uintData;

	explicit TestComponent1(uint32 uintData): uintData(uintData) {}
};

struct TestComponent2 final: pce::BaseComponent<TestComponent2> {
	[[maybe_unused]] static constexpr std::string_view COMPONENT_NAME = "TestComponent2";
	float floatData;

	explicit TestComponent2(float floatData): floatData(floatData) {}
};

TEST_CASE("Component Pool", "[Pool]") {
	SECTION("Distinct components have distinct ids and names") {
		// create test components
		auto c1 = TestComponent1(1);
		auto c2 = TestComponent2(2.f);

		// check static GetTypeId returns the same thing when called
		// through the scope resolution operator and through the instance class
		REQUIRE(c1.GetTypeId() == TestComponent1::GetTypeId());
		REQUIRE(c2.GetTypeId() == TestComponent2::GetTypeId());
		REQUIRE_FALSE(c1.GetTypeId() == c2.GetTypeId());

		// same for GetName method
		REQUIRE(c1.GetName() == TestComponent1::GetName());
		REQUIRE(c2.GetName() == TestComponent2::GetName());
		REQUIRE_FALSE(c1.GetName() == c2.GetName());
	}

	SECTION("Pool creation and population with components") {
		// create pools for test components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();

		// pools are empty after creation
		REQUIRE(pool->Empty());

		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();

		// created entities have not got a component yet
		REQUIRE_FALSE(pool->Has(e1));
		REQUIRE_FALSE(pool->Has(e2));

		// emplace component for e1 with value 1
		pool->Emplace(e1, 1);
		REQUIRE(pool->Has(e1));
		// only one component per type for each entity is allowed
		REQUIRE_THROWS_AS(pool->Emplace(e1, 1), pce::AssertionException);

		// add component for e2 with value 2
		pool->Add(e2, TestComponent1(2));
		REQUIRE(pool->Has(e2));
		REQUIRE_THROWS_AS(pool->Add(e2, TestComponent1(2)), pce::AssertionException);
	}

	SECTION("Get components from pool") {
		// prepare pool and components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		const pce::Pool<TestComponent1>* pool1Const = pool.get();
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();

		// try to get component for entity that is not presented in pool
		REQUIRE_THROWS_AS(pool->Get(e1), pce::AssertionException);
		REQUIRE_THROWS_AS(pool1Const->Get(e1), pce::AssertionException);

		pool->Emplace(e1, 1);
		pool->Add(e2, TestComponent1(2));

		// components for distinct entities have distinct values
		REQUIRE(pool->Get(e1).uintData != pool->Get(e2).uintData);
		// check const getter
		REQUIRE(pool1Const->Get(e1).uintData != pool1Const->Get(e2).uintData);

		// non-const and const 'Get' return same components
		REQUIRE(pool->Get(e1).uintData == pool1Const->Get(e1).uintData);
		REQUIRE(pool->Get(e2).uintData == pool1Const->Get(e2).uintData);
	}

	SECTION("Modification of components in pool") {
		// prepare pool and components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		pool->Emplace(e1, 1);
		pool->Add(e2, TestComponent1(2));

		// modify component value for e1
		pool->Get(e1).uintData = 2;
		// test components value now same
		REQUIRE(pool->Get(e1).uintData == pool->Get(e2).uintData);

		pool->Get(e2).uintData = 1;
		REQUIRE(pool->Get(e1).uintData != pool->Get(e2).uintData);
	}

	SECTION("Remove components from pool and check existence") {
		// prepare pool and components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		auto e3 = entityManager.CreateEntity();
		std::vector entities{e1, e2, e3};
		for (auto i = 0; i < entities.size(); ++i) {
			pool->Emplace(entities[i], i + 1);
		}

		// remove first created component
		pool->Remove(entities.front());
		// check no component for e1 after remove
		REQUIRE_FALSE(pool->Has(entities.front()));

		// removing e1 caused swapping e1 with e3
		std::vector expected{e3, e2};
		REQUIRE(pool->GetEntities() == expected);

		// destroy e1
		entityManager.DestroyEntity(e1);
		// reuse index of e1 (now e1 has increased generation)
		auto e4 = entityManager.CreateEntity();
		// emplace e4 in pool
		pool->Emplace(e4, 1);
		REQUIRE(pool->Has(e4));
		// e1 has same index as e4, but different generation
		REQUIRE_FALSE(pool->Has(e1));

		// remove last added component
		pool->Remove(e4);
		REQUIRE_FALSE(pool->Has(e4));

		// test double remove
		REQUIRE_THROWS_AS(pool->Remove(e4), pce::AssertionException);
	}

	SECTION("Replace component with 'Set' method") {
		// prepare pool and components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();

		// test Set is not allowed for entity without component
		REQUIRE_THROWS_AS(pool->Set(e1, TestComponent1(1)), pce::AssertionException);
		pool->Emplace(e1, 1);

		// replace with new component
		pool->Set(e1, TestComponent1(2));
		REQUIRE(pool->Get(e1).uintData == 2);
	}

	SECTION("Clear pool") {
		// prepare pool and components
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		auto entityManager = pce::EntityManager();
		std::vector entities{
			entityManager.CreateEntity(),
			entityManager.CreateEntity(),
			entityManager.CreateEntity()
		};
		for (auto i = 0; i < entities.size(); ++i) {
			pool->Emplace(entities[i], i + 1);
		}
		// test all entities have components in pool
		REQUIRE(pool->GetEntities() == entities);

		// clear pool
		pool->Clear();
		REQUIRE(pool->GetEntities().empty());
		REQUIRE(pool->Empty());
	}

	SECTION("Clear empty pool") {
		// prepare
		auto pool = pce::Pool<TestComponent1>();
		// clear
		pool.Clear();
		// check
		REQUIRE(pool.Empty());
	}
}

TEST_CASE("PoolManager") {
	SECTION("Register components") {
		// prepare
		auto poolManager = pce::PoolManager();
		// register component
		poolManager.RegisterComponent<TestComponent1>();
		// double registration is not allowed
		REQUIRE_THROWS_AS(poolManager.RegisterComponent<TestComponent1>(), pce::AssertionException);
		// check if component pool created
		REQUIRE(poolManager.HasComponent<TestComponent1>());
		// check if no pool for not registered component
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>());
		const auto& poolManagerConst = poolManager;
		// check with const manger
		REQUIRE(poolManagerConst.HasComponent<TestComponent1>());
		REQUIRE_FALSE(poolManagerConst.HasComponent<TestComponent2>());
	}

	SECTION("Register components without resize") {
		// prepare
		auto poolManager = pce::PoolManager();
		// do resize once registering component with greater id
		if (TestComponent1::GetTypeId() > TestComponent2::GetTypeId()) {
			poolManager.RegisterComponent<TestComponent1>();
			// check there is pool for TestComponent1
			REQUIRE(poolManager.HasComponent<TestComponent1>());
			// check there is NO pool for TestComponent2
			REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>());
			poolManager.RegisterComponent<TestComponent2>();
			// check pool for TestComponent2 created
			REQUIRE(poolManager.HasComponent<TestComponent2>());
		} else {
			poolManager.RegisterComponent<TestComponent2>();
			// check there is pool for TestComponent2
			REQUIRE(poolManager.HasComponent<TestComponent2>());
			// check there is NO pool for TestComponent1
			REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>());
			poolManager.RegisterComponent<TestComponent1>();
			// check pool for TestComponent1 created
			REQUIRE(poolManager.HasComponent<TestComponent1>());
		}
		REQUIRE(poolManager.HasComponent<TestComponent1>());
		REQUIRE(poolManager.HasComponent<TestComponent2>());
	}

	SECTION("Try emplace unregistered component") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		auto e1 = entityManager.CreateEntity();
		// try to add/emplace components without registration
		REQUIRE_THROWS_AS(poolManager.AddComponent(e1, TestComponent1(1)), pce::AssertionException);
		REQUIRE_THROWS_AS(poolManager.EmplaceComponent<TestComponent2>(e1, 2.f), pce::AssertionException);
	}

	SECTION("Create component in pool") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		const auto& poolManagerConst = poolManager;
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		// add components
		poolManager.AddComponent(e1, TestComponent1(1));
		poolManager.EmplaceComponent<TestComponent1>(e2, 2);
		// check that components are created
		REQUIRE(poolManager.HasComponent<TestComponent1>(e1));
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>(e2));
		REQUIRE(poolManagerConst.HasComponent<TestComponent1>(e1));
		REQUIRE_FALSE(poolManagerConst.HasComponent<TestComponent2>(e2));
		// double emplace/add is not allowed
		REQUIRE_THROWS_AS(poolManager.AddComponent(e1, TestComponent1(1)), pce::AssertionException);
		REQUIRE_THROWS_AS(poolManager.EmplaceComponent<TestComponent1>(e1, 2), pce::AssertionException);
	}

	SECTION("Create components of distinct types") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.RegisterComponent<TestComponent2>();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		// add components
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		poolManager.EmplaceComponent<TestComponent2>(e2, 2.f);
		// check e1 has TestComponent1 component
		REQUIRE(poolManager.HasComponent<TestComponent1>(e1));
		// check e1 doesn't have TestComponent2
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>(e1));
		// check e2 has TestComponent2 component
		REQUIRE(poolManager.HasComponent<TestComponent2>(e2));
		// check e2 doesn't have TestComponent1
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>(e2));
	}

	SECTION("Getting components") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.RegisterComponent<TestComponent2>();
		const auto& poolManagerConst = poolManager;
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		poolManager.EmplaceComponent<TestComponent2>(e2, 2.f);
		// check we can get components
		REQUIRE(poolManager.GetComponent<TestComponent1>(e1).uintData == 1);
		REQUIRE(poolManagerConst.GetComponent<TestComponent1>(e1).uintData == 1);
		REQUIRE_THROWS_AS(poolManager.GetComponent<TestComponent2>(e1), pce::AssertionException);
		REQUIRE_THROWS_AS(poolManagerConst.GetComponent<TestComponent2>(e1), pce::AssertionException);
		REQUIRE(poolManager.GetComponent<TestComponent2>(e2).floatData == 2.f);
		REQUIRE(poolManagerConst.GetComponent<TestComponent2>(e2).floatData == 2.f);
		REQUIRE_THROWS_AS(poolManager.GetComponent<TestComponent1>(e2), pce::AssertionException);
		REQUIRE_THROWS_AS(poolManagerConst.GetComponent<TestComponent1>(e2), pce::AssertionException);
	}

	SECTION("Removing components") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();
		// try to remove component of unregistered component type
		REQUIRE_THROWS_AS(poolManager.RemoveComponent<TestComponent1>(e1), pce::AssertionException);
		// register and emplace components
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.RegisterComponent<TestComponent2>();
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		poolManager.EmplaceComponent<TestComponent2>(e2, 2.f);
		// remove from e1 component of type TestComponent1
		poolManager.RemoveComponent<TestComponent1>(e1);
		// check if e1 has no TestComponent1 component
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>(e1));
		// check that TestComponent2 of e2 is not affected
		REQUIRE(poolManager.HasComponent<TestComponent2>(e2));
	}

	SECTION("Clear all components") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		auto e1 = entityManager.CreateEntity();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.RegisterComponent<TestComponent2>();
		auto sig = pce::details::Signature();
		sig.set(TestComponent1::GetTypeId());
		sig.set(TestComponent2::GetTypeId());
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		poolManager.EmplaceComponent<TestComponent2>(e1, 2.f);
		// clear components
		poolManager.ClearComponents(e1, sig);
		// check
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>(e1));
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>(e1));
	}

	SECTION("Clear one component") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		// clear component
		auto sig = pce::details::Signature();
		sig.set(TestComponent1::GetTypeId());
		poolManager.ClearComponents(e1, sig);
		// check
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>(e1));
	}

	SECTION("Clear unregistered component") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		// clear component
		auto sig = pce::details::Signature();
		sig.set(TestComponent1::GetTypeId());
		sig.set(TestComponent2::GetTypeId());
		// check
		REQUIRE_THROWS_AS(poolManager.ClearComponents(e1, sig), pce::AssertionException);
	}

	SECTION("Check clearing one component does not affect another") {
		// prepare
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<TestComponent1>();
		poolManager.RegisterComponent<TestComponent2>();
		poolManager.EmplaceComponent<TestComponent1>(e1, 1);
		poolManager.EmplaceComponent<TestComponent2>(e1, 2.f);
		// clear component TestComponent1
		auto sig = pce::details::Signature();
		sig.set(TestComponent1::GetTypeId());
		poolManager.ClearComponents(e1, sig);
		// check TestComponent1 was removed
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>(e1));
		// check TestComponent2 is not affected
		REQUIRE(poolManager.HasComponent<TestComponent2>(e1));
		// clear component TestComponent2
		sig.reset(TestComponent1::GetTypeId());
		sig.set(TestComponent2::GetTypeId());
		poolManager.ClearComponents(e1, sig);
		// check TestComponent2 was removed
		REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>(e1));
	}

	SECTION("Try remove component from unregistered pool") {
		auto entityManager = pce::EntityManager();
		auto e1 = entityManager.CreateEntity();
		auto poolManager = pce::PoolManager();
		// register the component with the greater id
		if (TestComponent1::GetTypeId() > TestComponent2::GetTypeId()) {
			poolManager.RegisterComponent<TestComponent1>();
			poolManager.EmplaceComponent<TestComponent1>(e1, 1);
			// check there is a pool for TestComponent1
			REQUIRE(poolManager.HasComponent<TestComponent1>());
			// check there is no pool for TestComponent2
			REQUIRE_FALSE(poolManager.HasComponent<TestComponent2>());
		} else {
			poolManager.RegisterComponent<TestComponent2>();
			poolManager.EmplaceComponent<TestComponent2>(e1, 2.f);
			// check there is a pool for TestComponent2
			REQUIRE(poolManager.HasComponent<TestComponent2>());
			// check there is no pool for TestComponent1
			REQUIRE_FALSE(poolManager.HasComponent<TestComponent1>());
		}
		// try to clear an unregistered component
		auto sig = pce::details::Signature();
		sig.set(TestComponent1::GetTypeId());
		sig.set(TestComponent2::GetTypeId());
		REQUIRE_THROWS_AS(poolManager.ClearComponents(e1, sig), pce::AssertionException);
	}
}
