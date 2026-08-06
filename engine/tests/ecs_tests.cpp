//
// Created by Vladimir on 26.07.2026.
//

#include <array>
#include <ranges>
#include <unordered_map>
#include <catch2/catch_test_macros.hpp>

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
		pce::Entity e1_moved = std::move(e1_copy); // NOLINT(*-move-const-arg)
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

	bool operator==(const TestComponent1& other) const {
		return uintData == other.uintData;
	}
};

struct TestComponent2 final: pce::BaseComponent<TestComponent2> {
	[[maybe_unused]] static constexpr std::string_view COMPONENT_NAME = "TestComponent2";
	float floatData;

	explicit TestComponent2(float floatData): floatData(floatData) {}
};

struct MoveOnlyComponent final: pce::BaseComponent<MoveOnlyComponent> {
	[[maybe_unused]] static constexpr std::string_view COMPONENT_NAME = "MoveOnlyComponent";

	std::unique_ptr<int> value;

	explicit MoveOnlyComponent(int v): value(std::make_unique<int>(v)) {}

	MoveOnlyComponent(const MoveOnlyComponent&) = delete;

	MoveOnlyComponent& operator=(const MoveOnlyComponent&) = delete;

	MoveOnlyComponent(MoveOnlyComponent&&) noexcept = default;

	MoveOnlyComponent& operator=(MoveOnlyComponent&&) noexcept = default;
};

