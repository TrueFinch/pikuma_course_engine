//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <optional>
#include <string_view>

#include "TextureHandle.h"

namespace pce {
	struct TextureData;

	// Light SDL-free texture registry interface (DIP): AssetsManager only works through this.
	class ITextureRegistry {
	public:
		virtual ~ITextureRegistry() = default;

		// Loads an image via TextureLoader and registers a GPU texture.
		virtual TextureHandle LoadTexture(std::string_view filePath) = 0;

		// Registers a GPU texture from ready-made RGBA8 (row-major) pixels.
		// todo: maybe should hide this method
		virtual TextureHandle CreateTexture(const TextureData& data) = 0;

		// Removes GPU texture. Safe for {0} and unknown handles.
		virtual void DestroyTexture(TextureHandle texture) noexcept = 0;

		// Metadata of the registered texture (nullopt - handle not found).
		virtual std::optional<TextureInfo> GetTextureInfo(TextureHandle texture) const = 0;
	};
} // namespace pce
