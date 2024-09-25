#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/FontManagerTemp.h"

#include "API/Text/msdfgenAPI.h"
#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Utility
{
	template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static void CreateAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height, Rendering::TextureSpecification& textureSpec, Buffer& buffer)
	{
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		textureSpec.Width = bitmap.width;
		textureSpec.Height = bitmap.height;
		textureSpec.Format = Rendering::ImageFormat::RGB8;
		textureSpec.GenerateMipMaps = false;

		buffer.Allocate(bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.Format));
		memcpy_s(buffer.Data, buffer.Size, bitmap.pixels, bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.Format));
	}


	/*template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMipMaps = false;
		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * Utility::ImageFormatToBytes(spec.Format));
		return texture;
	}*/

}

namespace Kargono::Assets
{
	Ref<RuntimeUI::Font> Assets::FontManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<RuntimeUI::Font> newFont = CreateRef<RuntimeUI::Font>();
		Assets::FontMetaData metadata = *asset.Data.GetSpecificMetaData<FontMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		auto& fontCharacters = newFont->GetCharacters();

		// Create Texture
		Rendering::TextureSpecification spec;
		spec.Width = static_cast<uint32_t>(metadata.AtlasWidth);
		spec.Height = static_cast<uint32_t>(metadata.AtlasHeight);
		spec.Format = Rendering::ImageFormat::RGB8;
		spec.GenerateMipMaps = false;
		Ref<Rendering::Texture2D> texture = Rendering::Texture2D::Create(spec);
		texture->SetData((void*)currentResource.Data, spec.Width * spec.Height * Utility::ImageFormatToBytes(spec.Format));
		newFont->m_AtlasTexture = texture;

		newFont->SetLineHeight(metadata.LineHeight);

		for (auto& [character, characterStruct] : metadata.Characters)
		{
			fontCharacters.insert(std::pair<unsigned char, RuntimeUI::Character>(character, characterStruct));
		}

		currentResource.Release();
		return newFont;
	}
}
