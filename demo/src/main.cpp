//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include <iostream>
#include <SDL2/SDL.h>

#include <engine/coreModule/Game.h>

#include "ecsModule/systems/MovementSystem.h"
#include "engine/ecsModule/components/TransformComponent.h"
#include "engine/ecsModule/components/VelocityComponent.h"

int main(int argc, char* argv[]) {
	pce::Game game;
	game.Initialize();

	auto& registry = game.GetRegistry();
	registry.RegisterComponent<pce::TransformComponent>();
	registry.RegisterComponent<pce::VelocityComponent>();

	auto& systemManager = game.GetSystemManager();
	systemManager.EmplaceSystem<pce_demo::MovementSystem>();

	for (auto i = 0; i < 100; ++i) {
		auto e = registry.CreateEntity();
		registry.EmplaceComponent<pce::TransformComponent>(e, glm::vec2{i * 10.f, 0.f}, glm::vec2{1.f, 1.f}, 0.f);
		registry.EmplaceComponent<pce::VelocityComponent>(e, glm::vec2{0.f, -1.f}, glm::vec2{0.f, 0.f}, 100.f);
	}

	game.Run();
	game.Destroy();

	return 0;
}
