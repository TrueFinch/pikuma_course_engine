//
// Created by Vladimir Glushkov on 19.08.2026.
//

#pragma once

#include <vector>

#include "engine/utilsModule/Types.h"

namespace pce {
	// Decoded image: RGBA8, row-major (width * height * 4 bytes).
	// Contract: pixel bytes are in order R,G,B,A.
	struct TextureData {
		uint16 width{};
		uint16 height{};
		std::vector<uint8> rgba;
	};
} // namespace pce
