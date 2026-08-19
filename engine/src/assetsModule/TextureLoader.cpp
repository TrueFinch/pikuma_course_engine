//
// Created by Vladimir Glushkov on 19.08.2026.
//

#include "engine/assetsModule/TextureLoader.h"

#include <memory>
#include <SDL2/SDL_image.h>

#include "engine/logModule/Log.h"

std::optional<pce::TextureData> pce::TextureLoader::Load(std::string_view filePath) const {
	auto surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(
		IMG_Load(std::string(filePath).c_str()), SDL_FreeSurface
	);
	if (!surface) {
		logError("[TextureLoader::Load]: failed to load image '{}': {}", filePath, IMG_GetError());
		return std::nullopt;
	}

	// SDL_PIXELFORMAT_RGBA32 - “byte” format: pixels in memory R, G, B, A, regardless of
	// endianness (in little-endian this is an alias for ABGR8888, in big-endian it is RGBA8888).
	// After conversion TextureData = row-major RGBA8 without any channel swaps.
	auto converted = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(
		SDL_ConvertSurfaceFormat(surface.get(), SDL_PIXELFORMAT_RGBA32, 0), SDL_FreeSurface
	);
	surface.reset();

	if (!converted) {
		logError("[TextureLoader::Load]: failed to convert surface '{}': {}", filePath, SDL_GetError());
		return std::nullopt;
	}

	TextureData data;
	data.width = static_cast<uint32>(converted->w);
	data.height = static_cast<uint32>(converted->h);
	data.rgba.reserve(static_cast<size_t>(converted->w) * converted->h * 4);

	const auto* pixels = static_cast<const uint8*>(converted->pixels);
	for (int y = 0; y < converted->h; ++y) {
		const auto* row = pixels + static_cast<size_t>(y) * converted->pitch;
		data.rgba.insert(data.rgba.end(), row, row + static_cast<size_t>(converted->w) * 4);
	}

	return data;
}
