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
		renderQueue.AddSprite(transform.position, {
								static_cast<float>(sprite.width),
								static_cast<float>(sprite.height)
							});
	});

	//TODO: draw game objects
}
