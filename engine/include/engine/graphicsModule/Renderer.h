//
// Created by Vladimir Glushkov on 16.08.2026.
//

#pragma once

#include <memory>
#include <vector>

#include "ITextureRegistry.h"

struct SDL_Window;
struct SDL_Renderer;

namespace pce {
	class RenderQueue;

	class Renderer {
	public:
		static std::unique_ptr<Renderer> Create(SDL_Window* window);

		~Renderer();

		Renderer(const Renderer&) = delete;

		Renderer& operator=(const Renderer&) = delete;

		void BeginFrame() const;

		void Flush(const RenderQueue& queue) const;

		void EndFrame() const;

		ITextureRegistry& GetTextureRegistry();

		const ITextureRegistry& GetTextureRegistry() const;

		// todo: add render target later
	private:
		explicit Renderer(SDL_Renderer* renderer);

		struct Impl;

		std::unique_ptr<Impl> m_impl{nullptr};
	};
} // namespace pce
