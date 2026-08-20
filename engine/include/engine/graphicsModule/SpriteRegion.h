//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <fmt/base.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "TextureHandle.h"

namespace pce {
	struct SpriteRegion {
		TextureHandle texture;
		glm::vec4 uv{0.f, 0.f, 1.f, 1.f};
		glm::uint16 width{};
		glm::uint16 height{};
		glm::vec2 pivot{0.5f, 0.5f};
		bool rotated{false};
	};
} // namespace pce

template<>
struct fmt::formatter<pce::SpriteRegion> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::SpriteRegion& spriteFrame, format_context& ctx) {
		return format_to(
			ctx.out(),
			"texture id: {}, uv: [{}, {}, {}, {}], width: {}, height: {}, pivot: [{}, {}], rotated: {}",
			spriteFrame.texture.id,
			spriteFrame.uv.x, spriteFrame.uv.y, spriteFrame.uv.z, spriteFrame.uv.w,
			spriteFrame.width,
			spriteFrame.height,
			spriteFrame.pivot.x, spriteFrame.pivot.y,
			spriteFrame.rotated
		);
	}
};
