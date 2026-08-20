//
// Created by Vladimir Glushkov on 18.12.2024.
//
#pragma once

#include <memory>

#include "engine/ecsModule/ECS.h"
#include "engine/graphicsModule/RenderQueue.h"
#include "engine/utilsModule/Types.h"

namespace pce {
	class AssetsManager;
	class GraphicsContext;

	class Game {
	public:
		Game();

		~Game();

		[[nodiscard]] bool Initialize();

		void Run();

		void Destroy();

		[[nodiscard]] Registry& GetRegistry();

		[[nodiscard]] SystemManager& GetSystemManager();

		[[nodiscard]] RenderQueue& GetRenderQueue();

		[[nodiscard]] AssetsManager& GetAssetsManager();

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

		RenderQueue m_renderQueue;
		std::unique_ptr<GraphicsContext> m_graphics;
		std::unique_ptr<AssetsManager> m_assetsManager;

		Registry m_registry;
		SystemManager m_systemManager;
	};
}
