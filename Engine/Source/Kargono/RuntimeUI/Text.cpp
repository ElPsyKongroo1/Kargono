#include "kgpch.h"

#include "Kargono/RuntimeUI/Text.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Rendering/RenderingEngine.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Rendering/RenderCommand.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Projects/Project.h"

#include "API/Text/msdfgenAPI.h"


namespace Kargono::Utility
{
	template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Rendering::Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
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
	static Rendering::RendererInputSpec s_TextInputSpec{};
	static Ref<std::vector<Math::vec3>> s_Vertices;
	static Ref<std::vector<Math::vec2>> s_TexCoordinates;

	void Text::Init()
	{
		// TODO: Change to Proper Boolean Check
		if (!s_TextInputSpec.Shader)
		{
			// TODO: Unreleased Heap Data with Buffer
			Rendering::ShaderSpecification textShaderSpec {Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::TextTexture, false, true, false, Rendering::RenderingType::DrawTriangle, false};
			auto [uuid, localShader] = Assets::AssetManager::GetShader(textShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			s_TextInputSpec.ShapeComponent = new ShapeComponent();
			s_TextInputSpec.ShapeComponent->CurrentShape = Rendering::ShapeTypes::Quad;

			s_TextInputSpec.Shader = localShader;
			s_TextInputSpec.ShapeComponent->Shader = localShader;
			s_TextInputSpec.Buffer = localBuffer;
			s_TexCoordinates = CreateRef<std::vector<Math::vec2>>();
			s_TexCoordinates->push_back({ 0.0f, 0.0f });
			s_TexCoordinates->push_back({ 0.0f, 1.0f });
			s_TexCoordinates->push_back({ 1.0f, 1.0f });
			s_TexCoordinates->push_back({ 0.0f, 0.0f });
			s_TexCoordinates->push_back({ 1.0f, 1.0f });
			s_TexCoordinates->push_back({ 1.0f, 0.0f });
			s_TextInputSpec.ShapeComponent->TextureCoordinates = s_TexCoordinates;

			s_Vertices = CreateRef<std::vector<Math::vec3>>();
		}
		KG_VERIFY(true, "Runtime Text Engine Init")
	}

	Ref<Font> Text::InstantiateEditorFont(const std::filesystem::path& filepath)
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

	void Font::PushTextData(const std::string& string, Math::vec3 translation, const glm::vec4& color, float scale)
	{
		Ref<Rendering::Texture2D> fontAtlas = m_AtlasTexture;

		s_TextInputSpec.ShapeComponent->Texture = fontAtlas;
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, "a_Color", s_TextInputSpec.Buffer, s_TextInputSpec.Shader);

		double x = translation.x;
		double y = translation.y;
		//double fsScale = scale / (metrics.ascenderY - metrics.descenderY);

		for (size_t i = 0; i < string.size(); i++)
		{
			char character = string[i];
			Character glyph;

			switch (character)
			{
			case '\n':
				{
					x = translation.x;
					y -= scale * m_LineHeight;
					continue;
				}
			case '\r':
				{
					continue;
				}
			case '\t':
				{
					character = ' ';
					break;
				}
			}

			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				glyph = m_Characters.at('?');
			}

			// Glyph inside Atlas size
			glm::vec2 texCoordMin(glyph.TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.TexCoordinateMax);

			// Glyph size when rendered
			
			glm::vec2 quadMin(glyph.QuadMin);
			glm::vec2 quadMax(glyph.QuadMax);

			quadMin *= scale, quadMax *= scale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / fontAtlas->GetWidth();
			float texelHeight = 1.0f / fontAtlas->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			s_Vertices->clear();
			s_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			s_Vertices->push_back({ quadMin, translation.z });											// 0, 0
			s_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			s_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			s_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			s_Vertices->push_back({ quadMax, translation.z });											// 1, 1

			s_TexCoordinates->clear();
			s_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			s_TexCoordinates->push_back(texCoordMin);										// 0, 0
			s_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			s_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			s_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			s_TexCoordinates->push_back(texCoordMax);										// 1, 1

			s_TextInputSpec.ShapeComponent->Vertices = s_Vertices;

			Rendering::RenderingEngine::SubmitDataToRenderer(s_TextInputSpec);

			// TODO: Add Kerning back! Info on Kerning is here: https://freetype.org/freetype2/docs/glyphs/glyphs-4.html
			//double advance = 0;
			//char nextCharacter = string[i + 1];
			//fontGeometry.getAdvance(advance, character, nextCharacter);
			//float kerningOffset = 0.0f;
			//x += fsScale * advance + kerningOffset;

			x += scale * glyph.Advance;

		}
	}

	Math::vec2 Font::CalculateTextSize(const std::string& text)
	{
		Math::vec2 outputSize {0.0f};
		std::string::const_iterator currentCharacter;
		for (currentCharacter = text.begin(); currentCharacter != text.end(); currentCharacter++)
		{
			if (!m_Characters.contains(*currentCharacter))
			{
				continue;
			}
			Character glyph = m_Characters[*currentCharacter];
			if (glyph.Size.y > outputSize.y) { outputSize.y = glyph.Size.y; }
			outputSize.x += glyph.Advance;
		}
		//TODO: FIX THIS MAGIC NUMBER PLEASE
		outputSize.y /= 42.0f;
		return outputSize;
	}
}
