//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include <bitset>
#include <vector>

#include "engine/utilsModule/Types.h"

namespace pce {
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


	namespace details {
		using ComponentTypeId = uint32;
		static inline ComponentTypeId ComponentLastId = 0;
	}

	template<typename Derived>
	class BaseComponent {
	protected:
		~BaseComponent() = default;

		static inline details::ComponentTypeId id = details::ComponentLastId++;

	public:
		using ComponentTypeId = std::size_t;

		[[nodiscard]] static ComponentTypeId GetTypeId() noexcept {
			return id;
		}

		[[nodiscard]] static constexpr std::string_view GetName() noexcept {
			return Derived::COMPONENT_NAME;
		}
	};

	class ISystem {
	public:
		virtual ~ISystem() = default;

		virtual void Update(float dt) = 0;

		virtual void FixedUpdate(float dt) = 0;
	};
}

template<>
struct std::hash<pce::Entity> {
	size_t operator()(const pce::Entity& entity) const noexcept;
};