static_assert(!std::is_copy_constructible_v<MoveOnlyComponent>);
static_assert(std::is_move_constructible_v<MoveOnlyComponent>);

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

	SECTION("Sparse entity indices") {
		auto pool = std::make_unique<pce::Pool<TestComponent1>>();
		auto entityManager = pce::EntityManager();
		constexpr int N = 1000;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (int i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
		}

		pool->Emplace(entities.front(), 0); // index 0
		pool->Emplace(entities[N / 2], 500); // index 500
		pool->Emplace(entities.back(), 999); // index 999

		// holes in 'm_entityToIndex' report no component
		REQUIRE_FALSE(pool->Has(entities[1]));
		REQUIRE_THROWS_AS(pool->Get(entities[1]), pce::AssertionException);

		REQUIRE(pool->GetEntities() == std::vector<pce::Entity>{entities[0], entities[500], entities[999]});

		// removing a middle sparse entry swaps the last one into the hole
		pool->Remove(entities[500]);
		REQUIRE_FALSE(pool->Has(entities[500]));
		REQUIRE(pool->Has(entities[999]));
		REQUIRE(pool->Get(entities[999]).uintData == 999);
		REQUIRE(pool->GetEntities() == std::vector<pce::Entity>{entities[0], entities[999]});
	}

	SECTION("Stress test: many move-only components with removals") {
		constexpr int N = 10'000;
		auto pool = std::make_unique<pce::Pool<MoveOnlyComponent>>();
		auto entityManager = pce::EntityManager();

		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (int i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool->Emplace(entities.back(), i);
		}

		// all components are present and hold correct data
		for (int i = 0; i < N; ++i) {
			REQUIRE(pool->Has(entities[i]));
			REQUIRE(*pool->Get(entities[i]).value == i);
		}

		// removing every second component drives the swap-with-last path
		for (int i = 0; i < N; i += 2) {
			pool->Remove(entities[i]);
		}

		// survivors keep intact data after many moves
		for (int i = 1; i < N; i += 2) {
			REQUIRE(pool->Has(entities[i]));
			REQUIRE(*pool->Get(entities[i]).value == i);
		}
		REQUIRE(pool->GetEntities().size() == N / 2);
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

	SECTION("Move-only components via PoolManager") {
		auto entityManager = pce::EntityManager();
		auto poolManager = pce::PoolManager();
		poolManager.RegisterComponent<MoveOnlyComponent>();
		auto e1 = entityManager.CreateEntity();
		auto e2 = entityManager.CreateEntity();

		poolManager.EmplaceComponent<MoveOnlyComponent>(e1, 10);
		poolManager.AddComponent(e2, MoveOnlyComponent(20));

		REQUIRE(poolManager.HasComponent<MoveOnlyComponent>(e1));
		REQUIRE(poolManager.HasComponent<MoveOnlyComponent>(e2));
		REQUIRE(*poolManager.GetComponent<MoveOnlyComponent>(e1).value == 10);
		REQUIRE(*poolManager.GetComponent<MoveOnlyComponent>(e2).value == 20);

		poolManager.RemoveComponent<MoveOnlyComponent>(e1);
		REQUIRE_FALSE(poolManager.HasComponent<MoveOnlyComponent>(e1));

		auto sig = pce::details::Signature{};
		sig.set(MoveOnlyComponent::GetTypeId());
		poolManager.ClearComponents(e2, sig);
		REQUIRE_FALSE(poolManager.HasComponent<MoveOnlyComponent>(e2));
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

TEST_CASE("Pool Stress", "[Pool][Stress]") {
	SECTION("Sparse indices with move-only components") {
		constexpr int N = 100'000;
		auto pool = std::make_unique<pce::Pool<MoveOnlyComponent>>();
		auto entityManager = pce::EntityManager();

		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (int i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
		}

		// component on every 100th index -> 'm_entityToIndex' resizes with holes
		for (int i = 0; i < N; i += 100) {
			pool->Emplace(entities[i], i);
		}

		// dense holes report no component
		REQUIRE_FALSE(pool->Has(entities[1]));
		REQUIRE(pool->GetEntities().size() == N / 100);

		// remove half of the sparse components
		for (int i = 0; i < N; i += 200) {
			pool->Remove(entities[i]);
		}

		// the other half survived with intact data
		for (int i = 100; i < N; i += 200) {
			REQUIRE(pool->Has(entities[i]));
			REQUIRE(*pool->Get(entities[i]).value == i);
		}
	}
}

TEST_CASE("EntityView", "[EntityView]") {
	SECTION("Empty view is not allowed") {
		pce::Pool<TestComponent1>* testComponent1Pool = nullptr;
		// pools not allowed to be null
		REQUIRE_THROWS_AS(pce::EntityView(testComponent1Pool), pce::AssertionException);
		pce::Pool<TestComponent2> testComponent2Pool;
		REQUIRE_THROWS_AS(pce::EntityView(&testComponent2Pool, testComponent1Pool), pce::AssertionException);
	}
	SECTION("Process only entities with the given component") {
		// prepare data
		pce::Pool<TestComponent1> pool;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			if (i % 2 == 0) {
				pool.Emplace(entities[i], i);
			}
		}
		// process only entities with TestComponent1
		auto view = pce::EntityView(&pool);
		std::array<bool, N> processedEntities{};
		processedEntities.fill(false);
		view.Each([&processedEntities](pce::Entity entity, TestComponent1& component) {
			processedEntities[entity.GetIndex()] = true;
			component.uintData += 1;
		});
		// entities with TestComponent1 are expected to be processed
		// and all uintData fields are expected to be increased
		for (auto i = 0; i < N; ++i) {
			if (i % 2 == 0) {
				REQUIRE(processedEntities[i] == true);
				REQUIRE(pool.Has(entities[i]));
				REQUIRE(pool.Get(entities[i]).uintData == i + 1);
			} else {
				REQUIRE_FALSE(processedEntities[i]);
			}
		}
	}
	SECTION("Process entities only with all components") {
		// prepare data
		pce::Pool<TestComponent1> pool1;
		pce::Pool<TestComponent2> pool2;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			if (i % 2 == 0 && i % 3 == 0) {
				pool1.Emplace(entities[i], i);
				pool2.Emplace(entities[i], static_cast<float>(i));
			} else if (i % 2 == 0) {
				pool1.Emplace(entities[i], i);
			} else if (i % 2 == 1) {
				pool2.Emplace(entities[i], static_cast<float>(i));
			}
		}
		// process only entities with TestComponent1
		auto view = pce::EntityView(&pool1, &pool2);
		std::array<bool, N> processedEntities{};
		processedEntities.fill(false);
		view.Each([&processedEntities](pce::Entity entity, TestComponent1& component1, TestComponent2& component2) {
			processedEntities[entity.GetIndex()] = true;
		});
		// entities with both TestComponent1 and TestComponent2 are expected to be processed
		for (auto i = 0; i < N; ++i) {
			if (i % 2 == 0 && i % 3 == 0) {
				REQUIRE(processedEntities[entities[i].GetIndex()]);
			} else {
				REQUIRE_FALSE(processedEntities[entities[i].GetIndex()]);
			}
		}
	}
	SECTION("The view is empty on disjoint pools") {
		// prepare data
		pce::Pool<TestComponent1> pool1;
		pce::Pool<TestComponent2> pool2;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			if (i % 2 == 0) {
				pool1.Emplace(entities[i], i);
			} else {
				pool2.Emplace(entities[i], static_cast<float>(i));
			}
		}
		// create view on disjoint pools
		auto view = pce::EntityView(&pool1, &pool2);
		std::vector<pce::Entity> processedEntities;
		view.Each([&processedEntities](pce::Entity entity, TestComponent1& component1, TestComponent2& component2) {
			processedEntities.push_back(entity);
		});
		// no entities have both TestComponent1 and TestComponent2
		REQUIRE(processedEntities.empty());
	}
	SECTION("Changing order of components can affect iteration order") {
		// prepare data
		pce::Pool<TestComponent1> pool1;
		pce::Pool<TestComponent2> pool2;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool1.Emplace(entities[i], i);
		}
		for (auto entity: entities | std::views::reverse) {
			pool2.Emplace(entity, static_cast<float>(entity.GetIndex()));
		}
		// create two view and reverse order of pools of components
		std::vector<pce::Entity> processedEntities;
		pce::EntityView(&pool1, &pool2).Each(
			[&processedEntities](pce::Entity entity, TestComponent1&, TestComponent2&) {
				processedEntities.push_back(entity);
			});
		std::vector<pce::Entity> processedEntitiesReverse;
		pce::EntityView(&pool2, &pool1).Each(
			[&processedEntitiesReverse](pce::Entity entity, TestComponent2&, TestComponent1&) {
				processedEntitiesReverse.push_back(entity);
			});
		// check that the iteration order is reversed
		for (auto i = 0; i < N; ++i) {
			REQUIRE(processedEntities[i] == processedEntitiesReverse[N - i - 1]);
		}
	}
	SECTION("Both signatures of each iterates in identical order") {
		// prepare data
		pce::Pool<TestComponent1> pool;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool.Emplace(entities[i], i);
		}
		// create view and iterate with different overloads of 'Each' method
		auto view = pce::EntityView(&pool);
		std::vector<TestComponent1> processedComponents1;
		view.Each([&processedComponents1](pce::Entity entity, const TestComponent1& component1) {
			processedComponents1.push_back(component1);
		});
		std::vector<TestComponent1> processedComponents2;
		view.Each([&processedComponents2](const TestComponent1& component1) {
			processedComponents2.push_back(component1);
		});
		// components must be same on each position
		REQUIRE(processedComponents1 == processedComponents2);
	}
	SECTION("Entity argument in each is same as in Pool's entities vector") {
		// prepare data
		pce::Pool<TestComponent1> pool;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool.Emplace(entities[i], i);
		}
		// create view and immediately call Each
		auto i = 0;
		pce::EntityView(&pool).Each([&i, &entities](pce::Entity entity, TestComponent1&) {
			// check that entities are same
			REQUIRE(entity == entities[i++]);
		});
	}
	SECTION("Components can be modified in EntityView::Each") {
		// prepare data
		pce::Pool<TestComponent1> pool;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool.Emplace(entities[i], i);
		}
		// create view and immediately call Each
		pce::EntityView(&pool).Each([](pce::Entity entity, TestComponent1& component) {
			component.uintData += 1;
		});
		// check that all uintData is increased
		for (auto i = 0; i < N; ++i) {
			REQUIRE(i + 1 == pool.Get(entities[i]).uintData);
		}
	}
	SECTION("EntityView reflects the current state of the pools") {
		// prepare data
		pce::Pool<TestComponent1> pool;
		auto entityManager = pce::EntityManager();
		constexpr int N = 100;
		std::vector<pce::Entity> entities;
		entities.reserve(N);
		for (auto i = 0; i < N; ++i) {
			entities.push_back(entityManager.CreateEntity());
			pool.Emplace(entities[i], i);
		}
		// create view
		int iterationCount1 = 0;
		auto view = pce::EntityView(&pool);
		view.Each([&iterationCount1](pce::Entity, TestComponent1&) {
			iterationCount1 += 1;
		});
		// check that all uintData is increased
		for (auto i = 0; i < N; ++i) {
			if (i % 2 == 0) {
				pool.Remove(entities[i]);
			}
		}
		int iterationCount2 = 0;
		view.Each([&iterationCount2](pce::Entity, TestComponent1&) {
			iterationCount2 += 1;
		});
		// half of components removed and iteration count halved
		REQUIRE(iterationCount1 / 2 == iterationCount2);
	}
}

