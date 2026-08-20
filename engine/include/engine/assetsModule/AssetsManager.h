//
// Created by Vladimir Glushkov on 18.08.2026.
//

#pragma once
#include <memory>
#include <optional>

#include "engine/graphicsModule/SpriteRegion.h"
#include "engine/graphicsModule/TextureHandle.h"


struct SDL_Texture;

namespace pce {
	class ITextureRegistry;

	class AssetsManager final {
	public:
		explicit AssetsManager(ITextureRegistry& textureRegistry);

		~AssetsManager();

		AssetsManager(const AssetsManager&) = delete;

		AssetsManager& operator=(const AssetsManager&) = delete;

		TextureHandle LoadTexture(std::string_view filePath);

		std::optional<TextureInfo> GetTextureInfo(TextureHandle textureHandle) const;

		std::optional<SpriteRegion> GetSpriteRegion(TextureHandle textureHandle) const;

		// todo: implement after including atlases in engine
		// SpriteRegion GetSpriteRegion(TextureHandle textureHandle, std::string_view regionName) const;

		void Unload(TextureHandle texture);

		void ClearAssets();

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
} // namespace pce
