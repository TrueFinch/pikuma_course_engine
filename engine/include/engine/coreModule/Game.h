//
// Created by Vladimir Glushkov on 18.12.2024.
//
#pragma once

#include <memory>
#include <SDL2/SDL.h>

namespace pce {
	class Game {
	public:
		Game();

		~Game() = default;

		void Initialize();

		void Run();

		void Destroy();

	private:
		void ProcessInput();

		void Update();

		void Render();

		bool m_isRunning = false;

		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window;
		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer;
	};
}
