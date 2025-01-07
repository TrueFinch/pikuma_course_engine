//
// Created by Vladimir Glushkov on 18.12.2024.
//
#pragma once

#include <memory>
#include <SDL2/SDL.h>

#include "engine/utilsModule/Types.h"

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

		void Delay();

		void Update();

		void Render();

		bool m_isRunning = false;
		uint32 m_fps = 60.f;
		uint32 m_prevFrameMillis = 0;
		uint32 m_milliPerFrame = 1000 / m_fps;
		float m_deltaTime = 0;

		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window;
		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer;
	};
}
