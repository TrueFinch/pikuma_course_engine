//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <optional>
#include <string_view>

#include "engine/assetsModule/TextureData.h"

namespace pce {
	// Decodes an image (PNG/JPEG/PPM/...) to RGBA8. SDL_image is hidden in .cpp.
	class TextureLoader {
	public:
		// TODO(assets): C++23 — replace with std::expected<TextureData, AssetLoadError>
		std::optional<TextureData> Load(std::string_view filePath) const;
	};
} // namespace pce
