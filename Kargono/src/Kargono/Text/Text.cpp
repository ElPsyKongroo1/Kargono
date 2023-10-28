#include "kgpch.h"

#include "Text.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Renderer/RenderCommand.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>


namespace Kargono
{
	static Shader::RendererInputSpec s_TextInputSpec{};
	static Ref<std::vector<glm::vec3>> s_Vertices;
	static Ref<std::vector<glm::vec2>> s_TexCoordinates;

	void Text::Init()
	{
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			KG_CORE_ASSERT(false, "FreeType Library Not Initialized Properly");
			return;
		}

		FT_Face face;
		if (FT_New_Face(ft, "resources/fonts/impact.ttf", 0, &face))
		{
			KG_CORE_ERROR("Font not Loaded Correctly!");
			return;
		}

		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (unsigned char character {0}; character < 128; character++ )
		{
			// Load Character Glyph
			if (FT_Load_Char(face, character, FT_LOAD_RENDER))
			{
				KG_CORE_ERROR("Font Character not Loaded Correctly!");
				return;
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

			// TODO: REMOVE THIS CODE IS ICKYYY
			static char fileName = 'A';
			std::string hello = "Hello";
			fileName++;
			std::string path = "resources/cache/" + hello + fileName;
			FileSystem::WriteFileImage(path, face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, FileSystem::FileTypes::bmp);

			Character mapCharacter = {
				Texture2D::Create(texture, face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		  glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<uint32_t>(face->glyph->advance.x)
			};
			m_Characters.insert(std::pair<unsigned char, Character>(character, mapCharacter));
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		// TODO: Change to Proper Boolean Check
		if (!s_TextInputSpec.Shader)
		{
			// TODO: Unreleased Heap Data with Buffer
			Shader::ShaderSpecification textShaderSpec {Shader::ColorInputType::FlatColor, Shader::TextureInputType::TextTexture, false, true, false, Shape::RenderingType::DrawTriangle, false};
			auto [uuid, localShader] = AssetManager::GetShader(textShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Shader::SetDataAtInputLocation<glm::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, "a_Color", localBuffer, localShader);

			s_TextInputSpec.ShapeComponent = new ShapeComponent();
			s_TextInputSpec.ShapeComponent->CurrentShape = Shape::ShapeTypes::Quad;

			s_TextInputSpec.Shader = localShader;
			s_TextInputSpec.ShapeComponent->Shader = localShader;
			s_TextInputSpec.Buffer = localBuffer;
			s_TexCoordinates = CreateRef<std::vector<glm::vec2>>();
			s_TexCoordinates->push_back({ 0.0f, 0.0f });
			s_TexCoordinates->push_back({ 0.0f, 1.0f });
			s_TexCoordinates->push_back({ 1.0f, 1.0f });
			s_TexCoordinates->push_back({ 0.0f, 0.0f });
			s_TexCoordinates->push_back({ 1.0f, 1.0f });
			s_TexCoordinates->push_back({ 1.0f, 0.0f });
			s_TextInputSpec.ShapeComponent->TextureCoordinates = s_TexCoordinates;

			s_Vertices = CreateRef<std::vector<glm::vec3>>();
		}
	}

	void Text::RenderText(const EditorCamera& camera , std::string text, float x, float y, float scale, glm::vec3 color)
	{
		RenderCommand::EnableDepthTesting(false);
		// Iterate through all characters
		glm::mat4 cameraViewMatrix = glm::inverse(camera.GetViewMatrix());
		glm::mat4 orthographicProjection = glm::ortho((float)0, (float)1280,
			(float)0, (float)720, (float)0, (float)100);
		glm::mat4 outputMatrix = orthographicProjection;
		Renderer::BeginScene(outputMatrix);

		//Renderer::BeginScene(camera);

		Shader::SetDataAtInputLocation<glm::vec4>({ color, 1.0f }, "a_Color", s_TextInputSpec.Buffer, s_TextInputSpec.Shader);

		std::string::const_iterator currentCharacter;
		for (currentCharacter = text.begin(); currentCharacter != text.end(); currentCharacter++)
		{
			Character ch = m_Characters[*currentCharacter];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float charWidth = ch.Size.x * scale;
			float charHeight = ch.Size.y * scale;

			s_Vertices->clear();
			s_Vertices->push_back({ xpos, ypos + charHeight, 0.0f });
			s_Vertices->push_back({ xpos, ypos, 0.0f });
			s_Vertices->push_back({ xpos + charWidth, ypos, 0.0f });
			s_Vertices->push_back({ xpos, ypos + charHeight, 0.0f });
			s_Vertices->push_back({ xpos + charWidth, ypos, 0.0f });
			s_Vertices->push_back({ xpos + charWidth, ypos + charHeight, 0.0f });

			s_TextInputSpec.ShapeComponent->Vertices = s_Vertices;
			s_TextInputSpec.ShapeComponent->Texture = ch.Texture;

			Renderer::SubmitDataToRenderer(s_TextInputSpec);

			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		Renderer::EndScene();
		RenderCommand::EnableDepthTesting(true);
	}
}
