//
// Created by Vladimir on 16.08.2026.
//

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "engine/graphicsModule/RenderQueue.h"

TEST_CASE("RenderQueue", "[RenderQueue]") {
	SECTION("Empty queue") {
		pce::RenderQueue queue;
		REQUIRE(queue.IsEmpty());
		REQUIRE(queue.DrawCalls().empty());
		REQUIRE(queue.Vertices().empty());
		REQUIRE(queue.Indices().empty());
	}

	SECTION("AddSprite produces one draw call with 4 vertices and 6 indices") {
		pce::RenderQueue queue;
		queue.AddSprite({10.f, 20.f}, {30.f, 40.f});
		REQUIRE_FALSE(queue.IsEmpty());
		REQUIRE(queue.DrawCalls().size() == 1);
		REQUIRE(queue.Vertices().size() == 4);
		REQUIRE(queue.Indices().size() == 6);

		const auto& call = queue.DrawCalls()[0];
		REQUIRE(call.texture.id == 0);
		REQUIRE(call.vertexOffset == 0);
		REQUIRE(call.vertexCount == 4);
		REQUIRE(call.indexOffset == 0);
		REQUIRE(call.indexCount == 6);
		REQUIRE(call.layer == 0);
		const auto v = queue.Vertices();
		// position = центр спрайта (половина размера вычитается из позиции)
		REQUIRE(v[0].position == glm::vec2{-5.f, 0.f});
		REQUIRE(v[2].position == glm::vec2{25.f, 40.f});
	}

	SECTION("Multiple sprites produce correct offsets") {
		pce::RenderQueue queue;
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});
		queue.AddSprite({0.f, 0.f}, {10.f, 10.f});
		REQUIRE(queue.DrawCalls().size() == 2);
		REQUIRE(queue.DrawCalls()[1].vertexOffset == 4);
		REQUIRE(queue.DrawCalls()[1].indexOffset == 6);
		REQUIRE(queue.Vertices().size() == 8);
		REQUIRE(queue.Indices().size() == 12);
	}

	SECTION("AddMesh appends raw geometry") {
		pce::RenderQueue queue;
		std::array<pce::Vertex, 3> vertices{
			pce::Vertex{{0.f, 0.f}, 0xFFFFFFFF, {0.f, 0.f}},
			pce::Vertex{{1.f, 0.f}, 0xFFFFFFFF, {1.f, 0.f}},
			pce::Vertex{{0.f, 1.f}, 0xFFFFFFFF, {0.f, 1.f}},
		};
		std::array<uint32, 3> indices{0, 1, 2};
		queue.AddMesh({}, vertices, indices);
		REQUIRE(queue.DrawCalls().size() == 1);
		REQUIRE(queue.Vertices().size() == 3);
		REQUIRE(queue.Indices().size() == 3);
		REQUIRE(queue.DrawCalls()[0].vertexCount == 3);
		REQUIRE(queue.DrawCalls()[0].indexCount == 3);
	}

	SECTION("Clear resets the queue") {
		pce::RenderQueue queue;
		queue.AddSprite({}, {});
		queue.Clear();
		REQUIRE(queue.IsEmpty());
		REQUIRE(queue.Vertices().empty());
		REQUIRE(queue.Indices().empty());
		REQUIRE(queue.DrawCalls().empty());
	}
}
