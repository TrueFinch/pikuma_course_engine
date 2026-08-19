//
// Created by Vladimir Glushkov on 16.08.2026.
//

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "engine/graphicsModule/RenderQueue.h"

TEST_CASE("RenderQueue", "[RenderQueue]") {
	pce::RenderQueue queue;

	SECTION("Empty queue") {
		REQUIRE(queue.IsEmpty());
		REQUIRE(queue.Batches().empty());
	}

	SECTION("AddSprite produces one batch with 4 vertices and 6 indices") {
		queue.AddSprite({10.f, 20.f}, {30.f, 40.f});
		REQUIRE_FALSE(queue.IsEmpty());

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		const auto& batch = batches[0];
		REQUIRE(batch.key.texture.id == 0);
		REQUIRE(batch.key.layer == 0);
		REQUIRE(batch.vertices.size() == 4);
		REQUIRE(batch.indices.size() == 6);

		// position is in center of sprite (pivot 0.5 0.5)
		REQUIRE(batch.vertices[0].position == glm::vec2{-5.f, 0.f});
		REQUIRE(batch.vertices[2].position == glm::vec2{25.f, 40.f});
	}

	SECTION("Sprites with the same key are batched together") {
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		REQUIRE(batches[0].vertices.size() == 12);
		REQUIRE(batches[0].indices.size() == 18);
	}

	SECTION("Indices are batch-relative (baseVertex offset)") {
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		const std::array<uint32, 6> first{0, 1, 2, 0, 2, 3};
		const std::array<uint32, 6> second{4, 5, 6, 4, 6, 7};
		for (auto i = 0; i < 6; ++i) {
			REQUIRE(batches[0].indices[i] == first[i]);
			REQUIRE(batches[0].indices[i + 6] == second[i]);
		}
	}

	SECTION("Different textures produce separate batches") {
		queue.AddTexturedQuad({1}, {0.f, 0.f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f});
		queue.AddSprite({0.f, 0.f}, {0.5f, 0.5f});
		queue.AddTexturedQuad({2}, {0.f, 0.f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f});

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 3);
		// inside layer 0 batches sorted by texture id
		REQUIRE(batches[0].key.texture.id == 0);
		REQUIRE(batches[1].key.texture.id == 1);
		REQUIRE(batches[2].key.texture.id == 2);
	}

	SECTION("Interleaved textures still produce one batch per key") {
		for (auto i = 0; i < 100; ++i) {
			queue.AddTexturedQuad({1}, {0.5f, 0.5f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f});
			queue.AddTexturedQuad({2}, {0.5f, 0.5f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f});
		}

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 2);
		REQUIRE(batches[0].vertices.size() == 400);
		REQUIRE(batches[1].vertices.size() == 400);
	}

	SECTION("Batches are sorted by (layer, texture) regardless of add order") {
		queue.AddTexturedQuad({1}, {0.f, 0.f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f}, 0xFFFFFFFF, 0.f, 1);
		queue.AddTexturedQuad({2}, {0.f, 0.f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f}, 0xFFFFFFFF, 0.f, 0);
		queue.AddTexturedQuad({1}, {0.f, 0.f}, {10.f, 10.f}, {0.f, 0.f, 1.f, 1.f}, 0xFFFFFFFF, 0.f, 0);

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 3);
		// layer ordered: all zero layers are first, then layer 1
		REQUIRE(batches[0].key.layer == 0);
		REQUIRE(batches[0].key.texture.id == 1);
		REQUIRE(batches[1].key.layer == 0);
		REQUIRE(batches[1].key.texture.id == 2);
		REQUIRE(batches[2].key.layer == 1);
		REQUIRE(batches[2].key.texture.id == 1);
	}

	SECTION("AddMesh appends raw geometry with batch-relative indices") {
		std::array vertices{
			pce::Vertex{{0.f, 0.f}, 0xFFFFFFFF, {0.f, 0.f}},
			pce::Vertex{{1.f, 0.f}, 0xFFFFFFFF, {1.f, 0.f}},
			pce::Vertex{{0.f, 1.f}, 0xFFFFFFFF, {0.f, 1.f}},
		};
		std::array<uint32, 3> indices{0, 1, 2};

		queue.AddMesh({}, vertices, indices);
		queue.AddMesh({}, vertices, indices);

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		REQUIRE(batches[0].vertices.size() == 6);
		REQUIRE(batches[0].indices.size() == 6);
		// second mesh: indices offsets by baseVertex
		REQUIRE(batches[0].indices[0] == 0);
		REQUIRE(batches[0].indices[3] == 3);
		REQUIRE(batches[0].indices[4] == 4);
		REQUIRE(batches[0].indices[5] == 5);
	}

	SECTION("Clear resets the queue") {
		queue.AddSprite({}, {});
		queue.Clear();
		REQUIRE(queue.IsEmpty());
		REQUIRE(queue.Batches().empty());
	}
}

