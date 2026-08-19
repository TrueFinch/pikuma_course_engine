//
// Created by Vladimir Glushkov on 19.08.2026.
//

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "engine/assetsModule/TextureLoader.h"

namespace {
	std::filesystem::path WriteTempFile(const std::string& fileName, const std::string& contents) {
		const auto path = std::filesystem::temp_directory_path() / fileName;
		std::ofstream out(path, std::ios::binary | std::ios::trunc);
		out << contents;
		return path;
	}
}

TEST_CASE("TextureLoader", "[TextureLoader]") {
	SECTION("Loads a 2x2 PPM image into RGBA8 row-major data") {
		// P6 PPM: red, green, blue, white.
		const std::string body =
				std::string("\xFF\x00\x00", 3) + std::string("\x00\xFF\x00", 3) +
				std::string("\x00\x00\xFF", 3) + std::string("\xFF\xFF\xFF", 3);
		const std::string ppm = "P6\n2 2\n255\n" + body;
		const auto path = WriteTempFile("pce_texture_2x2.ppm", ppm);

		pce::TextureLoader loader;
		const auto data = loader.Load(path.string());
		std::filesystem::remove(path);

		REQUIRE(data.has_value());
		REQUIRE(data->width == 2);
		REQUIRE(data->height == 2);
		REQUIRE(data->rgba.size() == 2 * 2 * 4);

		// Pixel 1 — is red: R,G,B,A.
		REQUIRE(data->rgba[0] == 0xFF);
		REQUIRE(data->rgba[1] == 0x00);
		REQUIRE(data->rgba[2] == 0x00);
		REQUIRE(data->rgba[3] == 0xFF);
		// Pixel 2 — is green.
		REQUIRE(data->rgba[4] == 0x00);
		REQUIRE(data->rgba[5] == 0xFF);
		REQUIRE(data->rgba[6] == 0x00);
		REQUIRE(data->rgba[7] == 0xFF);
	}

	SECTION("Returns nullopt for a missing file") {
		pce::TextureLoader loader;
		REQUIRE_FALSE(loader.Load("pce_definitely_missing_file.png").has_value());
	}
}
