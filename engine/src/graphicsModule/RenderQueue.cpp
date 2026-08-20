//
// Created by Vladimir on 16.08.2026.
//

#include "engine/graphicsModule/RenderQueue.h"

#include <algorithm>
#include <cmath>

void pce::RenderQueue::AddSprite(
	glm::vec2 position, glm::vec2 size, uint32 color, float rotation, int layer
) {
	AddTexturedQuad({}, position, size, {0.f, 0.f, 1.f, 1.f}, color, rotation, layer);
}

void pce::RenderQueue::AddTexturedQuad(
	TextureHandle texture, glm::vec2 position, glm::vec2 size, glm::vec4 uv,
	uint32 color, float rotation, int layer
) {
	const glm::vec2 halfSize = size / 2.f;
	const float cosA = std::cos(rotation);
	const float sinA = std::sin(rotation);

	const std::array<glm::vec2, 4> localCorners({
		{-halfSize.x, -halfSize.y}, {halfSize.x, -halfSize.y},
		{halfSize.x, halfSize.y}, {-halfSize.x, halfSize.y},
	});

	const std::array<glm::vec2, 4> uvs({
		{uv.x, uv.y}, {uv.z, uv.y},
		{uv.z, uv.w}, {uv.x, uv.w},
	});

	auto& batch = GetBatch(BatchKey{texture, layer});

	const auto baseVertex = static_cast<uint32>(batch.vertices.size());
	for (auto i = 0; i < 4; ++i) {
		const glm::vec2 rotated{
			localCorners[i].x * cosA - localCorners[i].y * sinA,
			localCorners[i].x * sinA + localCorners[i].y * cosA,
		};
		batch.vertices.emplace_back(position + rotated, color, uvs[i]);
	}

	const std::array<uint32, 6> quadIndices{0, 1, 2, 0, 2, 3,};
	for (auto index: quadIndices) {
		batch.indices.emplace_back(baseVertex + index);
	}
}

void pce::RenderQueue::AddSpriteRegion(
	const SpriteRegion& frame, glm::vec2 position, glm::vec2 size,
	uint32 color, float rotation, int layer
) {
	// Pivot is normalized (0..1): (0.5, 0.5) - position in the center of the frame (like AddTexturedQuad),
	// (0, 0) - the position is the left-upper corner of the sprite.
	position -= (frame.pivot - glm::vec2{0.5f}) * size;

	if (!frame.rotated) {
		AddTexturedQuad(frame.texture, position, size, frame.uv, color, rotation, layer);
		return;
	}

	// Rotated region (spine convention: stored rotated 90° clockwise).
	// Unwind - UV transposition: TL→(u0,v1), TR→(u0,v0), BR→(u1,v0), BL→(u1,v1).
	const glm::vec2 halfSize = size / 2.f;
	const float cosA = std::cos(rotation);
	const float sinA = std::sin(rotation);

	const std::array<glm::vec2, 4> localCorners({
		{-halfSize.x, -halfSize.y}, {halfSize.x, -halfSize.y},
		{halfSize.x, halfSize.y}, {-halfSize.x, halfSize.y},
	});
	const std::array<glm::vec2, 4> uvs({
		{frame.uv.x, frame.uv.w}, {frame.uv.x, frame.uv.y},
		{frame.uv.z, frame.uv.y}, {frame.uv.z, frame.uv.w},
	});

	std::array<Vertex, 4> vertices;
	for (auto i = 0; i < 4; ++i) {
		const glm::vec2 rotated{
			localCorners[i].x * cosA - localCorners[i].y * sinA,
			localCorners[i].x * sinA + localCorners[i].y * cosA,
		};
		vertices[i] = {position + rotated, color, uvs[i]};
	}
	constexpr std::array<uint32, 6> quadIndices{0, 1, 2, 0, 2, 3,};
	AddMesh(frame.texture, vertices, quadIndices, layer);
}

void pce::RenderQueue::AddMesh(
	TextureHandle texture, std::span<const Vertex> vertices, std::span<const uint32> indices,
	int layer
) {
	auto& batch = GetBatch(BatchKey{texture, layer});

	const auto baseVertex = static_cast<uint32>(batch.vertices.size());
	batch.vertices.insert(batch.vertices.end(), vertices.begin(), vertices.end());
	for (const auto& index: indices) {
		batch.indices.emplace_back(baseVertex + index);
	}
}

void pce::RenderQueue::SortBatches() {
	std::stable_sort(m_batches.begin(), m_batches.end(), [](const Batch& a, const Batch& b) {
		if (a.key.layer != b.key.layer) {
			return a.key.layer < b.key.layer;
		}
		return a.key.texture < b.key.texture;
	});
	m_batchIndex.clear();
	for (auto i = 0; i < m_batches.size(); ++i) {
		m_batchIndex.emplace(m_batches[i].key, i);
	}
	m_lastBatchIndex = std::numeric_limits<decltype(m_lastBatchIndex)>::max();
}

std::span<const pce::Batch> pce::RenderQueue::Batches() const {
	return m_batches;
}

void pce::RenderQueue::Clear() noexcept {
	m_batches.clear();
	m_batchIndex.clear();
	m_lastBatchIndex = std::numeric_limits<decltype(m_lastBatchIndex)>::max();
}

bool pce::RenderQueue::IsEmpty() const noexcept {
	return m_batches.empty();
}

pce::Batch& pce::RenderQueue::GetBatch(const BatchKey& key) {
	auto batchIndex = m_lastBatchIndex;
	if (batchIndex == std::numeric_limits<decltype(m_lastBatchIndex)>::max()
		|| m_batches[batchIndex].key != key
	) {
		batchIndex = GetOrCreateBatchIndex(key);
		m_lastBatchIndex = batchIndex;
	}
	return m_batches[batchIndex];
}

size_t pce::RenderQueue::GetOrCreateBatchIndex(const BatchKey& key) {
	const auto it = m_batchIndex.find(key);
	if (it != m_batchIndex.end()) {
		return it->second;
	}
	const auto index = m_batches.size();
	m_batches.emplace_back(key);
	m_batchIndex.emplace(key, index);
	return index;
}
