//
// Created by Vladimir Glushkov on 18.08.2026.
//

#include "engine/assetsModule/AssetsManager.h"
#include "engine/graphicsModule/ITextureRegistry.h"
#include "engine/utilsModule/Assert.h"


struct pce::AssetsManager::Impl {
	ITextureRegistry& textureRegistry;

	explicit Impl(ITextureRegistry& textureRegistry): textureRegistry(textureRegistry) {}
};

pce::AssetsManager::AssetsManager(ITextureRegistry& textureRegistry)
	: m_impl(std::make_unique<Impl>(textureRegistry)) {}

pce::AssetsManager::~AssetsManager() = default;

// ReSharper disable once CppMemberFunctionMayBeConst
pce::TextureHandle pce::AssetsManager::LoadTexture(std::string_view filePath) {
	PCE_ASSERT(!filePath.empty(), "[AssetsManager::LoadTexture]: filePath is empty!");
	const auto handle = m_impl->textureRegistry.LoadTexture(filePath);
	return handle;
}

std::optional<pce::TextureInfo> pce::AssetsManager::GetTextureInfo(TextureHandle textureHandle) const {
	return m_impl->textureRegistry.GetTextureInfo(textureHandle);
}

std::optional<pce::SpriteRegion> pce::AssetsManager::GetSpriteRegion(TextureHandle textureHandle) const {
	const auto textureInfo = GetTextureInfo(textureHandle);
	if (!textureInfo) {
		return std::nullopt;
	}
	SpriteRegion region{
		.texture = textureInfo->texture,
		.width = textureInfo->width,
		.height = textureInfo->height,
	};
	return region;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void pce::AssetsManager::Unload(TextureHandle texture) {
	// todo check if texture has atlas then destroy it too
	m_impl->textureRegistry.DestroyTexture(texture);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void pce::AssetsManager::ClearAssets() {
	m_impl->textureRegistry.Clear();
}
