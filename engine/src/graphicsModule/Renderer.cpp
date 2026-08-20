//
// Created by Vladimir on 16.08.2026.
//

#include "engine/graphicsModule/Renderer.h"

#include <optional>
#include <SDL_render.h>

#include "engine/assetsModule/TextureLoader.h"
#include "engine/graphicsModule/ITextureRegistry.h"
#include "engine/graphicsModule/RenderQueue.h"
#include "engine/utilsModule/Assert.h"


namespace pce {
	// Наш Vertex ложится 1:1 на SDL_Vertex (position: SDL_FPoint, color: SDL_Color, tex_coord: SDL_FPoint)
	static_assert(sizeof(Vertex) == sizeof(SDL_Vertex));
	static_assert(offsetof(Vertex, position) == offsetof(SDL_Vertex, position));
	static_assert(offsetof(Vertex, uv) == offsetof(SDL_Vertex, tex_coord));
	static_assert(offsetof(Vertex, color) == offsetof(SDL_Vertex, color));
	static_assert(sizeof(uint32) == sizeof(int));

	class TextureRegistry: public ITextureRegistry {
	public:
		explicit TextureRegistry(SDL_Renderer* renderer)
			: m_renderer(renderer) {}

		~TextureRegistry() override = default;

		TextureRegistry(const TextureRegistry&) = delete;

		TextureRegistry& operator=(const TextureRegistry&) = delete;

		TextureHandle LoadTexture(std::string_view filePath) override {
			// todo check if texture was already loaded
			const auto data = m_loader.Load(filePath);
			if (!data) {
				logError("[TextureRegistry::LoadTexture]: Failed to load texture from {}!]", filePath);
				return {};
			}
			return CreateTexture(data.value());
		}

		TextureHandle CreateTexture(const TextureData& data) override {
			PCE_ASSERT(!data.rgba.empty(), "[TextureRegistry::CreateTexture]: Empty pixel data!]");
			PCE_ASSERT(data.width > 0 && data.height > 0,
						"[TextureRegistry::CreateTexture]: Invalid texture dimensions!]");

			// SDL_PIXELFORMAT_RGBA32 - “byte” format: pixels in memory go "R, G, B, A" regardless of
			// endianness (little-endian: ABGR8888, big-endian: RGBA8888). This is exactly the order of TextureData,
			// therefore channel conversion/swap is NOT needed.
			auto sdlTexture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(SDL_CreateTexture(
					m_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
					static_cast<int>(data.width), static_cast<int>(data.height)), SDL_DestroyTexture);
			if (!sdlTexture) {
				logError("[TextureRegistry::CreateTexture]: SDL_CreateTexture failed: {}", SDL_GetError());
				return {};
			}

			const auto updateRes = SDL_UpdateTexture(
				sdlTexture.get(), nullptr, data.rgba.data(), static_cast<int>(data.width) * 4);
			if (updateRes != 0) {
				logError("[TextureRegistry::CreateTexture]: SDL_UpdateTexture failed: {}", SDL_GetError());
				return {};
			}

			const TextureHandle id{m_nextTextureID++};
			m_textures.emplace(id, std::move(sdlTexture));
			m_textureInfos.emplace(id, TextureInfo{id, data.width, data.height});
			return id;
		}

		void DestroyTexture(TextureHandle texture) noexcept override {
			m_textures.erase(texture);
			m_textureInfos.erase(texture);
		}

		std::optional<TextureInfo> GetTextureInfo(TextureHandle texture) const override {
			// TODO(assets): C++23 — replace with std::expected<TextureInfo, AssetLoadError>
			const auto it = m_textureInfos.find(texture);
			if (it == m_textureInfos.end()) {
				return std::nullopt;
			}
			return it->second;
		}

		SDL_Texture* Resolve(TextureHandle texture) const noexcept {
			const auto it = m_textures.find(texture);
			if (it == m_textures.end()) {
				return nullptr;
			}
			return it->second.get();
		}

		void Clear() override {
			m_textures.clear();
			m_textureInfos.clear();
		}

	private:
		SDL_Renderer* m_renderer;
		TextureLoader m_loader{};

		// 0 is reserved for no texture
		uint32 m_nextTextureID{1};
		std::unordered_map<TextureHandle, std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>> m_textures;
		std::unordered_map<TextureHandle, TextureInfo> m_textureInfos;
	};

	struct Renderer::Impl {
		explicit Impl(SDL_Renderer* renderer): m_renderer{renderer, SDL_DestroyRenderer}, m_textures(renderer) {}

		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer;
		TextureRegistry m_textures;
	};
}


std::unique_ptr<pce::Renderer> pce::Renderer::Create(SDL_Window* window) {
	PCE_ASSERT(window, "[Renderer::Create]: window is null!]");
	SDL_Renderer* sdlRenderer{SDL_CreateRenderer(window, -1, 0)};
	PCE_ASSERT(sdlRenderer, "[Renderer::Create]: renderer is null!]");
	if (!sdlRenderer) {
		return nullptr;
	}
	// trick to be able to use make_unique
	struct UniqueRenderer: Renderer {
		UniqueRenderer(SDL_Renderer* renderer): Renderer(renderer) {}
	};
	return std::make_unique<UniqueRenderer>(sdlRenderer);
}

pce::Renderer::~Renderer() {}

void pce::Renderer::BeginFrame() const {
	SDL_SetRenderDrawColor(m_impl->m_renderer.get(), 0, 0, 0, 255);
	SDL_RenderClear(m_impl->m_renderer.get());
}

void pce::Renderer::Flush(const RenderQueue& queue) const {
	if (queue.IsEmpty()) {
		return;
	}
	const auto batches = queue.Batches();

	for (const auto& batch: batches) {
		SDL_Texture* texture = m_impl->m_textures.Resolve(batch.key.texture);
		auto renderRes = SDL_RenderGeometry(
			m_impl->m_renderer.get(), texture,
			reinterpret_cast<const SDL_Vertex*>(batch.vertices.data()),
			static_cast<int>(batch.vertices.size()),
			reinterpret_cast<const int*>(batch.indices.data()),
			static_cast<int>(batch.indices.size())
		);
		PCE_ASSERT(renderRes == 0, "[Renderer::Flush]: Failed to render queue!]");
	}
}

void pce::Renderer::EndFrame() const {
	SDL_RenderPresent(m_impl->m_renderer.get());
}

pce::ITextureRegistry& pce::Renderer::GetTextureRegistry() {
	return m_impl->m_textures;
}

const pce::ITextureRegistry& pce::Renderer::GetTextureRegistry() const {
	return m_impl->m_textures;
}

pce::Renderer::Renderer(SDL_Renderer* renderer)
	: m_impl{std::make_unique<Impl>(renderer)} {}
