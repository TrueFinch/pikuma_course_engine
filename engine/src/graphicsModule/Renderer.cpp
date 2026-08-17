//
// Created by Vladimir on 16.08.2026.
//

#include "engine/graphicsModule/Renderer.h"

#include <SDL_render.h>

#include "engine/graphicsModule/RenderQueue.h"
#include "engine/utilsModule/Assert.h"


namespace pce {
	// Наш Vertex ложится 1:1 на SDL_Vertex (position: SDL_FPoint, color: SDL_Color, tex_coord: SDL_FPoint)
	static_assert(sizeof(Vertex) == sizeof(SDL_Vertex));
	static_assert(offsetof(Vertex, position) == offsetof(SDL_Vertex, position));
	static_assert(offsetof(Vertex, uv) == offsetof(SDL_Vertex, tex_coord));
	static_assert(offsetof(Vertex, color) == offsetof(SDL_Vertex, color));
	static_assert(sizeof(uint32) == sizeof(int));

	struct Renderer::Impl {
		explicit Impl(SDL_Renderer* renderer): m_renderer{renderer, SDL_DestroyRenderer} {}

		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer;
	};
}


std::unique_ptr<pce::Renderer> pce::Renderer::Create(SDL_Window* window) {
	PCE_ASSERT(window, "[Renderer::Create]: window is null!]");
	SDL_Renderer* sdlRenderer{SDL_CreateRenderer(window, -1, 0)};
	PCE_ASSERT(sdlRenderer, "[Renderer::Create]: renderer is null!]");
	if (!sdlRenderer) {
		return nullptr;
	}
	// trick to be able to use make_unique
	struct UniqueRenderer: Renderer {
		UniqueRenderer(SDL_Renderer* renderer): Renderer(renderer) {}
	};
	return std::make_unique<UniqueRenderer>(sdlRenderer);
}

pce::Renderer::~Renderer() {}

void pce::Renderer::BeginFrame() const {
	SDL_SetRenderDrawColor(m_impl->m_renderer.get(), 0, 0, 0, 255);
	SDL_RenderClear(m_impl->m_renderer.get());
}

void pce::Renderer::Flush(const RenderQueue& queue) const {
	if (queue.IsEmpty()) {
		return;
	}
	const auto batches = queue.Batches();

	for (const auto& batch: batches) {
		SDL_Texture* texture = nullptr;
		auto renderRes = SDL_RenderGeometry(
			m_impl->m_renderer.get(), texture,
			reinterpret_cast<const SDL_Vertex*>(batch.vertices.data()),
			static_cast<int>(batch.vertices.size()),
			reinterpret_cast<const int*>(batch.indices.data()),
			static_cast<int>(batch.indices.size())
		);
		PCE_ASSERT(renderRes == 0, "[Renderer::Flush]: Failed to render queue!]");
	}
}

void pce::Renderer::EndFrame() const {
	SDL_RenderPresent(m_impl->m_renderer.get());
}

pce::Renderer::Renderer(SDL_Renderer* renderer)
	: m_impl{std::make_unique<Impl>(renderer)} {}
