//
// Created by Vladimir Glushkov on 14.08.2026.
//

#pragma once

#include "engine/ecsModule/ECS.h"

namespace pce {
	struct SpriteComponent: virtual BaseComponent<SpriteComponent> {
		static constexpr std::string_view COMPONENT_NAME = "SpriteComponent";
		int width{};
		int height{};

		explicit SpriteComponent() = default;

		explicit SpriteComponent(int width, int height): width(width), height(height) {}
	};
} // pce

template<>
struct fmt::formatter<pce::SpriteComponent> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::SpriteComponent& component, format_context& ctx) {
		return format_to(
			ctx.out(),
			"{}: {{width: {}, height: {}}}",
			pce::SpriteComponent::GetName(),
			component.width,
			component.height
		);
	}
};