TEST_CASE("Registry entity management", "[Registry][EntityManager]") {
	SECTION("Entity creation") {
		// prepare
		auto registry = pce::Registry();
		auto e1 = registry.CreateEntity();
		auto e2 = registry.CreateEntity();
		// check entities is alive
		REQUIRE(registry.IsEntityAlive(e1));
		REQUIRE(e1.GetIndex() == 0);
		REQUIRE(e1.GetGeneration() == 0);
		REQUIRE(registry.IsEntityAlive(e2));
		REQUIRE(e2.GetIndex() == 1);
		REQUIRE(e2.GetGeneration() == 0);
	}
	SECTION("Entity removing") {
		// prepare
		auto registry = pce::Registry();
		auto e1 = registry.CreateEntity();
		auto e2 = registry.CreateEntity();
		// destroy entity
		registry.DestroyEntity(e1);
		// check e1 is dead and e2 is alive
		REQUIRE_FALSE(registry.IsEntityAlive(e1));
		REQUIRE(registry.IsEntityAlive(e2));
	}
	SECTION("Double destroy is not allowed") {
		// prepare
		auto registry = pce::Registry();
		auto e1 = registry.CreateEntity();
		// destroy entity
		registry.DestroyEntity(e1);
		// double destroy is not allowed
		REQUIRE_THROWS_AS(registry.DestroyEntity(e1), pce::AssertionException);
	}
	SECTION("Entity reuse after destroy") {
		// prepare
		auto registry = pce::Registry();
		auto e1 = registry.CreateEntity();
		// destroy entity
		registry.DestroyEntity(e1);
		// reuse entity's index
		auto e2 = registry.CreateEntity();
		// check indexes of e1 and e2 is same
		REQUIRE(e1.GetIndex() == e2.GetIndex());
		// check generations of e1 and e2 is different
		REQUIRE(e1.GetGeneration() != e2.GetGeneration());
	}
}

