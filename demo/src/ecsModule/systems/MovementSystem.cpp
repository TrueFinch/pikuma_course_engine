//
// Created by Vladimir Glushkov on 23.07.2026.
//

#include "MovementSystem.h"

#include "engine/ecsModule/components/TransformComponent.h"
#include "engine/ecsModule/components/VelocityComponent.h"
#include "engine/logModule/Log.h"

pce_demo::MovementSystem::MovementSystem() = default;

void pce_demo::MovementSystem::Update(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) {
	auto view = registry.View<pce::TransformComponent, pce::VelocityComponent>();
	view.Each([](const pce::Entity& entity, pce::TransformComponent& transform, pce::VelocityComponent&) {
		pce::log("{}\n\t{}", entity, transform);
	});
}

void pce_demo::MovementSystem::FixedUpdate(pce::Registry& registry, pce::CommandBuffer& commandBuffer, float dt) {}
