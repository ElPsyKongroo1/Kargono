#include "kgpch.h"

#include "Kargono/UI/TextEngine.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/Texture.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include "glad/glad.h"


namespace Kargono::UI
{
	static RendererInputSpec s_TextInputSpec{};
	static Ref<std::vector<Math::vec3>> s_Vertices;
	static Ref<std::vector<Math::vec2>> s_TexCoordinates;

	void TextEngine::Init()
	{
		// TODO: Change to Proper Boolean Check
		if (!s_TextInputSpec.Shader)
		{
			// TODO: Unreleased Heap Data with Buffer
			ShaderSpecification textShaderSpec {ColorInputType::FlatColor, TextureInputType::TextTexture, false, true, false, RenderingType::DrawTriangle, false};
			auto [uuid, localShader] = Assets::AssetManager::GetShader(textShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			s_TextInputSpec.ShapeComponent = new ShapeComponent();
			s_TextInputSpec.ShapeComponent->CurrentShape = ShapeTypes::Quad;

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
	}

	Ref<Font> TextEngine::InstantiateFontEditor(const std::filesystem::path& editorPath)
	{
		Ref<Font> newFont = CreateRef<Font>();

		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			KG_CORE_ASSERT(false, "FreeType Library Not Initialized Properly");
			return nullptr;
		}

		FT_Face face;
		if (FT_New_Face(ft, editorPath.string().c_str(), 0, &face))
		{
			KG_CORE_ERROR("Font not Loaded Correctly!");
			return nullptr;
		}

		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		double widthSum{ 0 };
		double heightSum{ 0 };
		uint32_t count{ 0 };

		for (unsigned char character{ 0 }; character < 128; character++)
		{
			// Load Character Glyph
			if (FT_Load_Char(face, character, FT_LOAD_RENDER))
			{
				KG_CORE_ERROR("Font Character not Loaded Correctly!");
				return nullptr;
			}
			// TODO: Create API for Textures!
			// Generate Texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			Character mapCharacter = {
				Texture2D::Create(texture, face->glyph->bitmap.width, face->glyph->bitmap.rows),
			Math::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		  Math::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<uint32_t>(face->glyph->advance.x)
			};

			widthSum += mapCharacter.Advance >> 6;
			heightSum += face->glyph->bitmap.rows;
			count++;
			newFont->GetCharacters().insert(std::pair<unsigned char, Character>(character, mapCharacter));
		}

		newFont->m_AverageWidth = static_cast<float>(widthSum / count);
		newFont->m_AverageHeight = static_cast<float>(heightSum / count);
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return newFont;
	}

	

	void Font::PushTextData(std::string text, Math::vec3 translation, float scale, Math::vec3 color)
	{
		float testNumber{ 0.0f }; // TODO: REMOVE!!!!!
		Shader::SetDataAtInputLocation<Math::vec4>({ color, 1.0f }, "a_Color", s_TextInputSpec.Buffer, s_TextInputSpec.Shader);

		std::string::const_iterator currentCharacter;
		for (currentCharacter = text.begin(); currentCharacter != text.end(); currentCharacter++)
		{
			Character ch = m_Characters[*currentCharacter];

			float xpos = translation.x + ch.Bearing.x * scale;
			float ypos = translation.y - (ch.Size.y - ch.Bearing.y) * scale;

			float charWidth = ch.Size.x * scale;
			float charHeight = ch.Size.y * scale;

			s_Vertices->clear();
			s_Vertices->push_back({ xpos, ypos + charHeight, translation.z });
			s_Vertices->push_back({ xpos, ypos, translation.z });
			s_Vertices->push_back({ xpos + charWidth, ypos, translation.z });
			s_Vertices->push_back({ xpos, ypos + charHeight, translation.z });
			s_Vertices->push_back({ xpos + charWidth, ypos, translation.z });
			s_Vertices->push_back({ xpos + charWidth, ypos + charHeight, translation.z });

			s_TextInputSpec.ShapeComponent->Vertices = s_Vertices;
			s_TextInputSpec.ShapeComponent->Texture = ch.Texture;

			Renderer::SubmitDataToRenderer(s_TextInputSpec);
			testNumber += (ch.Advance >> 6) * scale;
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			translation.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		//KG_CORE_ERROR("The test number is {}", testNumber);
	}

	Math::vec2 Font::CalculateTextSize(const std::string& text)
	{
		Math::vec2 outputSize {0.0f};
		std::string::const_iterator currentCharacter;
		for (currentCharacter = text.begin(); currentCharacter != text.end(); currentCharacter++)
		{
			Character ch = m_Characters[*currentCharacter];

			if (ch.Size.y > outputSize.y) { outputSize.y = static_cast<float>(ch.Size.y); }
			outputSize.x += (ch.Advance >> 6);
		}

		return outputSize;
	}
}
