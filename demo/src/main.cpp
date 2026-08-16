//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include <iostream>
#include <SDL2/SDL.h>

#include <engine/coreModule/Game.h>

#include "ecsModule/systems/MovementSystem.h"
#include "ecsModule/systems/RenderSystem.h"
#include "engine/ecsModule/components/SpriteComponent.h"
#include "engine/ecsModule/components/TransformComponent.h"
#include "engine/ecsModule/components/VelocityComponent.h"

int main(int argc, char* argv[]) {
	pce::Game game;
	if (!game.Initialize()) {
		return 0;
	}

	auto& registry = game.GetRegistry();
	pce::log("{}", pce::TransformComponent::GetTypeId());
	pce::log("{}", pce::VelocityComponent::GetTypeId());
	pce::log("{}", pce::SpriteComponent::GetTypeId());
	registry.RegisterComponent<pce::TransformComponent>();
	registry.RegisterComponent<pce::VelocityComponent>();
	registry.RegisterComponent<pce::SpriteComponent>();

	auto& systemManager = game.GetSystemManager();
	systemManager.EmplaceSystem<pce_demo::MovementSystem>();
	// todo add topological sorting for systems
	// until that need to create RenderSystem last

	systemManager.EmplaceSystem<pce_demo::RenderSystem>(game.GetRenderQueue());

	for (auto r = 0; r < 1'000; ++r) {
		for (auto c = 0; c < 1'000; ++c) {
			auto e = registry.CreateEntity();
			registry.EmplaceComponent<pce::TransformComponent>(e, glm::vec2{r, c} * 10.f, glm::vec2{1.f, 1.f}, 0.f);
			registry.EmplaceComponent<pce::VelocityComponent>(e, glm::vec2{0.f, 10.f}, glm::vec2{0.f, 0.f}, 100.f);
			registry.EmplaceComponent<pce::SpriteComponent>(e, 5, 5);
		}
	}

	game.Run();
	game.Destroy();

	return 0;
}
