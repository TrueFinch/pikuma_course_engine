cmake_minimum_required(VERSION 3.30)
project(Engine LANGUAGES CXX)

add_library(${ENGINE_LIB} STATIC)
set_target_properties(${ENGINE_LIB}
		PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
target_compile_options(${ENGINE_LIB} PRIVATE -std=c++20)

# header files
set(ENGINE_HEADERS
		include/engine/assetsModule/AssetsManager.h
		include/engine/assetsModule/TextureData.h
		include/engine/assetsModule/TextureLoader.h

		include/engine/coreModule/Game.h
		include/engine/utilsModule/Types.h

		include/engine/logModule/Log.h
		include/engine/logModule/LogObserver.h
		include/engine/logModule/LogManager.h
		include/engine/logModule/LogManagerInstance.h

		include/engine/logModule/SpdLogger.h

		include/engine/ecsModule/components/TransformComponent.h
		include/engine/ecsModule/components/VelocityComponent.h
		include/engine/ecsModule/components/SpriteComponent.h
		include/engine/ecsModule/ECS.h

		include/engine/utilsModule/GlmFormat.h
		include/engine/utilsModule/Assert.h

		include/engine/graphicsModule/GraphicsContext.h
		include/engine/graphicsModule/RenderQueue.h
		include/engine/graphicsModule/Renderer.h
		include/engine/graphicsModule/TextureHandle.h
		include/engine/graphicsModule/SpriteRegion.h
		include/engine/graphicsModule/ITextureRegistry.h
)

# source files
set(ENGINE_SOURCES
		src/assetsModule/AssetsManager.cpp
		src/assetsModule/TextureLoader.cpp

		src/coreModule/Game.cpp
		src/logModule/Log.cpp
		src/logModule/LogManager.cpp
		src/logModule/observers/SpdLogger.cpp

		src/ecsModule/ECS.cpp

		src/graphicsModule/GraphicsContext.cpp
		src/graphicsModule/RenderQueue.cpp
		src/graphicsModule/Renderer.cpp
)

target_sources(${ENGINE_LIB}
		PRIVATE
		${ENGINE_HEADERS}
		${ENGINE_SOURCES}
)

# dependencies
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(${ENGINE_LIB}
		PUBLIC ${PNG_INCLUDE_DIR}
		PUBLIC ${fmt_INCLUDE_DIR}
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		src
)

target_link_libraries(${ENGINE_LIB}
		PUBLIC
		$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>

		PUBLIC
		$<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>

		PUBLIC PNG::PNG
		PUBLIC glm::glm

		PUBLIC spdlog::spdlog_header_only
		PUBLIC fmt::fmt-header-only
)

#TODO create install script later

if (BUILD_ENGINE_TESTS)
	add_subdirectory(tests)
endif ()
