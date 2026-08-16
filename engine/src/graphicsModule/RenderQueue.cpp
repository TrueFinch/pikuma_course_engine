//
// Created by Vladimir on 16.08.2026.
//

#include "engine/graphicsModule/RenderQueue.h"

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

	const auto baseVertex = static_cast<uint32>(m_vertices.size());
	for (auto i = 0; i < 4; ++i) {
		const glm::vec2 rotated{
			localCorners[i].x * cosA - localCorners[i].y * sinA,
			localCorners[i].x * sinA + localCorners[i].y * cosA,
		};
		m_vertices.emplace_back(position + rotated, color, uvs[i]);
	}

	const auto baseIndex = static_cast<uint32>(m_indices.size());
	const std::array<uint32, 6> quadIndices({
		baseVertex + 0, baseVertex + 1, baseVertex + 2,
		baseVertex + 0, baseVertex + 2, baseVertex + 3,
	});

	m_indices.insert(m_indices.end(), quadIndices.begin(), quadIndices.end());
	m_drawCalls.emplace_back(
		texture, baseVertex, 4,
		baseIndex, 6, layer
	);
}

void pce::RenderQueue::AddMesh(
	TextureHandle texture, std::span<const Vertex> vertices, std::span<const uint32> indices,
	int layer
) {
	const auto baseVertex = static_cast<uint32>(m_vertices.size());
	const auto baseIndex = static_cast<uint32>(m_indices.size());

	m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
	m_indices.insert(m_indices.end(), indices.begin(), indices.end());

	m_drawCalls.emplace_back(
		texture, baseVertex, static_cast<uint32>(vertices.size()),
		baseIndex, static_cast<uint32>(indices.size()), layer
	);
}

std::span<const pce::Vertex> pce::RenderQueue::Vertices() const {
	return m_vertices;
}

std::span<const uint32> pce::RenderQueue::Indices() const {
	return m_indices;
}

std::span<const pce::DrawCall> pce::RenderQueue::DrawCalls() const {
	return m_drawCalls;
}

void pce::RenderQueue::Clear() noexcept {
	m_vertices.clear();
	m_indices.clear();
	m_drawCalls.clear();
}

bool pce::RenderQueue::IsEmpty() const noexcept {
	return m_drawCalls.empty();
}
