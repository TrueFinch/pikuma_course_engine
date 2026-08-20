//
// Created by Vladimir Glushkov on 14.08.2026.
//

#pragma once

#include "engine/ecsModule/ECS.h"
#include "engine/graphicsModule/SpriteRegion.h"

namespace pce {
	struct SpriteComponent: virtual BaseComponent<SpriteComponent> {
		static constexpr std::string_view COMPONENT_NAME = "SpriteComponent";
		SpriteRegion region;

		explicit SpriteComponent() = default;

		explicit SpriteComponent(SpriteRegion region): region(region) {}
	};
} // namespace pce

template<>
struct fmt::formatter<pce::SpriteComponent> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::SpriteComponent& component, format_context& ctx) {
		return format_to(
			ctx.out(),
			"{}: {{region: {}}}",
			pce::SpriteComponent::GetName(),
			component.region
		);
	}
};
