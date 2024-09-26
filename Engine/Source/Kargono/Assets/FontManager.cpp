#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/Serialization/yamlcppAPI.h"
#include "API/Text/msdfgenAPI.h"

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
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_FontRegistry {};
	std::unordered_map<AssetHandle, Ref<RuntimeUI::Font>> AssetManager::s_Fonts {};

	void AssetManager::DeserializeFontRegistry()
	{
		// Clear current registry and open registry in current project 
		s_FontRegistry.clear();
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& fontRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";

		if (!std::filesystem::exists(fontRegistryLocation))
		{
			KG_WARN("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(fontRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", fontRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Font Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.FileLocation = metadata["FileLocation"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving font specific metadata 
				if (newAsset.Data.Type == Assets::AssetType::Font)
				{
					Ref<Assets::FontMetaData> fontMetaData = CreateRef<Assets::FontMetaData>();

					fontMetaData->AtlasWidth = metadata["AtlasWidth"].as<float>();
					fontMetaData->AtlasHeight = metadata["AtlasHeight"].as<float>();
					fontMetaData->LineHeight = metadata["LineHeight"].as<float>();

					auto characters = metadata["Characters"];
					auto& characterVector = fontMetaData->Characters;
					for (auto character : characters)
					{
						RuntimeUI::Character newCharacter{};
						newCharacter.Size = character["Size"].as<Math::vec2>();
						newCharacter.Advance = character["Advance"].as<float>();
						newCharacter.TexCoordinateMin = character["TexCoordinateMin"].as<Math::vec2>();
						newCharacter.TexCoordinateMax = character["TexCoordinateMax"].as<Math::vec2>();
						newCharacter.QuadMin = character["QuadMin"].as<Math::vec2>();
						newCharacter.QuadMax = character["QuadMax"].as<Math::vec2>();
						characterVector.push_back(std::pair<unsigned char, RuntimeUI::Character>(static_cast<uint8_t>(character["Character"].as<uint32_t>()), newCharacter));
					}

					newAsset.Data.SpecificFileData = fontMetaData;

				}

				// Add asset to in memory registry 
				s_FontRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeFontRegistry()
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& fontRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Font";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_FontRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "FileLocation" << YAML::Value << asset.Data.FileLocation.string();
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Font)
			{
				Assets::FontMetaData* metadata = static_cast<Assets::FontMetaData*>(asset.Data.SpecificFileData.get());


				out << YAML::Key << "AtlasWidth" << YAML::Value << metadata->AtlasWidth;
				out << YAML::Key << "AtlasHeight" << YAML::Value << metadata->AtlasHeight;
				out << YAML::Key << "LineHeight" << YAML::Value << metadata->LineHeight;

				out << YAML::Key << "Characters" << YAML::Value << YAML::BeginSeq;
				for (auto& [character, characterStruct] : metadata->Characters)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Character" << YAML::Value << static_cast<uint32_t>(character);
					out << YAML::Key << "Size" << YAML::Value << characterStruct.Size;
					out << YAML::Key << "TexCoordinateMin" << YAML::Value << characterStruct.TexCoordinateMin;
					out << YAML::Key << "TexCoordinateMax" << YAML::Value << characterStruct.TexCoordinateMax;
					out << YAML::Key << "QuadMin" << YAML::Value << characterStruct.QuadMin;
					out << YAML::Key << "QuadMax" << YAML::Value << characterStruct.QuadMax;
					out << YAML::Key << "Advance" << YAML::Value << characterStruct.Advance;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(fontRegistryLocation.parent_path());

		std::ofstream fout(fontRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset)
	{
		// Create Buffers
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		msdf_atlas::FontGeometry fontGeometry;
		float lineHeight{ 0 };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> characters {};

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KG_ASSERT(ft, "MSDFGEN failed to initialize!");

		std::string fileString = filePath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
		if (!font)
		{
			KG_ERROR("Font not loaded correctly from filepath: " + filePath.string());
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
			msdf_atlas::Workload([&glyphs = glyphs, &coloringSeed](int i, int threadNo) -> bool {
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
			unsigned char character = static_cast<uint8_t>(glyphGeometry.getCodepoint());
			RuntimeUI::Character characterStruct{};

			// Fill the texture location inside Atlas
			double al, ab, ar, at;
			glyphGeometry.getQuadAtlasBounds(al, ab, ar, at);
			characterStruct.TexCoordinateMin = { (float)al, (float)ab };
			characterStruct.TexCoordinateMax = { (float)ar, (float)at };
			// Fill the Bounding Box Size when Rendering
			double pl, pb, pr, pt;
			glyphGeometry.getQuadPlaneBounds(pl, pb, pr, pt);
			characterStruct.QuadMin = { (float)pl, (float)pb };
			characterStruct.QuadMax = { (float)pr, (float)pt };
			// Fill the Advance
			characterStruct.Advance = (float)glyphGeometry.getAdvance();
			// Fill Glyph Size
			int32_t glyphWidth, glyphHeight;
			glyphGeometry.getBoxSize(glyphWidth, glyphHeight);
			characterStruct.Size = { glyphWidth, glyphHeight };
			characters.push_back({ character, characterStruct });
		}

		// Save Binary Intermediate into File
		std::string intermediatePath = "Fonts/Intermediates/" + (std::string)newAsset.Handle + ".kgfont";
		std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveAssetDirectory() / intermediatePath;
		Utility::FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Font;
		newAsset.Data.FileLocation = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), filePath);
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::FontMetaData> metadata = CreateRef<Assets::FontMetaData>();
		metadata->AtlasWidth = static_cast<float>(textureSpec.Width);
		metadata->AtlasHeight = static_cast<float>(textureSpec.Height);
		metadata->LineHeight = lineHeight;
		metadata->Characters = characters;
		newAsset.Data.SpecificFileData = metadata;

		buffer.Release();
	}

	//===================================================================================================================================================
	
	AssetHandle AssetManager::ImportNewFontFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_FontRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateFontIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Font
		s_FontRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeFontRegistry(); // Update Registry File on Disk

		s_Fonts.insert({ newHandle, InstantiateFontIntoMemory(newAsset) });

		return newHandle;
	}

	void AssetManager::ClearFontRegistry()
	{
		s_FontRegistry.clear();
		s_Fonts.clear();
	}
	
	Ref<RuntimeUI::Font> AssetManager::InstantiateFontIntoMemory(Assets::Asset& asset)
	{
		Assets::FontMetaData metadata = *static_cast<Assets::FontMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<RuntimeUI::Font> newFont = CreateRef<RuntimeUI::Font>();
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

	Ref<RuntimeUI::Font> AssetManager::GetFont(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving font!");

		if (s_Fonts.contains(handle)) { return s_Fonts[handle]; }

		if (s_FontRegistry.contains(handle))
		{
			auto asset = s_FontRegistry[handle];

			Ref<RuntimeUI::Font> newFont = InstantiateFontIntoMemory(asset);
			s_Fonts.insert({ asset.Handle, newFont });
			return newFont;
		}

		KG_WARN("No font is associated with provided handle!");
		return nullptr;
	}
}
