//
// Created by Vladimir Glushkov on 14.08.2026.
//

#pragma once

#include "engine/ecsModule/ECS.h"

namespace pce {
	class RenderQueue;
}

namespace pce_demo {
	class RenderSystem final: public pce::ISystem {
	public:
		explicit RenderSystem(pce::RenderQueue& renderQueue): m_renderQueue(renderQueue) {}

		pce::SystemPhase GetPhase() const noexcept override;

		void Update(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) override;

	private:
		pce::RenderQueue& m_renderQueue;
	};
} // pce
