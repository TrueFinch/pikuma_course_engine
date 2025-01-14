//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include "engine/coreModule/Game.h"

#include <iostream>

#include "engine/logModule/Log.h"
#include "engine/logModule/LogManager.h"
#include "engine/logModule/LogManagerInstance.h"
#include "engine/utilsModule/Types.h"
#include "logModule/observers/SpdLogger.h"

pce::Game::Game()
	: m_window(nullptr, SDL_DestroyWindow)
	, m_renderer(nullptr, SDL_DestroyRenderer) {}

void pce::Game::Initialize() {
	// init game systems
	logModule::LogManagerInstance::Init(logModule::LogManager::Create());
	logModule::LogManagerInstance::GetInstance().RegisterObserver(std::make_unique<logModule::SpdLogger>());

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		logError("SDL_Init Error: {}", SDL_GetError());
		return;
	}

	// TODO: move to settings
	int width = 1280, height = 800;
	// create sdl window
	m_window.reset(SDL_CreateWindow(
		nullptr,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_RESIZABLE
	));

	if (!m_window) {
		logError("SDL_CreateWindow Error: {}", SDL_GetError());
		return;
	}
	// create sdl renderer
	m_renderer.reset(SDL_CreateRenderer(
		m_window.get(), -1, 0
	));
	if (!m_renderer) {
		logError("SDL_CreateRenderer Error: {}", SDL_GetError());
		return;
	}

	// TODO: add toggle fullscreen to settings
	// SDL_SetWindowFullscreen(m_window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void pce::Game::Run() {
	m_isRunning = true;
	while (m_isRunning) {
		ProcessInput();
		Update();
		Render();
		Delay();
	}
}

void pce::Game::Destroy() {
	m_renderer.reset(nullptr);
	m_window.reset(nullptr);
	SDL_Quit();
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
	//TODO: update game systems
}

void pce::Game::Render() {
	SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255);
	SDL_RenderClear(m_renderer.get());

	//TODO: draw game objects

	SDL_RenderPresent(m_renderer.get());
}
