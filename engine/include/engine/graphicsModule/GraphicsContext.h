//
// Created by Vladimir Glushkov on 16.08.2026.
//

#pragma once

#include "Renderer.h"

namespace pce {
	class GraphicsContext {
	public:
		GraphicsContext();

		~GraphicsContext();

		GraphicsContext(const GraphicsContext&) = delete;

		GraphicsContext& operator=(const GraphicsContext&) = delete;

		[[nodiscard]] bool Initialize(int width, int height);

		void Shutdown();

		[[nodiscard]] Renderer& GetRenderer();

		[[nodiscard]] const Renderer& GetRenderer() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
} // namespace pce
