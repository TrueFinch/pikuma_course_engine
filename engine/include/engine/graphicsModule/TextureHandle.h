//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <compare>
#include <fmt/base.h>

#include "engine/utilsModule/Types.h"

namespace pce {
	struct TextureHandle {
		uint32 id{};

		auto operator<=>(const TextureHandle&) const = default;
	};

	struct TextureInfo {
		TextureHandle texture;
		uint16 width{};
		uint16 height{};
	};
} // namespace pce

template<>
struct std::hash<pce::TextureHandle> {
	size_t operator()(const pce::TextureHandle& handle) const noexcept {
		return std::hash<decltype(pce::TextureHandle::id)>{}(handle.id);
	}
};

template<>
struct fmt::formatter<pce::TextureHandle> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::TextureHandle& texture, format_context& ctx) {
		return format_to(
			ctx.out(),
			"id: {}",
			texture.id
		);
	}
};

template<>
struct fmt::formatter<pce::TextureInfo> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const pce::TextureInfo& textureInfo, format_context& ctx) {
		return format_to(
			ctx.out(),
			"id: {}, width: {}, height: {}",
			textureInfo.texture,
			textureInfo.width,
			textureInfo.height
		);
	}
};