TEST_CASE("Registry component registration", "[Registry][ComponentManager]") {
	SECTION("Components can be added after registration") {
		// prepare
		auto registry = pce::Registry();
		auto e1 = registry.CreateEntity();
		// register TestComponent1
		registry.RegisterComponent<TestComponent1>();
		REQUIRE(registry.IsComponentRegistered<TestComponent1>());
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// e1 entity has a component of type TestComponent1
		REQUIRE(registry.HasComponent<TestComponent1>(e1));
	}
	SECTION("Different components can be registered") {
		// prepare
		auto registry = pce::Registry();
		// no component type is registered yet
		REQUIRE_FALSE(registry.IsComponentRegistered<TestComponent1>());
		REQUIRE_FALSE(registry.IsComponentRegistered<TestComponent2>());
		registry.RegisterComponent<TestComponent1>();
		// registering one type does not register the other
		REQUIRE(registry.IsComponentRegistered<TestComponent1>());
		REQUIRE_FALSE(registry.IsComponentRegistered<TestComponent2>());
		registry.RegisterComponent<TestComponent2>();
		// check component types are registered
		REQUIRE(registry.IsComponentRegistered<TestComponent1>());
		REQUIRE(registry.IsComponentRegistered<TestComponent2>());
	}
	SECTION("Registering the same component type is not allowed") {
		// prepare
		auto registry = pce::Registry();
		registry.RegisterComponent<TestComponent1>();
		// try register second time
		REQUIRE_THROWS_AS(registry.RegisterComponent<TestComponent1>(), pce::AssertionException);
	}
}

