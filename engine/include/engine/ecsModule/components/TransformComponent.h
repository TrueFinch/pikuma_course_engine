//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include "engine/ecsModule/ECS.h"
#include "engine/utilsModule/GlmFormat.h"
#include "glm/glm.hpp"

namespace pce {
	struct TransformComponent: virtual BaseComponent<TransformComponent> {
		static constexpr std::string_view COMPONENT_NAME = "TransformComponent";
		glm::vec2 position;
		glm::vec2 scale;
		float rotation;

		explicit TransformComponent() = default;

		explicit TransformComponent(glm::vec2&& aPosition, glm::vec2&& aScale, float aRotation)
			: position(aPosition), scale(aScale), rotation(aRotation) {}
	};
} // pce

template<>
struct fmt::formatter<pce::TransformComponent> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::TransformComponent& component, format_context& ctx) {
		return format_to(
			ctx.out(),
			"{}: {{position: {}, scale: {}, rotation: {}}}",
			component.GetName(),
			component.position,
			component.scale,
			component.rotation
		);
	}
};
