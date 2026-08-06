//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include <bitset>
#include <format>

#include <functional>
#include <limits>
#include <memory>
#include <vector>
#include <fmt/base.h>

#include "engine/utilsModule/Assert.h"
#include "engine/utilsModule/Types.h"

namespace pce {
#pragma region Entity

	namespace details {
		constexpr uint32 MAX_COMPONENTS = 32;
		using Signature = std::bitset<MAX_COMPONENTS>;
	}

	class EntityManager;

	struct Entity {
		friend EntityManager;
		friend std::hash<Entity>;
		using ValueType = uint32;

	protected:
		ValueType m_value;

		static constexpr ValueType INDEX_BITS = 20;
		static constexpr ValueType GENERATION_BITS = 12;
		static constexpr ValueType INDEX_MASK = (1u << INDEX_BITS) - 1u;

	public:
		constexpr Entity(const Entity& other) noexcept = default;

		constexpr Entity& operator=(const Entity& other) noexcept = default;

		constexpr Entity(Entity&& other) noexcept = default;

		constexpr Entity& operator=(Entity&& other) noexcept = default;

		[[nodiscard]] constexpr ValueType GetIndex() const noexcept {
			return m_value & INDEX_MASK;
		}

		[[nodiscard]] constexpr ValueType GetGeneration() const noexcept {
			return m_value >> INDEX_BITS;
		}

		constexpr explicit operator ValueType() const noexcept = delete;

		constexpr auto operator<=>(const Entity&) const noexcept = default;

	private:
		static constexpr Entity MakeEntity(const ValueType index, const ValueType generation) noexcept {
			return Entity((index & INDEX_MASK) | (generation << INDEX_BITS));
		}

		constexpr Entity() noexcept = default;

		constexpr explicit Entity(const ValueType v) noexcept: m_value(v) {}
	};

	class EntityManager {
	public:
		Entity CreateEntity();

		void DestroyEntity(const Entity& entity);

		void SetSignature(const Entity& entity, const details::Signature& signature);

		[[nodiscard]] details::Signature& GetSignature(const Entity& entity);

		[[nodiscard]] bool IsAlive(const Entity& entity) const;

		[[nodiscard]] const details::Signature& GetSignature(const Entity& entity) const;

	private:
		std::vector<uint32> m_generations;
		std::vector<uint32> m_freeList;
		std::vector<details::Signature> m_entityComponentSignatures;
	};

#pragma endregion
#pragma region Component

	namespace details {
		using ComponentTypeId = uint32;
		static inline ComponentTypeId ComponentLastId = 0;
	}

	template<typename Derived>
	struct BaseComponent {
	protected:
		~BaseComponent() = default;

		static inline details::ComponentTypeId id = details::ComponentLastId++;

	public:
		[[nodiscard]] static details::ComponentTypeId GetTypeId() noexcept {
			return id;
		}

		[[nodiscard]] static constexpr std::string_view GetName() noexcept {
			return Derived::COMPONENT_NAME;
		}
	};

	class IPool {
	public:
		virtual ~IPool() = default;

		virtual void Remove(const Entity& entity) = 0;

		[[nodiscard]] virtual bool Has(const Entity& entity) const = 0;
	};

	template<typename TComponent>
		requires std::derived_from<TComponent, BaseComponent<TComponent>>
	class Pool: public IPool {
		static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

	public:
		Pool() = default;

		Pool(const Pool&) = delete;

		Pool(Pool&&) = delete;

		Pool& operator=(const Pool&) = delete;

		Pool& operator=(Pool&&) = delete;

		~Pool() override = default;

		[[nodiscard]] bool Empty() const {
			return m_components.empty();
		}

		void Clear() {
			m_components.clear();
			m_entities.clear();
			m_entityToIndex.clear();
		}

		void Add(const Entity& entity, TComponent&& component) {
			Emplace(entity, std::move(component));
		}