TEST_CASE("Registry component addition", "[Registry][ComponentManager]") {
	auto registry = pce::Registry();
	registry.RegisterComponent<TestComponent1>();
	auto e1 = registry.CreateEntity();
	auto e2 = registry.CreateEntity();
	SECTION("EmplaceComponent/AddComponent create component") {
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.AddComponent(e2, TestComponent1(1));
		REQUIRE(registry.HasComponent<TestComponent1>(e1));
		REQUIRE(registry.HasComponent<TestComponent1>(e2));
	}
	SECTION("EmplaceComponent/AddComponent same component to entity is not allowed") {
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// try emplace second time
		REQUIRE_THROWS_AS(registry.EmplaceComponent<TestComponent1>(e1, 1), pce::AssertionException);
		registry.AddComponent(e2, TestComponent1(1));
		// try emplace second time
		REQUIRE_THROWS_AS(registry.AddComponent(e2, TestComponent1(1)), pce::AssertionException);
	}
	SECTION("Components can not be added before registering") {
		REQUIRE_THROWS_AS(registry.EmplaceComponent<TestComponent2>(e1, 2.f), pce::AssertionException);
	}
	SECTION("GetComponent gives access to created component") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.AddComponent(e2, TestComponent1(2));
		// test GetComponent
		REQUIRE(registry.GetComponent<TestComponent1>(e1).uintData == 1);
		const auto& registryConst = registry;
		REQUIRE(registryConst.GetComponent<TestComponent1>(e2).uintData == 2);
	}
	SECTION("Not allowed to GetComponent if component was not created") {
		// try to get component without adding it
		REQUIRE_THROWS_AS(registry.GetComponent<TestComponent1>(e1), pce::AssertionException);
	}
	SECTION("Component can be modified through GetComponent") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.GetComponent<TestComponent1>(e1).uintData = 2;
		// check component value was changed
		REQUIRE(registry.GetComponent<TestComponent1>(e1).uintData == 2);
	}
	SECTION("Adding component on a dead entity is not allowed") {
		// destroy entity e1
		registry.DestroyEntity(e1);
		// try emplace component on a dead entity
		REQUIRE_THROWS_AS(registry.EmplaceComponent<TestComponent1>(e1, 1), pce::AssertionException);
		// AddComponent routes through EmplaceComponent and must throw as well
		REQUIRE_THROWS_AS(registry.AddComponent(e1, TestComponent1(1)), pce::AssertionException);
		// failed emplace must not leave stale entries in the pool
		// reuse the index and ensure the fresh entity is fully usable
		auto reused = registry.CreateEntity();
		REQUIRE(reused.GetIndex() == e1.GetIndex());
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(reused));
		registry.EmplaceComponent<TestComponent1>(reused, 5);
		REQUIRE(registry.GetComponent<TestComponent1>(reused).uintData == 5);
		// the view must not visit any stale entity left by the failed emplace
		std::vector<pce::Entity> visited;
		registry.View<TestComponent1>().Each([&visited](const pce::Entity& entity, const TestComponent1&) {
			visited.push_back(entity);
		});
		REQUIRE(visited.size() == 1);
		REQUIRE(visited[0] == reused);
	}
	SECTION("Move-only component can be added") {
		// prepare
		registry.RegisterComponent<MoveOnlyComponent>();
		registry.EmplaceComponent<MoveOnlyComponent>(e1, 1);
		// check component was added with given value
		REQUIRE(registry.HasComponent<MoveOnlyComponent>(e1));
		REQUIRE(*registry.GetComponent<MoveOnlyComponent>(e1).value == 1);
	}
}

