#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Assets/Asset.h"

#include <string>
#include <filesystem>


namespace Kargono {

	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual std::string GetPath() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		// Create Texture using intermediate format
		static Ref<Texture2D> Create(Buffer buffer, const TextureMetaData& metadata);

		// Create unmanaged texture outside of AssetManager. Used for Editor Textures only.
		// Runtime related textures should use AssetManager.
		static Ref<Texture2D> CreateEditorTexture(const std::filesystem::path& path);

	};
}

