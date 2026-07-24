//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include <glm/vec2.hpp>

#include "engine/ecsModule/ECS.h"

namespace pce {
	struct VelocityComponent: virtual BaseComponent<VelocityComponent> {
		static constexpr std::string_view COMPONENT_NAME = "VelocityComponent";
		glm::vec2 velocity;
		glm::vec2 acceleration;
		float maxSpeed = 10.0f;

		explicit VelocityComponent() = default;

		explicit VelocityComponent(glm::vec2&& aVelocity, glm::vec2&& aAcceleration, float aMaxSpeed)
			: velocity(aVelocity), acceleration(aAcceleration), maxSpeed(aMaxSpeed) {}
	};
}