TEST_CASE("Registry component removal", "[Registry][ComponentManager]") {
	auto registry = pce::Registry();
	registry.RegisterComponent<TestComponent1>();
	auto e1 = registry.CreateEntity();
	auto e2 = registry.CreateEntity();
	SECTION("RemoveComponent removes the component") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// remove
		registry.RemoveComponent<TestComponent1>(e1);
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(e1));
	}
	SECTION("Component can be added again after removal") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// remove
		registry.RemoveComponent<TestComponent1>(e1);
		// emplace back again
		registry.EmplaceComponent<TestComponent1>(e1, 2);
		REQUIRE(registry.HasComponent<TestComponent1>(e1));
		REQUIRE(registry.GetComponent<TestComponent1>(e1).uintData == 2);
	}
	SECTION("Remove of unregistered component is not allowed") {
		REQUIRE_THROWS_AS(registry.RemoveComponent<TestComponent2>(e1), pce::AssertionException);
	}
	SECTION("Remove of a component of a dead entity is not allowed") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.DestroyEntity(e1);
		// try to remove component of a dead entity
		REQUIRE_THROWS_AS(registry.RemoveComponent<TestComponent1>(e1), pce::AssertionException);
	}
	SECTION("Remove of unexisting component is not allowed") {
		// emplace component for e1
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// try to remove component of e2
		REQUIRE_THROWS_AS(registry.RemoveComponent<TestComponent1>(e2), pce::AssertionException);
		// remove component of e1
		registry.RemoveComponent<TestComponent1>(e1);
		REQUIRE_THROWS_AS(registry.RemoveComponent<TestComponent1>(e1), pce::AssertionException);
	}
	SECTION("Remove of one component does not affect others") {
		// prepare
		registry.RegisterComponent<TestComponent2>();
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.EmplaceComponent<TestComponent2>(e1, 2.f);
		// remove TestComponent1
		registry.RemoveComponent<TestComponent1>(e1);
		// TestComponent1 is not exist and can not be accessed
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(e1));
		REQUIRE_THROWS_AS(registry.GetComponent<TestComponent1>(e1), pce::AssertionException);
		// TestComponent2 exists and have same value
		REQUIRE(registry.HasComponent<TestComponent2>(e1));
		REQUIRE(registry.GetComponent<TestComponent2>(e1).floatData == 2.f);
	}
}

TEST_CASE("Registry entity component management", "[Registry][ComponentManager][EntityManager]") {
	auto registry = pce::Registry();
	registry.RegisterComponent<TestComponent1>();
	registry.RegisterComponent<TestComponent2>();
	auto e1 = registry.CreateEntity();
	auto e2 = registry.CreateEntity();
	SECTION("DestroyEntity removes all components") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.EmplaceComponent<TestComponent2>(e1, 2.f);
		registry.DestroyEntity(e1);
		// check e1 components are not accessable
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(e1));
		REQUIRE_FALSE(registry.HasComponent<TestComponent2>(e1));
		REQUIRE_THROWS_AS(registry.GetComponent<TestComponent1>(e1), pce::AssertionException);
		REQUIRE_THROWS_AS(registry.GetComponent<TestComponent2>(e1), pce::AssertionException);
		const auto& registryConst = registry;
		REQUIRE_THROWS_AS(registryConst.GetComponent<TestComponent1>(e1), pce::AssertionException);
	}
	SECTION("DestroyEntity does not affect other entity's components") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.EmplaceComponent<TestComponent2>(e1, 1.f);
		registry.EmplaceComponent<TestComponent1>(e2, 2);
		registry.EmplaceComponent<TestComponent2>(e2, 2.f);
		registry.DestroyEntity(e1);
		// check e2 components are not affected
		REQUIRE(registry.HasComponent<TestComponent1>(e2));
		REQUIRE(registry.HasComponent<TestComponent2>(e2));
		REQUIRE(registry.GetComponent<TestComponent1>(e2).uintData == 2);
		REQUIRE(registry.GetComponent<TestComponent2>(e2).floatData == 2.f);
	}
	SECTION("Destroy entity with move only component") {
		// prepare
		registry.RegisterComponent<MoveOnlyComponent>();
		registry.EmplaceComponent<MoveOnlyComponent>(e1, 1);
		registry.EmplaceComponent<MoveOnlyComponent>(e2, 2);
		// destroy
		registry.DestroyEntity(e1);
		// check
		REQUIRE_FALSE(registry.HasComponent<MoveOnlyComponent>(e1));
		REQUIRE_THROWS_AS(registry.GetComponent<TestComponent1>(e1), pce::AssertionException);
		REQUIRE(registry.HasComponent<MoveOnlyComponent>(e2));
		REQUIRE(*registry.GetComponent<MoveOnlyComponent>(e2).value == 2);
	}
	SECTION("Reusing an entity creates a new entity with no leftover components") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// destroy entity
		registry.DestroyEntity(e1);
		// reuse index for new entity
		e2 = registry.CreateEntity();
		REQUIRE(e2.GetIndex() == e1.GetIndex());
		REQUIRE(e2.GetGeneration() != e1.GetGeneration());
		// old component is not exist
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(e1));
		registry.EmplaceComponent<TestComponent1>(e2, 2);
		REQUIRE(registry.GetComponent<TestComponent1>(e2).uintData == 2);
	}
}