		template<typename... TArgs>
			requires std::is_constructible_v<TComponent, TArgs...>
		void Emplace(const Entity& entity, TArgs&&... args) {
			PCE_ASSERT(!Has(entity), "Component already exists on entity!");
			const auto entityIndex = entity.GetIndex();
			if (entityIndex >= m_entityToIndex.size()) {
				// resize m_entityToIndex to handle new entity
				m_entityToIndex.resize(entityIndex + 1, INVALID_INDEX);
			}
			const auto newIndex = m_components.size();
			m_entities.emplace_back(entity);
			m_components.emplace_back(std::forward<TArgs>(args)...);
			m_entityToIndex[entityIndex] = newIndex;
		}

		void Set(const Entity& entity, TComponent&& component) {
			PCE_ASSERT(Has(entity), "Component not found on entity!");
			const auto entityIndex = entity.GetIndex();
			const auto index = m_entityToIndex[entityIndex];
			m_components[index] = std::move(component);
		}

		void Remove(const Entity& entity) override {
			PCE_ASSERT(Has(entity), "Component not found on entity!");
			const auto entityIndex = entity.GetIndex();
			const auto index = m_entityToIndex[entityIndex];
			const auto lastIndex = m_components.size() - 1;
			if (index != lastIndex) {
				const auto lastEntity = m_entities[lastIndex];
				// swap removing entity with last to do pop back
				m_components[index] = std::move(m_components[lastIndex]);
				m_entities[index] = std::move(m_entities[lastIndex]);
				// update index for swaped entity
				m_entityToIndex[lastEntity.GetIndex()] = index;
			}
			m_entityToIndex[entityIndex] = INVALID_INDEX;
			m_entities.pop_back();
			m_components.pop_back();
		}

		[[nodiscard]] TComponent& Get(const Entity& entity) {
			PCE_ASSERT(Has(entity), "Component not found on entity!");
			return m_components[m_entityToIndex[entity.GetIndex()]];
		}

		[[nodiscard]] const TComponent& Get(const Entity& entity) const {
			PCE_ASSERT(Has(entity), "Component not found on entity!");
			return m_components[m_entityToIndex[entity.GetIndex()]];
		}

		[[nodiscard]] bool Has(const Entity& entity) const override {
			const auto entityIndex = entity.GetIndex();
			if (entityIndex >= m_entityToIndex.size()) {
				// no element in 'm_entityToIndex' for the given entity
				return false;
			}
			const auto index = m_entityToIndex[entityIndex];
			if (index == INVALID_INDEX) {
				// no component was saved for the given entity
				return false;
			}
			// check that both the generation and the id match
			return m_entities[index] == entity;
		}

		[[nodiscard]] const std::vector<Entity>& GetEntities() const {
			return m_entities;
		}

	private:
		// components
		std::vector<TComponent> m_components;
		// entities
		std::vector<Entity> m_entities;
		// entity index to m_data/m_dense index
		std::vector<size_t> m_entityToIndex;
	};

	class PoolManager final {
		friend class Registry;

	public:
		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		void RegisterComponent() {
			const auto componentId = TComponent::GetTypeId();
			if (componentId >= m_componentPools.size()) {
				m_componentPools.resize(componentId + 1);
			}
			PCE_ASSERT(!m_componentPools[componentId], "Component already registered!");
			m_componentPools[componentId] = std::make_unique<Pool<TComponent>>();
		}

		template<typename TComponent>
			requires std::derived_from<std::remove_cvref_t<TComponent>, BaseComponent<std::remove_cvref_t<TComponent>>>
		void AddComponent(const Entity& entity, TComponent&& component) {
			EmplaceComponent<TComponent>(entity, std::forward<TComponent>(component));
		}

