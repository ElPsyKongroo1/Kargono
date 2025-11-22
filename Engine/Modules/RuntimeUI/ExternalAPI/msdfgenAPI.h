#pragma once

#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "msdf-atlas-gen/FontGeometry.h"

#include "Modules/Rendering/Assets/Texture2D.h"

namespace Kargono::Utility
{
	template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Rendering::Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		UNREFERENCED_PARAMETER(fontGeometry);
		UNREFERENCED_PARAMETER(fontSize);
		UNREFERENCED_PARAMETER(fontName);

		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		Rendering::TextureSpecification spec;
		spec.m_Width = bitmap.width;
		spec.m_Height = bitmap.height;
		spec.m_Format = Rendering::ImageFormat::RGB8;
		spec.m_GenerateMipMaps = false;
		Ref<Rendering::Texture2D> texture{ CreateRef<Rendering::Texture2D>() }; 
		texture->RegisterTexture(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
		return texture;
	}

	template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static void CreateAtlas(std::string_view fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height, Rendering::TextureSpecification& textureSpec, Buffer& buffer)
	{
		UNREFERENCED_PARAMETER(fontGeometry);
		UNREFERENCED_PARAMETER(fontSize);
		UNREFERENCED_PARAMETER(fontName);

		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		textureSpec.m_Width = bitmap.width;
		textureSpec.m_Height = bitmap.height;
		textureSpec.m_Format = Rendering::ImageFormat::RGB8;
		textureSpec.m_GenerateMipMaps = false;

		buffer.Allocate(bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.m_Format));
		memcpy(buffer.m_Data, bitmap.pixels, buffer.m_Size);
	}
}