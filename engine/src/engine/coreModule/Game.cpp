//
// Created by Vladimir on 05.01.2023.
//
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>

#include "SDL_image.h"
#include "Game.h"

using namespace pce;

int width = 1280, height = 800;

Game::Game() : m_window(nullptr, SDL_DestroyWindow), m_renderer(nullptr, SDL_DestroyRenderer) {
}

void Game::Initialize() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "[Game::Initialize] ERROR: failed to initialize sdl!" << std::endl;
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
		std::cerr << "[Game::Initialize] ERROR: failed to create sdl window!" << std::endl;
		return;
	}

	m_renderer.reset(SDL_CreateRenderer(
			m_window.get(), -1, 0
	));
	if (!m_renderer) {
		std::cerr << "[Game::Initialize] ERROR: failed to create sdl renderer!" << std::endl;
		return;
	}

//	SDL_SetWindowFullscreen(
//		m_window.get(),
//		SDL_WINDOW_FULLSCREEN
//	);

	m_isRunning = true;
}

void Game::Run() {
	while (m_isRunning) {
		ProcessInput();
		Update();
		Render();
	}
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

void Game::Update() {

}

void Game::Render() {
	SDL_SetRenderDrawColor(m_renderer.get(), 100, 100, 100, 255);
	SDL_RenderClear(m_renderer.get());

	SDL_Surface* s = IMG_Load("../assets/images/tank-tiger-right.png");
	if (!s) {
		std::cout << SDL_GetError() << std::endl;
		std::cout << "image is not loaded!" << ' ' << std::filesystem::current_path() << std::endl;
		std::cout << SDL_GetBasePath() << std::endl;
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(m_renderer.get(), s);
	SDL_FreeSurface(s);

	SDL_Rect dstRect = { 10, 10, 32, 32 };

	SDL_RenderCopy(m_renderer.get(), t, nullptr, &dstRect);
	SDL_DestroyTexture(t);

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
}
