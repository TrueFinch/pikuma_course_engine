//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include <engine/coreModule/Game.h>

#include "ecsModule/systems/MovementSystem.h"
#include "ecsModule/systems/RenderSystem.h"
#include "engine/assetsModule/AssetsManager.h"
#include "engine/ecsModule/components/SpriteComponent.h"
#include "engine/ecsModule/components/TransformComponent.h"
#include "engine/ecsModule/components/VelocityComponent.h"

int main(int argc, char* argv[]) {
	pce::Game game;
	if (!game.Initialize()) {
		return 1;
	}

	auto& registry = game.GetRegistry();
	registry.RegisterComponent<pce::TransformComponent>();
	registry.RegisterComponent<pce::VelocityComponent>();
	registry.RegisterComponent<pce::SpriteComponent>();

	auto& systemManager = game.GetSystemManager();
	systemManager.EmplaceSystem<pce_demo::MovementSystem>();
	systemManager.EmplaceSystem<pce_demo::RenderSystem>(game.GetRenderQueue());

	auto& assetsManager = game.GetAssetsManager();

	const auto texture = assetsManager.LoadTexture("assets/images/landing-base.png");
	const auto spriteRegion = assetsManager.GetSpriteRegion(texture);
	PCE_ASSERT(spriteRegion.has_value(), "Sprite region is nullopt!");
	for (auto r = 0; r < 1'00; ++r) {
		for (auto c = 0; c < 1'00; ++c) {
			auto e = registry.CreateEntity();
			registry.EmplaceComponent<pce::TransformComponent>(
				e, glm::vec2{r * spriteRegion->height, c * spriteRegion->width}, glm::vec2{1.f, 1.f}, 0.f);
			registry.EmplaceComponent<pce::VelocityComponent>(e, glm::vec2{0.f, 10.f}, glm::vec2{0.f, 0.f}, 100.f);
			registry.EmplaceComponent<pce::SpriteComponent>(e, spriteRegion.value());
		}
	}

	game.Run();
	game.Destroy();

	return 0;
}
