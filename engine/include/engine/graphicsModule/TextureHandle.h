//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <compare>

#include "engine/utilsModule/Types.h"

namespace pce {
	struct TextureHandle {
		uint32 id{};

		auto operator<=>(const TextureHandle&) const = default;
	};

	struct TextureInfo {
		uint32 width{};
		uint32 height{};
	};
} // namespace pce

template<>
struct std::hash<pce::TextureHandle> {
	size_t operator()(const pce::TextureHandle& handle) const noexcept {
		return std::hash<decltype(pce::TextureHandle::id)>{}(handle.id);
	}
};
