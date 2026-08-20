//
// Created by Vladimir Glushkov on 16.08.2026.
//

#pragma once

#include <compare>
#include <limits>
#include <span>
#include <unordered_map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/graphicsModule/SpriteRegion.h"
#include "engine/graphicsModule/TextureHandle.h"
#include "engine/utilsModule/Types.h"

namespace pce {
	struct Vertex {
		glm::vec2 position{};
		uint32 color = 0xFFFFFFFF; // RGBA
		glm::vec2 uv; // 0..1
		// IMPORTANT: order of fields must match with SDL_Vertex
	};

	struct BatchKey {
		TextureHandle texture;
		int layer = 0;

		explicit BatchKey(TextureHandle texture, int layer): texture{texture}, layer{layer} {};

		auto operator<=>(const BatchKey&) const = default;
	};

	struct Batch {
		BatchKey key;
		std::vector<Vertex> vertices{};
		std::vector<uint32> indices{};
	};
} // namespace pce

template<>
struct std::hash<pce::BatchKey> {
	size_t operator()(const pce::BatchKey& key) const noexcept {
		size_t h = std::hash<uint32>{}(key.texture.id);
		h ^= std::hash<int>{}(key.layer) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};

namespace pce {
	class RenderQueue {
	public:
		void AddSprite(glm::vec2 position, glm::vec2 size, uint32 color = 0xFFFFFFFF, float rotation = 0.f,
						int layer = 0);

		void AddTexturedQuad(TextureHandle texture, glm::vec2 position, glm::vec2 size,
							glm::vec4 uv, uint32 color = 0xFFFFFFFF,
							float rotation = 0.f, int layer = 0);

		// Sprite from the atlas: it takes into account pivot (position shift) and rotated
		// (transposed UVs). For a non-rotated frame, delegates to AddTexturedQuad.
		void AddSpriteRegion(const SpriteRegion& frame, glm::vec2 position, glm::vec2 size,
							uint32 color = 0xFFFFFFFF, float rotation = 0.f, int layer = 0);

		void AddMesh(TextureHandle texture, std::span<const Vertex> vertices, std::span<const uint32> indices,
					int layer = 0);

		void SortBatches();

		[[nodiscard]] std::span<const Batch> Batches() const;

		void Clear() noexcept;

		[[nodiscard]] bool IsEmpty() const noexcept;

	private:
		[[nodiscard]] Batch& GetBatch(const BatchKey& key);

		[[nodiscard]] size_t GetOrCreateBatchIndex(const BatchKey& key);

		std::vector<Batch> m_batches;
		std::unordered_map<BatchKey, size_t> m_batchIndex;
		size_t m_lastBatchIndex = std::numeric_limits<size_t>::max();
	};
} // namespace pce
