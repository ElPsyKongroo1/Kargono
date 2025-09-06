#include "kgpch.h"

#include "Modules/RuntimeUI/Font.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Core/Engine.h"
#include "Modules/ECS/EngineComponents.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Texture.h"
#include "Kargono/Projects/Project.h"

#include "Modules/RuntimeUI/ExternalAPI/msdfgenAPI.h"


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
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = Rendering::ImageFormat::RGB8;
		spec.GenerateMipMaps = false;
		Ref<Rendering::Texture2D> texture = Rendering::Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
		return texture;
	}
}

namespace Kargono::RuntimeUI
{
	void FontContext::Init()
	{
		// TODO: Change to Proper Boolean Check
		if (!m_TextInputSpec.m_Shader)
		{
			// TODO: Unreleased Heap Data with Buffer
			Rendering::ShaderSpecification textShaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::TextTexture, false, true, true, Rendering::RenderingType::DrawTriangle, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(textShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, 
				Utility::FileSystem::CRCFromString("a_Color"),
				localBuffer, localShader);

			m_TextInputSpec.m_ShapeComponent = new ECS::ShapeComponent();
			m_TextInputSpec.m_ShapeComponent->CurrentShape = Rendering::ShapeTypes::Quad;

			m_TextInputSpec.m_Shader = localShader;
			m_TextInputSpec.m_ShapeComponent->Shader = localShader;
			m_TextInputSpec.m_Buffer = localBuffer;
			m_TexCoordinates = CreateRef<std::vector<Math::vec2>>();
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 0.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 0.0f });
			m_TextInputSpec.m_ShapeComponent->TextureCoordinates = m_TexCoordinates;

			m_Vertices = CreateRef<std::vector<Math::vec3>>();
		}
		KG_VERIFY(true, "Runtime Text Engine Init")
	}

	void FontContext::Terminate()
	{
		m_TextInputSpec.ClearData();
	}


	Ref<Font> FontContext::InstantiateEditorFont(const std::filesystem::path& filepath)
	{
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		msdf_atlas::FontGeometry fontGeometry;
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KG_ASSERT(ft, "MSDFGEN failed to initialize!");

		Ref<Font> newFont = CreateRef<Font>();

		std::string fileString = filepath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
		if (!font)
		{
			KG_ERROR("Font not loaded correctly from filepath: " + filepath.string());
			return nullptr;
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


		newFont->m_AtlasTexture = Utility::CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, glyphs, fontGeometry, width, height);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		const auto& metrics = fontGeometry.getMetrics();
		newFont->m_LineHeight = static_cast<float>(metrics.lineHeight);
		

		const auto& glyphMetrics = fontGeometry.getGlyphs();
		newFont->m_Characters.clear();
		for (auto& glyphGeometry : glyphMetrics)
		{
			unsigned char character = static_cast<uint8_t>(glyphGeometry.getCodepoint());
			Character characterStruct{};

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
			newFont->m_Characters.insert_or_assign(character, characterStruct);
		}

		return newFont;
	}

	void FontContext::SetID(uint32_t id)
	{
		Rendering::Shader::SetDataAtInputLocation<uint32_t>(id, 
			Utility::FileSystem::CRCFromString("a_EntityID"),
			m_TextInputSpec.m_Buffer, m_TextInputSpec.m_Shader);
	}

	void Font::OnRenderMultiLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale, int maxLineWidth)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };
		UNREFERENCED_PARAMETER(maxLineWidth);
		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->Texture = m_AtlasTexture;
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
			glm::vec2 texCoordMin(glyph.TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.QuadMin);
			glm::vec2 quadMax(glyph.QuadMax);

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
			fontContext.m_TextInputSpec.m_ShapeComponent->Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);				
			fontContext.m_TextInputSpec.m_ShapeComponent->TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.Advance;
		}
	}

	void Font::OnRenderSingleLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };

		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->Texture = m_AtlasTexture;
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
			glm::vec2 texCoordMin(glyph.TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.QuadMin);
			glm::vec2 quadMax(glyph.QuadMax);

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
			fontContext.m_TextInputSpec.m_ShapeComponent->Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);
			fontContext.m_TextInputSpec.m_ShapeComponent->TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.Advance;
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
			float ySize{ glyph.QuadMax.y - glyph.QuadMin.y };
			if (ySize > outputSize.y)
			{ 
				outputSize.y = ySize; 
			}

			// Get the total x-axis length of the text
			outputSize.x += glyph.Advance;
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
			accumulatedXPosition += glyph.Advance * textScalingFactor;

			// Check if the (middle position of the this glyph) surpases the mouse position
			if (accumulatedXPosition - glyph.Advance * 0.5f * textScalingFactor > mouseXPosition)
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
					wordWidth += m_Characters.at(text[wordIndex]).Advance;
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
			if (glyph.QuadMax.y - glyph.QuadMin.y > lineHeight)
			{
				lineHeight = glyph.QuadMax.y - glyph.QuadMin.y;
			}

			// Shift the location to the next character
			xLocation += scale * glyph.Advance;
		}

		// Get data for final line
		metadata.m_LineBreaks.push_back({ lastBreak , text.size()});
		metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
	}
}
