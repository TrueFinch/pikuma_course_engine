//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include "engine/coreModule/Game.h"

#include <SDL_events.h>
#include <SDL_timer.h>

#include "engine/ecsModule/ECS.h"
#include "engine/graphicsModule/GraphicsContext.h"
#include "engine/logModule/Log.h"
#include "engine/logModule/LogManager.h"
#include "engine/logModule/LogManagerInstance.h"
#include "engine/logModule/SpdLogger.h"
#include "engine/utilsModule/Types.h"

pce::Game::Game() = default;

pce::Game::~Game() = default;

bool pce::Game::Initialize() {
	// init game systems
	logModule::LogManagerInstance::Init(logModule::LogManager::Create());
	logModule::LogManagerInstance::GetInstance().RegisterObserver(std::make_unique<logModule::SpdLogger>());
	m_graphics = std::make_unique<GraphicsContext>();
	return m_graphics->Initialize(1280, 800);

	// TODO: add toggle fullscreen to settings
	// SDL_SetWindowFullscreen(m_window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void pce::Game::Run() {
	m_prevFrameMillis = SDL_GetTicks();
	m_isRunning = true;
	while (m_isRunning) {
		ProcessInput();
		Update();
		Render();
		Delay();
	}
}

void pce::Game::Destroy() {
	m_graphics.reset(); // GraphicsContext::~GraphicsContext -> SDL_Quit
}

pce::Registry& pce::Game::GetRegistry() {
	return m_registry;
}

pce::SystemManager& pce::Game::GetSystemManager() {
	return m_systemManager;
}

pce::RenderQueue& pce::Game::GetRenderQueue() {
	return m_renderQueue;
}

void pce::Game::ProcessInput() {
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
			default:
				logWarning("Unknown SDL_EventType (hex): {:#x}", sdlEvent.type);
		}
	}
}

void pce::Game::Delay() {
	if (const uint32 ticks = SDL_GetTicks(); m_milliPerFrame + m_prevFrameMillis > ticks) {
		SDL_Delay(m_milliPerFrame + m_prevFrameMillis - ticks);
	}
}

void pce::Game::Update() {
	m_deltaTime = static_cast<float>(SDL_GetTicks() - m_prevFrameMillis) / 1000.f;
	m_prevFrameMillis = SDL_GetTicks();
	log("{}", m_deltaTime);
	m_systemManager.Update(m_registry, m_deltaTime);
}

void pce::Game::Render() {
	auto& renderer = m_graphics->GetRenderer();
	renderer.BeginFrame();
	m_renderQueue.SortBatches();
	renderer.Flush(m_renderQueue);
	renderer.EndFrame();
	m_renderQueue.Clear();
}
