//
// Created by Vladimir on 05.01.2023.
//
#include <filesystem>
#include <iostream>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>


#include "Game.h"
#include "logModule/LogManager.h"
#include "ecsModule/ECS.h"
#include "ecsModule/components/TransformComponent.h"
#include "ecsModule/components/RigidbodyComponent.h"
#include "ecsModule/systems/MovementSystem.h"
#include "ecsModule/systems/RenderSystem.h"

using namespace pce;

//TODO move to settings file
int width = 1280, height = 800;

Game::Game() : m_window(nullptr, SDL_DestroyWindow), m_renderer(nullptr, SDL_DestroyRenderer) {
}

void Game::Initialize() {
	//init systems //TODO move systems creation to some system's controller
	logModule::LogManagerInstance::Init(logModule::LogManager::Create());
	ecsModule::RegistryInstance::Init(ecsModule::Registry::Create());

	pce::log("Start game initializing");
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		pce::logError("[Game::Initialize] ERROR: failed to initialize sdl!");
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);


	m_window.reset(SDL_CreateWindow(
			nullptr,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_RESIZABLE
	));
	if (!m_window) {
		pce::logError("[Game::Initialize] ERROR: failed to create sdl window!");
		return;
	}

	m_renderer.reset(SDL_CreateRenderer(
			m_window.get(), -1, 0
	));
	if (!m_renderer) {
		pce::logError("[Game::Initialize] ERROR: failed to create sdl renderer!");
		return;
	}

//	SDL_SetWindowFullscreen(
//		m_window.get(),
//		SDL_WINDOW_FULLSCREEN
//	);

	Setup();
	m_isRunning = true;
}

void Game::Run() {
	while (m_isRunning) {
		ProcessInput();
		Delay();
		Update();
		Render();
	}
}

void Game::Setup() {
	// create ECS systems
	auto& registry = ecsModule::RegistryInstance::GetInstance();

	registry.AddSystem<ecsModule::systems::MovementSystem>();
	registry.AddSystem<ecsModule::systems::RenderSystem>();
	registry.GetSystem<ecsModule::systems::RenderSystem>().lock()->SetRenderer(m_renderer.get());

	auto tank = registry.CreateEntity();
	tank.AddComponent<ecsModule::components::TransformComponent>(glm::vec2(10.f, 10.f), glm::vec2(1.f, 1.f), 0.f);
	tank.AddComponent<ecsModule::components::RigidbodyComponent>(glm::vec2(50.f, 10.f));
}

void Game::ProcessInput() {
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
			case SDL_QUIT: {
				m_isRunning = false;
				break;
			}
			case SDL_KEYDOWN: {
				if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
					m_isRunning = false;
				}
				break;
			}
		}
	}
}

void Game::Delay() const {
	if (!m_capFPS) {
		return;
	}
	if (const uint32 ticks = SDL_GetTicks(); m_milliPerFrame + m_prevFrameMillis > ticks) {
		SDL_Delay(m_milliPerFrame + m_prevFrameMillis - ticks);
	}
}

void Game::Update() {
	m_deltaTime = static_cast<float>(SDL_GetTicks() - m_prevFrameMillis) / 1000.f;
	m_prevFrameMillis = SDL_GetTicks();

	auto& registry = ecsModule::RegistryInstance::GetInstance();

	registry.Update(m_deltaTime);

	// TODO: add other systems here
	// TODO: need something like system update order that can be changeable without re-compile
	registry.GetSystem<ecsModule::systems::MovementSystem>().lock()->Update(m_deltaTime);
}

glm::vec2 tank_pos = { 0, 0 };

void Game::Render() {
	SDL_SetRenderDrawColor(m_renderer.get(), 100, 100, 100, 255);
	SDL_RenderClear(m_renderer.get());

	auto& registry = ecsModule::RegistryInstance::GetInstance();
	registry.GetSystem<ecsModule::systems::RenderSystem>().lock()->Update(m_deltaTime);

	//TODO
	int w, h;
	SDL_GetRendererOutputSize(m_renderer.get(), &w, &h);
	float scale = std::min(static_cast<float>(w) / width, static_cast<float>(h) / height);
	SDL_RenderSetScale(m_renderer.get(), scale, scale);

	SDL_RenderPresent(m_renderer.get());
}

void Game::Destroy() {
	m_renderer.reset(nullptr);
	m_window.reset(nullptr);
	SDL_Quit();
	logModule::LogManagerInstance::GetInstance().Teardown();
	logModule::LogManagerInstance::Cleanup();
}