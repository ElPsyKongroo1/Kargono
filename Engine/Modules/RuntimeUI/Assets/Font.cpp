#include "kgpch.h"

#include "Modules/RuntimeUI/Assets/Font.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Assets/Texture.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/RuntimeUI/FontContext.h"

#include "Modules/RuntimeUI/ExternalAPI/msdfgenAPI.h"
#include "API/ImageProcessing/stbAPI.h"


namespace Kargono::RuntimeUI
{
	void FontMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext& metadataContext = *(Assets::SerializeMetaDataContext*)context;

		// Get context fields
		YAML::Emitter& emitter = *metadataContext.m_Serializer;

		emitter << YAML::Key << "AtlasWidth" << YAML::Value << m_AtlasWidth;
		emitter << YAML::Key << "AtlasHeight" << YAML::Value << m_AtlasHeight;
		emitter << YAML::Key << "LineHeight" << YAML::Value << m_LineHeight;
		emitter << YAML::Key << "Ascender" << YAML::Value << m_Ascender;
		emitter << YAML::Key << "Descender" << YAML::Value << m_Descender;

		emitter << YAML::Key << "Characters" << YAML::Value << YAML::BeginSeq;
		for (auto& [character, characterStruct] : m_Characters)
		{
			emitter << YAML::BeginMap;
			emitter << YAML::Key << "Character" << YAML::Value << static_cast<uint32_t>(character);
			emitter << YAML::Key << "Size" << YAML::Value << characterStruct.m_Size;
			emitter << YAML::Key << "TexCoordinateMin" << YAML::Value << characterStruct.m_TexCoordinateMin;
			emitter << YAML::Key << "TexCoordinateMax" << YAML::Value << characterStruct.m_TexCoordinateMax;
			emitter << YAML::Key << "QuadMin" << YAML::Value << characterStruct.m_QuadMin;
			emitter << YAML::Key << "QuadMax" << YAML::Value << characterStruct.m_QuadMax;
			emitter << YAML::Key << "Advance" << YAML::Value << characterStruct.m_Advance;
			emitter << YAML::EndMap;
		}
		emitter << YAML::EndSeq;
	}

	void FontMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext& assetContext = *(Assets::DeserializeMetaDataContext*)context;

		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;

		m_AtlasWidth = metadataNode["AtlasWidth"].as<float>();
		m_AtlasHeight = metadataNode["AtlasHeight"].as<float>();
		m_LineHeight = metadataNode["LineHeight"].as<float>();
		m_Ascender = metadataNode["Ascender"].as<float>();
		m_Descender = metadataNode["Descender"].as<float>();

		YAML::Node characters = metadataNode["Characters"];
		for (YAML::Node character : characters)
		{
			// Add new character to the character list
			std::pair<unsigned char, RuntimeUI::Character>& newCharacter = m_Characters.emplace_back();

			// Fill the character fields
			newCharacter.first = static_cast<uint8_t>(character["Character"].as<uint32_t>());
			newCharacter.second.m_Size = character["Size"].as<Math::vec2>();
			newCharacter.second.m_Advance = character["Advance"].as<float>();
			newCharacter.second.m_TexCoordinateMin = character["TexCoordinateMin"].as<Math::vec2>();
			newCharacter.second.m_TexCoordinateMax = character["TexCoordinateMax"].as<Math::vec2>();
			newCharacter.second.m_QuadMin = character["QuadMin"].as<Math::vec2>();
			newCharacter.second.m_QuadMax = character["QuadMax"].as<Math::vec2>();
		}
	}

	void Font::Serialize(void* context)
	{
		KG_ERROR("Serialization not implemented for font");
	}

	void Font::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext* assetContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<Font>() };

		FontMetaData fontMetadata = *metadata->GetSpecificMetaData<FontMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);

		// Create Texture
		Rendering::TextureSpecification spec;
		spec.m_Width = static_cast<uint32_t>(fontMetadata.m_AtlasWidth);
		spec.m_Height = static_cast<uint32_t>(fontMetadata.m_AtlasHeight);
		spec.m_Format = Rendering::ImageFormat::RGB8;
		spec.m_GenerateMipMaps = false;
		Ref<Rendering::Texture2D> texture = Rendering::Texture2D::Create(spec);
		texture->SetData((void*)currentResource.m_Data, spec.m_Width * spec.m_Height * Utility::ImageFormatToBytes(spec.m_Format));
		m_AtlasTexture = texture;

		m_LineHeight = fontMetadata.m_LineHeight;
		m_Ascender = fontMetadata.m_Ascender;
		m_Descender = fontMetadata.m_Descender;

		for (auto& [character, characterStruct] : fontMetadata.m_Characters)
		{
			m_Characters.insert(std::pair<unsigned char, RuntimeUI::Character>(character, characterStruct));
		}

		currentResource.Release();
	}

	void Font::CreateFromName(Assets::Metadata& metadata)
	{
		KG_TRACE_CRITICAL("We are calling the create font w/ name, " 
			"but I'm skeptical of this function bruhh");
		const std::filesystem::path assetPath{ metadata.GetAssetFullFilePath<Font>() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Name" << YAML::Value << metadata.m_Name; // Output font name
		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully created font inside asset directory at {}", assetPath);
	}
	void Font::CreateFromFile(Assets::Metadata& metadata,
		const std::filesystem::path& sourcePath)
	{
		std::string_view intermediateExtension
		{
			GetIntermediateExtensions().front().StringView()
		};
		const std::filesystem::path intermediatePath
		{
			metadata.GetAssetFullIntermediatePath<Font>(intermediateExtension)
		};

		// Create Buffers
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		msdf_atlas::FontGeometry fontGeometry;
		float lineHeight{ 0 };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> characters{};

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KG_ASSERT(ft, "MSDFGEN failed to initialize!");

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, sourcePath.string().c_str());
		if (!font)
		{
			KG_ERROR("Font not loaded correctly from filepath: " + sourcePath.string());
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
			character.second.m_TexCoordinateMin = { (float)al, (float)ab };
			character.second.m_TexCoordinateMax = { (float)ar, (float)at };
			// Fill the Bounding Box Size when Rendering
			double pl, pb, pr, pt;
			glyphGeometry.getQuadPlaneBounds(pl, pb, pr, pt);
			character.second.m_QuadMin = { (float)pl, (float)pb };
			character.second.m_QuadMax = { (float)pr, (float)pt };
			// Fill the Advance
			character.second.m_Advance = (float)glyphGeometry.getAdvance();
			// Fill Glyph Size
			int32_t glyphWidth, glyphHeight;
			glyphGeometry.getBoxSize(glyphWidth, glyphHeight);
			character.second.m_Size = { glyphWidth, glyphHeight };
		}

		// Save Binary Intermediate into File
		Utility::FileSystem::WriteFileBinary(intermediatePath, buffer);

		// Load data into In-Memory Metadata object
		FontMetaData& fontMetadata = *metadata.GetSpecificMetaData<FontMetaData>();
		fontMetadata.m_AtlasWidth = static_cast<float>(textureSpec.m_Width);
		fontMetadata.m_AtlasHeight = static_cast<float>(textureSpec.m_Height);
		fontMetadata.m_LineHeight = lineHeight;
		fontMetadata.m_Characters = characters;
		fontMetadata.m_Ascender = (float)metrics.ascenderY;
		fontMetadata.m_Descender = (float)metrics.descenderY;
		buffer.Release();
	}

	void Font::OnRenderMultiLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale, int maxLineWidth)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };
		UNREFERENCED_PARAMETER(maxLineWidth);
		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->m_Texture = m_AtlasTexture;
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, 
			Utility::FileSystem::CRCFromString("a_Color"),
			fontContext.m_TextInputSpec.m_Buffer, fontContext.m_TextInputSpec.m_Shader);

		// Initialize the active location where text is being rendered
		double xLocation{ translation.x };
		double yLocation{ translation.y };

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
		{
			// Get the active character from the string
			char character = string[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Move to next line
				xLocation = translation.x;
				yLocation -= scale * m_LineHeight;
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Coordinates of the glyph in the texture atlas
			glm::vec2 texCoordMin(glyph.m_TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.m_TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.m_QuadMin);
			glm::vec2 quadMax(glyph.m_QuadMax);

			// Scale the rendering size of the glyph
			quadMin *= scale;
			quadMax *= scale;

			// Adjust the quad location based on the current location
			quadMin += glm::vec2(xLocation, yLocation);
			quadMax += glm::vec2(xLocation, yLocation);

			// Adjust the texture coordinates based on the texture atlas size
			float texelWidth = 1.0f / m_AtlasTexture->GetWidth();
			float texelHeight = 1.0f / m_AtlasTexture->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// Submit the quad location data to the renderer
			fontContext.m_Vertices->clear();
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMin, translation.z });											// 0, 0
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMax, translation.z });											// 1, 1
			fontContext.m_TextInputSpec.m_ShapeComponent->m_Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);				
			fontContext.m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}
	}

	void Font::OnRenderSingleLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };

		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->m_Texture = m_AtlasTexture;
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, 
			Utility::FileSystem::CRCFromString("a_Color"),
			fontContext.m_TextInputSpec.m_Buffer, fontContext.m_TextInputSpec.m_Shader);

		// Initialize the active location where text is being rendered
		double xLocation{ translation.x };
		double yLocation{ translation.y };

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
		{
			// Get the active character from the string
			char character = string[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Skip newline
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Coordinates of the glyph in the texture atlas
			glm::vec2 texCoordMin(glyph.m_TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.m_TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.m_QuadMin);
			glm::vec2 quadMax(glyph.m_QuadMax);

			// Scale the rendering size of the glyph
			quadMin *= scale;
			quadMax *= scale;

			// Adjust the quad location based on the current location
			quadMin += glm::vec2(xLocation, yLocation);
			quadMax += glm::vec2(xLocation, yLocation);

			// Adjust the texture coordinates based on the texture atlas size
			float texelWidth = 1.0f / m_AtlasTexture->GetWidth();
			float texelHeight = 1.0f / m_AtlasTexture->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// Submit the quad location data to the renderer
			fontContext.m_Vertices->clear();
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMin, translation.z });											// 0, 0
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMax, translation.z });											// 1, 1
			fontContext.m_TextInputSpec.m_ShapeComponent->m_Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);
			fontContext.m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}
	}

	Math::vec2 Font::GetSingleLineTextSize(std::string_view text)
	{
		// Loop through all glyphs of provided text and generate the text's x and y extents
		Math::vec2 outputSize{ 0.0f };
		std::string::const_iterator currentCharacter;
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Early out if glyph does not exist
			if (!m_Characters.contains(text[characterIndex]))
			{
				continue;
			}
			Character glyph = m_Characters[text[characterIndex]];

			// Get the maximum y glyph size for the text
			float ySize{ glyph.m_QuadMax.y - glyph.m_QuadMin.y };
			if (ySize > outputSize.y)
			{ 
				outputSize.y = ySize; 
			}

			// Get the total x-axis length of the text
			outputSize.x += glyph.m_Advance;
		}

		// Magic number to center the text
		//outputSize.y /= 45.5f; // TODO: FIX THIS MAGIC NUMBER PLEASE
		return outputSize;
	}
	size_t Font::GetIndexFromMousePosition(std::string_view text, float textStartPoint, float mouseXPosition, float textScalingFactor)
	{
		// Loop through all glyphs of provided text and generate the text's x and y extents
		float accumulatedXPosition{ textStartPoint };
		std::string::const_iterator currentCharacter;
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Early out if glyph does not exist
			if (!m_Characters.contains(text[characterIndex]))
			{
				continue;
			}
			Character glyph = m_Characters[text[characterIndex]];

			// Get the total x-axis length of the text
			accumulatedXPosition += glyph.m_Advance * textScalingFactor;

			// Check if the (middle position of the this glyph) surpases the mouse position
			if (accumulatedXPosition - glyph.m_Advance * 0.5f * textScalingFactor > mouseXPosition)
			{
				return characterIndex;
			}
		}

		// Return the text's size if no position is identified
		return text.size();
	}
	void Font::GetMultiLineTextMetadata(std::string_view text, MultiLineTextDimensions& metadata, float scale, int maxLineWidth)
	{
		// Initialize the active location where text is being rendered
		float initialXLocation{ 0.0f };
		float initialYLocation{ 0.0f };
		float xLocation{ initialXLocation };
		float yLocation{ initialYLocation };
		float lineHeight{ 0.0f };
		bool wrapText{ maxLineWidth > 0 };
		size_t activeWordEnding{ 0 };
		int lastBreak{0};

		// Reset metadata
		metadata = {};

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Check if we should evaluate the word this letter exists in
			if (wrapText && characterIndex >= activeWordEnding)
			{
				// Get active word's terminal location and word width
				size_t wordIndex{ characterIndex };
				double wordWidth{ 0.0f };
				while (wordIndex < text.size())
				{
					if (std::isspace(text[wordIndex]))
					{
						activeWordEnding = wordIndex;
						break;
					}
					wordWidth += m_Characters.at(text[wordIndex]).m_Advance;
					wordIndex++;
				}
				wordWidth *= scale;

				// Check if newline is appropriate for active word
				if (xLocation + wordWidth > initialXLocation + maxLineWidth)
				{
					metadata.m_LineBreaks.push_back({ lastBreak, (int)characterIndex });
					metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
					lastBreak = (int)characterIndex;
					lineHeight = 0.0f;
					xLocation = initialXLocation;
					yLocation -= scale * m_LineHeight;
				}
			}

			// Get the active character from the string
			char character = text[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Move to next line
				metadata.m_LineBreaks.push_back({ lastBreak, (int)characterIndex });
				metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
				lastBreak = (int)characterIndex;
				lineHeight = 0.0f;
				xLocation = initialXLocation;
				yLocation -= scale * m_LineHeight;
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Get the line height from each glyph
			if (glyph.m_QuadMax.y - glyph.m_QuadMin.y > lineHeight)
			{
				lineHeight = glyph.m_QuadMax.y - glyph.m_QuadMin.y;
			}

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}

		// Get data for final line
		metadata.m_LineBreaks.push_back({ lastBreak , text.size()});
		metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
	}
}