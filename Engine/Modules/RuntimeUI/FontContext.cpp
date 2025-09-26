#include "kgpch.h"

#include "Modules/RuntimeUI/FontContext.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/RuntimeUI/ExternalAPI/msdfgenAPI.h"

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

			m_TextInputSpec.m_ShapeComponent = new Rendering::ShapeComponent();
			m_TextInputSpec.m_ShapeComponent->m_CurrentShape = Rendering::ShapeTypes::Quad;

			m_TextInputSpec.m_Shader = localShader;
			m_TextInputSpec.m_ShapeComponent->m_Shader = localShader;
			m_TextInputSpec.m_Buffer = localBuffer;
			m_TexCoordinates = CreateRef<std::vector<Math::vec2>>();
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 0.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 0.0f });
			m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = m_TexCoordinates;

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
			characterStruct.m_TexCoordinateMin = { (float)al, (float)ab };
			characterStruct.m_TexCoordinateMax = { (float)ar, (float)at };
			// Fill the Bounding Box Size when Rendering
			double pl, pb, pr, pt;
			glyphGeometry.getQuadPlaneBounds(pl, pb, pr, pt);
			characterStruct.m_QuadMin = { (float)pl, (float)pb };
			characterStruct.m_QuadMax = { (float)pr, (float)pt };
			// Fill the Advance
			characterStruct.m_Advance = (float)glyphGeometry.getAdvance();
			// Fill Glyph Size
			int32_t glyphWidth, glyphHeight;
			glyphGeometry.getBoxSize(glyphWidth, glyphHeight);
			characterStruct.m_Size = { glyphWidth, glyphHeight };
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
}