//
// Created by Vladimir Glushkov on 14.08.2026.
//

#include "RenderSystem.h"

#include "engine/ecsModule/components/SpriteComponent.h"
#include "engine/ecsModule/components/TransformComponent.h"
#include "engine/graphicsModule/RenderQueue.h"

pce::SystemPhase pce_demo::RenderSystem::GetPhase() const noexcept {
	return pce::SystemPhase::Render;
}

void pce_demo::RenderSystem::Update(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) {
	auto view = registry.View<pce::TransformComponent, pce::SpriteComponent>();
	view.Each([&renderQueue = m_renderQueue](pce::TransformComponent& transform, pce::SpriteComponent& sprite) {
		renderQueue.AddSpriteRegion(
			sprite.region,
			transform.position,
			{sprite.region.width, sprite.region.height},
			0xFFFFFFFF,
			transform.rotation,
			0);
	});

	//TODO: draw game objects
}
