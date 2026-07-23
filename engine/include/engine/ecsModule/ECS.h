//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include <bitset>

#include "engine/utilsModule/Types.h"

namespace pce {
	struct Entity {
		using ValueType = uint32;

		ValueType value;

		constexpr auto operator<=>(const Entity&) const noexcept = default;
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