TEST_CASE("Registry iteration with view", "[Registry][EntityView]") {
	auto registry = pce::Registry();
	registry.RegisterComponent<TestComponent1>();
	registry.RegisterComponent<TestComponent2>();
	auto e1 = registry.CreateEntity();
	auto e2 = registry.CreateEntity();
	auto e3 = registry.CreateEntity();
	SECTION("View iterates entities that have the component") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.EmplaceComponent<TestComponent1>(e3, 3);
		// iterate
		std::vector<pce::Entity> entities;
		registry.View<TestComponent1>().Each([&entities](const pce::Entity& entity, const TestComponent1&) {
			entities.push_back(entity);
		});
		// check
		REQUIRE(entities.size() == 2);
		REQUIRE((entities[0] == e1 && entities[1] == e3));
	}
	SECTION("View of multiple components requires entity to have all of them") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		registry.EmplaceComponent<TestComponent1>(e2, 2);
		registry.EmplaceComponent<TestComponent2>(e2, 2.f);
		registry.EmplaceComponent<TestComponent2>(e3, 3.f);
		// iterate
		std::vector<pce::Entity> entities;
		registry.View<TestComponent1, TestComponent2>().Each(
			[&entities](const pce::Entity& entity, const auto&, const auto&) {
				entities.push_back(entity);
			});
		// check
		REQUIRE(entities.size() == 1);
		REQUIRE(entities[0] == e2);
	}
	SECTION("Removal of the component removes its entity from view") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// iterate
		std::vector<pce::Entity> entities;
		registry.View<TestComponent1>().Each([&entities](const pce::Entity& entity, const TestComponent1&) {
			entities.push_back(entity);
		});
		// check iteration meets e1 entity
		REQUIRE(entities.size() == 1);
		REQUIRE(entities[0] == e1);
		// remove component
		registry.RemoveComponent<TestComponent1>(e1);
		// iterate
		entities.clear();
		registry.View<TestComponent1>().Each([&entities](const pce::Entity& entity, const TestComponent1&) {
			entities.push_back(entity);
		});
		// check no entities was met
		REQUIRE(entities.empty());
	}
	SECTION("Destroying the entity removes it from view") {
		// prepare
		registry.EmplaceComponent<TestComponent1>(e1, 1);
		// iterate
		std::vector<pce::Entity> entities;
		registry.View<TestComponent1>().Each([&entities](const pce::Entity& entity, const TestComponent1&) {
			entities.push_back(entity);
		});
		// check iteration meets e1 entity
		REQUIRE(entities.size() == 1);
		REQUIRE(entities[0] == e1);
		// remove component
		registry.DestroyEntity(e1);
		// iterate
		entities.clear();
		registry.View<TestComponent1>().Each([&entities](const pce::Entity& entity, const TestComponent1&) {
			entities.push_back(entity);
		});
		// check no entities was met
		REQUIRE(entities.empty());
	}
}