		template<typename TComponent, typename... TArgs>
			requires std::is_constructible_v<TComponent, TArgs...>
					&& std::derived_from<TComponent, BaseComponent<TComponent>>
		void EmplaceComponent(const Entity& entity, TArgs&&... args) {
			auto pool = GetPool<TComponent>();
			PCE_ASSERT(pool, "Component pool is not registered!");
			pool->Emplace(entity, std::forward<TArgs>(args)...);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		void RemoveComponent(const Entity& entity) {
			auto pool = GetPool<TComponent>();
			PCE_ASSERT(pool, "Component pool is not registered!");
			pool->Remove(entity);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] bool HasComponent() const {
			return GetPool<TComponent>();
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] bool HasComponent(const Entity& entity) const {
			auto pool = GetPool<TComponent>();
			if (!pool) {
				return false;
			}
			return pool->Has(entity);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] TComponent& GetComponent(const Entity& entity) {
			auto pool = GetPool<TComponent>();
			PCE_ASSERT(pool, "Component pool is not registered!");
			return pool->Get(entity);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] const TComponent& GetComponent(const Entity& entity) const {
			auto pool = GetPool<TComponent>();
			PCE_ASSERT(pool, "Component pool is not registered!");
			return pool->Get(entity);
		}

		void ClearComponents(const Entity& entity, const details::Signature& signature) const;

	private:
		template<typename TComponent>
		Pool<TComponent>* GetPool() const {
			const auto componentId = TComponent::GetTypeId();
			if (componentId >= m_componentPools.size() || !m_componentPools[componentId]) {
				return nullptr;
			}
			return static_cast<Pool<TComponent>*>(m_componentPools[componentId].get());
		}

		[[nodiscard]] IPool* GetPool(details::ComponentTypeId componentId) const {
			if (componentId >= m_componentPools.size() || !m_componentPools[componentId]) {
				return nullptr;
			}
			return m_componentPools[componentId].get();
		}

		std::vector<std::unique_ptr<IPool>> m_componentPools;
	};

#pragma endregion
#pragma region System
	class Registry;
	class CommandBuffer;

	class ISystem {
	public:
		virtual ~ISystem() = default;

		virtual void Update(Registry& registry, CommandBuffer& commandBuffer, float dt) = 0;

		virtual void FixedUpdate(Registry& registry, CommandBuffer& commandBuffer, float dt) = 0;
	};

	class SystemManager final {
	public:
		template<typename TSystem, typename... TArgs>
			requires std::is_constructible_v<TSystem, TArgs...>
					&& std::derived_from<TSystem, ISystem>
		void EmplaceSystem(TArgs&&... args) {
			m_systems.emplace_back(std::make_unique<TSystem>(std::forward<TArgs>(args)...));
			EmplaceBuffer();
		}

		void Update(Registry& registry, float dt);

	private:
		void EmplaceBuffer();

		std::vector<std::unique_ptr<ISystem>> m_systems;
		std::vector<CommandBuffer> m_commandBuffers;
	};

	template<typename... TComponents>
	class EntityView final {
	public:
		explicit EntityView(Pool<TComponents>*... pools): m_pools(std::make_tuple(pools...)) {
			size_t minSize = std::numeric_limits<size_t>::max();
			auto inspectPool = [&minSize, this](auto* pool) {
				PCE_ASSERT(pool, "Component pool is not registered!");
				if (pool->GetEntities().size() < minSize) {
					minSize = pool->GetEntities().size();
					m_shortestEntities = &pool->GetEntities();
				}
			};
			(inspectPool(std::get<Pool<TComponents>*>(m_pools)), ...);
		}

		template<typename Func>
			requires std::is_invocable_v<Func, Entity, TComponents&...>
					|| std::is_invocable_v<Func, TComponents&...>
		void Each(Func&& func) {
			for (const Entity& entity: *m_shortestEntities) {
				if (!HasAllComponents(entity)) {
					continue;
				}
				if constexpr (std::is_invocable_v<Func, Entity, TComponents&...>) {
					func(entity, std::get<Pool<TComponents>*>(m_pools)->Get(entity)...);
				} else {
					func(std::get<Pool<TComponents>*>(m_pools)->Get(entity)...);
				}
			}
		}

	private:
		[[nodiscard]] bool HasAllComponents(const Entity& entity) const {
			auto hasComponent = [](auto* pool, const Entity& entity) {
				return pool->Has(entity);
			};
			return (hasComponent(std::get<Pool<TComponents>*>(m_pools), entity) && ...);
		}

		std::tuple<Pool<TComponents>*...> m_pools;
		const std::vector<Entity>* m_shortestEntities = nullptr;
	};

	class Registry final {
	public:
		~Registry() = default;

		Entity CreateEntity();

		[[nodiscard]] bool IsEntityAlive(const Entity& entity) const;

		void DestroyEntity(const Entity& entity);

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		void RegisterComponent() {
			PCE_ASSERT(!m_poolManager.HasComponent<TComponent>(), "Component already registered!");
			m_poolManager.RegisterComponent<TComponent>();
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] bool IsComponentRegistered() const {
			return m_poolManager.HasComponent<TComponent>();
		}

