//
// Created by Vladimir on 16.08.2026.
//

#include "engine/graphicsModule/GraphicsContext.h"

#include <SDL.h>
#include <SDL_video.h>

#include "engine/utilsModule/Assert.h"

namespace pce {
	struct GraphicsContext::Impl {
		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window{nullptr, SDL_DestroyWindow};
		std::unique_ptr<Renderer> m_renderer;
	};
}

pce::GraphicsContext::GraphicsContext() = default;

pce::GraphicsContext::~GraphicsContext() {
	Shutdown();
}

bool pce::GraphicsContext::Initialize(int width, int height) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		PCE_ASSERT(false, "Failed to initialize SDL!");
		logError("SDL_Init Error: {}", SDL_GetError());
		return false;
	}

	m_impl = std::make_unique<GraphicsContext::Impl>();
	m_impl->m_window.reset(SDL_CreateWindow(
		nullptr,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_RESIZABLE
	));
	if (!m_impl->m_window) {
		PCE_ASSERT(false, "Failed to create window!");
		Shutdown();
		return false;
	}

	m_impl->m_renderer = Renderer::Create(m_impl->m_window.get());
	if (!m_impl->m_renderer) {
		PCE_ASSERT(false, "Failed to create renderer!");
		Shutdown();
		return false;
	}

	return true;
}

void pce::GraphicsContext::Shutdown() {
	m_impl.reset();
	SDL_Quit();
}

pce::Renderer& pce::GraphicsContext::GetRenderer() {
	PCE_ASSERT(m_impl && m_impl->m_renderer, "GraphicsContext is not initialized!");
	return *m_impl->m_renderer.get();
}

const pce::Renderer& pce::GraphicsContext::GetRenderer() const {
	PCE_ASSERT(m_impl && m_impl->m_renderer, "GraphicsContext is not initialized!");
	return *m_impl->m_renderer.get();
}