TEST_CASE("RenderQueue.AddSpriteFrame", "[RenderQueue]") {
	pce::RenderQueue queue;

	SECTION("Non-rotated frame delegates like AddTexturedQuad") {
		pce::SpriteFrame frame;
		frame.texture = {1};
		frame.uv = {0.f, 0.f, 0.5f, 0.5f};
		frame.width = 10;
		frame.height = 20;
		frame.pivot = {0.5f, 0.5f};
		frame.rotated = false;

		queue.AddSpriteFrame(frame, {100.f, 100.f}, {10.f, 20.f});

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		const auto& batch = batches[0];
		REQUIRE(batch.key.texture.id == 1);
		REQUIRE(batch.vertices.size() == 4);
		// pivot (0.5, 0.5) - position in the center of the frame, UV as in AddTexturedQuad.
		REQUIRE(batch.vertices[0].position == glm::vec2{95.f, 90.f});
		REQUIRE(batch.vertices[0].uv == glm::vec2{0.f, 0.f});
		REQUIRE(batch.vertices[1].uv == glm::vec2{0.5f, 0.f});
	}

	SECTION("Pivot shifts the draw position") {
		pce::SpriteFrame frame;
		frame.texture = {1};
		frame.width = 10;
		frame.height = 10;
		frame.pivot = {0.f, 0.f}; // left-upper corner of the sprite
		frame.rotated = false;

		queue.AddSpriteFrame(frame, {100.f, 100.f}, {10.f, 10.f});

		const auto batches = queue.Batches();
		const auto& batch = batches[0];
		// position - (pivot - 0.5) * size: (100,100) - (-5,-5) = (105,105) — quad center.
		REQUIRE(batch.vertices[0].position == glm::vec2{100.f, 100.f});
		REQUIRE(batch.vertices[2].position == glm::vec2{110.f, 110.f});
	}

	SECTION("Rotated frame transposes UVs") {
		pce::SpriteFrame frame;
		frame.texture = {1};
		frame.uv = {0.f, 0.5f, 0.5f, 1.f}; // (u0, v0, u1, v1)
		frame.width = 4; // the displayed dimensions are already expanded (region.height)
		frame.height = 2; // region.width
		frame.pivot = {0.5f, 0.5f};
		frame.rotated = true;

		queue.AddSpriteFrame(frame, {0.f, 0.f}, {4.f, 2.f});

		const auto batches = queue.Batches();
		REQUIRE(batches.size() == 1);
		const auto& batch = batches[0];
		REQUIRE(batch.vertices.size() == 4);
		// Promotion: TL→(u0,v1), TR→(u0,v0), BR→(u1,v0), BL→(u1,v1).
		REQUIRE(batch.vertices[0].uv == glm::vec2{0.f, 1.f});
		REQUIRE(batch.vertices[1].uv == glm::vec2{0.f, 0.5f});
		REQUIRE(batch.vertices[2].uv == glm::vec2{0.5f, 0.5f});
		REQUIRE(batch.vertices[3].uv == glm::vec2{0.5f, 1.f});
	}
}
