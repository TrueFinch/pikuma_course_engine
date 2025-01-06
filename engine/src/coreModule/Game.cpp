//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include <iostream>
#include <ostream>

#include "engine/coreModule/Game.h"

pce::Game::Game()
	: m_window(nullptr, SDL_DestroyWindow)
	, m_renderer(nullptr, SDL_DestroyRenderer) {}

void pce::Game::Initialize() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return;
	}
	// create sdl window
	int width = 1280, height = 800;
	m_window.reset(SDL_CreateWindow(
		nullptr,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_RESIZABLE
	));

	if (!m_window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return;
	}
	// create sdl renderer
	m_renderer.reset(SDL_CreateRenderer(
		m_window.get(), -1, 0
	));
	if (!m_renderer) {
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return;
	}
}

void pce::Game::Run() {
	m_isRunning = true;
	while (m_isRunning) {
		ProcessInput();
		Update();
		Render();
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
				std::cout << "Unknown SDL_EventType (hex): 0x" << std::hex << sdlEvent.type << std::endl;
		}
	}
}

void pce::Game::Update() {
	//TODO: update game systems
}

void pce::Game::Render() {
	SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255);
	SDL_RenderClear(m_renderer.get());

	//TODO: draw game objects

	SDL_RenderPresent(m_renderer.get());
}
