//
// Created by Vladimir Glushkov on 16.08.2026.
//

#pragma once

#include <compare>
#include <span>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/utilsModule/Types.h"

namespace pce {
	struct Vertex {
		glm::vec2 position{};
		uint32 color = 0xFFFFFFFF; // RGBA
		glm::vec2 uv; // 0..1
		// IMPORTANT: order of fields must match with SDL_Vertex
	};

	struct TextureHandle {
		uint32 id = 0;

		auto operator<=>(const TextureHandle&) const = default;
	};

	struct DrawCall {
		TextureHandle texture;
		uint32 vertexOffset = 0;
		uint32 vertexCount = 0;
		uint32 indexOffset = 0;
		uint32 indexCount = 0;
		int layer = 0; // z ordering
	};

	class RenderQueue {
	public:
		void AddSprite(glm::vec2 position, glm::vec2 size, uint32 color = 0xFFFFFFFF, float rotation = 0.f,
						int layer = 0);

		void AddTexturedQuad(TextureHandle texture, glm::vec2 position, glm::vec2 size,
							glm::vec4 uv, uint32 color = 0xFFFFFFFF,
							float rotation = 0.f, int layer = 0);

		void AddMesh(TextureHandle texture, std::span<const Vertex> vertices, std::span<const uint32> indices,
					int layer = 0);

		[[nodiscard]] std::span<const Vertex> Vertices() const;

		[[nodiscard]] std::span<const uint32> Indices() const;

		[[nodiscard]] std::span<const DrawCall> DrawCalls() const;

		void Clear() noexcept;

		[[nodiscard]] bool IsEmpty() const noexcept;

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32> m_indices;
		std::vector<DrawCall> m_drawCalls;
	};
} // namespace pce
