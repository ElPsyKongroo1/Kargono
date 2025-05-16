#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/FontManager.h"
#include "Modules/Rendering/Texture.h"

#include "Modules/RuntimeUI/ExternalAPI/msdfgenAPI.h"
#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Utility
{
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

		textureSpec.Width = bitmap.width;
		textureSpec.Height = bitmap.height;
		textureSpec.Format = Rendering::ImageFormat::RGB8;
		textureSpec.GenerateMipMaps = false;

		buffer.Allocate(bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.Format));
		memcpy(buffer.Data, bitmap.pixels, buffer.Size);
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
	Ref<RuntimeUI::Font> Assets::FontManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
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

		newFont->m_LineHeight = metadata.LineHeight;
		newFont->m_Ascender = metadata.Ascender;
		newFont->m_Descender = metadata.Descender;

		for (auto& [character, characterStruct] : metadata.Characters)
		{
			fontCharacters.insert(std::pair<unsigned char, RuntimeUI::Character>(character, characterStruct));
		}

		currentResource.Release();
		return newFont;
	}
	void Assets::FontManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::FontMetaData* metadata = currentAsset.Data.GetSpecificMetaData<FontMetaData>();

		serializer << YAML::Key << "AtlasWidth" << YAML::Value << metadata->AtlasWidth;
		serializer << YAML::Key << "AtlasHeight" << YAML::Value << metadata->AtlasHeight;
		serializer << YAML::Key << "LineHeight" << YAML::Value << metadata->LineHeight;
		serializer << YAML::Key << "Ascender" << YAML::Value << metadata->Ascender;
		serializer << YAML::Key << "Descender" << YAML::Value << metadata->Descender;

		serializer << YAML::Key << "Characters" << YAML::Value << YAML::BeginSeq;
		for (auto& [character, characterStruct] : metadata->Characters)
		{
			serializer << YAML::BeginMap;
			serializer << YAML::Key << "Character" << YAML::Value << static_cast<uint32_t>(character);
			serializer << YAML::Key << "Size" << YAML::Value << characterStruct.Size;
			serializer << YAML::Key << "TexCoordinateMin" << YAML::Value << characterStruct.TexCoordinateMin;
			serializer << YAML::Key << "TexCoordinateMax" << YAML::Value << characterStruct.TexCoordinateMax;
			serializer << YAML::Key << "QuadMin" << YAML::Value << characterStruct.QuadMin;
			serializer << YAML::Key << "QuadMax" << YAML::Value << characterStruct.QuadMax;
			serializer << YAML::Key << "Advance" << YAML::Value << characterStruct.Advance;
			serializer << YAML::EndMap;
		}
		serializer << YAML::EndSeq;
	}
	void FontManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << std::string(name); // Output font name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created font inside asset directory at {}", assetPath);
	}
	void Assets::FontManager::CreateAssetIntermediateFromFile(AssetInfo& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation)
	{
		// Create Buffers
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		msdf_atlas::FontGeometry fontGeometry;
		float lineHeight{ 0 };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> characters {};

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KG_ASSERT(ft, "MSDFGEN failed to initialize!");

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fullFileLocation.string().c_str());
		if (!font)
		{
			KG_ERROR("Font not loaded correctly from filepath: " + fullFileLocation.string());
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// From imgui_draw.cpp
		static const CharsetRange charsetRanges[] =
		{
			{0x0020, 0x00FF}
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t character = range.Begin; character <= range.End; character++)
			{
				charset.add(character);
			}
		}

		double fontScale = 1.0;
		fontGeometry = msdf_atlas::FontGeometry(&glyphs);
		int glyphsLoaded = fontGeometry.loadCharset(font, fontScale, charset);
		KG_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint();
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);
		int32_t remaining = atlasPacker.pack(glyphs.data(), (int32_t)glyphs.size());
		KG_ASSERT(remaining == 0);

		int32_t width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull

		// if MSDF || MTSDF
		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = glyphs, &coloringSeed](int i, int threadNo) -> bool 
			{
				UNREFERENCED_PARAMETER(threadNo);
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
			}, static_cast<int32_t>(glyphs.size())).finish(numAvailableThread);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}
		Buffer buffer{};
		Rendering::TextureSpecification textureSpec{};
		Utility::CreateAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, glyphs, fontGeometry, width, height, textureSpec, buffer);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		const auto& metrics = fontGeometry.getMetrics();
		lineHeight = static_cast<float>(metrics.lineHeight);

		const auto& glyphMetrics = fontGeometry.getGlyphs();
		for (auto& glyphGeometry : glyphMetrics)
		{
			std::pair<unsigned char, RuntimeUI::Character>& character = characters.emplace_back();

			character.first = static_cast<uint8_t>(glyphGeometry.getCodepoint());

			// Fill the texture location inside Atlas
			double al, ab, ar, at;
			glyphGeometry.getQuadAtlasBounds(al, ab, ar, at);
			character.second.TexCoordinateMin = { (float)al, (float)ab };
			character.second.TexCoordinateMax = { (float)ar, (float)at };
			// Fill the Bounding Box Size when Rendering
			double pl, pb, pr, pt;
			glyphGeometry.getQuadPlaneBounds(pl, pb, pr, pt);
			character.second.QuadMin = { (float)pl, (float)pb };
			character.second.QuadMax = { (float)pr, (float)pt };
			// Fill the Advance
			character.second.Advance = (float)glyphGeometry.getAdvance();
			// Fill Glyph Size
			int32_t glyphWidth, glyphHeight;
			glyphGeometry.getBoxSize(glyphWidth, glyphHeight);
			character.second.Size = { glyphWidth, glyphHeight };
		}

		// Save Binary Intermediate into File
		Utility::FileSystem::WriteFileBinary(fullIntermediateLocation, buffer);

		// Load data into In-Memory Metadata object
		Ref<Assets::FontMetaData> metadata = CreateRef<Assets::FontMetaData>();
		metadata->AtlasWidth = static_cast<float>(textureSpec.Width);
		metadata->AtlasHeight = static_cast<float>(textureSpec.Height);
		metadata->LineHeight = lineHeight;
		metadata->Characters = characters;
		metadata->Ascender = (float)metrics.ascenderY;
		metadata->Descender = (float)metrics.descenderY;
		newAsset.Data.SpecificFileData = metadata;
		buffer.Release();
	}
	void Assets::FontManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::FontMetaData> fontMetaData = CreateRef<Assets::FontMetaData>();

		fontMetaData->AtlasWidth = metadataNode["AtlasWidth"].as<float>();
		fontMetaData->AtlasHeight = metadataNode["AtlasHeight"].as<float>();
		fontMetaData->LineHeight = metadataNode["LineHeight"].as<float>();
		fontMetaData->Ascender = metadataNode["Ascender"].as<float>();
		fontMetaData->Descender = metadataNode["Descender"].as<float>();

		YAML::Node characters = metadataNode["Characters"];
		auto& characterVector = fontMetaData->Characters;
		for (YAML::Node character : characters)
		{
			// Add new character to the character list
			std::pair<unsigned char, RuntimeUI::Character>& newCharacter = characterVector.emplace_back();

			// Fill the character fields
			newCharacter.first = static_cast<uint8_t>(character["Character"].as<uint32_t>());
			newCharacter.second.Size = character["Size"].as<Math::vec2>();
			newCharacter.second.Advance = character["Advance"].as<float>();
			newCharacter.second.TexCoordinateMin = character["TexCoordinateMin"].as<Math::vec2>();
			newCharacter.second.TexCoordinateMax = character["TexCoordinateMax"].as<Math::vec2>();
			newCharacter.second.QuadMin = character["QuadMin"].as<Math::vec2>();
			newCharacter.second.QuadMax = character["QuadMax"].as<Math::vec2>();
		}

		currentAsset.Data.SpecificFileData = fontMetaData;
	}
}
