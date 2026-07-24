//
// Created by Vladimir Glushkov on 23.07.2026.
//

#pragma once

#include "engine/ecsModule/ECS.h"

namespace pce_demo {
	class MovementSystem: public pce::ISystem {
	public:
		MovementSystem();

		void Update(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) override;

		void FixedUpdate(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) override;
	};
}