		template<typename TComponent>
			requires std::derived_from<std::remove_cvref_t<TComponent>, BaseComponent<std::remove_cvref_t<TComponent>>>
		void AddComponent(const Entity& entity, TComponent&& component) {
			EmplaceComponent<TComponent>(entity, std::forward<TComponent>(component));
		}

		template<typename TComponent, typename... TArgs>
			requires std::is_constructible_v<TComponent, TArgs...>
					&& std::derived_from<TComponent, BaseComponent<TComponent>>
		void EmplaceComponent(const Entity& entity, TArgs&&... args) {
			PCE_ASSERT(!HasComponent<TComponent>(entity), "Component already exists on entity!");
			m_poolManager.EmplaceComponent<TComponent>(entity, std::forward<TArgs>(args)...);
			const auto componentId = TComponent::GetTypeId();
			m_entityManager.GetSignature(entity).set(componentId);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		void RemoveComponent(const Entity& entity) {
			PCE_ASSERT(HasComponent<TComponent>(entity), "Component does not exist on entity!");
			m_poolManager.RemoveComponent<TComponent>(entity);
			const auto componentId = TComponent::GetTypeId();
			m_entityManager.GetSignature(entity).reset(componentId);
		}


		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		[[nodiscard]] bool HasComponent(const Entity& entity) const {
			return m_entityManager.IsAlive(entity)
					&& m_entityManager.GetSignature(entity).test(TComponent::GetTypeId())
					&& m_poolManager.HasComponent<TComponent>(entity);
		}


		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		TComponent& GetComponent(const Entity& entity) {
			PCE_ASSERT(m_entityManager.IsAlive(entity), "Entity is not alive!");
			return m_poolManager.GetComponent<TComponent>(entity);
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		const TComponent& GetComponent(const Entity& entity) const {
			PCE_ASSERT(m_entityManager.IsAlive(entity), "Entity is not alive!");
			return m_poolManager.GetComponent<TComponent>(entity);
		}

		template<typename... TComponents>
		[[nodiscard]] auto View() const {
			return EntityView<TComponents...>(m_poolManager.GetPool<TComponents>()...);
		}

	private:
		EntityManager m_entityManager;
		PoolManager m_poolManager;
	};

	class CommandBuffer final {
	public:
		CommandBuffer() = default;

		CommandBuffer(const CommandBuffer&) = delete;

		CommandBuffer& operator=(const CommandBuffer&) = delete;

		CommandBuffer(CommandBuffer&&) = default;

		CommandBuffer& operator=(CommandBuffer&&) = default;

		void CreateEntity(std::function<void(Entity, Registry&)>&& callback);

		template<typename TComponent, typename... TArgs>
			requires std::is_constructible_v<TComponent, TArgs...>
					&& std::derived_from<TComponent, BaseComponent<TComponent>>
		void EmplaceComponent(const Entity& entity, TArgs&&... args) {
			m_commands.emplace_back([entity, ...args = std::forward<TArgs>(args)](Registry& registry) {
				registry.EmplaceComponent<TComponent>(entity, std::forward<TArgs>(args)...);
			});
		}

		template<typename TComponent>
			requires std::derived_from<TComponent, BaseComponent<TComponent>>
		void RemoveComponent(const Entity& entity) {
			m_commands.emplace_back([entity](Registry& registry) {
				registry.RemoveComponent<TComponent>(entity);
			});
		}

		void DestroyEntity(const Entity& entity);

		void ProcessCommands(Registry& registry);

		[[nodiscard]] bool Empty() const noexcept;

	private:
		std::vector<std::function<void(Registry&)>> m_commands;
	};

#pragma endregion
}

template<>
struct std::hash<pce::Entity> {
	size_t operator()(const pce::Entity& entity) const noexcept;
};

template<>
struct fmt::formatter<pce::Entity> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::Entity& entity, format_context& ctx) {
		return format_to(
			ctx.out(),
			"Entity[Index: {}, Gen: {}]",
			entity.GetIndex(),
			entity.GetGeneration()
		);
	}
};