TEST_CASE("Registry Stress", "[Registry][Stress]") {
	// prepare
	auto registry = pce::Registry();
	registry.RegisterComponent<TestComponent1>();
	registry.RegisterComponent<TestComponent2>();
	registry.RegisterComponent<MoveOnlyComponent>();
	constexpr int N = 100'000;
	std::vector<pce::Entity> entities;
	entities.reserve(N);
	for (auto i = 0; i < N; i++) {
		auto entity = registry.CreateEntity();
		registry.EmplaceComponent<TestComponent1>(entity, i);
		registry.EmplaceComponent<TestComponent2>(entity, static_cast<float>(i));
		registry.EmplaceComponent<MoveOnlyComponent>(entity, i);
		entities.push_back(entity);
	}
	for (const auto& entity: entities) {
		REQUIRE(registry.GetComponent<TestComponent1>(entity).uintData == entity.GetIndex());
		REQUIRE(registry.GetComponent<TestComponent2>(entity).floatData == static_cast<float>(entity.GetIndex()));
		REQUIRE(*registry.GetComponent<MoveOnlyComponent>(entity).value == entity.GetIndex());
	}
	// remove TestComponent1 from odd entities
	for (const auto& entity: entities) {
		if (entity.GetIndex() % 2 == 1) {
			registry.RemoveComponent<TestComponent1>(entity);
		}
	}
	// component TestComponent1 removal does not affect other component
	for (const auto& entity: entities) {
		if (entity.GetIndex() % 2 == 0) {
			REQUIRE(registry.GetComponent<TestComponent1>(entity).uintData == entity.GetIndex());
		} else {
			REQUIRE_FALSE(registry.HasComponent<TestComponent1>(entity));
		}
		REQUIRE(registry.GetComponent<TestComponent2>(entity).floatData == static_cast<float>(entity.GetIndex()));
		REQUIRE(*registry.GetComponent<MoveOnlyComponent>(entity).value == entity.GetIndex());
	}
	// destroy even entities
	std::vector<pce::Entity> oddEntities;
	std::vector<pce::Entity> destroyedEntities;
	for (const auto& entity: entities) {
		if (entity.GetIndex() % 2 == 0) {
			registry.DestroyEntity(entity);
			destroyedEntities.push_back(entity);
		} else {
			oddEntities.push_back(entity);
		}
	}
	// check destroyed entities are dead
	for (const auto& entity: destroyedEntities) {
		REQUIRE_FALSE(registry.IsEntityAlive(entity));
	}
	// check even entities is not available, odd entities are not affected
	for (const auto& entity: entities) {
		if (entity.GetIndex() % 2 == 0) {
			REQUIRE_FALSE(registry.IsEntityAlive(entity));
			REQUIRE_THROWS_AS(registry.GetComponent<TestComponent1>(entity), pce::AssertionException);
		} else {
			REQUIRE(registry.GetComponent<TestComponent2>(entity).floatData == static_cast<float>(entity.GetIndex()));
			REQUIRE(*registry.GetComponent<MoveOnlyComponent>(entity).value == entity.GetIndex());
		}
	}
	entities = oddEntities;
	// populate with new entities reusing old indices
	for (auto i = N - 1; i >= 0; --i) {
		if (i % 2 == 1) {
			continue;
		}
		auto entity = registry.CreateEntity();
		// check entity was reused
		REQUIRE(entity.GetIndex() == i);
		REQUIRE(entity.GetGeneration() == 1);
		// new entity with reused index does not have components
		REQUIRE_FALSE(registry.HasComponent<TestComponent1>(entity));
		REQUIRE_FALSE(registry.HasComponent<TestComponent2>(entity));
		REQUIRE_FALSE(registry.HasComponent<MoveOnlyComponent>(entity));

		REQUIRE_NOTHROW(registry.EmplaceComponent<TestComponent1>(entity, i));
		REQUIRE_NOTHROW(registry.EmplaceComponent<TestComponent2>(entity, i));
		REQUIRE_NOTHROW(registry.EmplaceComponent<MoveOnlyComponent>(entity, i));
	}
	// check destroyed entities are still dead
	for (const auto& entity: destroyedEntities) {
		REQUIRE_FALSE(registry.IsEntityAlive(entity));
	}
}
